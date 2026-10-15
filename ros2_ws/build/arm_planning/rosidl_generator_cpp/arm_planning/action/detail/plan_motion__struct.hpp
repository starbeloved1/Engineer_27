// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from arm_planning:action/PlanMotion.idl
// generated code does not contain a copyright notice

#ifndef ARM_PLANNING__ACTION__DETAIL__PLAN_MOTION__STRUCT_HPP_
#define ARM_PLANNING__ACTION__DETAIL__PLAN_MOTION__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'target_pose'
#include "geometry_msgs/msg/detail/pose_stamped__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__arm_planning__action__PlanMotion_Goal __attribute__((deprecated))
#else
# define DEPRECATED__arm_planning__action__PlanMotion_Goal __declspec(deprecated)
#endif

namespace arm_planning
{

namespace action
{

// message struct
template<class ContainerAllocator>
struct PlanMotion_Goal_
{
  using Type = PlanMotion_Goal_<ContainerAllocator>;

  explicit PlanMotion_Goal_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : target_pose(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->target_type = 0;
      std::fill<typename std::array<double, 6>::iterator, double>(this->joint_positions.begin(), this->joint_positions.end(), 0.0);
      this->execute = false;
      this->velocity_scaling = 0.0;
      this->acceleration_scaling = 0.0;
      this->planning_time = 0.0;
    }
  }

  explicit PlanMotion_Goal_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : joint_positions(_alloc),
    target_pose(_alloc, _init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->target_type = 0;
      std::fill<typename std::array<double, 6>::iterator, double>(this->joint_positions.begin(), this->joint_positions.end(), 0.0);
      this->execute = false;
      this->velocity_scaling = 0.0;
      this->acceleration_scaling = 0.0;
      this->planning_time = 0.0;
    }
  }

  // field types and members
  using _target_type_type =
    uint8_t;
  _target_type_type target_type;
  using _joint_positions_type =
    std::array<double, 6>;
  _joint_positions_type joint_positions;
  using _target_pose_type =
    geometry_msgs::msg::PoseStamped_<ContainerAllocator>;
  _target_pose_type target_pose;
  using _execute_type =
    bool;
  _execute_type execute;
  using _velocity_scaling_type =
    double;
  _velocity_scaling_type velocity_scaling;
  using _acceleration_scaling_type =
    double;
  _acceleration_scaling_type acceleration_scaling;
  using _planning_time_type =
    double;
  _planning_time_type planning_time;

  // setters for named parameter idiom
  Type & set__target_type(
    const uint8_t & _arg)
  {
    this->target_type = _arg;
    return *this;
  }
  Type & set__joint_positions(
    const std::array<double, 6> & _arg)
  {
    this->joint_positions = _arg;
    return *this;
  }
  Type & set__target_pose(
    const geometry_msgs::msg::PoseStamped_<ContainerAllocator> & _arg)
  {
    this->target_pose = _arg;
    return *this;
  }
  Type & set__execute(
    const bool & _arg)
  {
    this->execute = _arg;
    return *this;
  }
  Type & set__velocity_scaling(
    const double & _arg)
  {
    this->velocity_scaling = _arg;
    return *this;
  }
  Type & set__acceleration_scaling(
    const double & _arg)
  {
    this->acceleration_scaling = _arg;
    return *this;
  }
  Type & set__planning_time(
    const double & _arg)
  {
    this->planning_time = _arg;
    return *this;
  }

  // constant declarations
  static constexpr uint8_t JOINT =
    0u;
  static constexpr uint8_t POSE =
    1u;

