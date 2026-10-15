// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from arm_planning:action/PlanMotion.idl
// generated code does not contain a copyright notice
#include "arm_planning/action/detail/plan_motion__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `target_pose`
#include "geometry_msgs/msg/detail/pose_stamped__functions.h"

bool
arm_planning__action__PlanMotion_Goal__init(arm_planning__action__PlanMotion_Goal * msg)
{
  if (!msg) {
    return false;
  }
  // target_type
  // joint_positions
  // target_pose
  if (!geometry_msgs__msg__PoseStamped__init(&msg->target_pose)) {
    arm_planning__action__PlanMotion_Goal__fini(msg);
    return false;
  }
  // execute
  // velocity_scaling
  // acceleration_scaling
  // planning_time
  return true;
}

void
arm_planning__action__PlanMotion_Goal__fini(arm_planning__action__PlanMotion_Goal * msg)
{
  if (!msg) {
    return;
  }
  // target_type
  // joint_positions
  // target_pose
  geometry_msgs__msg__PoseStamped__fini(&msg->target_pose);
  // execute
  // velocity_scaling
  // acceleration_scaling
  // planning_time
}

bool
arm_planning__action__PlanMotion_Goal__are_equal(const arm_planning__action__PlanMotion_Goal * lhs, const arm_planning__action__PlanMotion_Goal * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // target_type
  if (lhs->target_type != rhs->target_type) {
    return false;
  }
  // joint_positions
  for (size_t i = 0; i < 6; ++i) {
    if (lhs->joint_positions[i] != rhs->joint_positions[i]) {
      return false;
    }
  }
  // target_pose
  if (!geometry_msgs__msg__PoseStamped__are_equal(
      &(lhs->target_pose), &(rhs->target_pose)))
  {
    return false;
  }
  // execute
  if (lhs->execute != rhs->execute) {
    return false;
  }
  // velocity_scaling
  if (lhs->velocity_scaling != rhs->velocity_scaling) {
    return false;
  }
  // acceleration_scaling
  if (lhs->acceleration_scaling != rhs->acceleration_scaling) {
    return false;
  }
  // planning_time
  if (lhs->planning_time != rhs->planning_time) {
    return false;
  }
  return true;
}

bool
arm_planning__action__PlanMotion_Goal__copy(
  const arm_planning__action__PlanMotion_Goal * input,
  arm_planning__action__PlanMotion_Goal * output)
{
  if (!input || !output) {
    return false;
  }
  // target_type
  output->target_type = input->target_type;
  // joint_positions
  for (size_t i = 0; i < 6; ++i) {
    output->joint_positions[i] = input->joint_positions[i];
  }
  // target_pose
  if (!geometry_msgs__msg__PoseStamped__copy(
      &(input->target_pose), &(output->target_pose)))
  {
    return false;
  }
  // execute
  output->execute = input->execute;
  // velocity_scaling
  output->velocity_scaling = input->velocity_scaling;
  // acceleration_scaling
  output->acceleration_scaling = input->acceleration_scaling;
  // planning_time
  output->planning_time = input->planning_time;
  return true;
}

arm_planning__action__PlanMotion_Goal *
arm_planning__action__PlanMotion_Goal__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_Goal * msg = (arm_planning__action__PlanMotion_Goal *)allocator.allocate(sizeof(arm_planning__action__PlanMotion_Goal), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(arm_planning__action__PlanMotion_Goal));
  bool success = arm_planning__action__PlanMotion_Goal__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
