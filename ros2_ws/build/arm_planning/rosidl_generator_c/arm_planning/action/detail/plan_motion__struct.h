// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from arm_planning:action/PlanMotion.idl
// generated code does not contain a copyright notice

#ifndef ARM_PLANNING__ACTION__DETAIL__PLAN_MOTION__STRUCT_H_
#define ARM_PLANNING__ACTION__DETAIL__PLAN_MOTION__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

/// Constant 'JOINT'.
enum
{
  arm_planning__action__PlanMotion_Goal__JOINT = 0
};

/// Constant 'POSE'.
enum
{
  arm_planning__action__PlanMotion_Goal__POSE = 1
};

// Include directives for member types
// Member 'target_pose'
#include "geometry_msgs/msg/detail/pose_stamped__struct.h"

/// Struct defined in action/PlanMotion in the package arm_planning.
typedef struct arm_planning__action__PlanMotion_Goal
{
  uint8_t target_type;
  double joint_positions[6];
  geometry_msgs__msg__PoseStamped target_pose;
  /// execute=false only plans and publishes the RViz preview.
  bool execute;
  /// Zero selects the offline defaults (0.2, 0.2, 5 seconds).
  double velocity_scaling;
  double acceleration_scaling;
  double planning_time;
} arm_planning__action__PlanMotion_Goal;

// Struct for a sequence of arm_planning__action__PlanMotion_Goal.
typedef struct arm_planning__action__PlanMotion_Goal__Sequence
{
  arm_planning__action__PlanMotion_Goal * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} arm_planning__action__PlanMotion_Goal__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'message'
#include "rosidl_runtime_c/string.h"
// Member 'trajectory'
#include "moveit_msgs/msg/detail/robot_trajectory__struct.h"

/// Struct defined in action/PlanMotion in the package arm_planning.
typedef struct arm_planning__action__PlanMotion_Result
{
  bool success;
  int32_t error_code;
  rosidl_runtime_c__String message;
  moveit_msgs__msg__RobotTrajectory trajectory;
  double selected_joint_positions[6];
  double planning_time;
  double joint_path_length;
  uint32_t validation_samples;
  uint32_t ik_candidates;
} arm_planning__action__PlanMotion_Result;

// Struct for a sequence of arm_planning__action__PlanMotion_Result.
typedef struct arm_planning__action__PlanMotion_Result__Sequence
{
  arm_planning__action__PlanMotion_Result * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} arm_planning__action__PlanMotion_Result__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'stage'
// Member 'detail'
// already included above
// #include "rosidl_runtime_c/string.h"

/// Struct defined in action/PlanMotion in the package arm_planning.
typedef struct arm_planning__action__PlanMotion_Feedback
{
  rosidl_runtime_c__String stage;
  rosidl_runtime_c__String detail;
} arm_planning__action__PlanMotion_Feedback;

// Struct for a sequence of arm_planning__action__PlanMotion_Feedback.
typedef struct arm_planning__action__PlanMotion_Feedback__Sequence
{
  arm_planning__action__PlanMotion_Feedback * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} arm_planning__action__PlanMotion_Feedback__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'goal_id'
#include "unique_identifier_msgs/msg/detail/uuid__struct.h"
// Member 'goal'
#include "arm_planning/action/detail/plan_motion__struct.h"

/// Struct defined in action/PlanMotion in the package arm_planning.
typedef struct arm_planning__action__PlanMotion_SendGoal_Request
{
  unique_identifier_msgs__msg__UUID goal_id;
  arm_planning__action__PlanMotion_Goal goal;
} arm_planning__action__PlanMotion_SendGoal_Request;

// Struct for a sequence of arm_planning__action__PlanMotion_SendGoal_Request.
typedef struct arm_planning__action__PlanMotion_SendGoal_Request__Sequence
{
  arm_planning__action__PlanMotion_SendGoal_Request * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} arm_planning__action__PlanMotion_SendGoal_Request__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'stamp'
#include "builtin_interfaces/msg/detail/time__struct.h"

/// Struct defined in action/PlanMotion in the package arm_planning.
typedef struct arm_planning__action__PlanMotion_SendGoal_Response
{
  bool accepted;
  builtin_interfaces__msg__Time stamp;
} arm_planning__action__PlanMotion_SendGoal_Response;

// Struct for a sequence of arm_planning__action__PlanMotion_SendGoal_Response.
typedef struct arm_planning__action__PlanMotion_SendGoal_Response__Sequence
{
  arm_planning__action__PlanMotion_SendGoal_Response * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} arm_planning__action__PlanMotion_SendGoal_Response__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'goal_id'
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__struct.h"

/// Struct defined in action/PlanMotion in the package arm_planning.
typedef struct arm_planning__action__PlanMotion_GetResult_Request
{
  unique_identifier_msgs__msg__UUID goal_id;
} arm_planning__action__PlanMotion_GetResult_Request;

// Struct for a sequence of arm_planning__action__PlanMotion_GetResult_Request.
typedef struct arm_planning__action__PlanMotion_GetResult_Request__Sequence
{
  arm_planning__action__PlanMotion_GetResult_Request * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} arm_planning__action__PlanMotion_GetResult_Request__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'result'
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.h"

/// Struct defined in action/PlanMotion in the package arm_planning.
typedef struct arm_planning__action__PlanMotion_GetResult_Response
{
  int8_t status;
  arm_planning__action__PlanMotion_Result result;
} arm_planning__action__PlanMotion_GetResult_Response;

// Struct for a sequence of arm_planning__action__PlanMotion_GetResult_Response.
typedef struct arm_planning__action__PlanMotion_GetResult_Response__Sequence
{
  arm_planning__action__PlanMotion_GetResult_Response * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} arm_planning__action__PlanMotion_GetResult_Response__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'goal_id'
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__struct.h"
// Member 'feedback'
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.h"

/// Struct defined in action/PlanMotion in the package arm_planning.
typedef struct arm_planning__action__PlanMotion_FeedbackMessage
{
  unique_identifier_msgs__msg__UUID goal_id;
  arm_planning__action__PlanMotion_Feedback feedback;
} arm_planning__action__PlanMotion_FeedbackMessage;

// Struct for a sequence of arm_planning__action__PlanMotion_FeedbackMessage.
typedef struct arm_planning__action__PlanMotion_FeedbackMessage__Sequence
{
  arm_planning__action__PlanMotion_FeedbackMessage * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} arm_planning__action__PlanMotion_FeedbackMessage__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // ARM_PLANNING__ACTION__DETAIL__PLAN_MOTION__STRUCT_H_