  // pointer types
  using RawPtr =
    arm_planning::action::PlanMotion_Goal_<ContainerAllocator> *;
  using ConstRawPtr =
    const arm_planning::action::PlanMotion_Goal_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<arm_planning::action::PlanMotion_Goal_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<arm_planning::action::PlanMotion_Goal_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      arm_planning::action::PlanMotion_Goal_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<arm_planning::action::PlanMotion_Goal_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      arm_planning::action::PlanMotion_Goal_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<arm_planning::action::PlanMotion_Goal_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<arm_planning::action::PlanMotion_Goal_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<arm_planning::action::PlanMotion_Goal_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__arm_planning__action__PlanMotion_Goal
    std::shared_ptr<arm_planning::action::PlanMotion_Goal_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__arm_planning__action__PlanMotion_Goal
    std::shared_ptr<arm_planning::action::PlanMotion_Goal_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const PlanMotion_Goal_ & other) const
  {
    if (this->target_type != other.target_type) {
      return false;
    }
    if (this->joint_positions != other.joint_positions) {
      return false;
    }
    if (this->target_pose != other.target_pose) {
      return false;
    }
    if (this->execute != other.execute) {
      return false;
    }
    if (this->velocity_scaling != other.velocity_scaling) {
      return false;
    }
    if (this->acceleration_scaling != other.acceleration_scaling) {
      return false;
    }
    if (this->planning_time != other.planning_time) {
      return false;
    }
    return true;
  }
  bool operator!=(const PlanMotion_Goal_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct PlanMotion_Goal_

// alias to use template instance with default allocator
using PlanMotion_Goal =
  arm_planning::action::PlanMotion_Goal_<std::allocator<void>>;

// constant definitions
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t PlanMotion_Goal_<ContainerAllocator>::JOINT;
#endif  // __cplusplus < 201703L
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t PlanMotion_Goal_<ContainerAllocator>::POSE;
#endif  // __cplusplus < 201703L

}  // namespace action

}  // namespace arm_planning


// Include directives for member types
// Member 'trajectory'
#include "moveit_msgs/msg/detail/robot_trajectory__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__arm_planning__action__PlanMotion_Result __attribute__((deprecated))
#else
# define DEPRECATED__arm_planning__action__PlanMotion_Result __declspec(deprecated)
#endif

namespace arm_planning
{

namespace action
{

// message struct
template<class ContainerAllocator>
struct PlanMotion_Result_
{
  using Type = PlanMotion_Result_<ContainerAllocator>;

  explicit PlanMotion_Result_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : trajectory(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->success = false;
      this->error_code = 0l;
      this->message = "";
      std::fill<typename std::array<double, 6>::iterator, double>(this->selected_joint_positions.begin(), this->selected_joint_positions.end(), 0.0);
      this->planning_time = 0.0;
      this->joint_path_length = 0.0;
      this->validation_samples = 0ul;
      this->ik_candidates = 0ul;
    }
  }

  explicit PlanMotion_Result_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : message(_alloc),
    trajectory(_alloc, _init),
    selected_joint_positions(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->success = false;
      this->error_code = 0l;
      this->message = "";
      std::fill<typename std::array<double, 6>::iterator, double>(this->selected_joint_positions.begin(), this->selected_joint_positions.end(), 0.0);
      this->planning_time = 0.0;
      this->joint_path_length = 0.0;
      this->validation_samples = 0ul;
      this->ik_candidates = 0ul;
    }
  }

  // field types and members
  using _success_type =
    bool;
  _success_type success;
  using _error_code_type =
    int32_t;
  _error_code_type error_code;
  using _message_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _message_type message;
  using _trajectory_type =
    moveit_msgs::msg::RobotTrajectory_<ContainerAllocator>;
  _trajectory_type trajectory;
  using _selected_joint_positions_type =
    std::array<double, 6>;
  _selected_joint_positions_type selected_joint_positions;
  using _planning_time_type =
    double;
  _planning_time_type planning_time;
  using _joint_path_length_type =
    double;
  _joint_path_length_type joint_path_length;
  using _validation_samples_type =
    uint32_t;
  _validation_samples_type validation_samples;
  using _ik_candidates_type =
    uint32_t;
  _ik_candidates_type ik_candidates;

