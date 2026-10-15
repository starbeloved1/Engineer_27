// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from arm_planning:action/PlanMotion.idl
// generated code does not contain a copyright notice

#ifndef ARM_PLANNING__ACTION__DETAIL__PLAN_MOTION__BUILDER_HPP_
#define ARM_PLANNING__ACTION__DETAIL__PLAN_MOTION__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "arm_planning/action/detail/plan_motion__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace arm_planning
{

namespace action
{

namespace builder
{

class Init_PlanMotion_Goal_planning_time
{
public:
  explicit Init_PlanMotion_Goal_planning_time(::arm_planning::action::PlanMotion_Goal & msg)
  : msg_(msg)
  {}
  ::arm_planning::action::PlanMotion_Goal planning_time(::arm_planning::action::PlanMotion_Goal::_planning_time_type arg)
  {
    msg_.planning_time = std::move(arg);
    return std::move(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Goal msg_;
};

class Init_PlanMotion_Goal_acceleration_scaling
{
public:
  explicit Init_PlanMotion_Goal_acceleration_scaling(::arm_planning::action::PlanMotion_Goal & msg)
  : msg_(msg)
  {}
  Init_PlanMotion_Goal_planning_time acceleration_scaling(::arm_planning::action::PlanMotion_Goal::_acceleration_scaling_type arg)
  {
    msg_.acceleration_scaling = std::move(arg);
    return Init_PlanMotion_Goal_planning_time(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Goal msg_;
};

class Init_PlanMotion_Goal_velocity_scaling
{
public:
  explicit Init_PlanMotion_Goal_velocity_scaling(::arm_planning::action::PlanMotion_Goal & msg)
  : msg_(msg)
  {}
  Init_PlanMotion_Goal_acceleration_scaling velocity_scaling(::arm_planning::action::PlanMotion_Goal::_velocity_scaling_type arg)
  {
    msg_.velocity_scaling = std::move(arg);
    return Init_PlanMotion_Goal_acceleration_scaling(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Goal msg_;
};

class Init_PlanMotion_Goal_execute
{
public:
  explicit Init_PlanMotion_Goal_execute(::arm_planning::action::PlanMotion_Goal & msg)
  : msg_(msg)
  {}
  Init_PlanMotion_Goal_velocity_scaling execute(::arm_planning::action::PlanMotion_Goal::_execute_type arg)
  {
    msg_.execute = std::move(arg);
    return Init_PlanMotion_Goal_velocity_scaling(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Goal msg_;
};

class Init_PlanMotion_Goal_target_pose
{
public:
  explicit Init_PlanMotion_Goal_target_pose(::arm_planning::action::PlanMotion_Goal & msg)
  : msg_(msg)
  {}
  Init_PlanMotion_Goal_execute target_pose(::arm_planning::action::PlanMotion_Goal::_target_pose_type arg)
  {
    msg_.target_pose = std::move(arg);
    return Init_PlanMotion_Goal_execute(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Goal msg_;
};

class Init_PlanMotion_Goal_joint_positions
{
public:
  explicit Init_PlanMotion_Goal_joint_positions(::arm_planning::action::PlanMotion_Goal & msg)
  : msg_(msg)
  {}
  Init_PlanMotion_Goal_target_pose joint_positions(::arm_planning::action::PlanMotion_Goal::_joint_positions_type arg)
  {
    msg_.joint_positions = std::move(arg);
    return Init_PlanMotion_Goal_target_pose(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Goal msg_;
};

class Init_PlanMotion_Goal_target_type
{
public:
  Init_PlanMotion_Goal_target_type()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_PlanMotion_Goal_joint_positions target_type(::arm_planning::action::PlanMotion_Goal::_target_type_type arg)
  {
    msg_.target_type = std::move(arg);
    return Init_PlanMotion_Goal_joint_positions(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Goal msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::arm_planning::action::PlanMotion_Goal>()
{
  return arm_planning::action::builder::Init_PlanMotion_Goal_target_type();
}

}  // namespace arm_planning


namespace arm_planning
{

namespace action
{

namespace builder
{

class Init_PlanMotion_Result_ik_candidates
{
public:
  explicit Init_PlanMotion_Result_ik_candidates(::arm_planning::action::PlanMotion_Result & msg)
  : msg_(msg)
  {}
  ::arm_planning::action::PlanMotion_Result ik_candidates(::arm_planning::action::PlanMotion_Result::_ik_candidates_type arg)
  {
    msg_.ik_candidates = std::move(arg);
    return std::move(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Result msg_;
};

class Init_PlanMotion_Result_validation_samples
{
public:
  explicit Init_PlanMotion_Result_validation_samples(::arm_planning::action::PlanMotion_Result & msg)
  : msg_(msg)
  {}
  Init_PlanMotion_Result_ik_candidates validation_samples(::arm_planning::action::PlanMotion_Result::_validation_samples_type arg)
  {
    msg_.validation_samples = std::move(arg);
    return Init_PlanMotion_Result_ik_candidates(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Result msg_;
};

class Init_PlanMotion_Result_joint_path_length
{
public:
  explicit Init_PlanMotion_Result_joint_path_length(::arm_planning::action::PlanMotion_Result & msg)
  : msg_(msg)
  {}
  Init_PlanMotion_Result_validation_samples joint_path_length(::arm_planning::action::PlanMotion_Result::_joint_path_length_type arg)
  {
    msg_.joint_path_length = std::move(arg);
    return Init_PlanMotion_Result_validation_samples(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Result msg_;
};

class Init_PlanMotion_Result_planning_time
{
public:
  explicit Init_PlanMotion_Result_planning_time(::arm_planning::action::PlanMotion_Result & msg)
  : msg_(msg)
  {}
  Init_PlanMotion_Result_joint_path_length planning_time(::arm_planning::action::PlanMotion_Result::_planning_time_type arg)
  {
    msg_.planning_time = std::move(arg);
    return Init_PlanMotion_Result_joint_path_length(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Result msg_;
};

class Init_PlanMotion_Result_selected_joint_positions
{
public:
  explicit Init_PlanMotion_Result_selected_joint_positions(::arm_planning::action::PlanMotion_Result & msg)
  : msg_(msg)
  {}
  Init_PlanMotion_Result_planning_time selected_joint_positions(::arm_planning::action::PlanMotion_Result::_selected_joint_positions_type arg)
  {
    msg_.selected_joint_positions = std::move(arg);
    return Init_PlanMotion_Result_planning_time(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Result msg_;
};

class Init_PlanMotion_Result_trajectory
{
public:
  explicit Init_PlanMotion_Result_trajectory(::arm_planning::action::PlanMotion_Result & msg)
  : msg_(msg)
  {}
  Init_PlanMotion_Result_selected_joint_positions trajectory(::arm_planning::action::PlanMotion_Result::_trajectory_type arg)
  {
    msg_.trajectory = std::move(arg);
    return Init_PlanMotion_Result_selected_joint_positions(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Result msg_;
};

class Init_PlanMotion_Result_message
{
public:
  explicit Init_PlanMotion_Result_message(::arm_planning::action::PlanMotion_Result & msg)
  : msg_(msg)
  {}
  Init_PlanMotion_Result_trajectory message(::arm_planning::action::PlanMotion_Result::_message_type arg)
  {
    msg_.message = std::move(arg);
    return Init_PlanMotion_Result_trajectory(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Result msg_;
};

class Init_PlanMotion_Result_error_code
{
public:
  explicit Init_PlanMotion_Result_error_code(::arm_planning::action::PlanMotion_Result & msg)
  : msg_(msg)
  {}
  Init_PlanMotion_Result_message error_code(::arm_planning::action::PlanMotion_Result::_error_code_type arg)
  {
    msg_.error_code = std::move(arg);
    return Init_PlanMotion_Result_message(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Result msg_;
};

class Init_PlanMotion_Result_success
{
public:
  Init_PlanMotion_Result_success()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_PlanMotion_Result_error_code success(::arm_planning::action::PlanMotion_Result::_success_type arg)
  {
    msg_.success = std::move(arg);
    return Init_PlanMotion_Result_error_code(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Result msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::arm_planning::action::PlanMotion_Result>()
{
  return arm_planning::action::builder::Init_PlanMotion_Result_success();
}

}  // namespace arm_planning


namespace arm_planning
{

namespace action
{

namespace builder
{

class Init_PlanMotion_Feedback_detail
{
public:
  explicit Init_PlanMotion_Feedback_detail(::arm_planning::action::PlanMotion_Feedback & msg)
  : msg_(msg)
  {}
  ::arm_planning::action::PlanMotion_Feedback detail(::arm_planning::action::PlanMotion_Feedback::_detail_type arg)
  {
    msg_.detail = std::move(arg);
    return std::move(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Feedback msg_;
};

class Init_PlanMotion_Feedback_stage
{
public:
  Init_PlanMotion_Feedback_stage()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_PlanMotion_Feedback_detail stage(::arm_planning::action::PlanMotion_Feedback::_stage_type arg)
  {
    msg_.stage = std::move(arg);
    return Init_PlanMotion_Feedback_detail(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_Feedback msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::arm_planning::action::PlanMotion_Feedback>()
{
  return arm_planning::action::builder::Init_PlanMotion_Feedback_stage();
}

}  // namespace arm_planning


namespace arm_planning
{

namespace action
{

namespace builder
{

class Init_PlanMotion_SendGoal_Request_goal
{
public:
  explicit Init_PlanMotion_SendGoal_Request_goal(::arm_planning::action::PlanMotion_SendGoal_Request & msg)
  : msg_(msg)
  {}
  ::arm_planning::action::PlanMotion_SendGoal_Request goal(::arm_planning::action::PlanMotion_SendGoal_Request::_goal_type arg)
  {
    msg_.goal = std::move(arg);
    return std::move(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_SendGoal_Request msg_;
};

class Init_PlanMotion_SendGoal_Request_goal_id
{
public:
  Init_PlanMotion_SendGoal_Request_goal_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_PlanMotion_SendGoal_Request_goal goal_id(::arm_planning::action::PlanMotion_SendGoal_Request::_goal_id_type arg)
  {
    msg_.goal_id = std::move(arg);
    return Init_PlanMotion_SendGoal_Request_goal(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_SendGoal_Request msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::arm_planning::action::PlanMotion_SendGoal_Request>()
{
  return arm_planning::action::builder::Init_PlanMotion_SendGoal_Request_goal_id();
}

}  // namespace arm_planning


namespace arm_planning
{

namespace action
{

namespace builder
{

class Init_PlanMotion_SendGoal_Response_stamp
{
public:
  explicit Init_PlanMotion_SendGoal_Response_stamp(::arm_planning::action::PlanMotion_SendGoal_Response & msg)
  : msg_(msg)
  {}
  ::arm_planning::action::PlanMotion_SendGoal_Response stamp(::arm_planning::action::PlanMotion_SendGoal_Response::_stamp_type arg)
  {
    msg_.stamp = std::move(arg);
    return std::move(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_SendGoal_Response msg_;
};

class Init_PlanMotion_SendGoal_Response_accepted
{
public:
  Init_PlanMotion_SendGoal_Response_accepted()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_PlanMotion_SendGoal_Response_stamp accepted(::arm_planning::action::PlanMotion_SendGoal_Response::_accepted_type arg)
  {
    msg_.accepted = std::move(arg);
    return Init_PlanMotion_SendGoal_Response_stamp(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_SendGoal_Response msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::arm_planning::action::PlanMotion_SendGoal_Response>()
{
  return arm_planning::action::builder::Init_PlanMotion_SendGoal_Response_accepted();
}

}  // namespace arm_planning


namespace arm_planning
{

namespace action
{

namespace builder
{

class Init_PlanMotion_GetResult_Request_goal_id
{
public:
  Init_PlanMotion_GetResult_Request_goal_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::arm_planning::action::PlanMotion_GetResult_Request goal_id(::arm_planning::action::PlanMotion_GetResult_Request::_goal_id_type arg)
  {
    msg_.goal_id = std::move(arg);
    return std::move(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_GetResult_Request msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::arm_planning::action::PlanMotion_GetResult_Request>()
{
  return arm_planning::action::builder::Init_PlanMotion_GetResult_Request_goal_id();
}

}  // namespace arm_planning


namespace arm_planning
{

namespace action
{

namespace builder
{

class Init_PlanMotion_GetResult_Response_result
{
public:
  explicit Init_PlanMotion_GetResult_Response_result(::arm_planning::action::PlanMotion_GetResult_Response & msg)
  : msg_(msg)
  {}
  ::arm_planning::action::PlanMotion_GetResult_Response result(::arm_planning::action::PlanMotion_GetResult_Response::_result_type arg)
  {
    msg_.result = std::move(arg);
    return std::move(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_GetResult_Response msg_;
};

class Init_PlanMotion_GetResult_Response_status
{
public:
  Init_PlanMotion_GetResult_Response_status()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_PlanMotion_GetResult_Response_result status(::arm_planning::action::PlanMotion_GetResult_Response::_status_type arg)
  {
    msg_.status = std::move(arg);
    return Init_PlanMotion_GetResult_Response_result(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_GetResult_Response msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::arm_planning::action::PlanMotion_GetResult_Response>()
{
  return arm_planning::action::builder::Init_PlanMotion_GetResult_Response_status();
}

}  // namespace arm_planning


namespace arm_planning
{

namespace action
{

namespace builder
{

class Init_PlanMotion_FeedbackMessage_feedback
{
public:
  explicit Init_PlanMotion_FeedbackMessage_feedback(::arm_planning::action::PlanMotion_FeedbackMessage & msg)
  : msg_(msg)
  {}
  ::arm_planning::action::PlanMotion_FeedbackMessage feedback(::arm_planning::action::PlanMotion_FeedbackMessage::_feedback_type arg)
  {
    msg_.feedback = std::move(arg);
    return std::move(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_FeedbackMessage msg_;
};

class Init_PlanMotion_FeedbackMessage_goal_id
{
public:
  Init_PlanMotion_FeedbackMessage_goal_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_PlanMotion_FeedbackMessage_feedback goal_id(::arm_planning::action::PlanMotion_FeedbackMessage::_goal_id_type arg)
  {
    msg_.goal_id = std::move(arg);
    return Init_PlanMotion_FeedbackMessage_feedback(msg_);
  }

private:
  ::arm_planning::action::PlanMotion_FeedbackMessage msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::arm_planning::action::PlanMotion_FeedbackMessage>()
{
  return arm_planning::action::builder::Init_PlanMotion_FeedbackMessage_goal_id();
}

}  // namespace arm_planning

#endif  // ARM_PLANNING__ACTION__DETAIL__PLAN_MOTION__BUILDER_HPP_
