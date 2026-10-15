
#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};


#[link(name = "arm_planning__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__arm_planning__action__PlanMotion_Goal() -> *const std::ffi::c_void;
}

#[link(name = "arm_planning__rosidl_generator_c")]
extern "C" {
    fn arm_planning__action__PlanMotion_Goal__init(msg: *mut PlanMotion_Goal) -> bool;
    fn arm_planning__action__PlanMotion_Goal__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_Goal>, size: usize) -> bool;
    fn arm_planning__action__PlanMotion_Goal__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_Goal>);
    fn arm_planning__action__PlanMotion_Goal__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<PlanMotion_Goal>, out_seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_Goal>) -> bool;
}

// Corresponds to arm_planning__action__PlanMotion_Goal
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[allow(non_camel_case_types)]
#[repr(C)]
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
    pub target_pose: geometry_msgs::msg::rmw::PoseStamped,

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
    unsafe {
      let mut msg = std::mem::zeroed();
      if !arm_planning__action__PlanMotion_Goal__init(&mut msg as *mut _) {
        panic!("Call to arm_planning__action__PlanMotion_Goal__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for PlanMotion_Goal {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_Goal__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_Goal__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_Goal__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for PlanMotion_Goal {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for PlanMotion_Goal where Self: Sized {
  const TYPE_NAME: &'static str = "arm_planning/action/PlanMotion_Goal";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__arm_planning__action__PlanMotion_Goal() }
  }
}


#[link(name = "arm_planning__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__arm_planning__action__PlanMotion_Result() -> *const std::ffi::c_void;
}

#[link(name = "arm_planning__rosidl_generator_c")]
extern "C" {
    fn arm_planning__action__PlanMotion_Result__init(msg: *mut PlanMotion_Result) -> bool;
    fn arm_planning__action__PlanMotion_Result__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_Result>, size: usize) -> bool;
    fn arm_planning__action__PlanMotion_Result__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_Result>);
    fn arm_planning__action__PlanMotion_Result__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<PlanMotion_Result>, out_seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_Result>) -> bool;
}

// Corresponds to arm_planning__action__PlanMotion_Result
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[allow(non_camel_case_types)]
#[repr(C)]
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
    pub message: rosidl_runtime_rs::String,


    // This member is not documented.
    #[allow(missing_docs)]
    pub trajectory: moveit_msgs::msg::rmw::RobotTrajectory,


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
    unsafe {
      let mut msg = std::mem::zeroed();
      if !arm_planning__action__PlanMotion_Result__init(&mut msg as *mut _) {
        panic!("Call to arm_planning__action__PlanMotion_Result__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for PlanMotion_Result {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_Result__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_Result__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_Result__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for PlanMotion_Result {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for PlanMotion_Result where Self: Sized {
  const TYPE_NAME: &'static str = "arm_planning/action/PlanMotion_Result";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__arm_planning__action__PlanMotion_Result() }
  }
}


#[link(name = "arm_planning__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__arm_planning__action__PlanMotion_Feedback() -> *const std::ffi::c_void;
}

#[link(name = "arm_planning__rosidl_generator_c")]
extern "C" {
    fn arm_planning__action__PlanMotion_Feedback__init(msg: *mut PlanMotion_Feedback) -> bool;
    fn arm_planning__action__PlanMotion_Feedback__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_Feedback>, size: usize) -> bool;
    fn arm_planning__action__PlanMotion_Feedback__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_Feedback>);
    fn arm_planning__action__PlanMotion_Feedback__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<PlanMotion_Feedback>, out_seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_Feedback>) -> bool;
}

// Corresponds to arm_planning__action__PlanMotion_Feedback
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[allow(non_camel_case_types)]
#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct PlanMotion_Feedback {

    // This member is not documented.
    #[allow(missing_docs)]
    pub stage: rosidl_runtime_rs::String,


    // This member is not documented.
    #[allow(missing_docs)]
    pub detail: rosidl_runtime_rs::String,

}



impl Default for PlanMotion_Feedback {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !arm_planning__action__PlanMotion_Feedback__init(&mut msg as *mut _) {
        panic!("Call to arm_planning__action__PlanMotion_Feedback__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for PlanMotion_Feedback {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_Feedback__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_Feedback__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_Feedback__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for PlanMotion_Feedback {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for PlanMotion_Feedback where Self: Sized {
  const TYPE_NAME: &'static str = "arm_planning/action/PlanMotion_Feedback";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__arm_planning__action__PlanMotion_Feedback() }
  }
}


#[link(name = "arm_planning__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__arm_planning__action__PlanMotion_FeedbackMessage() -> *const std::ffi::c_void;
}