  // setters for named parameter idiom
  Type & set__success(
    const bool & _arg)
  {
    this->success = _arg;
    return *this;
  }
  Type & set__error_code(
    const int32_t & _arg)
  {
    this->error_code = _arg;
    return *this;
  }
  Type & set__message(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->message = _arg;
    return *this;
  }
  Type & set__trajectory(
    const moveit_msgs::msg::RobotTrajectory_<ContainerAllocator> & _arg)
  {
    this->trajectory = _arg;
    return *this;
  }
  Type & set__selected_joint_positions(
    const std::array<double, 6> & _arg)
  {
    this->selected_joint_positions = _arg;
    return *this;
  }
  Type & set__planning_time(
    const double & _arg)
  {
    this->planning_time = _arg;
    return *this;
  }
  Type & set__joint_path_length(
    const double & _arg)
  {
    this->joint_path_length = _arg;
    return *this;
  }
  Type & set__validation_samples(
    const uint32_t & _arg)
  {
    this->validation_samples = _arg;
    return *this;
  }
  Type & set__ik_candidates(
    const uint32_t & _arg)
  {
    this->ik_candidates = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    arm_planning::action::PlanMotion_Result_<ContainerAllocator> *;
  using ConstRawPtr =
    const arm_planning::action::PlanMotion_Result_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<arm_planning::action::PlanMotion_Result_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<arm_planning::action::PlanMotion_Result_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      arm_planning::action::PlanMotion_Result_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<arm_planning::action::PlanMotion_Result_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      arm_planning::action::PlanMotion_Result_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<arm_planning::action::PlanMotion_Result_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<arm_planning::action::PlanMotion_Result_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<arm_planning::action::PlanMotion_Result_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__arm_planning__action__PlanMotion_Result
    std::shared_ptr<arm_planning::action::PlanMotion_Result_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__arm_planning__action__PlanMotion_Result
    std::shared_ptr<arm_planning::action::PlanMotion_Result_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const PlanMotion_Result_ & other) const
  {
    if (this->success != other.success) {
      return false;
    }
    if (this->error_code != other.error_code) {
      return false;
    }
    if (this->message != other.message) {
      return false;
    }
    if (this->trajectory != other.trajectory) {
      return false;
    }
    if (this->selected_joint_positions != other.selected_joint_positions) {
      return false;
    }
    if (this->planning_time != other.planning_time) {
      return false;
    }
    if (this->joint_path_length != other.joint_path_length) {
      return false;
    }
    if (this->validation_samples != other.validation_samples) {
      return false;
    }
    if (this->ik_candidates != other.ik_candidates) {
      return false;
    }
    return true;
  }
  bool operator!=(const PlanMotion_Result_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct PlanMotion_Result_

// alias to use template instance with default allocator
using PlanMotion_Result =
  arm_planning::action::PlanMotion_Result_<std::allocator<void>>;

// constant definitions

}  // namespace action

}  // namespace arm_planning


#ifndef _WIN32
# define DEPRECATED__arm_planning__action__PlanMotion_Feedback __attribute__((deprecated))
#else
# define DEPRECATED__arm_planning__action__PlanMotion_Feedback __declspec(deprecated)
#endif

namespace arm_planning
{

namespace action
{

// message struct
template<class ContainerAllocator>
struct PlanMotion_Feedback_
{
  using Type = PlanMotion_Feedback_<ContainerAllocator>;

  explicit PlanMotion_Feedback_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->stage = "";
      this->detail = "";
    }
  }

  explicit PlanMotion_Feedback_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : stage(_alloc),
    detail(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->stage = "";
      this->detail = "";
    }
  }

  // field types and members
  using _stage_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _stage_type stage;
  using _detail_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _detail_type detail;

  // setters for named parameter idiom
  Type & set__stage(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->stage = _arg;
    return *this;
  }
  Type & set__detail(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->detail = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    arm_planning::action::PlanMotion_Feedback_<ContainerAllocator> *;
  using ConstRawPtr =
    const arm_planning::action::PlanMotion_Feedback_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<arm_planning::action::PlanMotion_Feedback_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<arm_planning::action::PlanMotion_Feedback_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      arm_planning::action::PlanMotion_Feedback_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<arm_planning::action::PlanMotion_Feedback_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      arm_planning::action::PlanMotion_Feedback_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<arm_planning::action::PlanMotion_Feedback_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<arm_planning::action::PlanMotion_Feedback_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<arm_planning::action::PlanMotion_Feedback_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__arm_planning__action__PlanMotion_Feedback
    std::shared_ptr<arm_planning::action::PlanMotion_Feedback_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__arm_planning__action__PlanMotion_Feedback
    std::shared_ptr<arm_planning::action::PlanMotion_Feedback_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const PlanMotion_Feedback_ & other) const
  {
    if (this->stage != other.stage) {
      return false;
    }
    if (this->detail != other.detail) {
      return false;
    }
    return true;
  }
  bool operator!=(const PlanMotion_Feedback_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct PlanMotion_Feedback_

// alias to use template instance with default allocator
using PlanMotion_Feedback =
  arm_planning::action::PlanMotion_Feedback_<std::allocator<void>>;

// constant definitions

}  // namespace action

}  // namespace arm_planning


