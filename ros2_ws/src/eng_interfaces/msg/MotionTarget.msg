# Topic 输入目标；每次请求使用新的非空 request_id，用于关联反馈、结果和取消。
string request_id
uint8 JOINT=0
uint8 POSE=1
uint8 target_type
float64[6] joint_positions
geometry_msgs/PoseStamped target_pose
# 默认 false，只规划；true 时请求模拟执行。
bool execute
# 为 0 时使用规划服务端 YAML 的默认值。
float64 velocity_scaling
float64 acceleration_scaling
float64 planning_time