arm_planning__action__PlanMotion_Goal__destroy(arm_planning__action__PlanMotion_Goal * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    arm_planning__action__PlanMotion_Goal__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
arm_planning__action__PlanMotion_Goal__Sequence__init(arm_planning__action__PlanMotion_Goal__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_Goal * data = NULL;

  if (size) {
    data = (arm_planning__action__PlanMotion_Goal *)allocator.zero_allocate(size, sizeof(arm_planning__action__PlanMotion_Goal), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = arm_planning__action__PlanMotion_Goal__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        arm_planning__action__PlanMotion_Goal__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
arm_planning__action__PlanMotion_Goal__Sequence__fini(arm_planning__action__PlanMotion_Goal__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      arm_planning__action__PlanMotion_Goal__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

arm_planning__action__PlanMotion_Goal__Sequence *
arm_planning__action__PlanMotion_Goal__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_Goal__Sequence * array = (arm_planning__action__PlanMotion_Goal__Sequence *)allocator.allocate(sizeof(arm_planning__action__PlanMotion_Goal__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = arm_planning__action__PlanMotion_Goal__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
arm_planning__action__PlanMotion_Goal__Sequence__destroy(arm_planning__action__PlanMotion_Goal__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    arm_planning__action__PlanMotion_Goal__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
arm_planning__action__PlanMotion_Goal__Sequence__are_equal(const arm_planning__action__PlanMotion_Goal__Sequence * lhs, const arm_planning__action__PlanMotion_Goal__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!arm_planning__action__PlanMotion_Goal__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
arm_planning__action__PlanMotion_Goal__Sequence__copy(
  const arm_planning__action__PlanMotion_Goal__Sequence * input,
  arm_planning__action__PlanMotion_Goal__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(arm_planning__action__PlanMotion_Goal);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    arm_planning__action__PlanMotion_Goal * data =
      (arm_planning__action__PlanMotion_Goal *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!arm_planning__action__PlanMotion_Goal__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          arm_planning__action__PlanMotion_Goal__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!arm_planning__action__PlanMotion_Goal__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}


// Include directives for member types
// Member `message`
#include "rosidl_runtime_c/string_functions.h"
// Member `trajectory`
#include "moveit_msgs/msg/detail/robot_trajectory__functions.h"

bool
arm_planning__action__PlanMotion_Result__init(arm_planning__action__PlanMotion_Result * msg)
{
  if (!msg) {
    return false;
  }
  // success
  // error_code
  // message
  if (!rosidl_runtime_c__String__init(&msg->message)) {
    arm_planning__action__PlanMotion_Result__fini(msg);
    return false;
  }
  // trajectory
  if (!moveit_msgs__msg__RobotTrajectory__init(&msg->trajectory)) {
    arm_planning__action__PlanMotion_Result__fini(msg);
    return false;
  }
  // selected_joint_positions
  // planning_time
  // joint_path_length
  // validation_samples
  // ik_candidates
  return true;
}

void
arm_planning__action__PlanMotion_Result__fini(arm_planning__action__PlanMotion_Result * msg)
{
  if (!msg) {
    return;
  }
  // success
  // error_code
  // message
  rosidl_runtime_c__String__fini(&msg->message);
  // trajectory
  moveit_msgs__msg__RobotTrajectory__fini(&msg->trajectory);
  // selected_joint_positions
  // planning_time
  // joint_path_length
  // validation_samples
  // ik_candidates
}

bool
arm_planning__action__PlanMotion_Result__are_equal(const arm_planning__action__PlanMotion_Result * lhs, const arm_planning__action__PlanMotion_Result * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // success
  if (lhs->success != rhs->success) {
    return false;
  }
  // error_code
  if (lhs->error_code != rhs->error_code) {
    return false;
  }
  // message
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->message), &(rhs->message)))
  {
    return false;
  }
  // trajectory
  if (!moveit_msgs__msg__RobotTrajectory__are_equal(
      &(lhs->trajectory), &(rhs->trajectory)))
  {
    return false;
  }
  // selected_joint_positions
  for (size_t i = 0; i < 6; ++i) {
    if (lhs->selected_joint_positions[i] != rhs->selected_joint_positions[i]) {
      return false;
    }
  }
  // planning_time
  if (lhs->planning_time != rhs->planning_time) {
    return false;
  }
  // joint_path_length
  if (lhs->joint_path_length != rhs->joint_path_length) {
    return false;
  }
  // validation_samples
  if (lhs->validation_samples != rhs->validation_samples) {
    return false;
  }
  // ik_candidates
  if (lhs->ik_candidates != rhs->ik_candidates) {
    return false;
  }
  return true;
}

bool
arm_planning__action__PlanMotion_Result__copy(
  const arm_planning__action__PlanMotion_Result * input,
  arm_planning__action__PlanMotion_Result * output)
{
  if (!input || !output) {
    return false;
  }
  // success
  output->success = input->success;
  // error_code
  output->error_code = input->error_code;
  // message
  if (!rosidl_runtime_c__String__copy(
      &(input->message), &(output->message)))
  {
    return false;
  }
  // trajectory
  if (!moveit_msgs__msg__RobotTrajectory__copy(
      &(input->trajectory), &(output->trajectory)))
  {
    return false;
  }
  // selected_joint_positions
  for (size_t i = 0; i < 6; ++i) {
    output->selected_joint_positions[i] = input->selected_joint_positions[i];
  }
  // planning_time
  output->planning_time = input->planning_time;
  // joint_path_length
  output->joint_path_length = input->joint_path_length;
  // validation_samples
  output->validation_samples = input->validation_samples;
  // ik_candidates
  output->ik_candidates = input->ik_candidates;
  return true;
}

arm_planning__action__PlanMotion_Result *
arm_planning__action__PlanMotion_Result__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_Result * msg = (arm_planning__action__PlanMotion_Result *)allocator.allocate(sizeof(arm_planning__action__PlanMotion_Result), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(arm_planning__action__PlanMotion_Result));
  bool success = arm_planning__action__PlanMotion_Result__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
arm_planning__action__PlanMotion_Result__destroy(arm_planning__action__PlanMotion_Result * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    arm_planning__action__PlanMotion_Result__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
arm_planning__action__PlanMotion_Result__Sequence__init(arm_planning__action__PlanMotion_Result__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_Result * data = NULL;

  if (size) {
    data = (arm_planning__action__PlanMotion_Result *)allocator.zero_allocate(size, sizeof(arm_planning__action__PlanMotion_Result), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = arm_planning__action__PlanMotion_Result__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        arm_planning__action__PlanMotion_Result__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
arm_planning__action__PlanMotion_Result__Sequence__fini(arm_planning__action__PlanMotion_Result__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      arm_planning__action__PlanMotion_Result__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

arm_planning__action__PlanMotion_Result__Sequence *
arm_planning__action__PlanMotion_Result__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_Result__Sequence * array = (arm_planning__action__PlanMotion_Result__Sequence *)allocator.allocate(sizeof(arm_planning__action__PlanMotion_Result__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = arm_planning__action__PlanMotion_Result__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
arm_planning__action__PlanMotion_Result__Sequence__destroy(arm_planning__action__PlanMotion_Result__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    arm_planning__action__PlanMotion_Result__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
arm_planning__action__PlanMotion_Result__Sequence__are_equal(const arm_planning__action__PlanMotion_Result__Sequence * lhs, const arm_planning__action__PlanMotion_Result__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!arm_planning__action__PlanMotion_Result__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
arm_planning__action__PlanMotion_Result__Sequence__copy(
  const arm_planning__action__PlanMotion_Result__Sequence * input,
  arm_planning__action__PlanMotion_Result__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(arm_planning__action__PlanMotion_Result);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    arm_planning__action__PlanMotion_Result * data =
      (arm_planning__action__PlanMotion_Result *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!arm_planning__action__PlanMotion_Result__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          arm_planning__action__PlanMotion_Result__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!arm_planning__action__PlanMotion_Result__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}


// Include directives for member types
// Member `stage`
// Member `detail`
// already included above
// #include "rosidl_runtime_c/string_functions.h"

bool
arm_planning__action__PlanMotion_Feedback__init(arm_planning__action__PlanMotion_Feedback * msg)
{
  if (!msg) {
    return false;
  }
  // stage
  if (!rosidl_runtime_c__String__init(&msg->stage)) {
    arm_planning__action__PlanMotion_Feedback__fini(msg);
    return false;
  }
  // detail
  if (!rosidl_runtime_c__String__init(&msg->detail)) {
    arm_planning__action__PlanMotion_Feedback__fini(msg);
    return false;
  }
  return true;
}

void
arm_planning__action__PlanMotion_Feedback__fini(arm_planning__action__PlanMotion_Feedback * msg)
{
  if (!msg) {
    return;
  }
  // stage
  rosidl_runtime_c__String__fini(&msg->stage);
  // detail
  rosidl_runtime_c__String__fini(&msg->detail);
}

bool
arm_planning__action__PlanMotion_Feedback__are_equal(const arm_planning__action__PlanMotion_Feedback * lhs, const arm_planning__action__PlanMotion_Feedback * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // stage
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->stage), &(rhs->stage)))
  {
    return false;
  }
  // detail
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->detail), &(rhs->detail)))
  {
    return false;
  }
  return true;
}

bool
arm_planning__action__PlanMotion_Feedback__copy(
  const arm_planning__action__PlanMotion_Feedback * input,
  arm_planning__action__PlanMotion_Feedback * output)
{
  if (!input || !output) {
    return false;
  }
  // stage
  if (!rosidl_runtime_c__String__copy(
      &(input->stage), &(output->stage)))
  {
    return false;
  }
  // detail
  if (!rosidl_runtime_c__String__copy(
      &(input->detail), &(output->detail)))
  {
    return false;
  }
  return true;
}

arm_planning__action__PlanMotion_Feedback *
arm_planning__action__PlanMotion_Feedback__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_Feedback * msg = (arm_planning__action__PlanMotion_Feedback *)allocator.allocate(sizeof(arm_planning__action__PlanMotion_Feedback), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(arm_planning__action__PlanMotion_Feedback));
  bool success = arm_planning__action__PlanMotion_Feedback__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
arm_planning__action__PlanMotion_Feedback__destroy(arm_planning__action__PlanMotion_Feedback * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    arm_planning__action__PlanMotion_Feedback__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
arm_planning__action__PlanMotion_Feedback__Sequence__init(arm_planning__action__PlanMotion_Feedback__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_Feedback * data = NULL;

  if (size) {
    data = (arm_planning__action__PlanMotion_Feedback *)allocator.zero_allocate(size, sizeof(arm_planning__action__PlanMotion_Feedback), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = arm_planning__action__PlanMotion_Feedback__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        arm_planning__action__PlanMotion_Feedback__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
arm_planning__action__PlanMotion_Feedback__Sequence__fini(arm_planning__action__PlanMotion_Feedback__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      arm_planning__action__PlanMotion_Feedback__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

arm_planning__action__PlanMotion_Feedback__Sequence *
arm_planning__action__PlanMotion_Feedback__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_Feedback__Sequence * array = (arm_planning__action__PlanMotion_Feedback__Sequence *)allocator.allocate(sizeof(arm_planning__action__PlanMotion_Feedback__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = arm_planning__action__PlanMotion_Feedback__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
arm_planning__action__PlanMotion_Feedback__Sequence__destroy(arm_planning__action__PlanMotion_Feedback__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    arm_planning__action__PlanMotion_Feedback__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
arm_planning__action__PlanMotion_Feedback__Sequence__are_equal(const arm_planning__action__PlanMotion_Feedback__Sequence * lhs, const arm_planning__action__PlanMotion_Feedback__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!arm_planning__action__PlanMotion_Feedback__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
arm_planning__action__PlanMotion_Feedback__Sequence__copy(
  const arm_planning__action__PlanMotion_Feedback__Sequence * input,
  arm_planning__action__PlanMotion_Feedback__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(arm_planning__action__PlanMotion_Feedback);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    arm_planning__action__PlanMotion_Feedback * data =
      (arm_planning__action__PlanMotion_Feedback *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!arm_planning__action__PlanMotion_Feedback__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          arm_planning__action__PlanMotion_Feedback__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!arm_planning__action__PlanMotion_Feedback__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}


// Include directives for member types
// Member `goal_id`
#include "unique_identifier_msgs/msg/detail/uuid__functions.h"
// Member `goal`
// already included above
// #include "arm_planning/action/detail/plan_motion__functions.h"

bool
arm_planning__action__PlanMotion_SendGoal_Request__init(arm_planning__action__PlanMotion_SendGoal_Request * msg)
{
  if (!msg) {
    return false;
  }
  // goal_id
  if (!unique_identifier_msgs__msg__UUID__init(&msg->goal_id)) {
    arm_planning__action__PlanMotion_SendGoal_Request__fini(msg);
    return false;
  }
  // goal
  if (!arm_planning__action__PlanMotion_Goal__init(&msg->goal)) {
    arm_planning__action__PlanMotion_SendGoal_Request__fini(msg);
    return false;
  }
  return true;
}

void
arm_planning__action__PlanMotion_SendGoal_Request__fini(arm_planning__action__PlanMotion_SendGoal_Request * msg)
{
  if (!msg) {
    return;
  }
  // goal_id
  unique_identifier_msgs__msg__UUID__fini(&msg->goal_id);
  // goal
  arm_planning__action__PlanMotion_Goal__fini(&msg->goal);
}

bool
arm_planning__action__PlanMotion_SendGoal_Request__are_equal(const arm_planning__action__PlanMotion_SendGoal_Request * lhs, const arm_planning__action__PlanMotion_SendGoal_Request * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // goal_id
  if (!unique_identifier_msgs__msg__UUID__are_equal(
      &(lhs->goal_id), &(rhs->goal_id)))
  {
    return false;
  }
  // goal
  if (!arm_planning__action__PlanMotion_Goal__are_equal(
      &(lhs->goal), &(rhs->goal)))
  {
    return false;
  }
  return true;
}

bool
arm_planning__action__PlanMotion_SendGoal_Request__copy(
  const arm_planning__action__PlanMotion_SendGoal_Request * input,
  arm_planning__action__PlanMotion_SendGoal_Request * output)
{
  if (!input || !output) {
    return false;
  }
  // goal_id
  if (!unique_identifier_msgs__msg__UUID__copy(
      &(input->goal_id), &(output->goal_id)))
  {
    return false;
  }
  // goal
  if (!arm_planning__action__PlanMotion_Goal__copy(
      &(input->goal), &(output->goal)))
  {
    return false;
  }
  return true;
}

arm_planning__action__PlanMotion_SendGoal_Request *
arm_planning__action__PlanMotion_SendGoal_Request__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_SendGoal_Request * msg = (arm_planning__action__PlanMotion_SendGoal_Request *)allocator.allocate(sizeof(arm_planning__action__PlanMotion_SendGoal_Request), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(arm_planning__action__PlanMotion_SendGoal_Request));
  bool success = arm_planning__action__PlanMotion_SendGoal_Request__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
arm_planning__action__PlanMotion_SendGoal_Request__destroy(arm_planning__action__PlanMotion_SendGoal_Request * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    arm_planning__action__PlanMotion_SendGoal_Request__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
arm_planning__action__PlanMotion_SendGoal_Request__Sequence__init(arm_planning__action__PlanMotion_SendGoal_Request__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_SendGoal_Request * data = NULL;

  if (size) {
    data = (arm_planning__action__PlanMotion_SendGoal_Request *)allocator.zero_allocate(size, sizeof(arm_planning__action__PlanMotion_SendGoal_Request), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = arm_planning__action__PlanMotion_SendGoal_Request__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        arm_planning__action__PlanMotion_SendGoal_Request__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
arm_planning__action__PlanMotion_SendGoal_Request__Sequence__fini(arm_planning__action__PlanMotion_SendGoal_Request__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      arm_planning__action__PlanMotion_SendGoal_Request__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

arm_planning__action__PlanMotion_SendGoal_Request__Sequence *
arm_planning__action__PlanMotion_SendGoal_Request__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_SendGoal_Request__Sequence * array = (arm_planning__action__PlanMotion_SendGoal_Request__Sequence *)allocator.allocate(sizeof(arm_planning__action__PlanMotion_SendGoal_Request__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = arm_planning__action__PlanMotion_SendGoal_Request__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
arm_planning__action__PlanMotion_SendGoal_Request__Sequence__destroy(arm_planning__action__PlanMotion_SendGoal_Request__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    arm_planning__action__PlanMotion_SendGoal_Request__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
arm_planning__action__PlanMotion_SendGoal_Request__Sequence__are_equal(const arm_planning__action__PlanMotion_SendGoal_Request__Sequence * lhs, const arm_planning__action__PlanMotion_SendGoal_Request__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!arm_planning__action__PlanMotion_SendGoal_Request__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
arm_planning__action__PlanMotion_SendGoal_Request__Sequence__copy(
  const arm_planning__action__PlanMotion_SendGoal_Request__Sequence * input,
  arm_planning__action__PlanMotion_SendGoal_Request__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(arm_planning__action__PlanMotion_SendGoal_Request);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    arm_planning__action__PlanMotion_SendGoal_Request * data =
      (arm_planning__action__PlanMotion_SendGoal_Request *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!arm_planning__action__PlanMotion_SendGoal_Request__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          arm_planning__action__PlanMotion_SendGoal_Request__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!arm_planning__action__PlanMotion_SendGoal_Request__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}


// Include directives for member types
// Member `stamp`
#include "builtin_interfaces/msg/detail/time__functions.h"

bool
arm_planning__action__PlanMotion_SendGoal_Response__init(arm_planning__action__PlanMotion_SendGoal_Response * msg)
{
  if (!msg) {
    return false;
  }
  // accepted
  // stamp
  if (!builtin_interfaces__msg__Time__init(&msg->stamp)) {
    arm_planning__action__PlanMotion_SendGoal_Response__fini(msg);
    return false;
  }
  return true;
}

void
arm_planning__action__PlanMotion_SendGoal_Response__fini(arm_planning__action__PlanMotion_SendGoal_Response * msg)
{
  if (!msg) {
    return;
  }
  // accepted
  // stamp
  builtin_interfaces__msg__Time__fini(&msg->stamp);
}

bool
arm_planning__action__PlanMotion_SendGoal_Response__are_equal(const arm_planning__action__PlanMotion_SendGoal_Response * lhs, const arm_planning__action__PlanMotion_SendGoal_Response * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // accepted
  if (lhs->accepted != rhs->accepted) {
    return false;
  }
  // stamp
  if (!builtin_interfaces__msg__Time__are_equal(
      &(lhs->stamp), &(rhs->stamp)))
  {
    return false;
  }
  return true;
}

bool
arm_planning__action__PlanMotion_SendGoal_Response__copy(
  const arm_planning__action__PlanMotion_SendGoal_Response * input,
  arm_planning__action__PlanMotion_SendGoal_Response * output)
{
  if (!input || !output) {
    return false;
  }
  // accepted
  output->accepted = input->accepted;
  // stamp
  if (!builtin_interfaces__msg__Time__copy(
      &(input->stamp), &(output->stamp)))
  {
    return false;
  }
  return true;
}

arm_planning__action__PlanMotion_SendGoal_Response *
arm_planning__action__PlanMotion_SendGoal_Response__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_SendGoal_Response * msg = (arm_planning__action__PlanMotion_SendGoal_Response *)allocator.allocate(sizeof(arm_planning__action__PlanMotion_SendGoal_Response), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(arm_planning__action__PlanMotion_SendGoal_Response));
  bool success = arm_planning__action__PlanMotion_SendGoal_Response__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
arm_planning__action__PlanMotion_SendGoal_Response__destroy(arm_planning__action__PlanMotion_SendGoal_Response * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    arm_planning__action__PlanMotion_SendGoal_Response__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
arm_planning__action__PlanMotion_SendGoal_Response__Sequence__init(arm_planning__action__PlanMotion_SendGoal_Response__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_SendGoal_Response * data = NULL;

  if (size) {
    data = (arm_planning__action__PlanMotion_SendGoal_Response *)allocator.zero_allocate(size, sizeof(arm_planning__action__PlanMotion_SendGoal_Response), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = arm_planning__action__PlanMotion_SendGoal_Response__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        arm_planning__action__PlanMotion_SendGoal_Response__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
arm_planning__action__PlanMotion_SendGoal_Response__Sequence__fini(arm_planning__action__PlanMotion_SendGoal_Response__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      arm_planning__action__PlanMotion_SendGoal_Response__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

arm_planning__action__PlanMotion_SendGoal_Response__Sequence *
arm_planning__action__PlanMotion_SendGoal_Response__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_SendGoal_Response__Sequence * array = (arm_planning__action__PlanMotion_SendGoal_Response__Sequence *)allocator.allocate(sizeof(arm_planning__action__PlanMotion_SendGoal_Response__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = arm_planning__action__PlanMotion_SendGoal_Response__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
arm_planning__action__PlanMotion_SendGoal_Response__Sequence__destroy(arm_planning__action__PlanMotion_SendGoal_Response__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    arm_planning__action__PlanMotion_SendGoal_Response__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
arm_planning__action__PlanMotion_SendGoal_Response__Sequence__are_equal(const arm_planning__action__PlanMotion_SendGoal_Response__Sequence * lhs, const arm_planning__action__PlanMotion_SendGoal_Response__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!arm_planning__action__PlanMotion_SendGoal_Response__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
arm_planning__action__PlanMotion_SendGoal_Response__Sequence__copy(
  const arm_planning__action__PlanMotion_SendGoal_Response__Sequence * input,
  arm_planning__action__PlanMotion_SendGoal_Response__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(arm_planning__action__PlanMotion_SendGoal_Response);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    arm_planning__action__PlanMotion_SendGoal_Response * data =
      (arm_planning__action__PlanMotion_SendGoal_Response *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!arm_planning__action__PlanMotion_SendGoal_Response__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          arm_planning__action__PlanMotion_SendGoal_Response__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!arm_planning__action__PlanMotion_SendGoal_Response__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}


// Include directives for member types
// Member `goal_id`
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__functions.h"

bool
arm_planning__action__PlanMotion_GetResult_Request__init(arm_planning__action__PlanMotion_GetResult_Request * msg)
{
  if (!msg) {
    return false;
  }
  // goal_id
  if (!unique_identifier_msgs__msg__UUID__init(&msg->goal_id)) {
    arm_planning__action__PlanMotion_GetResult_Request__fini(msg);
    return false;
  }
  return true;
}

void
arm_planning__action__PlanMotion_GetResult_Request__fini(arm_planning__action__PlanMotion_GetResult_Request * msg)
{
  if (!msg) {
    return;
  }
  // goal_id
  unique_identifier_msgs__msg__UUID__fini(&msg->goal_id);
}

bool
arm_planning__action__PlanMotion_GetResult_Request__are_equal(const arm_planning__action__PlanMotion_GetResult_Request * lhs, const arm_planning__action__PlanMotion_GetResult_Request * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // goal_id
  if (!unique_identifier_msgs__msg__UUID__are_equal(
      &(lhs->goal_id), &(rhs->goal_id)))
  {
    return false;
  }
  return true;
}

bool
arm_planning__action__PlanMotion_GetResult_Request__copy(
  const arm_planning__action__PlanMotion_GetResult_Request * input,
  arm_planning__action__PlanMotion_GetResult_Request * output)
{
  if (!input || !output) {
    return false;
  }
  // goal_id
  if (!unique_identifier_msgs__msg__UUID__copy(
      &(input->goal_id), &(output->goal_id)))
  {
    return false;
  }
  return true;
}

arm_planning__action__PlanMotion_GetResult_Request *
arm_planning__action__PlanMotion_GetResult_Request__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_GetResult_Request * msg = (arm_planning__action__PlanMotion_GetResult_Request *)allocator.allocate(sizeof(arm_planning__action__PlanMotion_GetResult_Request), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(arm_planning__action__PlanMotion_GetResult_Request));
  bool success = arm_planning__action__PlanMotion_GetResult_Request__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
arm_planning__action__PlanMotion_GetResult_Request__destroy(arm_planning__action__PlanMotion_GetResult_Request * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    arm_planning__action__PlanMotion_GetResult_Request__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
arm_planning__action__PlanMotion_GetResult_Request__Sequence__init(arm_planning__action__PlanMotion_GetResult_Request__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_GetResult_Request * data = NULL;

  if (size) {
    data = (arm_planning__action__PlanMotion_GetResult_Request *)allocator.zero_allocate(size, sizeof(arm_planning__action__PlanMotion_GetResult_Request), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = arm_planning__action__PlanMotion_GetResult_Request__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        arm_planning__action__PlanMotion_GetResult_Request__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
arm_planning__action__PlanMotion_GetResult_Request__Sequence__fini(arm_planning__action__PlanMotion_GetResult_Request__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      arm_planning__action__PlanMotion_GetResult_Request__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

arm_planning__action__PlanMotion_GetResult_Request__Sequence *
arm_planning__action__PlanMotion_GetResult_Request__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_GetResult_Request__Sequence * array = (arm_planning__action__PlanMotion_GetResult_Request__Sequence *)allocator.allocate(sizeof(arm_planning__action__PlanMotion_GetResult_Request__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = arm_planning__action__PlanMotion_GetResult_Request__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
arm_planning__action__PlanMotion_GetResult_Request__Sequence__destroy(arm_planning__action__PlanMotion_GetResult_Request__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    arm_planning__action__PlanMotion_GetResult_Request__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
arm_planning__action__PlanMotion_GetResult_Request__Sequence__are_equal(const arm_planning__action__PlanMotion_GetResult_Request__Sequence * lhs, const arm_planning__action__PlanMotion_GetResult_Request__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!arm_planning__action__PlanMotion_GetResult_Request__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
arm_planning__action__PlanMotion_GetResult_Request__Sequence__copy(
  const arm_planning__action__PlanMotion_GetResult_Request__Sequence * input,
  arm_planning__action__PlanMotion_GetResult_Request__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(arm_planning__action__PlanMotion_GetResult_Request);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    arm_planning__action__PlanMotion_GetResult_Request * data =
      (arm_planning__action__PlanMotion_GetResult_Request *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!arm_planning__action__PlanMotion_GetResult_Request__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          arm_planning__action__PlanMotion_GetResult_Request__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!arm_planning__action__PlanMotion_GetResult_Request__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}


// Include directives for member types
// Member `result`
// already included above
// #include "arm_planning/action/detail/plan_motion__functions.h"

bool
arm_planning__action__PlanMotion_GetResult_Response__init(arm_planning__action__PlanMotion_GetResult_Response * msg)
{
  if (!msg) {
    return false;
  }
  // status
  // result
  if (!arm_planning__action__PlanMotion_Result__init(&msg->result)) {
    arm_planning__action__PlanMotion_GetResult_Response__fini(msg);
    return false;
  }
  return true;
}

void
arm_planning__action__PlanMotion_GetResult_Response__fini(arm_planning__action__PlanMotion_GetResult_Response * msg)
{
  if (!msg) {
    return;
  }
  // status
  // result
  arm_planning__action__PlanMotion_Result__fini(&msg->result);
}

bool
arm_planning__action__PlanMotion_GetResult_Response__are_equal(const arm_planning__action__PlanMotion_GetResult_Response * lhs, const arm_planning__action__PlanMotion_GetResult_Response * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // status
  if (lhs->status != rhs->status) {
    return false;
  }
  // result
  if (!arm_planning__action__PlanMotion_Result__are_equal(
      &(lhs->result), &(rhs->result)))
  {
    return false;
  }
  return true;
}

bool
arm_planning__action__PlanMotion_GetResult_Response__copy(
  const arm_planning__action__PlanMotion_GetResult_Response * input,
  arm_planning__action__PlanMotion_GetResult_Response * output)
{
  if (!input || !output) {
    return false;
  }
  // status
  output->status = input->status;
  // result
  if (!arm_planning__action__PlanMotion_Result__copy(
      &(input->result), &(output->result)))
  {
    return false;
  }
  return true;
}

arm_planning__action__PlanMotion_GetResult_Response *
arm_planning__action__PlanMotion_GetResult_Response__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_GetResult_Response * msg = (arm_planning__action__PlanMotion_GetResult_Response *)allocator.allocate(sizeof(arm_planning__action__PlanMotion_GetResult_Response), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(arm_planning__action__PlanMotion_GetResult_Response));
  bool success = arm_planning__action__PlanMotion_GetResult_Response__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
arm_planning__action__PlanMotion_GetResult_Response__destroy(arm_planning__action__PlanMotion_GetResult_Response * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    arm_planning__action__PlanMotion_GetResult_Response__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
arm_planning__action__PlanMotion_GetResult_Response__Sequence__init(arm_planning__action__PlanMotion_GetResult_Response__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_GetResult_Response * data = NULL;

  if (size) {
    data = (arm_planning__action__PlanMotion_GetResult_Response *)allocator.zero_allocate(size, sizeof(arm_planning__action__PlanMotion_GetResult_Response), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = arm_planning__action__PlanMotion_GetResult_Response__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        arm_planning__action__PlanMotion_GetResult_Response__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
arm_planning__action__PlanMotion_GetResult_Response__Sequence__fini(arm_planning__action__PlanMotion_GetResult_Response__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      arm_planning__action__PlanMotion_GetResult_Response__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

arm_planning__action__PlanMotion_GetResult_Response__Sequence *
arm_planning__action__PlanMotion_GetResult_Response__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_GetResult_Response__Sequence * array = (arm_planning__action__PlanMotion_GetResult_Response__Sequence *)allocator.allocate(sizeof(arm_planning__action__PlanMotion_GetResult_Response__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = arm_planning__action__PlanMotion_GetResult_Response__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
arm_planning__action__PlanMotion_GetResult_Response__Sequence__destroy(arm_planning__action__PlanMotion_GetResult_Response__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    arm_planning__action__PlanMotion_GetResult_Response__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
arm_planning__action__PlanMotion_GetResult_Response__Sequence__are_equal(const arm_planning__action__PlanMotion_GetResult_Response__Sequence * lhs, const arm_planning__action__PlanMotion_GetResult_Response__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!arm_planning__action__PlanMotion_GetResult_Response__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
arm_planning__action__PlanMotion_GetResult_Response__Sequence__copy(
  const arm_planning__action__PlanMotion_GetResult_Response__Sequence * input,
  arm_planning__action__PlanMotion_GetResult_Response__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(arm_planning__action__PlanMotion_GetResult_Response);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    arm_planning__action__PlanMotion_GetResult_Response * data =
      (arm_planning__action__PlanMotion_GetResult_Response *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!arm_planning__action__PlanMotion_GetResult_Response__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          arm_planning__action__PlanMotion_GetResult_Response__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!arm_planning__action__PlanMotion_GetResult_Response__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}


// Include directives for member types
// Member `goal_id`
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__functions.h"
// Member `feedback`
// already included above
// #include "arm_planning/action/detail/plan_motion__functions.h"

bool
arm_planning__action__PlanMotion_FeedbackMessage__init(arm_planning__action__PlanMotion_FeedbackMessage * msg)
{
  if (!msg) {
    return false;
  }
  // goal_id
  if (!unique_identifier_msgs__msg__UUID__init(&msg->goal_id)) {
    arm_planning__action__PlanMotion_FeedbackMessage__fini(msg);
    return false;
  }
  // feedback
  if (!arm_planning__action__PlanMotion_Feedback__init(&msg->feedback)) {
    arm_planning__action__PlanMotion_FeedbackMessage__fini(msg);
    return false;
  }
  return true;
}

void
arm_planning__action__PlanMotion_FeedbackMessage__fini(arm_planning__action__PlanMotion_FeedbackMessage * msg)
{
  if (!msg) {
    return;
  }
  // goal_id
  unique_identifier_msgs__msg__UUID__fini(&msg->goal_id);
  // feedback
  arm_planning__action__PlanMotion_Feedback__fini(&msg->feedback);
}

bool
arm_planning__action__PlanMotion_FeedbackMessage__are_equal(const arm_planning__action__PlanMotion_FeedbackMessage * lhs, const arm_planning__action__PlanMotion_FeedbackMessage * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // goal_id
  if (!unique_identifier_msgs__msg__UUID__are_equal(
      &(lhs->goal_id), &(rhs->goal_id)))
  {
    return false;
  }
  // feedback
  if (!arm_planning__action__PlanMotion_Feedback__are_equal(
      &(lhs->feedback), &(rhs->feedback)))
  {
    return false;
  }
  return true;
}

bool
arm_planning__action__PlanMotion_FeedbackMessage__copy(
  const arm_planning__action__PlanMotion_FeedbackMessage * input,
  arm_planning__action__PlanMotion_FeedbackMessage * output)
{
  if (!input || !output) {
    return false;
  }
  // goal_id
  if (!unique_identifier_msgs__msg__UUID__copy(
      &(input->goal_id), &(output->goal_id)))
  {
    return false;
  }
  // feedback
  if (!arm_planning__action__PlanMotion_Feedback__copy(
      &(input->feedback), &(output->feedback)))
  {
    return false;
  }
  return true;
}

arm_planning__action__PlanMotion_FeedbackMessage *
arm_planning__action__PlanMotion_FeedbackMessage__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_FeedbackMessage * msg = (arm_planning__action__PlanMotion_FeedbackMessage *)allocator.allocate(sizeof(arm_planning__action__PlanMotion_FeedbackMessage), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(arm_planning__action__PlanMotion_FeedbackMessage));
  bool success = arm_planning__action__PlanMotion_FeedbackMessage__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
arm_planning__action__PlanMotion_FeedbackMessage__destroy(arm_planning__action__PlanMotion_FeedbackMessage * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    arm_planning__action__PlanMotion_FeedbackMessage__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
arm_planning__action__PlanMotion_FeedbackMessage__Sequence__init(arm_planning__action__PlanMotion_FeedbackMessage__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_FeedbackMessage * data = NULL;

  if (size) {
    data = (arm_planning__action__PlanMotion_FeedbackMessage *)allocator.zero_allocate(size, sizeof(arm_planning__action__PlanMotion_FeedbackMessage), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = arm_planning__action__PlanMotion_FeedbackMessage__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        arm_planning__action__PlanMotion_FeedbackMessage__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
arm_planning__action__PlanMotion_FeedbackMessage__Sequence__fini(arm_planning__action__PlanMotion_FeedbackMessage__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      arm_planning__action__PlanMotion_FeedbackMessage__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

arm_planning__action__PlanMotion_FeedbackMessage__Sequence *
arm_planning__action__PlanMotion_FeedbackMessage__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  arm_planning__action__PlanMotion_FeedbackMessage__Sequence * array = (arm_planning__action__PlanMotion_FeedbackMessage__Sequence *)allocator.allocate(sizeof(arm_planning__action__PlanMotion_FeedbackMessage__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = arm_planning__action__PlanMotion_FeedbackMessage__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
arm_planning__action__PlanMotion_FeedbackMessage__Sequence__destroy(arm_planning__action__PlanMotion_FeedbackMessage__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    arm_planning__action__PlanMotion_FeedbackMessage__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
arm_planning__action__PlanMotion_FeedbackMessage__Sequence__are_equal(const arm_planning__action__PlanMotion_FeedbackMessage__Sequence * lhs, const arm_planning__action__PlanMotion_FeedbackMessage__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!arm_planning__action__PlanMotion_FeedbackMessage__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
arm_planning__action__PlanMotion_FeedbackMessage__Sequence__copy(
  const arm_planning__action__PlanMotion_FeedbackMessage__Sequence * input,
  arm_planning__action__PlanMotion_FeedbackMessage__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(arm_planning__action__PlanMotion_FeedbackMessage);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    arm_planning__action__PlanMotion_FeedbackMessage * data =
      (arm_planning__action__PlanMotion_FeedbackMessage *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!arm_planning__action__PlanMotion_FeedbackMessage__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          arm_planning__action__PlanMotion_FeedbackMessage__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!arm_planning__action__PlanMotion_FeedbackMessage__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