// Include directives for member types
// Member 'goal_id'
#include "unique_identifier_msgs/msg/detail/uuid__struct.hpp"
// Member 'goal'
#include "arm_planning/action/detail/plan_motion__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__arm_planning__action__PlanMotion_SendGoal_Request __attribute__((deprecated))
#else
# define DEPRECATED__arm_planning__action__PlanMotion_SendGoal_Request __declspec(deprecated)
#endif

namespace arm_planning
{

namespace action
{

// message struct
template<class ContainerAllocator>
struct PlanMotion_SendGoal_Request_
{
  using Type = PlanMotion_SendGoal_Request_<ContainerAllocator>;

  explicit PlanMotion_SendGoal_Request_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : goal_id(_init),
    goal(_init)
  {
    (void)_init;
  }

  explicit PlanMotion_SendGoal_Request_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : goal_id(_alloc, _init),
    goal(_alloc, _init)
  {
    (void)_init;
  }

  // field types and members
  using _goal_id_type =
    unique_identifier_msgs::msg::UUID_<ContainerAllocator>;
  _goal_id_type goal_id;
  using _goal_type =
    arm_planning::action::PlanMotion_Goal_<ContainerAllocator>;
  _goal_type goal;

  // setters for named parameter idiom
  Type & set__goal_id(
    const unique_identifier_msgs::msg::UUID_<ContainerAllocator> & _arg)
  {
    this->goal_id = _arg;
    return *this;
  }
  Type & set__goal(
    const arm_planning::action::PlanMotion_Goal_<ContainerAllocator> & _arg)
  {
    this->goal = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    arm_planning::action::PlanMotion_SendGoal_Request_<ContainerAllocator> *;
  using ConstRawPtr =
    const arm_planning::action::PlanMotion_SendGoal_Request_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<arm_planning::action::PlanMotion_SendGoal_Request_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<arm_planning::action::PlanMotion_SendGoal_Request_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      arm_planning::action::PlanMotion_SendGoal_Request_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<arm_planning::action::PlanMotion_SendGoal_Request_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      arm_planning::action::PlanMotion_SendGoal_Request_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<arm_planning::action::PlanMotion_SendGoal_Request_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<arm_planning::action::PlanMotion_SendGoal_Request_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<arm_planning::action::PlanMotion_SendGoal_Request_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__arm_planning__action__PlanMotion_SendGoal_Request
    std::shared_ptr<arm_planning::action::PlanMotion_SendGoal_Request_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__arm_planning__action__PlanMotion_SendGoal_Request
    std::shared_ptr<arm_planning::action::PlanMotion_SendGoal_Request_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const PlanMotion_SendGoal_Request_ & other) const
  {
    if (this->goal_id != other.goal_id) {
      return false;
    }
    if (this->goal != other.goal) {
      return false;
    }
    return true;
  }
  bool operator!=(const PlanMotion_SendGoal_Request_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct PlanMotion_SendGoal_Request_

// alias to use template instance with default allocator
using PlanMotion_SendGoal_Request =
  arm_planning::action::PlanMotion_SendGoal_Request_<std::allocator<void>>;

// constant definitions

}  // namespace action

}  // namespace arm_planning


// Include directives for member types
// Member 'stamp'
#include "builtin_interfaces/msg/detail/time__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__arm_planning__action__PlanMotion_SendGoal_Response __attribute__((deprecated))
#else
# define DEPRECATED__arm_planning__action__PlanMotion_SendGoal_Response __declspec(deprecated)
#endif

namespace arm_planning
{

namespace action
{

// message struct
template<class ContainerAllocator>
struct PlanMotion_SendGoal_Response_
{
  using Type = PlanMotion_SendGoal_Response_<ContainerAllocator>;

  explicit PlanMotion_SendGoal_Response_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : stamp(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->accepted = false;
    }
  }

