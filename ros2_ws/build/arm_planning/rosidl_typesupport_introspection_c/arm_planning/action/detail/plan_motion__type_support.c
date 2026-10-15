// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from arm_planning:action/PlanMotion.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "arm_planning/action/detail/plan_motion__rosidl_typesupport_introspection_c.h"
#include "arm_planning/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "arm_planning/action/detail/plan_motion__functions.h"
#include "arm_planning/action/detail/plan_motion__struct.h"


// Include directives for member types
// Member `target_pose`
#include "geometry_msgs/msg/pose_stamped.h"
// Member `target_pose`
#include "geometry_msgs/msg/detail/pose_stamped__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__PlanMotion_Goal_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  arm_planning__action__PlanMotion_Goal__init(message_memory);
}

void arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__PlanMotion_Goal_fini_function(void * message_memory)
{
  arm_planning__action__PlanMotion_Goal__fini(message_memory);
}

size_t arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__size_function__PlanMotion_Goal__joint_positions(
  const void * untyped_member)
{
  (void)untyped_member;
  return 6;
}

const void * arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__get_const_function__PlanMotion_Goal__joint_positions(
  const void * untyped_member, size_t index)
{
  const double * member =
    (const double *)(untyped_member);
  return &member[index];
}

void * arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__get_function__PlanMotion_Goal__joint_positions(
  void * untyped_member, size_t index)
{
  double * member =
    (double *)(untyped_member);
  return &member[index];
}

void arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__fetch_function__PlanMotion_Goal__joint_positions(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__get_const_function__PlanMotion_Goal__joint_positions(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__assign_function__PlanMotion_Goal__joint_positions(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__get_function__PlanMotion_Goal__joint_positions(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

static rosidl_typesupport_introspection_c__MessageMember arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__PlanMotion_Goal_message_member_array[7] = {
  {
    "target_type",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_UINT8,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Goal, target_type),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "joint_positions",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    6,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Goal, joint_positions),  // bytes offset in struct
    NULL,  // default value
    arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__size_function__PlanMotion_Goal__joint_positions,  // size() function pointer
    arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__get_const_function__PlanMotion_Goal__joint_positions,  // get_const(index) function pointer
    arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__get_function__PlanMotion_Goal__joint_positions,  // get(index) function pointer
    arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__fetch_function__PlanMotion_Goal__joint_positions,  // fetch(index, &value) function pointer
    arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__assign_function__PlanMotion_Goal__joint_positions,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "target_pose",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Goal, target_pose),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "execute",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_BOOLEAN,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Goal, execute),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "velocity_scaling",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Goal, velocity_scaling),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "acceleration_scaling",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Goal, acceleration_scaling),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "planning_time",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Goal, planning_time),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__PlanMotion_Goal_message_members = {
  "arm_planning__action",  // message namespace
  "PlanMotion_Goal",  // message name
  7,  // number of fields
  sizeof(arm_planning__action__PlanMotion_Goal),
  arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__PlanMotion_Goal_message_member_array,  // message members
  arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__PlanMotion_Goal_init_function,  // function to initialize message memory (memory has to be allocated)
  arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__PlanMotion_Goal_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__PlanMotion_Goal_message_type_support_handle = {
  0,
  &arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__PlanMotion_Goal_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_arm_planning
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_Goal)() {
  arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__PlanMotion_Goal_message_member_array[2].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, geometry_msgs, msg, PoseStamped)();
  if (!arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__PlanMotion_Goal_message_type_support_handle.typesupport_identifier) {
    arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__PlanMotion_Goal_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &arm_planning__action__PlanMotion_Goal__rosidl_typesupport_introspection_c__PlanMotion_Goal_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif

// already included above
// #include <stddef.h>
// already included above
// #include "arm_planning/action/detail/plan_motion__rosidl_typesupport_introspection_c.h"
// already included above
// #include "arm_planning/msg/rosidl_typesupport_introspection_c__visibility_control.h"
// already included above
// #include "rosidl_typesupport_introspection_c/field_types.h"
// already included above
// #include "rosidl_typesupport_introspection_c/identifier.h"
// already included above
// #include "rosidl_typesupport_introspection_c/message_introspection.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__functions.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.h"


// Include directives for member types
// Member `message`
#include "rosidl_runtime_c/string_functions.h"
// Member `trajectory`
#include "moveit_msgs/msg/robot_trajectory.h"
// Member `trajectory`
#include "moveit_msgs/msg/detail/robot_trajectory__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__PlanMotion_Result_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  arm_planning__action__PlanMotion_Result__init(message_memory);
}

void arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__PlanMotion_Result_fini_function(void * message_memory)
{
  arm_planning__action__PlanMotion_Result__fini(message_memory);
}

size_t arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__size_function__PlanMotion_Result__selected_joint_positions(
  const void * untyped_member)
{
  (void)untyped_member;
  return 6;
}

const void * arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__get_const_function__PlanMotion_Result__selected_joint_positions(
  const void * untyped_member, size_t index)
{
  const double * member =
    (const double *)(untyped_member);
  return &member[index];
}

void * arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__get_function__PlanMotion_Result__selected_joint_positions(
  void * untyped_member, size_t index)
{
  double * member =
    (double *)(untyped_member);
  return &member[index];
}

void arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__fetch_function__PlanMotion_Result__selected_joint_positions(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__get_const_function__PlanMotion_Result__selected_joint_positions(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__assign_function__PlanMotion_Result__selected_joint_positions(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__get_function__PlanMotion_Result__selected_joint_positions(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

static rosidl_typesupport_introspection_c__MessageMember arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__PlanMotion_Result_message_member_array[9] = {
  {
    "success",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_BOOLEAN,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Result, success),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "error_code",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_INT32,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Result, error_code),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "message",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_STRING,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Result, message),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "trajectory",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Result, trajectory),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "selected_joint_positions",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    6,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Result, selected_joint_positions),  // bytes offset in struct
    NULL,  // default value
    arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__size_function__PlanMotion_Result__selected_joint_positions,  // size() function pointer
    arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__get_const_function__PlanMotion_Result__selected_joint_positions,  // get_const(index) function pointer
    arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__get_function__PlanMotion_Result__selected_joint_positions,  // get(index) function pointer
    arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__fetch_function__PlanMotion_Result__selected_joint_positions,  // fetch(index, &value) function pointer
    arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__assign_function__PlanMotion_Result__selected_joint_positions,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "planning_time",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Result, planning_time),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "joint_path_length",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Result, joint_path_length),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "validation_samples",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_UINT32,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Result, validation_samples),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "ik_candidates",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_UINT32,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Result, ik_candidates),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__PlanMotion_Result_message_members = {
  "arm_planning__action",  // message namespace
  "PlanMotion_Result",  // message name
  9,  // number of fields
  sizeof(arm_planning__action__PlanMotion_Result),
  arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__PlanMotion_Result_message_member_array,  // message members
  arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__PlanMotion_Result_init_function,  // function to initialize message memory (memory has to be allocated)
  arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__PlanMotion_Result_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__PlanMotion_Result_message_type_support_handle = {
  0,
  &arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__PlanMotion_Result_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_arm_planning
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_Result)() {
  arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__PlanMotion_Result_message_member_array[3].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, moveit_msgs, msg, RobotTrajectory)();
  if (!arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__PlanMotion_Result_message_type_support_handle.typesupport_identifier) {
    arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__PlanMotion_Result_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &arm_planning__action__PlanMotion_Result__rosidl_typesupport_introspection_c__PlanMotion_Result_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif

// already included above
// #include <stddef.h>
// already included above
// #include "arm_planning/action/detail/plan_motion__rosidl_typesupport_introspection_c.h"
// already included above
// #include "arm_planning/msg/rosidl_typesupport_introspection_c__visibility_control.h"
// already included above
// #include "rosidl_typesupport_introspection_c/field_types.h"
// already included above
// #include "rosidl_typesupport_introspection_c/identifier.h"
// already included above
// #include "rosidl_typesupport_introspection_c/message_introspection.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__functions.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.h"


// Include directives for member types
// Member `stage`
// Member `detail`
// already included above
// #include "rosidl_runtime_c/string_functions.h"

#ifdef __cplusplus
extern "C"
{
#endif

void arm_planning__action__PlanMotion_Feedback__rosidl_typesupport_introspection_c__PlanMotion_Feedback_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  arm_planning__action__PlanMotion_Feedback__init(message_memory);
}

void arm_planning__action__PlanMotion_Feedback__rosidl_typesupport_introspection_c__PlanMotion_Feedback_fini_function(void * message_memory)
{
  arm_planning__action__PlanMotion_Feedback__fini(message_memory);
}

static rosidl_typesupport_introspection_c__MessageMember arm_planning__action__PlanMotion_Feedback__rosidl_typesupport_introspection_c__PlanMotion_Feedback_message_member_array[2] = {
  {
    "stage",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_STRING,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Feedback, stage),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "detail",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_STRING,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_Feedback, detail),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers arm_planning__action__PlanMotion_Feedback__rosidl_typesupport_introspection_c__PlanMotion_Feedback_message_members = {
  "arm_planning__action",  // message namespace
  "PlanMotion_Feedback",  // message name
  2,  // number of fields
  sizeof(arm_planning__action__PlanMotion_Feedback),
  arm_planning__action__PlanMotion_Feedback__rosidl_typesupport_introspection_c__PlanMotion_Feedback_message_member_array,  // message members
  arm_planning__action__PlanMotion_Feedback__rosidl_typesupport_introspection_c__PlanMotion_Feedback_init_function,  // function to initialize message memory (memory has to be allocated)
  arm_planning__action__PlanMotion_Feedback__rosidl_typesupport_introspection_c__PlanMotion_Feedback_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t arm_planning__action__PlanMotion_Feedback__rosidl_typesupport_introspection_c__PlanMotion_Feedback_message_type_support_handle = {
  0,
  &arm_planning__action__PlanMotion_Feedback__rosidl_typesupport_introspection_c__PlanMotion_Feedback_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_arm_planning
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_Feedback)() {
  if (!arm_planning__action__PlanMotion_Feedback__rosidl_typesupport_introspection_c__PlanMotion_Feedback_message_type_support_handle.typesupport_identifier) {
    arm_planning__action__PlanMotion_Feedback__rosidl_typesupport_introspection_c__PlanMotion_Feedback_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &arm_planning__action__PlanMotion_Feedback__rosidl_typesupport_introspection_c__PlanMotion_Feedback_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif

// already included above
// #include <stddef.h>
// already included above
// #include "arm_planning/action/detail/plan_motion__rosidl_typesupport_introspection_c.h"
// already included above
// #include "arm_planning/msg/rosidl_typesupport_introspection_c__visibility_control.h"
// already included above
// #include "rosidl_typesupport_introspection_c/field_types.h"
// already included above
// #include "rosidl_typesupport_introspection_c/identifier.h"
// already included above
// #include "rosidl_typesupport_introspection_c/message_introspection.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__functions.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.h"


// Include directives for member types
// Member `goal_id`
#include "unique_identifier_msgs/msg/uuid.h"
// Member `goal_id`
#include "unique_identifier_msgs/msg/detail/uuid__rosidl_typesupport_introspection_c.h"
// Member `goal`
#include "arm_planning/action/plan_motion.h"
// Member `goal`
// already included above
// #include "arm_planning/action/detail/plan_motion__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void arm_planning__action__PlanMotion_SendGoal_Request__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Request_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  arm_planning__action__PlanMotion_SendGoal_Request__init(message_memory);
}

void arm_planning__action__PlanMotion_SendGoal_Request__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Request_fini_function(void * message_memory)
{
  arm_planning__action__PlanMotion_SendGoal_Request__fini(message_memory);
}

static rosidl_typesupport_introspection_c__MessageMember arm_planning__action__PlanMotion_SendGoal_Request__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Request_message_member_array[2] = {
  {
    "goal_id",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_SendGoal_Request, goal_id),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "goal",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_SendGoal_Request, goal),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers arm_planning__action__PlanMotion_SendGoal_Request__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Request_message_members = {
  "arm_planning__action",  // message namespace
  "PlanMotion_SendGoal_Request",  // message name
  2,  // number of fields
  sizeof(arm_planning__action__PlanMotion_SendGoal_Request),
  arm_planning__action__PlanMotion_SendGoal_Request__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Request_message_member_array,  // message members
  arm_planning__action__PlanMotion_SendGoal_Request__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Request_init_function,  // function to initialize message memory (memory has to be allocated)
  arm_planning__action__PlanMotion_SendGoal_Request__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Request_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t arm_planning__action__PlanMotion_SendGoal_Request__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Request_message_type_support_handle = {
  0,
  &arm_planning__action__PlanMotion_SendGoal_Request__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Request_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_arm_planning
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_SendGoal_Request)() {
  arm_planning__action__PlanMotion_SendGoal_Request__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Request_message_member_array[0].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, unique_identifier_msgs, msg, UUID)();
  arm_planning__action__PlanMotion_SendGoal_Request__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Request_message_member_array[1].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_Goal)();
  if (!arm_planning__action__PlanMotion_SendGoal_Request__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Request_message_type_support_handle.typesupport_identifier) {
    arm_planning__action__PlanMotion_SendGoal_Request__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Request_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &arm_planning__action__PlanMotion_SendGoal_Request__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Request_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif

// already included above
// #include <stddef.h>
// already included above
// #include "arm_planning/action/detail/plan_motion__rosidl_typesupport_introspection_c.h"
// already included above
// #include "arm_planning/msg/rosidl_typesupport_introspection_c__visibility_control.h"
// already included above
// #include "rosidl_typesupport_introspection_c/field_types.h"
// already included above
// #include "rosidl_typesupport_introspection_c/identifier.h"
// already included above
// #include "rosidl_typesupport_introspection_c/message_introspection.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__functions.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.h"


// Include directives for member types
// Member `stamp`
#include "builtin_interfaces/msg/time.h"
// Member `stamp`
#include "builtin_interfaces/msg/detail/time__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void arm_planning__action__PlanMotion_SendGoal_Response__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Response_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  arm_planning__action__PlanMotion_SendGoal_Response__init(message_memory);
}

void arm_planning__action__PlanMotion_SendGoal_Response__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Response_fini_function(void * message_memory)
{
  arm_planning__action__PlanMotion_SendGoal_Response__fini(message_memory);
}

static rosidl_typesupport_introspection_c__MessageMember arm_planning__action__PlanMotion_SendGoal_Response__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Response_message_member_array[2] = {
  {
    "accepted",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_BOOLEAN,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_SendGoal_Response, accepted),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "stamp",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_SendGoal_Response, stamp),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers arm_planning__action__PlanMotion_SendGoal_Response__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Response_message_members = {
  "arm_planning__action",  // message namespace
  "PlanMotion_SendGoal_Response",  // message name
  2,  // number of fields
  sizeof(arm_planning__action__PlanMotion_SendGoal_Response),
  arm_planning__action__PlanMotion_SendGoal_Response__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Response_message_member_array,  // message members
  arm_planning__action__PlanMotion_SendGoal_Response__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Response_init_function,  // function to initialize message memory (memory has to be allocated)
  arm_planning__action__PlanMotion_SendGoal_Response__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Response_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t arm_planning__action__PlanMotion_SendGoal_Response__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Response_message_type_support_handle = {
  0,
  &arm_planning__action__PlanMotion_SendGoal_Response__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Response_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_arm_planning
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_SendGoal_Response)() {
  arm_planning__action__PlanMotion_SendGoal_Response__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Response_message_member_array[1].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, builtin_interfaces, msg, Time)();
  if (!arm_planning__action__PlanMotion_SendGoal_Response__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Response_message_type_support_handle.typesupport_identifier) {
    arm_planning__action__PlanMotion_SendGoal_Response__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Response_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &arm_planning__action__PlanMotion_SendGoal_Response__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Response_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif

#include "rosidl_runtime_c/service_type_support_struct.h"
// already included above
// #include "arm_planning/msg/rosidl_typesupport_introspection_c__visibility_control.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__rosidl_typesupport_introspection_c.h"
// already included above
// #include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/service_introspection.h"

// this is intentionally not const to allow initialization later to prevent an initialization race
static rosidl_typesupport_introspection_c__ServiceMembers arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_service_members = {
  "arm_planning__action",  // service namespace
  "PlanMotion_SendGoal",  // service name
  // these two fields are initialized below on the first access
  NULL,  // request message
  // arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Request_message_type_support_handle,
  NULL  // response message
  // arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_Response_message_type_support_handle
};

static rosidl_service_type_support_t arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_service_type_support_handle = {
  0,
  &arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_service_members,
  get_service_typesupport_handle_function,
};

// Forward declaration of request/response type support functions
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_SendGoal_Request)();

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_SendGoal_Response)();

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_arm_planning
const rosidl_service_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_SendGoal)() {
  if (!arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_service_type_support_handle.typesupport_identifier) {
    arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_service_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  rosidl_typesupport_introspection_c__ServiceMembers * service_members =
    (rosidl_typesupport_introspection_c__ServiceMembers *)arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_service_type_support_handle.data;

  if (!service_members->request_members_) {
    service_members->request_members_ =
      (const rosidl_typesupport_introspection_c__MessageMembers *)
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_SendGoal_Request)()->data;
  }
  if (!service_members->response_members_) {
    service_members->response_members_ =
      (const rosidl_typesupport_introspection_c__MessageMembers *)
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_SendGoal_Response)()->data;
  }

  return &arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_SendGoal_service_type_support_handle;
}

// already included above
// #include <stddef.h>
// already included above
// #include "arm_planning/action/detail/plan_motion__rosidl_typesupport_introspection_c.h"
// already included above
// #include "arm_planning/msg/rosidl_typesupport_introspection_c__visibility_control.h"
// already included above
// #include "rosidl_typesupport_introspection_c/field_types.h"
// already included above
// #include "rosidl_typesupport_introspection_c/identifier.h"
// already included above
// #include "rosidl_typesupport_introspection_c/message_introspection.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__functions.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.h"


// Include directives for member types
// Member `goal_id`
// already included above
// #include "unique_identifier_msgs/msg/uuid.h"
// Member `goal_id`
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void arm_planning__action__PlanMotion_GetResult_Request__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Request_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  arm_planning__action__PlanMotion_GetResult_Request__init(message_memory);
}

void arm_planning__action__PlanMotion_GetResult_Request__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Request_fini_function(void * message_memory)
{
  arm_planning__action__PlanMotion_GetResult_Request__fini(message_memory);
}

static rosidl_typesupport_introspection_c__MessageMember arm_planning__action__PlanMotion_GetResult_Request__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Request_message_member_array[1] = {
  {
    "goal_id",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_GetResult_Request, goal_id),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers arm_planning__action__PlanMotion_GetResult_Request__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Request_message_members = {
  "arm_planning__action",  // message namespace
  "PlanMotion_GetResult_Request",  // message name
  1,  // number of fields
  sizeof(arm_planning__action__PlanMotion_GetResult_Request),
  arm_planning__action__PlanMotion_GetResult_Request__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Request_message_member_array,  // message members
  arm_planning__action__PlanMotion_GetResult_Request__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Request_init_function,  // function to initialize message memory (memory has to be allocated)
  arm_planning__action__PlanMotion_GetResult_Request__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Request_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t arm_planning__action__PlanMotion_GetResult_Request__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Request_message_type_support_handle = {
  0,
  &arm_planning__action__PlanMotion_GetResult_Request__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Request_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_arm_planning
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_GetResult_Request)() {
  arm_planning__action__PlanMotion_GetResult_Request__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Request_message_member_array[0].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, unique_identifier_msgs, msg, UUID)();
  if (!arm_planning__action__PlanMotion_GetResult_Request__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Request_message_type_support_handle.typesupport_identifier) {
    arm_planning__action__PlanMotion_GetResult_Request__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Request_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &arm_planning__action__PlanMotion_GetResult_Request__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Request_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif

// already included above
// #include <stddef.h>
// already included above
// #include "arm_planning/action/detail/plan_motion__rosidl_typesupport_introspection_c.h"
// already included above
// #include "arm_planning/msg/rosidl_typesupport_introspection_c__visibility_control.h"
// already included above
// #include "rosidl_typesupport_introspection_c/field_types.h"
// already included above
// #include "rosidl_typesupport_introspection_c/identifier.h"
// already included above
// #include "rosidl_typesupport_introspection_c/message_introspection.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__functions.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.h"


// Include directives for member types
// Member `result`
// already included above
// #include "arm_planning/action/plan_motion.h"
// Member `result`
// already included above
// #include "arm_planning/action/detail/plan_motion__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void arm_planning__action__PlanMotion_GetResult_Response__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Response_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  arm_planning__action__PlanMotion_GetResult_Response__init(message_memory);
}

void arm_planning__action__PlanMotion_GetResult_Response__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Response_fini_function(void * message_memory)
{
  arm_planning__action__PlanMotion_GetResult_Response__fini(message_memory);
}

static rosidl_typesupport_introspection_c__MessageMember arm_planning__action__PlanMotion_GetResult_Response__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Response_message_member_array[2] = {
  {
    "status",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_INT8,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_GetResult_Response, status),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "result",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_GetResult_Response, result),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers arm_planning__action__PlanMotion_GetResult_Response__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Response_message_members = {
  "arm_planning__action",  // message namespace
  "PlanMotion_GetResult_Response",  // message name
  2,  // number of fields
  sizeof(arm_planning__action__PlanMotion_GetResult_Response),
  arm_planning__action__PlanMotion_GetResult_Response__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Response_message_member_array,  // message members
  arm_planning__action__PlanMotion_GetResult_Response__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Response_init_function,  // function to initialize message memory (memory has to be allocated)
  arm_planning__action__PlanMotion_GetResult_Response__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Response_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t arm_planning__action__PlanMotion_GetResult_Response__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Response_message_type_support_handle = {
  0,
  &arm_planning__action__PlanMotion_GetResult_Response__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Response_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_arm_planning
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_GetResult_Response)() {
  arm_planning__action__PlanMotion_GetResult_Response__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Response_message_member_array[1].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_Result)();
  if (!arm_planning__action__PlanMotion_GetResult_Response__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Response_message_type_support_handle.typesupport_identifier) {
    arm_planning__action__PlanMotion_GetResult_Response__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Response_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &arm_planning__action__PlanMotion_GetResult_Response__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Response_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif

// already included above
// #include "rosidl_runtime_c/service_type_support_struct.h"
// already included above
// #include "arm_planning/msg/rosidl_typesupport_introspection_c__visibility_control.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__rosidl_typesupport_introspection_c.h"
// already included above
// #include "rosidl_typesupport_introspection_c/identifier.h"
// already included above
// #include "rosidl_typesupport_introspection_c/service_introspection.h"

// this is intentionally not const to allow initialization later to prevent an initialization race
static rosidl_typesupport_introspection_c__ServiceMembers arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_GetResult_service_members = {
  "arm_planning__action",  // service namespace
  "PlanMotion_GetResult",  // service name
  // these two fields are initialized below on the first access
  NULL,  // request message
  // arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Request_message_type_support_handle,
  NULL  // response message
  // arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_GetResult_Response_message_type_support_handle
};

static rosidl_service_type_support_t arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_GetResult_service_type_support_handle = {
  0,
  &arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_GetResult_service_members,
  get_service_typesupport_handle_function,
};

// Forward declaration of request/response type support functions
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_GetResult_Request)();

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_GetResult_Response)();

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_arm_planning
const rosidl_service_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_GetResult)() {
  if (!arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_GetResult_service_type_support_handle.typesupport_identifier) {
    arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_GetResult_service_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  rosidl_typesupport_introspection_c__ServiceMembers * service_members =
    (rosidl_typesupport_introspection_c__ServiceMembers *)arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_GetResult_service_type_support_handle.data;

  if (!service_members->request_members_) {
    service_members->request_members_ =
      (const rosidl_typesupport_introspection_c__MessageMembers *)
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_GetResult_Request)()->data;
  }
  if (!service_members->response_members_) {
    service_members->response_members_ =
      (const rosidl_typesupport_introspection_c__MessageMembers *)
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_GetResult_Response)()->data;
  }

  return &arm_planning__action__detail__plan_motion__rosidl_typesupport_introspection_c__PlanMotion_GetResult_service_type_support_handle;
}

// already included above
// #include <stddef.h>
// already included above
// #include "arm_planning/action/detail/plan_motion__rosidl_typesupport_introspection_c.h"
// already included above
// #include "arm_planning/msg/rosidl_typesupport_introspection_c__visibility_control.h"
// already included above
// #include "rosidl_typesupport_introspection_c/field_types.h"
// already included above
// #include "rosidl_typesupport_introspection_c/identifier.h"
// already included above
// #include "rosidl_typesupport_introspection_c/message_introspection.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__functions.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.h"


// Include directives for member types
// Member `goal_id`
// already included above
// #include "unique_identifier_msgs/msg/uuid.h"
// Member `goal_id`
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__rosidl_typesupport_introspection_c.h"
// Member `feedback`
// already included above
// #include "arm_planning/action/plan_motion.h"
// Member `feedback`
// already included above
// #include "arm_planning/action/detail/plan_motion__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void arm_planning__action__PlanMotion_FeedbackMessage__rosidl_typesupport_introspection_c__PlanMotion_FeedbackMessage_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  arm_planning__action__PlanMotion_FeedbackMessage__init(message_memory);
}

