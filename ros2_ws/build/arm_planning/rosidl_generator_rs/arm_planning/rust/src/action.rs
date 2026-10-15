
#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};



// Corresponds to arm_planning__action__PlanMotion_Goal

// This struct is not documented.
#[allow(missing_docs)]

#[allow(non_camel_case_types)]
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct PlanMotion_Goal {

    // This member is not documented.
    #[allow(missing_docs)]
    pub target_type: u8,


    // This member is not documented.
    #[allow(missing_docs)]
    pub joint_positions: [f64; 6],


    // This member is not documented.
    #[allow(missing_docs)]
    pub target_pose: geometry_msgs::msg::PoseStamped,

    /// execute=false only plans and publishes the RViz preview.
    pub execute: bool,

    /// Zero selects the offline defaults (0.2, 0.2, 5 seconds).
    pub velocity_scaling: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub acceleration_scaling: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub planning_time: f64,

}

impl PlanMotion_Goal {

    // This constant is not documented.
    #[allow(missing_docs)]
    pub const JOINT: u8 = 0;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const POSE: u8 = 1;

}


impl Default for PlanMotion_Goal {
  fn default() -> Self {
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::action::rmw::PlanMotion_Goal::default())
  }
}

impl rosidl_runtime_rs::Message for PlanMotion_Goal {
  type RmwMsg = super::action::rmw::PlanMotion_Goal;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        target_type: msg.target_type,
        joint_positions: msg.joint_positions,
        target_pose: geometry_msgs::msg::PoseStamped::into_rmw_message(std::borrow::Cow::Owned(msg.target_pose)).into_owned(),
        execute: msg.execute,
        velocity_scaling: msg.velocity_scaling,
        acceleration_scaling: msg.acceleration_scaling,
        planning_time: msg.planning_time,
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
      target_type: msg.target_type,
        joint_positions: msg.joint_positions,
        target_pose: geometry_msgs::msg::PoseStamped::into_rmw_message(std::borrow::Cow::Borrowed(&msg.target_pose)).into_owned(),
      execute: msg.execute,
      velocity_scaling: msg.velocity_scaling,
      acceleration_scaling: msg.acceleration_scaling,
      planning_time: msg.planning_time,
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      target_type: msg.target_type,
      joint_positions: msg.joint_positions,
      target_pose: geometry_msgs::msg::PoseStamped::from_rmw_message(msg.target_pose),
      execute: msg.execute,
      velocity_scaling: msg.velocity_scaling,
      acceleration_scaling: msg.acceleration_scaling,
      planning_time: msg.planning_time,
    }
  }
}


// Corresponds to arm_planning__action__PlanMotion_Result

// This struct is not documented.
#[allow(missing_docs)]

#[allow(non_camel_case_types)]
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct PlanMotion_Result {

    // This member is not documented.
    #[allow(missing_docs)]
    pub success: bool,


    // This member is not documented.
    #[allow(missing_docs)]
    pub error_code: i32,


    // This member is not documented.
    #[allow(missing_docs)]
    pub message: std::string::String,


    // This member is not documented.
    #[allow(missing_docs)]
    pub trajectory: moveit_msgs::msg::RobotTrajectory,


    // This member is not documented.
    #[allow(missing_docs)]
    pub selected_joint_positions: [f64; 6],


    // This member is not documented.
    #[allow(missing_docs)]
    pub planning_time: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub joint_path_length: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub validation_samples: u32,


    // This member is not documented.
    #[allow(missing_docs)]
    pub ik_candidates: u32,

}



impl Default for PlanMotion_Result {
  fn default() -> Self {
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::action::rmw::PlanMotion_Result::default())
  }
}

impl rosidl_runtime_rs::Message for PlanMotion_Result {
  type RmwMsg = super::action::rmw::PlanMotion_Result;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        success: msg.success,
        error_code: msg.error_code,
        message: msg.message.as_str().into(),
        trajectory: moveit_msgs::msg::RobotTrajectory::into_rmw_message(std::borrow::Cow::Owned(msg.trajectory)).into_owned(),
        selected_joint_positions: msg.selected_joint_positions,
        planning_time: msg.planning_time,
        joint_path_length: msg.joint_path_length,
        validation_samples: msg.validation_samples,
        ik_candidates: msg.ik_candidates,
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
      success: msg.success,
      error_code: msg.error_code,
        message: msg.message.as_str().into(),
        trajectory: moveit_msgs::msg::RobotTrajectory::into_rmw_message(std::borrow::Cow::Borrowed(&msg.trajectory)).into_owned(),
        selected_joint_positions: msg.selected_joint_positions,
      planning_time: msg.planning_time,
      joint_path_length: msg.joint_path_length,
      validation_samples: msg.validation_samples,
      ik_candidates: msg.ik_candidates,
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      success: msg.success,
      error_code: msg.error_code,
      message: msg.message.to_string(),
      trajectory: moveit_msgs::msg::RobotTrajectory::from_rmw_message(msg.trajectory),
      selected_joint_positions: msg.selected_joint_positions,
      planning_time: msg.planning_time,
      joint_path_length: msg.joint_path_length,
      validation_samples: msg.validation_samples,
      ik_candidates: msg.ik_candidates,
    }
  }
}