#[link(name = "arm_planning__rosidl_generator_c")]
extern "C" {
    fn arm_planning__action__PlanMotion_FeedbackMessage__init(msg: *mut PlanMotion_FeedbackMessage) -> bool;
    fn arm_planning__action__PlanMotion_FeedbackMessage__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_FeedbackMessage>, size: usize) -> bool;
    fn arm_planning__action__PlanMotion_FeedbackMessage__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_FeedbackMessage>);
    fn arm_planning__action__PlanMotion_FeedbackMessage__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<PlanMotion_FeedbackMessage>, out_seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_FeedbackMessage>) -> bool;
}

// Corresponds to arm_planning__action__PlanMotion_FeedbackMessage
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[allow(non_camel_case_types)]
#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct PlanMotion_FeedbackMessage {

    // This member is not documented.
    #[allow(missing_docs)]
    pub goal_id: unique_identifier_msgs::msg::rmw::UUID,


    // This member is not documented.
    #[allow(missing_docs)]
    pub feedback: super::super::action::rmw::PlanMotion_Feedback,

}



impl Default for PlanMotion_FeedbackMessage {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !arm_planning__action__PlanMotion_FeedbackMessage__init(&mut msg as *mut _) {
        panic!("Call to arm_planning__action__PlanMotion_FeedbackMessage__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for PlanMotion_FeedbackMessage {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_FeedbackMessage__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_FeedbackMessage__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_FeedbackMessage__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for PlanMotion_FeedbackMessage {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for PlanMotion_FeedbackMessage where Self: Sized {
  const TYPE_NAME: &'static str = "arm_planning/action/PlanMotion_FeedbackMessage";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__arm_planning__action__PlanMotion_FeedbackMessage() }
  }
}




#[link(name = "arm_planning__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__arm_planning__action__PlanMotion_SendGoal_Request() -> *const std::ffi::c_void;
}

#[link(name = "arm_planning__rosidl_generator_c")]
extern "C" {
    fn arm_planning__action__PlanMotion_SendGoal_Request__init(msg: *mut PlanMotion_SendGoal_Request) -> bool;
    fn arm_planning__action__PlanMotion_SendGoal_Request__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_SendGoal_Request>, size: usize) -> bool;
    fn arm_planning__action__PlanMotion_SendGoal_Request__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_SendGoal_Request>);
    fn arm_planning__action__PlanMotion_SendGoal_Request__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<PlanMotion_SendGoal_Request>, out_seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_SendGoal_Request>) -> bool;
}

// Corresponds to arm_planning__action__PlanMotion_SendGoal_Request
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[allow(non_camel_case_types)]
#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct PlanMotion_SendGoal_Request {

    // This member is not documented.
    #[allow(missing_docs)]
    pub goal_id: unique_identifier_msgs::msg::rmw::UUID,


    // This member is not documented.
    #[allow(missing_docs)]
    pub goal: super::super::action::rmw::PlanMotion_Goal,

}



impl Default for PlanMotion_SendGoal_Request {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !arm_planning__action__PlanMotion_SendGoal_Request__init(&mut msg as *mut _) {
        panic!("Call to arm_planning__action__PlanMotion_SendGoal_Request__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for PlanMotion_SendGoal_Request {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_SendGoal_Request__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_SendGoal_Request__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_SendGoal_Request__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for PlanMotion_SendGoal_Request {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for PlanMotion_SendGoal_Request where Self: Sized {
  const TYPE_NAME: &'static str = "arm_planning/action/PlanMotion_SendGoal_Request";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__arm_planning__action__PlanMotion_SendGoal_Request() }
  }
}


#[link(name = "arm_planning__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__arm_planning__action__PlanMotion_SendGoal_Response() -> *const std::ffi::c_void;
}

#[link(name = "arm_planning__rosidl_generator_c")]
extern "C" {
    fn arm_planning__action__PlanMotion_SendGoal_Response__init(msg: *mut PlanMotion_SendGoal_Response) -> bool;
    fn arm_planning__action__PlanMotion_SendGoal_Response__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_SendGoal_Response>, size: usize) -> bool;
    fn arm_planning__action__PlanMotion_SendGoal_Response__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_SendGoal_Response>);
    fn arm_planning__action__PlanMotion_SendGoal_Response__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<PlanMotion_SendGoal_Response>, out_seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_SendGoal_Response>) -> bool;
}