  explicit PlanMotion_SendGoal_Response_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : stamp(_alloc, _init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->accepted = false;
    }
  }

  // field types and members
  using _accepted_type =
    bool;
  _accepted_type accepted;
  using _stamp_type =
    builtin_interfaces::msg::Time_<ContainerAllocator>;
  _stamp_type stamp;

  // setters for named parameter idiom
  Type & set__accepted(
    const bool & _arg)
  {
    this->accepted = _arg;
    return *this;
  }
  Type & set__stamp(
    const builtin_interfaces::msg::Time_<ContainerAllocator> & _arg)
  {
    this->stamp = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    arm_planning::action::PlanMotion_SendGoal_Response_<ContainerAllocator> *;
  using ConstRawPtr =
    const arm_planning::action::PlanMotion_SendGoal_Response_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<arm_planning::action::PlanMotion_SendGoal_Response_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<arm_planning::action::PlanMotion_SendGoal_Response_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      arm_planning::action::PlanMotion_SendGoal_Response_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<arm_planning::action::PlanMotion_SendGoal_Response_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      arm_planning::action::PlanMotion_SendGoal_Response_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<arm_planning::action::PlanMotion_SendGoal_Response_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<arm_planning::action::PlanMotion_SendGoal_Response_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<arm_planning::action::PlanMotion_SendGoal_Response_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__arm_planning__action__PlanMotion_SendGoal_Response
    std::shared_ptr<arm_planning::action::PlanMotion_SendGoal_Response_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__arm_planning__action__PlanMotion_SendGoal_Response
    std::shared_ptr<arm_planning::action::PlanMotion_SendGoal_Response_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const PlanMotion_SendGoal_Response_ & other) const
  {
    if (this->accepted != other.accepted) {
      return false;
    }
    if (this->stamp != other.stamp) {
      return false;
    }
    return true;
  }
  bool operator!=(const PlanMotion_SendGoal_Response_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct PlanMotion_SendGoal_Response_

// alias to use template instance with default allocator
using PlanMotion_SendGoal_Response =
  arm_planning::action::PlanMotion_SendGoal_Response_<std::allocator<void>>;

// constant definitions

}  // namespace action

}  // namespace arm_planning

namespace arm_planning
{

namespace action
{

struct PlanMotion_SendGoal
{
  using Request = arm_planning::action::PlanMotion_SendGoal_Request;
  using Response = arm_planning::action::PlanMotion_SendGoal_Response;
};

}  // namespace action

}  // namespace arm_planning


// Include directives for member types
// Member 'goal_id'
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__arm_planning__action__PlanMotion_GetResult_Request __attribute__((deprecated))
#else
# define DEPRECATED__arm_planning__action__PlanMotion_GetResult_Request __declspec(deprecated)
#endif

namespace arm_planning
{

namespace action
{

// message struct
template<class ContainerAllocator>
struct PlanMotion_GetResult_Request_
{
  using Type = PlanMotion_GetResult_Request_<ContainerAllocator>;

  explicit PlanMotion_GetResult_Request_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : goal_id(_init)
  {
    (void)_init;
  }

  explicit PlanMotion_GetResult_Request_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : goal_id(_alloc, _init)
  {
    (void)_init;
  }

  // field types and members
  using _goal_id_type =
    unique_identifier_msgs::msg::UUID_<ContainerAllocator>;
  _goal_id_type goal_id;

