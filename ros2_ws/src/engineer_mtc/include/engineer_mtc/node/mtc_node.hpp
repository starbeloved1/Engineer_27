#pragma once
#include "engineer_mtc/shared/energy_scene.hpp"
#include "engineer_mtc/shared/task_settings.hpp"
#include "engineer_mtc/shared/vision_target_buffer.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <engineer_mtc/msg/pick_status.hpp>
#include <geometry_msgs/msg/pose_array.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <moveit/task_constructor/task.h>
#include <moveit_msgs/msg/display_trajectory.hpp>
#include <moveit_task_constructor_msgs/action/execute_task_solution.hpp>
#include <mutex>
#include <rclcpp_action/rclcpp_action.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <serial_driver/msg/hardware_status.hpp>
#include <std_msgs/msg/bool.hpp>
#include <std_msgs/msg/u_int8.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <thread>
#include <visualization_msgs/msg/marker_array.hpp>

namespace engineer_mtc
{
/// 管理取矿与存车规划、分阶段执行和状态发布
class MtcNode : public rclcpp::Node
{
public:
  explicit MtcNode(const rclcpp::NodeOptions &options);
  ~MtcNode() override;

private:
  using Trigger = std_srvs::srv::Trigger;
  using Status = msg::PickStatus;
  using Execute = moveit_task_constructor_msgs::action::ExecuteTaskSolution;
  using GoalHandle = rclcpp_action::ClientGoalHandle<Execute>;

  enum class PlanRequest { Vision, Manual, Place };

  /**
   * @brief 从缓存中选择本次规划使用的矿位姿
   * @param manual true 使用手动目标，false 使用视觉目标
   * @return 手动目标保留输入坐标系；视觉目标返回基座下的多帧平均位姿
   */
  geometry_msgs::msg::PoseStamped selectTarget(bool manual);

  /**
   * @brief 按观测时间将矿位姿转换到基座坐标系
   * @param target 带参考坐标系和采集时间戳的矿位姿
   * @return 基座到矿坐标系的刚体变换，平移单位为m
   */
  Eigen::Isometry3d targetInBase(const geometry_msgs::msg::PoseStamped &target);

  /**
   * @brief 根据请求规划取矿或持矿存车任务
   * @param request 视觉取矿、手动取矿或持矿存车
   * @param[out] response 规划是否成功及结果说明
   * @note 成功后保存阶段解、发布预览，并将任务状态设为 READY
   */
  void plan(PlanRequest request, Trigger::Response &response);

  /// 检查当前关节反馈是否存在且满足 max_joint_age_
  void checkFreshJoints();

  // 状态反馈与可视化
  void publishTargets(const GraspTargets &targets, const builtin_interfaces::msg::Time &stamp);
  void publishPreview(const Eigen::Isometry3d &start, const Eigen::Isometry3d &end,bool place);
  /// 清除本节点发布的目标标记
  void clearPreview();
  /// 更新任务状态、业务阶段和说明，并发布状态消息
  void setStatus(uint8_t phase, uint8_t stage, const std::string &detail);
  /// 发布完整 pick_status 和业务阶段 pick_stag
  void publishStatus();
  /// 缓存关节反馈，结合硬件状态更新夹爪开合状态
  void updateJoints(sensor_msgs::msg::JointState::ConstSharedPtr message);
  /// 定时检查硬件有效性，并在空闲时更新场景中的目标矿
  void previewTarget();

  /**
   * @brief 将选中的 MTC 解按业务阶段存入 stage_solutions_
   * @param task 解所属的 Task，用于轨迹序列化
   * @param solution 当前任务选中的完整解
   * @param first_stage 起始数组下标，0 对应阶段 1
   * @param end_stage 结束数组下标，不包含此下标
   */
  void prepareExecution(moveit::task_constructor::Task &task,const moveit::task_constructor::SolutionBase &solution,size_t first_stage, size_t end_stage);