// Corresponds to arm_planning__action__PlanMotion_SendGoal_Response
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[allow(non_camel_case_types)]
#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct PlanMotion_SendGoal_Response {

    // This member is not documented.
    #[allow(missing_docs)]
    pub accepted: bool,


    // This member is not documented.
    #[allow(missing_docs)]
    pub stamp: builtin_interfaces::msg::rmw::Time,

}



impl Default for PlanMotion_SendGoal_Response {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !arm_planning__action__PlanMotion_SendGoal_Response__init(&mut msg as *mut _) {
        panic!("Call to arm_planning__action__PlanMotion_SendGoal_Response__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for PlanMotion_SendGoal_Response {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_SendGoal_Response__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_SendGoal_Response__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_SendGoal_Response__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for PlanMotion_SendGoal_Response {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for PlanMotion_SendGoal_Response where Self: Sized {
  const TYPE_NAME: &'static str = "arm_planning/action/PlanMotion_SendGoal_Response";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__arm_planning__action__PlanMotion_SendGoal_Response() }
  }
}


#[link(name = "arm_planning__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__arm_planning__action__PlanMotion_GetResult_Request() -> *const std::ffi::c_void;
}

#[link(name = "arm_planning__rosidl_generator_c")]
extern "C" {
    fn arm_planning__action__PlanMotion_GetResult_Request__init(msg: *mut PlanMotion_GetResult_Request) -> bool;
    fn arm_planning__action__PlanMotion_GetResult_Request__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_GetResult_Request>, size: usize) -> bool;
    fn arm_planning__action__PlanMotion_GetResult_Request__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_GetResult_Request>);
    fn arm_planning__action__PlanMotion_GetResult_Request__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<PlanMotion_GetResult_Request>, out_seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_GetResult_Request>) -> bool;
}

// Corresponds to arm_planning__action__PlanMotion_GetResult_Request
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[allow(non_camel_case_types)]
#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct PlanMotion_GetResult_Request {

    // This member is not documented.
    #[allow(missing_docs)]
    pub goal_id: unique_identifier_msgs::msg::rmw::UUID,

}



impl Default for PlanMotion_GetResult_Request {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !arm_planning__action__PlanMotion_GetResult_Request__init(&mut msg as *mut _) {
        panic!("Call to arm_planning__action__PlanMotion_GetResult_Request__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for PlanMotion_GetResult_Request {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_GetResult_Request__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_GetResult_Request__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_GetResult_Request__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for PlanMotion_GetResult_Request {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for PlanMotion_GetResult_Request where Self: Sized {
  const TYPE_NAME: &'static str = "arm_planning/action/PlanMotion_GetResult_Request";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__arm_planning__action__PlanMotion_GetResult_Request() }
  }
}


#[link(name = "arm_planning__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__arm_planning__action__PlanMotion_GetResult_Response() -> *const std::ffi::c_void;
}

#[link(name = "arm_planning__rosidl_generator_c")]
extern "C" {
    fn arm_planning__action__PlanMotion_GetResult_Response__init(msg: *mut PlanMotion_GetResult_Response) -> bool;
    fn arm_planning__action__PlanMotion_GetResult_Response__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_GetResult_Response>, size: usize) -> bool;
    fn arm_planning__action__PlanMotion_GetResult_Response__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_GetResult_Response>);
    fn arm_planning__action__PlanMotion_GetResult_Response__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<PlanMotion_GetResult_Response>, out_seq: *mut rosidl_runtime_rs::Sequence<PlanMotion_GetResult_Response>) -> bool;
}

// Corresponds to arm_planning__action__PlanMotion_GetResult_Response
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[allow(non_camel_case_types)]
#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct PlanMotion_GetResult_Response {

    // This member is not documented.
    #[allow(missing_docs)]
    pub status: i8,


    // This member is not documented.
    #[allow(missing_docs)]
    pub result: super::super::action::rmw::PlanMotion_Result,

}



impl Default for PlanMotion_GetResult_Response {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !arm_planning__action__PlanMotion_GetResult_Response__init(&mut msg as *mut _) {
        panic!("Call to arm_planning__action__PlanMotion_GetResult_Response__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for PlanMotion_GetResult_Response {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_GetResult_Response__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_GetResult_Response__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { arm_planning__action__PlanMotion_GetResult_Response__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for PlanMotion_GetResult_Response {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for PlanMotion_GetResult_Response where Self: Sized {
  const TYPE_NAME: &'static str = "arm_planning/action/PlanMotion_GetResult_Response";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__arm_planning__action__PlanMotion_GetResult_Response() }
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