  // setters for named parameter idiom
  Type & set__goal_id(
    const unique_identifier_msgs::msg::UUID_<ContainerAllocator> & _arg)
  {
    this->goal_id = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    arm_planning::action::PlanMotion_GetResult_Request_<ContainerAllocator> *;
  using ConstRawPtr =
    const arm_planning::action::PlanMotion_GetResult_Request_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<arm_planning::action::PlanMotion_GetResult_Request_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<arm_planning::action::PlanMotion_GetResult_Request_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      arm_planning::action::PlanMotion_GetResult_Request_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<arm_planning::action::PlanMotion_GetResult_Request_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      arm_planning::action::PlanMotion_GetResult_Request_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<arm_planning::action::PlanMotion_GetResult_Request_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<arm_planning::action::PlanMotion_GetResult_Request_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<arm_planning::action::PlanMotion_GetResult_Request_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__arm_planning__action__PlanMotion_GetResult_Request
    std::shared_ptr<arm_planning::action::PlanMotion_GetResult_Request_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__arm_planning__action__PlanMotion_GetResult_Request
    std::shared_ptr<arm_planning::action::PlanMotion_GetResult_Request_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const PlanMotion_GetResult_Request_ & other) const
  {
    if (this->goal_id != other.goal_id) {
      return false;
    }
    return true;
  }
  bool operator!=(const PlanMotion_GetResult_Request_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct PlanMotion_GetResult_Request_

// alias to use template instance with default allocator
using PlanMotion_GetResult_Request =
  arm_planning::action::PlanMotion_GetResult_Request_<std::allocator<void>>;

// constant definitions

}  // namespace action

}  // namespace arm_planning


// Include directives for member types
// Member 'result'
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__arm_planning__action__PlanMotion_GetResult_Response __attribute__((deprecated))
#else
# define DEPRECATED__arm_planning__action__PlanMotion_GetResult_Response __declspec(deprecated)
#endif

namespace arm_planning
{

namespace action
{

// message struct
template<class ContainerAllocator>
struct PlanMotion_GetResult_Response_
{
  using Type = PlanMotion_GetResult_Response_<ContainerAllocator>;

  explicit PlanMotion_GetResult_Response_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : result(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->status = 0;
    }
  }

  explicit PlanMotion_GetResult_Response_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : result(_alloc, _init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->status = 0;
    }
  }

  // field types and members
  using _status_type =
    int8_t;
  _status_type status;
  using _result_type =
    arm_planning::action::PlanMotion_Result_<ContainerAllocator>;
  _result_type result;

  // setters for named parameter idiom
  Type & set__status(
    const int8_t & _arg)
  {
    this->status = _arg;
    return *this;
  }
  Type & set__result(
    const arm_planning::action::PlanMotion_Result_<ContainerAllocator> & _arg)
  {
    this->result = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    arm_planning::action::PlanMotion_GetResult_Response_<ContainerAllocator> *;
  using ConstRawPtr =
    const arm_planning::action::PlanMotion_GetResult_Response_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<arm_planning::action::PlanMotion_GetResult_Response_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<arm_planning::action::PlanMotion_GetResult_Response_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      arm_planning::action::PlanMotion_GetResult_Response_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<arm_planning::action::PlanMotion_GetResult_Response_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      arm_planning::action::PlanMotion_GetResult_Response_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<arm_planning::action::PlanMotion_GetResult_Response_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<arm_planning::action::PlanMotion_GetResult_Response_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<arm_planning::action::PlanMotion_GetResult_Response_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__arm_planning__action__PlanMotion_GetResult_Response
    std::shared_ptr<arm_planning::action::PlanMotion_GetResult_Response_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__arm_planning__action__PlanMotion_GetResult_Response
    std::shared_ptr<arm_planning::action::PlanMotion_GetResult_Response_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const PlanMotion_GetResult_Response_ & other) const
  {
    if (this->status != other.status) {
      return false;
    }
    if (this->result != other.result) {
      return false;
    }
    return true;
  }
  bool operator!=(const PlanMotion_GetResult_Response_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct PlanMotion_GetResult_Response_

// alias to use template instance with default allocator
using PlanMotion_GetResult_Response =
  arm_planning::action::PlanMotion_GetResult_Response_<std::allocator<void>>;

// constant definitions

}  // namespace action

}  // namespace arm_planning

namespace arm_planning
{

namespace action
{

struct PlanMotion_GetResult
{
  using Request = arm_planning::action::PlanMotion_GetResult_Request;
  using Response = arm_planning::action::PlanMotion_GetResult_Response;
};

}  // namespace action

}  // namespace arm_planning


// Include directives for member types
// Member 'goal_id'
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__struct.hpp"
// Member 'feedback'
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__arm_planning__action__PlanMotion_FeedbackMessage __attribute__((deprecated))
#else
# define DEPRECATED__arm_planning__action__PlanMotion_FeedbackMessage __declspec(deprecated)
#endif

namespace arm_planning
{

namespace action
{

// message struct
template<class ContainerAllocator>
struct PlanMotion_FeedbackMessage_
{
  using Type = PlanMotion_FeedbackMessage_<ContainerAllocator>;