void arm_planning__action__PlanMotion_FeedbackMessage__rosidl_typesupport_introspection_c__PlanMotion_FeedbackMessage_fini_function(void * message_memory)
{
  arm_planning__action__PlanMotion_FeedbackMessage__fini(message_memory);
}

static rosidl_typesupport_introspection_c__MessageMember arm_planning__action__PlanMotion_FeedbackMessage__rosidl_typesupport_introspection_c__PlanMotion_FeedbackMessage_message_member_array[2] = {
  {
    "goal_id",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_FeedbackMessage, goal_id),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "feedback",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(arm_planning__action__PlanMotion_FeedbackMessage, feedback),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers arm_planning__action__PlanMotion_FeedbackMessage__rosidl_typesupport_introspection_c__PlanMotion_FeedbackMessage_message_members = {
  "arm_planning__action",  // message namespace
  "PlanMotion_FeedbackMessage",  // message name
  2,  // number of fields
  sizeof(arm_planning__action__PlanMotion_FeedbackMessage),
  arm_planning__action__PlanMotion_FeedbackMessage__rosidl_typesupport_introspection_c__PlanMotion_FeedbackMessage_message_member_array,  // message members
  arm_planning__action__PlanMotion_FeedbackMessage__rosidl_typesupport_introspection_c__PlanMotion_FeedbackMessage_init_function,  // function to initialize message memory (memory has to be allocated)
  arm_planning__action__PlanMotion_FeedbackMessage__rosidl_typesupport_introspection_c__PlanMotion_FeedbackMessage_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t arm_planning__action__PlanMotion_FeedbackMessage__rosidl_typesupport_introspection_c__PlanMotion_FeedbackMessage_message_type_support_handle = {
  0,
  &arm_planning__action__PlanMotion_FeedbackMessage__rosidl_typesupport_introspection_c__PlanMotion_FeedbackMessage_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_arm_planning
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_FeedbackMessage)() {
  arm_planning__action__PlanMotion_FeedbackMessage__rosidl_typesupport_introspection_c__PlanMotion_FeedbackMessage_message_member_array[0].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, unique_identifier_msgs, msg, UUID)();
  arm_planning__action__PlanMotion_FeedbackMessage__rosidl_typesupport_introspection_c__PlanMotion_FeedbackMessage_message_member_array[1].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, arm_planning, action, PlanMotion_Feedback)();
  if (!arm_planning__action__PlanMotion_FeedbackMessage__rosidl_typesupport_introspection_c__PlanMotion_FeedbackMessage_message_type_support_handle.typesupport_identifier) {
    arm_planning__action__PlanMotion_FeedbackMessage__rosidl_typesupport_introspection_c__PlanMotion_FeedbackMessage_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &arm_planning__action__PlanMotion_FeedbackMessage__rosidl_typesupport_introspection_c__PlanMotion_FeedbackMessage_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