// Corresponds to arm_planning__action__PlanMotion_Feedback

// This struct is not documented.
#[allow(missing_docs)]

#[allow(non_camel_case_types)]
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct PlanMotion_Feedback {

    // This member is not documented.
    #[allow(missing_docs)]
    pub stage: std::string::String,


    // This member is not documented.
    #[allow(missing_docs)]
    pub detail: std::string::String,

}



impl Default for PlanMotion_Feedback {
  fn default() -> Self {
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::action::rmw::PlanMotion_Feedback::default())
  }
}

impl rosidl_runtime_rs::Message for PlanMotion_Feedback {
  type RmwMsg = super::action::rmw::PlanMotion_Feedback;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        stage: msg.stage.as_str().into(),
        detail: msg.detail.as_str().into(),
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        stage: msg.stage.as_str().into(),
        detail: msg.detail.as_str().into(),
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      stage: msg.stage.to_string(),
      detail: msg.detail.to_string(),
    }
  }
}


// Corresponds to arm_planning__action__PlanMotion_FeedbackMessage

// This struct is not documented.
#[allow(missing_docs)]

#[allow(non_camel_case_types)]
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct PlanMotion_FeedbackMessage {

    // This member is not documented.
    #[allow(missing_docs)]
    pub goal_id: unique_identifier_msgs::msg::UUID,


    // This member is not documented.
    #[allow(missing_docs)]
    pub feedback: super::action::PlanMotion_Feedback,

}



impl Default for PlanMotion_FeedbackMessage {
  fn default() -> Self {
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::action::rmw::PlanMotion_FeedbackMessage::default())
  }
}

impl rosidl_runtime_rs::Message for PlanMotion_FeedbackMessage {
  type RmwMsg = super::action::rmw::PlanMotion_FeedbackMessage;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        goal_id: unique_identifier_msgs::msg::UUID::into_rmw_message(std::borrow::Cow::Owned(msg.goal_id)).into_owned(),
        feedback: super::action::PlanMotion_Feedback::into_rmw_message(std::borrow::Cow::Owned(msg.feedback)).into_owned(),
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        goal_id: unique_identifier_msgs::msg::UUID::into_rmw_message(std::borrow::Cow::Borrowed(&msg.goal_id)).into_owned(),
        feedback: super::action::PlanMotion_Feedback::into_rmw_message(std::borrow::Cow::Borrowed(&msg.feedback)).into_owned(),
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      goal_id: unique_identifier_msgs::msg::UUID::from_rmw_message(msg.goal_id),
      feedback: super::action::PlanMotion_Feedback::from_rmw_message(msg.feedback),
    }
  }
}






// Corresponds to arm_planning__action__PlanMotion_SendGoal_Request

// This struct is not documented.
#[allow(missing_docs)]

#[allow(non_camel_case_types)]
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct PlanMotion_SendGoal_Request {

    // This member is not documented.
    #[allow(missing_docs)]
    pub goal_id: unique_identifier_msgs::msg::UUID,


    // This member is not documented.
    #[allow(missing_docs)]
    pub goal: super::action::PlanMotion_Goal,

}



impl Default for PlanMotion_SendGoal_Request {
  fn default() -> Self {
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::action::rmw::PlanMotion_SendGoal_Request::default())
  }
}

impl rosidl_runtime_rs::Message for PlanMotion_SendGoal_Request {
  type RmwMsg = super::action::rmw::PlanMotion_SendGoal_Request;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        goal_id: unique_identifier_msgs::msg::UUID::into_rmw_message(std::borrow::Cow::Owned(msg.goal_id)).into_owned(),
        goal: super::action::PlanMotion_Goal::into_rmw_message(std::borrow::Cow::Owned(msg.goal)).into_owned(),
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        goal_id: unique_identifier_msgs::msg::UUID::into_rmw_message(std::borrow::Cow::Borrowed(&msg.goal_id)).into_owned(),
        goal: super::action::PlanMotion_Goal::into_rmw_message(std::borrow::Cow::Borrowed(&msg.goal)).into_owned(),
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      goal_id: unique_identifier_msgs::msg::UUID::from_rmw_message(msg.goal_id),
      goal: super::action::PlanMotion_Goal::from_rmw_message(msg.goal),
    }
  }
}