  /**
   * @brief 检查执行条件并启动轨迹执行线程
   * @param next true 继续分步任务，false 开始执行就绪任务
   * @param[out] response 请求是否受理及说明，最终结果由 pick_status 发布
   */
  void startExecution(bool next, Trigger::Response &response);
  /// 逐阶段下发解，管理临时碰撞许可并确认控制器和场景结果
  void runExecution();
  /// 请求取消活动轨迹、作废待执行解，并通过 response 返回受理结果
  void cancel(Trigger::Response &response);
  /// 检查硬件、轨迹限位及实测机械臂关节与阶段起点的一致性
  void checkStart(const moveit_task_constructor_msgs::msg::Solution &solution);
  /// 检查轨迹各点的位置数量、有限性及串口关节范围，角度单位为弧度
  static void checkTrajectoryLimits(const moveit_task_constructor_msgs::msg::Solution &solution);
  /// 根据有效硬件和关节反馈判断夹爪到位，closed 为 true 表示闭合
  bool gripperAt(bool closed);
  /// 检查反馈时效、自动取矿控制权、发送使能及夹爪故障状态
  void checkHardware();
  /// 缓存硬件反馈；控制权变化或反馈失效时请求取消当前任务
  void updateHardware(serial_driver::msg::HardwareStatus::ConstSharedPtr message);

  // 规划配置与输入缓存
  TaskSettings settings_;
  Eigen::Isometry3d object_to_tcp_;
  double max_target_age_, max_joint_age_, target_average_max_age_;
  int target_index_, target_average_samples_;
  VisionTargetBuffer vision_history_;
  std::string execution_mode_;
  std::atomic<bool> busy_{false};
  std::atomic<bool> cancel_requested_{false};
  std::atomic<bool> owned_{false};
  std::mutex input_mutex_, state_mutex_, goal_mutex_, scene_mutex_, command_mutex_;
  Status status_;
  geometry_msgs::msg::PoseArray::ConstSharedPtr vision_;
  geometry_msgs::msg::PoseStamped::ConstSharedPtr manual_;
  sensor_msgs::msg::JointState::ConstSharedPtr joints_;
  serial_driver::msg::HardwareStatus::ConstSharedPtr hardware_;
  double hardware_timeout_ = .1;
  std::chrono::steady_clock::time_point hardware_received_{};

  // 任务解与执行状态
  std::unique_ptr<moveit::task_constructor::Task> task_;
  std::unique_ptr<moveit::task_constructor::Task> pick_tail_task_;
  std::array<moveit_task_constructor_msgs::msg::Solution, 6> stage_solutions_;
  size_t next_stage_ = 0;
  size_t end_stage_ = 3;
  moveit_msgs::msg::AllowedCollisionMatrix baseline_acm_;
  bool extracted_ = false;
  bool contact_state_needs_review_ = false;
  bool execution_unconfirmed_ = false;
  std::atomic<bool> plan_valid_{false}; 
  bool preview_visible_ = false;
  std::chrono::steady_clock::time_point last_preview_update_{};
  std::thread worker_;

  // 场景、TF 和 ROS 通信对象
  std::unique_ptr<EnergyScene> energy_;
  std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  rclcpp::CallbackGroup::SharedPtr service_group_;
  rclcpp_action::Client<Execute>::SharedPtr execute_client_;
  GoalHandle::SharedPtr active_goal_;
  rclcpp::Subscription<geometry_msgs::msg::PoseArray>::SharedPtr vision_sub_;
  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr manual_sub_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joints_sub_;
  rclcpp::Subscription<serial_driver::msg::HardwareStatus>::SharedPtr hardware_sub_;
  rclcpp::Service<Trigger>::SharedPtr plan_service_, manual_service_, place_service_, execute_service_,
      next_service_, cancel_service_;
  rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr object_pub_, grasp_pub_,
      pregrasp_pub_;
  rclcpp::Publisher<moveit_msgs::msg::DisplayTrajectory>::SharedPtr display_pub_;
  rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr preview_pub_;
  rclcpp::Publisher<Status>::SharedPtr status_pub_;
  rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr stage_pub_;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr gripper_pub_;
  rclcpp::TimerBase::SharedPtr preview_timer_;
};
} // namespace engineer_mtc