  explicit PlanMotion_FeedbackMessage_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : goal_id(_init),
    feedback(_init)
  {
    (void)_init;
  }

  explicit PlanMotion_FeedbackMessage_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : goal_id(_alloc, _init),
    feedback(_alloc, _init)
  {
    (void)_init;
  }

  // field types and members
  using _goal_id_type =
    unique_identifier_msgs::msg::UUID_<ContainerAllocator>;
  _goal_id_type goal_id;
  using _feedback_type =
    arm_planning::action::PlanMotion_Feedback_<ContainerAllocator>;
  _feedback_type feedback;

  // setters for named parameter idiom
  Type & set__goal_id(
    const unique_identifier_msgs::msg::UUID_<ContainerAllocator> & _arg)
  {
    this->goal_id = _arg;
    return *this;
  }
  Type & set__feedback(
    const arm_planning::action::PlanMotion_Feedback_<ContainerAllocator> & _arg)
  {
    this->feedback = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    arm_planning::action::PlanMotion_FeedbackMessage_<ContainerAllocator> *;
  using ConstRawPtr =
    const arm_planning::action::PlanMotion_FeedbackMessage_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<arm_planning::action::PlanMotion_FeedbackMessage_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<arm_planning::action::PlanMotion_FeedbackMessage_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      arm_planning::action::PlanMotion_FeedbackMessage_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<arm_planning::action::PlanMotion_FeedbackMessage_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      arm_planning::action::PlanMotion_FeedbackMessage_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<arm_planning::action::PlanMotion_FeedbackMessage_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<arm_planning::action::PlanMotion_FeedbackMessage_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<arm_planning::action::PlanMotion_FeedbackMessage_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__arm_planning__action__PlanMotion_FeedbackMessage
    std::shared_ptr<arm_planning::action::PlanMotion_FeedbackMessage_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__arm_planning__action__PlanMotion_FeedbackMessage
    std::shared_ptr<arm_planning::action::PlanMotion_FeedbackMessage_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const PlanMotion_FeedbackMessage_ & other) const
  {
    if (this->goal_id != other.goal_id) {
      return false;
    }
    if (this->feedback != other.feedback) {
      return false;
    }
    return true;
  }
  bool operator!=(const PlanMotion_FeedbackMessage_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct PlanMotion_FeedbackMessage_

// alias to use template instance with default allocator
using PlanMotion_FeedbackMessage =
  arm_planning::action::PlanMotion_FeedbackMessage_<std::allocator<void>>;

// constant definitions

}  // namespace action

}  // namespace arm_planning

#include "action_msgs/srv/cancel_goal.hpp"
#include "action_msgs/msg/goal_info.hpp"
#include "action_msgs/msg/goal_status_array.hpp"

namespace arm_planning
{

namespace action
{

struct PlanMotion
{
  /// The goal message defined in the action definition.
  using Goal = arm_planning::action::PlanMotion_Goal;
  /// The result message defined in the action definition.
  using Result = arm_planning::action::PlanMotion_Result;
  /// The feedback message defined in the action definition.
  using Feedback = arm_planning::action::PlanMotion_Feedback;

  struct Impl
  {
    /// The send_goal service using a wrapped version of the goal message as a request.
    using SendGoalService = arm_planning::action::PlanMotion_SendGoal;
    /// The get_result service using a wrapped version of the result message as a response.
    using GetResultService = arm_planning::action::PlanMotion_GetResult;
    /// The feedback message with generic fields which wraps the feedback message.
    using FeedbackMessage = arm_planning::action::PlanMotion_FeedbackMessage;

    /// The generic service to cancel a goal.
    using CancelGoalService = action_msgs::srv::CancelGoal;
    /// The generic message for the status of a goal.
    using GoalStatusMessage = action_msgs::msg::GoalStatusArray;
  };
};

typedef struct PlanMotion PlanMotion;

}  // namespace action

}  // namespace arm_planning

#endif  // ARM_PLANNING__ACTION__DETAIL__PLAN_MOTION__STRUCT_HPP_