// Corresponds to arm_planning__action__PlanMotion_SendGoal_Response

// This struct is not documented.
#[allow(missing_docs)]

#[allow(non_camel_case_types)]
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct PlanMotion_SendGoal_Response {

    // This member is not documented.
    #[allow(missing_docs)]
    pub accepted: bool,


    // This member is not documented.
    #[allow(missing_docs)]
    pub stamp: builtin_interfaces::msg::Time,

}



impl Default for PlanMotion_SendGoal_Response {
  fn default() -> Self {
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::action::rmw::PlanMotion_SendGoal_Response::default())
  }
}

impl rosidl_runtime_rs::Message for PlanMotion_SendGoal_Response {
  type RmwMsg = super::action::rmw::PlanMotion_SendGoal_Response;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        accepted: msg.accepted,
        stamp: builtin_interfaces::msg::Time::into_rmw_message(std::borrow::Cow::Owned(msg.stamp)).into_owned(),
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
      accepted: msg.accepted,
        stamp: builtin_interfaces::msg::Time::into_rmw_message(std::borrow::Cow::Borrowed(&msg.stamp)).into_owned(),
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      accepted: msg.accepted,
      stamp: builtin_interfaces::msg::Time::from_rmw_message(msg.stamp),
    }
  }
}


// Corresponds to arm_planning__action__PlanMotion_GetResult_Request

// This struct is not documented.
#[allow(missing_docs)]

#[allow(non_camel_case_types)]
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct PlanMotion_GetResult_Request {

    // This member is not documented.
    #[allow(missing_docs)]
    pub goal_id: unique_identifier_msgs::msg::UUID,

}



impl Default for PlanMotion_GetResult_Request {
  fn default() -> Self {
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::action::rmw::PlanMotion_GetResult_Request::default())
  }
}

impl rosidl_runtime_rs::Message for PlanMotion_GetResult_Request {
  type RmwMsg = super::action::rmw::PlanMotion_GetResult_Request;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        goal_id: unique_identifier_msgs::msg::UUID::into_rmw_message(std::borrow::Cow::Owned(msg.goal_id)).into_owned(),
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        goal_id: unique_identifier_msgs::msg::UUID::into_rmw_message(std::borrow::Cow::Borrowed(&msg.goal_id)).into_owned(),
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      goal_id: unique_identifier_msgs::msg::UUID::from_rmw_message(msg.goal_id),
    }
  }
}


// Corresponds to arm_planning__action__PlanMotion_GetResult_Response

// This struct is not documented.
#[allow(missing_docs)]

#[allow(non_camel_case_types)]
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct PlanMotion_GetResult_Response {

    // This member is not documented.
    #[allow(missing_docs)]
    pub status: i8,


    // This member is not documented.
    #[allow(missing_docs)]
    pub result: super::action::PlanMotion_Result,

}



impl Default for PlanMotion_GetResult_Response {
  fn default() -> Self {
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::action::rmw::PlanMotion_GetResult_Response::default())
  }
}

impl rosidl_runtime_rs::Message for PlanMotion_GetResult_Response {
  type RmwMsg = super::action::rmw::PlanMotion_GetResult_Response;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        status: msg.status,
        result: super::action::PlanMotion_Result::into_rmw_message(std::borrow::Cow::Owned(msg.result)).into_owned(),
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
      status: msg.status,
        result: super::action::PlanMotion_Result::into_rmw_message(std::borrow::Cow::Borrowed(&msg.result)).into_owned(),
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      status: msg.status,
      result: super::action::PlanMotion_Result::from_rmw_message(msg.result),
    }
  }
}






#[link(name = "arm_planning__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_service_type_support_handle__arm_planning__action__PlanMotion_SendGoal() -> *const std::ffi::c_void;
}

// Corresponds to arm_planning__action__PlanMotion_SendGoal
#[allow(missing_docs, non_camel_case_types)]
pub struct PlanMotion_SendGoal;

impl rosidl_runtime_rs::Service for PlanMotion_SendGoal {
    type Request = PlanMotion_SendGoal_Request;
    type Response = PlanMotion_SendGoal_Response;

    fn get_type_support() -> *const std::ffi::c_void {
        // SAFETY: No preconditions for this function.
        unsafe { rosidl_typesupport_c__get_service_type_support_handle__arm_planning__action__PlanMotion_SendGoal() }
    }
}




#[link(name = "arm_planning__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_service_type_support_handle__arm_planning__action__PlanMotion_GetResult() -> *const std::ffi::c_void;
}

// Corresponds to arm_planning__action__PlanMotion_GetResult
#[allow(missing_docs, non_camel_case_types)]
pub struct PlanMotion_GetResult;

impl rosidl_runtime_rs::Service for PlanMotion_GetResult {
    type Request = PlanMotion_GetResult_Request;
    type Response = PlanMotion_GetResult_Response;

    fn get_type_support() -> *const std::ffi::c_void {
        // SAFETY: No preconditions for this function.
        unsafe { rosidl_typesupport_c__get_service_type_support_handle__arm_planning__action__PlanMotion_GetResult() }
    }
}






#[link(name = "arm_planning__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_action_type_support_handle__arm_planning__action__PlanMotion() -> *const std::ffi::c_void;
}

// Corresponds to arm_planning__action__PlanMotion
#[allow(missing_docs, non_camel_case_types)]
pub struct PlanMotion;

impl rosidl_runtime_rs::Action for PlanMotion {
  // --- Associated types for client library users ---
  /// The goal message defined in the action definition.
  type Goal = PlanMotion_Goal;

  /// The result message defined in the action definition.
  type Result = PlanMotion_Result;

  /// The feedback message defined in the action definition.
  type Feedback = PlanMotion_Feedback;

  // --- Associated types for client library implementation ---
  /// The feedback message with generic fields which wraps the feedback message.
  type FeedbackMessage = super::action::PlanMotion_FeedbackMessage;

  /// The send_goal service using a wrapped version of the goal message as a request.
  type SendGoalService = super::action::PlanMotion_SendGoal;

  /// The generic service to cancel a goal.
  type CancelGoalService = action_msgs::srv::rmw::CancelGoal;

  /// The get_result service using a wrapped version of the result message as a response.
  type GetResultService = super::action::PlanMotion_GetResult;

  // --- Methods for client library implementation ---
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_action_type_support_handle__arm_planning__action__PlanMotion() }
  }

  fn create_goal_request(
    goal_id: &[u8; 16],
    goal: super::action::rmw::PlanMotion_Goal,
  ) -> super::action::rmw::PlanMotion_SendGoal_Request {
   super::action::rmw::PlanMotion_SendGoal_Request {
      goal_id: unique_identifier_msgs::msg::rmw::UUID { uuid: *goal_id },
      goal,
    }
  }

  fn split_goal_request(
    request: super::action::rmw::PlanMotion_SendGoal_Request,
  ) -> (
    [u8; 16],
   super::action::rmw::PlanMotion_Goal,
  ) {
    (request.goal_id.uuid, request.goal)
  }

  fn create_goal_response(
    accepted: bool,
    stamp: (i32, u32),
  ) -> super::action::rmw::PlanMotion_SendGoal_Response {
   super::action::rmw::PlanMotion_SendGoal_Response {
      accepted,
      stamp: builtin_interfaces::msg::rmw::Time {
        sec: stamp.0,
        nanosec: stamp.1,
      },
    }
  }

  fn get_goal_response_accepted(
    response: &super::action::rmw::PlanMotion_SendGoal_Response,
  ) -> bool {
    response.accepted
  }

  fn get_goal_response_stamp(
    response: &super::action::rmw::PlanMotion_SendGoal_Response,
  ) -> (i32, u32) {
    (response.stamp.sec, response.stamp.nanosec)
  }

  fn create_feedback_message(
    goal_id: &[u8; 16],
    feedback: super::action::rmw::PlanMotion_Feedback,
  ) -> super::action::rmw::PlanMotion_FeedbackMessage {
    let mut message = super::action::rmw::PlanMotion_FeedbackMessage::default();
    message.goal_id.uuid = *goal_id;
    message.feedback = feedback;
    message
  }

  fn split_feedback_message(
    feedback: super::action::rmw::PlanMotion_FeedbackMessage,
  ) -> (
    [u8; 16],
   super::action::rmw::PlanMotion_Feedback,
  ) {
    (feedback.goal_id.uuid, feedback.feedback)
  }

  fn create_result_request(
    goal_id: &[u8; 16],
  ) -> super::action::rmw::PlanMotion_GetResult_Request {
   super::action::rmw::PlanMotion_GetResult_Request {
      goal_id: unique_identifier_msgs::msg::rmw::UUID { uuid: *goal_id },
    }
  }

  fn get_result_request_uuid(
    request: &super::action::rmw::PlanMotion_GetResult_Request,
  ) -> &[u8; 16] {
    &request.goal_id.uuid
  }

  fn create_result_response(
    status: i8,
    result: super::action::rmw::PlanMotion_Result,
  ) -> super::action::rmw::PlanMotion_GetResult_Response {
   super::action::rmw::PlanMotion_GetResult_Response {
      status,
      result,
    }
  }

  fn split_result_response(
    response: super::action::rmw::PlanMotion_GetResult_Response
  ) -> (
    i8,
   super::action::rmw::PlanMotion_Result,
  ) {
    (response.status, response.result)
  }
}


