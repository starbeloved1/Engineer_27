// generated from rosidl_typesupport_cpp/resource/idl__type_support.cpp.em
// with input from arm_planning:action/PlanMotion.idl
// generated code does not contain a copyright notice

#include "cstddef"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "arm_planning/action/detail/plan_motion__struct.hpp"
#include "rosidl_typesupport_cpp/identifier.hpp"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_c/type_support_map.h"
#include "rosidl_typesupport_cpp/message_type_support_dispatch.hpp"
#include "rosidl_typesupport_cpp/visibility_control.h"
#include "rosidl_typesupport_interface/macros.h"

namespace arm_planning
{

namespace action
{

namespace rosidl_typesupport_cpp
{

typedef struct _PlanMotion_Goal_type_support_ids_t
{
  const char * typesupport_identifier[2];
} _PlanMotion_Goal_type_support_ids_t;

static const _PlanMotion_Goal_type_support_ids_t _PlanMotion_Goal_message_typesupport_ids = {
  {
    "rosidl_typesupport_fastrtps_cpp",  // ::rosidl_typesupport_fastrtps_cpp::typesupport_identifier,
    "rosidl_typesupport_introspection_cpp",  // ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  }
};

typedef struct _PlanMotion_Goal_type_support_symbol_names_t
{
  const char * symbol_name[2];
} _PlanMotion_Goal_type_support_symbol_names_t;

#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

static const _PlanMotion_Goal_type_support_symbol_names_t _PlanMotion_Goal_message_typesupport_symbol_names = {
  {
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, arm_planning, action, PlanMotion_Goal)),
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, arm_planning, action, PlanMotion_Goal)),
  }
};

typedef struct _PlanMotion_Goal_type_support_data_t
{
  void * data[2];
} _PlanMotion_Goal_type_support_data_t;

static _PlanMotion_Goal_type_support_data_t _PlanMotion_Goal_message_typesupport_data = {
  {
    0,  // will store the shared library later
    0,  // will store the shared library later
  }
};

static const type_support_map_t _PlanMotion_Goal_message_typesupport_map = {
  2,
  "arm_planning",
  &_PlanMotion_Goal_message_typesupport_ids.typesupport_identifier[0],
  &_PlanMotion_Goal_message_typesupport_symbol_names.symbol_name[0],
  &_PlanMotion_Goal_message_typesupport_data.data[0],
};

static const rosidl_message_type_support_t PlanMotion_Goal_message_type_support_handle = {
  ::rosidl_typesupport_cpp::typesupport_identifier,
  reinterpret_cast<const type_support_map_t *>(&_PlanMotion_Goal_message_typesupport_map),
  ::rosidl_typesupport_cpp::get_message_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_cpp

}  // namespace action

}  // namespace arm_planning

namespace rosidl_typesupport_cpp
{

template<>
ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<arm_planning::action::PlanMotion_Goal>()
{
  return &::arm_planning::action::rosidl_typesupport_cpp::PlanMotion_Goal_message_type_support_handle;
}

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_cpp, arm_planning, action, PlanMotion_Goal)() {
  return get_message_type_support_handle<arm_planning::action::PlanMotion_Goal>();
}

#ifdef __cplusplus
}
#endif
}  // namespace rosidl_typesupport_cpp

// already included above
// #include "cstddef"
// already included above
// #include "rosidl_runtime_c/message_type_support_struct.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.hpp"
// already included above
// #include "rosidl_typesupport_cpp/identifier.hpp"
// already included above
// #include "rosidl_typesupport_cpp/message_type_support.hpp"
// already included above
// #include "rosidl_typesupport_c/type_support_map.h"
// already included above
// #include "rosidl_typesupport_cpp/message_type_support_dispatch.hpp"
// already included above
// #include "rosidl_typesupport_cpp/visibility_control.h"
// already included above
// #include "rosidl_typesupport_interface/macros.h"

namespace arm_planning
{

namespace action
{

namespace rosidl_typesupport_cpp
{

typedef struct _PlanMotion_Result_type_support_ids_t
{
  const char * typesupport_identifier[2];
} _PlanMotion_Result_type_support_ids_t;

static const _PlanMotion_Result_type_support_ids_t _PlanMotion_Result_message_typesupport_ids = {
  {
    "rosidl_typesupport_fastrtps_cpp",  // ::rosidl_typesupport_fastrtps_cpp::typesupport_identifier,
    "rosidl_typesupport_introspection_cpp",  // ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  }
};

typedef struct _PlanMotion_Result_type_support_symbol_names_t
{
  const char * symbol_name[2];
} _PlanMotion_Result_type_support_symbol_names_t;

#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

static const _PlanMotion_Result_type_support_symbol_names_t _PlanMotion_Result_message_typesupport_symbol_names = {
  {
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, arm_planning, action, PlanMotion_Result)),
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, arm_planning, action, PlanMotion_Result)),
  }
};

typedef struct _PlanMotion_Result_type_support_data_t
{
  void * data[2];
} _PlanMotion_Result_type_support_data_t;

static _PlanMotion_Result_type_support_data_t _PlanMotion_Result_message_typesupport_data = {
  {
    0,  // will store the shared library later
    0,  // will store the shared library later
  }
};

static const type_support_map_t _PlanMotion_Result_message_typesupport_map = {
  2,
  "arm_planning",
  &_PlanMotion_Result_message_typesupport_ids.typesupport_identifier[0],
  &_PlanMotion_Result_message_typesupport_symbol_names.symbol_name[0],
  &_PlanMotion_Result_message_typesupport_data.data[0],
};

static const rosidl_message_type_support_t PlanMotion_Result_message_type_support_handle = {
  ::rosidl_typesupport_cpp::typesupport_identifier,
  reinterpret_cast<const type_support_map_t *>(&_PlanMotion_Result_message_typesupport_map),
  ::rosidl_typesupport_cpp::get_message_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_cpp

}  // namespace action

}  // namespace arm_planning

namespace rosidl_typesupport_cpp
{

template<>
ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<arm_planning::action::PlanMotion_Result>()
{
  return &::arm_planning::action::rosidl_typesupport_cpp::PlanMotion_Result_message_type_support_handle;
}

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_cpp, arm_planning, action, PlanMotion_Result)() {
  return get_message_type_support_handle<arm_planning::action::PlanMotion_Result>();
}

#ifdef __cplusplus
}
#endif
}  // namespace rosidl_typesupport_cpp

// already included above
// #include "cstddef"
// already included above
// #include "rosidl_runtime_c/message_type_support_struct.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.hpp"
// already included above
// #include "rosidl_typesupport_cpp/identifier.hpp"
// already included above
// #include "rosidl_typesupport_cpp/message_type_support.hpp"
// already included above
// #include "rosidl_typesupport_c/type_support_map.h"
// already included above
// #include "rosidl_typesupport_cpp/message_type_support_dispatch.hpp"
// already included above
// #include "rosidl_typesupport_cpp/visibility_control.h"
// already included above
// #include "rosidl_typesupport_interface/macros.h"

namespace arm_planning
{

namespace action
{

namespace rosidl_typesupport_cpp
{

typedef struct _PlanMotion_Feedback_type_support_ids_t
{
  const char * typesupport_identifier[2];
} _PlanMotion_Feedback_type_support_ids_t;

static const _PlanMotion_Feedback_type_support_ids_t _PlanMotion_Feedback_message_typesupport_ids = {
  {
    "rosidl_typesupport_fastrtps_cpp",  // ::rosidl_typesupport_fastrtps_cpp::typesupport_identifier,
    "rosidl_typesupport_introspection_cpp",  // ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  }
};

typedef struct _PlanMotion_Feedback_type_support_symbol_names_t
{
  const char * symbol_name[2];
} _PlanMotion_Feedback_type_support_symbol_names_t;

#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

static const _PlanMotion_Feedback_type_support_symbol_names_t _PlanMotion_Feedback_message_typesupport_symbol_names = {
  {
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, arm_planning, action, PlanMotion_Feedback)),
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, arm_planning, action, PlanMotion_Feedback)),
  }
};

typedef struct _PlanMotion_Feedback_type_support_data_t
{
  void * data[2];
} _PlanMotion_Feedback_type_support_data_t;

static _PlanMotion_Feedback_type_support_data_t _PlanMotion_Feedback_message_typesupport_data = {
  {
    0,  // will store the shared library later
    0,  // will store the shared library later
  }
};

static const type_support_map_t _PlanMotion_Feedback_message_typesupport_map = {
  2,
  "arm_planning",
  &_PlanMotion_Feedback_message_typesupport_ids.typesupport_identifier[0],
  &_PlanMotion_Feedback_message_typesupport_symbol_names.symbol_name[0],
  &_PlanMotion_Feedback_message_typesupport_data.data[0],
};

static const rosidl_message_type_support_t PlanMotion_Feedback_message_type_support_handle = {
  ::rosidl_typesupport_cpp::typesupport_identifier,
  reinterpret_cast<const type_support_map_t *>(&_PlanMotion_Feedback_message_typesupport_map),
  ::rosidl_typesupport_cpp::get_message_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_cpp

}  // namespace action

}  // namespace arm_planning

namespace rosidl_typesupport_cpp
{

template<>
ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<arm_planning::action::PlanMotion_Feedback>()
{
  return &::arm_planning::action::rosidl_typesupport_cpp::PlanMotion_Feedback_message_type_support_handle;
}

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_cpp, arm_planning, action, PlanMotion_Feedback)() {
  return get_message_type_support_handle<arm_planning::action::PlanMotion_Feedback>();
}

#ifdef __cplusplus
}
#endif
}  // namespace rosidl_typesupport_cpp

// already included above
// #include "cstddef"
// already included above
// #include "rosidl_runtime_c/message_type_support_struct.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.hpp"
// already included above
// #include "rosidl_typesupport_cpp/identifier.hpp"
// already included above
// #include "rosidl_typesupport_cpp/message_type_support.hpp"
// already included above
// #include "rosidl_typesupport_c/type_support_map.h"
// already included above
// #include "rosidl_typesupport_cpp/message_type_support_dispatch.hpp"
// already included above
// #include "rosidl_typesupport_cpp/visibility_control.h"
// already included above
// #include "rosidl_typesupport_interface/macros.h"

namespace arm_planning
{

namespace action
{

namespace rosidl_typesupport_cpp
{

typedef struct _PlanMotion_SendGoal_Request_type_support_ids_t
{
  const char * typesupport_identifier[2];
} _PlanMotion_SendGoal_Request_type_support_ids_t;

static const _PlanMotion_SendGoal_Request_type_support_ids_t _PlanMotion_SendGoal_Request_message_typesupport_ids = {
  {
    "rosidl_typesupport_fastrtps_cpp",  // ::rosidl_typesupport_fastrtps_cpp::typesupport_identifier,
    "rosidl_typesupport_introspection_cpp",  // ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  }
};

typedef struct _PlanMotion_SendGoal_Request_type_support_symbol_names_t
{
  const char * symbol_name[2];
} _PlanMotion_SendGoal_Request_type_support_symbol_names_t;

#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

static const _PlanMotion_SendGoal_Request_type_support_symbol_names_t _PlanMotion_SendGoal_Request_message_typesupport_symbol_names = {
  {
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, arm_planning, action, PlanMotion_SendGoal_Request)),
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, arm_planning, action, PlanMotion_SendGoal_Request)),
  }
};

typedef struct _PlanMotion_SendGoal_Request_type_support_data_t
{
  void * data[2];
} _PlanMotion_SendGoal_Request_type_support_data_t;

static _PlanMotion_SendGoal_Request_type_support_data_t _PlanMotion_SendGoal_Request_message_typesupport_data = {
  {
    0,  // will store the shared library later
    0,  // will store the shared library later
  }
};

static const type_support_map_t _PlanMotion_SendGoal_Request_message_typesupport_map = {
  2,
  "arm_planning",
  &_PlanMotion_SendGoal_Request_message_typesupport_ids.typesupport_identifier[0],
  &_PlanMotion_SendGoal_Request_message_typesupport_symbol_names.symbol_name[0],
  &_PlanMotion_SendGoal_Request_message_typesupport_data.data[0],
};

static const rosidl_message_type_support_t PlanMotion_SendGoal_Request_message_type_support_handle = {
  ::rosidl_typesupport_cpp::typesupport_identifier,
  reinterpret_cast<const type_support_map_t *>(&_PlanMotion_SendGoal_Request_message_typesupport_map),
  ::rosidl_typesupport_cpp::get_message_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_cpp

}  // namespace action

}  // namespace arm_planning

namespace rosidl_typesupport_cpp
{

template<>
ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<arm_planning::action::PlanMotion_SendGoal_Request>()
{
  return &::arm_planning::action::rosidl_typesupport_cpp::PlanMotion_SendGoal_Request_message_type_support_handle;
}

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_cpp, arm_planning, action, PlanMotion_SendGoal_Request)() {
  return get_message_type_support_handle<arm_planning::action::PlanMotion_SendGoal_Request>();
}

#ifdef __cplusplus
}
#endif
}  // namespace rosidl_typesupport_cpp

// already included above
// #include "cstddef"
// already included above
// #include "rosidl_runtime_c/message_type_support_struct.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.hpp"
// already included above
// #include "rosidl_typesupport_cpp/identifier.hpp"
// already included above
// #include "rosidl_typesupport_cpp/message_type_support.hpp"
// already included above
// #include "rosidl_typesupport_c/type_support_map.h"
// already included above
// #include "rosidl_typesupport_cpp/message_type_support_dispatch.hpp"
// already included above
// #include "rosidl_typesupport_cpp/visibility_control.h"
// already included above
// #include "rosidl_typesupport_interface/macros.h"

namespace arm_planning
{

namespace action
{

namespace rosidl_typesupport_cpp
{

typedef struct _PlanMotion_SendGoal_Response_type_support_ids_t
{
  const char * typesupport_identifier[2];
} _PlanMotion_SendGoal_Response_type_support_ids_t;

static const _PlanMotion_SendGoal_Response_type_support_ids_t _PlanMotion_SendGoal_Response_message_typesupport_ids = {
  {
    "rosidl_typesupport_fastrtps_cpp",  // ::rosidl_typesupport_fastrtps_cpp::typesupport_identifier,
    "rosidl_typesupport_introspection_cpp",  // ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  }
};

typedef struct _PlanMotion_SendGoal_Response_type_support_symbol_names_t
{
  const char * symbol_name[2];
} _PlanMotion_SendGoal_Response_type_support_symbol_names_t;

#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

static const _PlanMotion_SendGoal_Response_type_support_symbol_names_t _PlanMotion_SendGoal_Response_message_typesupport_symbol_names = {
  {
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, arm_planning, action, PlanMotion_SendGoal_Response)),
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, arm_planning, action, PlanMotion_SendGoal_Response)),
  }
};

typedef struct _PlanMotion_SendGoal_Response_type_support_data_t
{
  void * data[2];
} _PlanMotion_SendGoal_Response_type_support_data_t;

static _PlanMotion_SendGoal_Response_type_support_data_t _PlanMotion_SendGoal_Response_message_typesupport_data = {
  {
    0,  // will store the shared library later
    0,  // will store the shared library later
  }
};

static const type_support_map_t _PlanMotion_SendGoal_Response_message_typesupport_map = {
  2,
  "arm_planning",
  &_PlanMotion_SendGoal_Response_message_typesupport_ids.typesupport_identifier[0],
  &_PlanMotion_SendGoal_Response_message_typesupport_symbol_names.symbol_name[0],
  &_PlanMotion_SendGoal_Response_message_typesupport_data.data[0],
};

static const rosidl_message_type_support_t PlanMotion_SendGoal_Response_message_type_support_handle = {
  ::rosidl_typesupport_cpp::typesupport_identifier,
  reinterpret_cast<const type_support_map_t *>(&_PlanMotion_SendGoal_Response_message_typesupport_map),
  ::rosidl_typesupport_cpp::get_message_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_cpp

}  // namespace action

}  // namespace arm_planning

namespace rosidl_typesupport_cpp
{

template<>
ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<arm_planning::action::PlanMotion_SendGoal_Response>()
{
  return &::arm_planning::action::rosidl_typesupport_cpp::PlanMotion_SendGoal_Response_message_type_support_handle;
}

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_cpp, arm_planning, action, PlanMotion_SendGoal_Response)() {
  return get_message_type_support_handle<arm_planning::action::PlanMotion_SendGoal_Response>();
}

#ifdef __cplusplus
}
#endif
}  // namespace rosidl_typesupport_cpp

// already included above
// #include "cstddef"
#include "rosidl_runtime_c/service_type_support_struct.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.hpp"
// already included above
// #include "rosidl_typesupport_cpp/identifier.hpp"
#include "rosidl_typesupport_cpp/service_type_support.hpp"
// already included above
// #include "rosidl_typesupport_c/type_support_map.h"
#include "rosidl_typesupport_cpp/service_type_support_dispatch.hpp"
// already included above
// #include "rosidl_typesupport_cpp/visibility_control.h"
// already included above
// #include "rosidl_typesupport_interface/macros.h"

namespace arm_planning
{

namespace action
{

namespace rosidl_typesupport_cpp
{

typedef struct _PlanMotion_SendGoal_type_support_ids_t
{
  const char * typesupport_identifier[2];
} _PlanMotion_SendGoal_type_support_ids_t;

static const _PlanMotion_SendGoal_type_support_ids_t _PlanMotion_SendGoal_service_typesupport_ids = {
  {
    "rosidl_typesupport_fastrtps_cpp",  // ::rosidl_typesupport_fastrtps_cpp::typesupport_identifier,
    "rosidl_typesupport_introspection_cpp",  // ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  }
};

typedef struct _PlanMotion_SendGoal_type_support_symbol_names_t
{
  const char * symbol_name[2];
} _PlanMotion_SendGoal_type_support_symbol_names_t;

#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

static const _PlanMotion_SendGoal_type_support_symbol_names_t _PlanMotion_SendGoal_service_typesupport_symbol_names = {
  {
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, arm_planning, action, PlanMotion_SendGoal)),
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, arm_planning, action, PlanMotion_SendGoal)),
  }
};

typedef struct _PlanMotion_SendGoal_type_support_data_t
{
  void * data[2];
} _PlanMotion_SendGoal_type_support_data_t;

static _PlanMotion_SendGoal_type_support_data_t _PlanMotion_SendGoal_service_typesupport_data = {
  {
    0,  // will store the shared library later
    0,  // will store the shared library later
  }
};

static const type_support_map_t _PlanMotion_SendGoal_service_typesupport_map = {
  2,
  "arm_planning",
  &_PlanMotion_SendGoal_service_typesupport_ids.typesupport_identifier[0],
  &_PlanMotion_SendGoal_service_typesupport_symbol_names.symbol_name[0],
  &_PlanMotion_SendGoal_service_typesupport_data.data[0],
};

static const rosidl_service_type_support_t PlanMotion_SendGoal_service_type_support_handle = {
  ::rosidl_typesupport_cpp::typesupport_identifier,
  reinterpret_cast<const type_support_map_t *>(&_PlanMotion_SendGoal_service_typesupport_map),
  ::rosidl_typesupport_cpp::get_service_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_cpp

}  // namespace action

}  // namespace arm_planning

namespace rosidl_typesupport_cpp
{

template<>
ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_service_type_support_t *
get_service_type_support_handle<arm_planning::action::PlanMotion_SendGoal>()
{
  return &::arm_planning::action::rosidl_typesupport_cpp::PlanMotion_SendGoal_service_type_support_handle;
}

}  // namespace rosidl_typesupport_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_service_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(rosidl_typesupport_cpp, arm_planning, action, PlanMotion_SendGoal)() {
  return ::rosidl_typesupport_cpp::get_service_type_support_handle<arm_planning::action::PlanMotion_SendGoal>();
}

#ifdef __cplusplus
}
#endif

// already included above
// #include "cstddef"
// already included above
// #include "rosidl_runtime_c/message_type_support_struct.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.hpp"
// already included above
// #include "rosidl_typesupport_cpp/identifier.hpp"
// already included above
// #include "rosidl_typesupport_cpp/message_type_support.hpp"
// already included above
// #include "rosidl_typesupport_c/type_support_map.h"
// already included above
// #include "rosidl_typesupport_cpp/message_type_support_dispatch.hpp"
// already included above
// #include "rosidl_typesupport_cpp/visibility_control.h"
// already included above
// #include "rosidl_typesupport_interface/macros.h"

namespace arm_planning
{

namespace action
{

namespace rosidl_typesupport_cpp
{

typedef struct _PlanMotion_GetResult_Request_type_support_ids_t
{
  const char * typesupport_identifier[2];
} _PlanMotion_GetResult_Request_type_support_ids_t;

static const _PlanMotion_GetResult_Request_type_support_ids_t _PlanMotion_GetResult_Request_message_typesupport_ids = {
  {
    "rosidl_typesupport_fastrtps_cpp",  // ::rosidl_typesupport_fastrtps_cpp::typesupport_identifier,
    "rosidl_typesupport_introspection_cpp",  // ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  }
};

typedef struct _PlanMotion_GetResult_Request_type_support_symbol_names_t
{
  const char * symbol_name[2];
} _PlanMotion_GetResult_Request_type_support_symbol_names_t;

#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

static const _PlanMotion_GetResult_Request_type_support_symbol_names_t _PlanMotion_GetResult_Request_message_typesupport_symbol_names = {
  {
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, arm_planning, action, PlanMotion_GetResult_Request)),
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, arm_planning, action, PlanMotion_GetResult_Request)),
  }
};

typedef struct _PlanMotion_GetResult_Request_type_support_data_t
{
  void * data[2];
} _PlanMotion_GetResult_Request_type_support_data_t;

static _PlanMotion_GetResult_Request_type_support_data_t _PlanMotion_GetResult_Request_message_typesupport_data = {
  {
    0,  // will store the shared library later
    0,  // will store the shared library later
  }
};

static const type_support_map_t _PlanMotion_GetResult_Request_message_typesupport_map = {
  2,
  "arm_planning",
  &_PlanMotion_GetResult_Request_message_typesupport_ids.typesupport_identifier[0],
  &_PlanMotion_GetResult_Request_message_typesupport_symbol_names.symbol_name[0],
  &_PlanMotion_GetResult_Request_message_typesupport_data.data[0],
};

static const rosidl_message_type_support_t PlanMotion_GetResult_Request_message_type_support_handle = {
  ::rosidl_typesupport_cpp::typesupport_identifier,
  reinterpret_cast<const type_support_map_t *>(&_PlanMotion_GetResult_Request_message_typesupport_map),
  ::rosidl_typesupport_cpp::get_message_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_cpp

}  // namespace action

}  // namespace arm_planning

namespace rosidl_typesupport_cpp
{

template<>
ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<arm_planning::action::PlanMotion_GetResult_Request>()
{
  return &::arm_planning::action::rosidl_typesupport_cpp::PlanMotion_GetResult_Request_message_type_support_handle;
}

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_cpp, arm_planning, action, PlanMotion_GetResult_Request)() {
  return get_message_type_support_handle<arm_planning::action::PlanMotion_GetResult_Request>();
}

#ifdef __cplusplus
}
#endif
}  // namespace rosidl_typesupport_cpp

// already included above
// #include "cstddef"
// already included above
// #include "rosidl_runtime_c/message_type_support_struct.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.hpp"
// already included above
// #include "rosidl_typesupport_cpp/identifier.hpp"
// already included above
// #include "rosidl_typesupport_cpp/message_type_support.hpp"
// already included above
// #include "rosidl_typesupport_c/type_support_map.h"
// already included above
// #include "rosidl_typesupport_cpp/message_type_support_dispatch.hpp"
// already included above
// #include "rosidl_typesupport_cpp/visibility_control.h"
// already included above
// #include "rosidl_typesupport_interface/macros.h"

namespace arm_planning
{

namespace action
{

namespace rosidl_typesupport_cpp
{

typedef struct _PlanMotion_GetResult_Response_type_support_ids_t
{
  const char * typesupport_identifier[2];
} _PlanMotion_GetResult_Response_type_support_ids_t;

static const _PlanMotion_GetResult_Response_type_support_ids_t _PlanMotion_GetResult_Response_message_typesupport_ids = {
  {
    "rosidl_typesupport_fastrtps_cpp",  // ::rosidl_typesupport_fastrtps_cpp::typesupport_identifier,
    "rosidl_typesupport_introspection_cpp",  // ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  }
};

typedef struct _PlanMotion_GetResult_Response_type_support_symbol_names_t
{
  const char * symbol_name[2];
} _PlanMotion_GetResult_Response_type_support_symbol_names_t;

#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

static const _PlanMotion_GetResult_Response_type_support_symbol_names_t _PlanMotion_GetResult_Response_message_typesupport_symbol_names = {
  {
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, arm_planning, action, PlanMotion_GetResult_Response)),
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, arm_planning, action, PlanMotion_GetResult_Response)),
  }
};

typedef struct _PlanMotion_GetResult_Response_type_support_data_t
{
  void * data[2];
} _PlanMotion_GetResult_Response_type_support_data_t;

static _PlanMotion_GetResult_Response_type_support_data_t _PlanMotion_GetResult_Response_message_typesupport_data = {
  {
    0,  // will store the shared library later
    0,  // will store the shared library later
  }
};

static const type_support_map_t _PlanMotion_GetResult_Response_message_typesupport_map = {
  2,
  "arm_planning",
  &_PlanMotion_GetResult_Response_message_typesupport_ids.typesupport_identifier[0],
  &_PlanMotion_GetResult_Response_message_typesupport_symbol_names.symbol_name[0],
  &_PlanMotion_GetResult_Response_message_typesupport_data.data[0],
};

static const rosidl_message_type_support_t PlanMotion_GetResult_Response_message_type_support_handle = {
  ::rosidl_typesupport_cpp::typesupport_identifier,
  reinterpret_cast<const type_support_map_t *>(&_PlanMotion_GetResult_Response_message_typesupport_map),
  ::rosidl_typesupport_cpp::get_message_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_cpp

}  // namespace action

}  // namespace arm_planning

namespace rosidl_typesupport_cpp
{

template<>
ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<arm_planning::action::PlanMotion_GetResult_Response>()
{
  return &::arm_planning::action::rosidl_typesupport_cpp::PlanMotion_GetResult_Response_message_type_support_handle;
}

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_cpp, arm_planning, action, PlanMotion_GetResult_Response)() {
  return get_message_type_support_handle<arm_planning::action::PlanMotion_GetResult_Response>();
}

#ifdef __cplusplus
}
#endif
}  // namespace rosidl_typesupport_cpp

// already included above
// #include "cstddef"
// already included above
// #include "rosidl_runtime_c/service_type_support_struct.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.hpp"
// already included above
// #include "rosidl_typesupport_cpp/identifier.hpp"
// already included above
// #include "rosidl_typesupport_cpp/service_type_support.hpp"
// already included above
// #include "rosidl_typesupport_c/type_support_map.h"
// already included above
// #include "rosidl_typesupport_cpp/service_type_support_dispatch.hpp"
// already included above
// #include "rosidl_typesupport_cpp/visibility_control.h"
// already included above
// #include "rosidl_typesupport_interface/macros.h"

namespace arm_planning
{

namespace action
{

namespace rosidl_typesupport_cpp
{

typedef struct _PlanMotion_GetResult_type_support_ids_t
{
  const char * typesupport_identifier[2];
} _PlanMotion_GetResult_type_support_ids_t;

static const _PlanMotion_GetResult_type_support_ids_t _PlanMotion_GetResult_service_typesupport_ids = {
  {
    "rosidl_typesupport_fastrtps_cpp",  // ::rosidl_typesupport_fastrtps_cpp::typesupport_identifier,
    "rosidl_typesupport_introspection_cpp",  // ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  }
};

typedef struct _PlanMotion_GetResult_type_support_symbol_names_t
{
  const char * symbol_name[2];
} _PlanMotion_GetResult_type_support_symbol_names_t;

#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

static const _PlanMotion_GetResult_type_support_symbol_names_t _PlanMotion_GetResult_service_typesupport_symbol_names = {
  {
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, arm_planning, action, PlanMotion_GetResult)),
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, arm_planning, action, PlanMotion_GetResult)),
  }
};

typedef struct _PlanMotion_GetResult_type_support_data_t
{
  void * data[2];
} _PlanMotion_GetResult_type_support_data_t;

static _PlanMotion_GetResult_type_support_data_t _PlanMotion_GetResult_service_typesupport_data = {
  {
    0,  // will store the shared library later
    0,  // will store the shared library later
  }
};

static const type_support_map_t _PlanMotion_GetResult_service_typesupport_map = {
  2,
  "arm_planning",
  &_PlanMotion_GetResult_service_typesupport_ids.typesupport_identifier[0],
  &_PlanMotion_GetResult_service_typesupport_symbol_names.symbol_name[0],
  &_PlanMotion_GetResult_service_typesupport_data.data[0],
};

static const rosidl_service_type_support_t PlanMotion_GetResult_service_type_support_handle = {
  ::rosidl_typesupport_cpp::typesupport_identifier,
  reinterpret_cast<const type_support_map_t *>(&_PlanMotion_GetResult_service_typesupport_map),
  ::rosidl_typesupport_cpp::get_service_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_cpp

}  // namespace action

}  // namespace arm_planning

namespace rosidl_typesupport_cpp
{

template<>
ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_service_type_support_t *
get_service_type_support_handle<arm_planning::action::PlanMotion_GetResult>()
{
  return &::arm_planning::action::rosidl_typesupport_cpp::PlanMotion_GetResult_service_type_support_handle;
}

}  // namespace rosidl_typesupport_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_service_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(rosidl_typesupport_cpp, arm_planning, action, PlanMotion_GetResult)() {
  return ::rosidl_typesupport_cpp::get_service_type_support_handle<arm_planning::action::PlanMotion_GetResult>();
}

#ifdef __cplusplus
}
#endif

// already included above
// #include "cstddef"
// already included above
// #include "rosidl_runtime_c/message_type_support_struct.h"
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.hpp"
// already included above
// #include "rosidl_typesupport_cpp/identifier.hpp"
// already included above
// #include "rosidl_typesupport_cpp/message_type_support.hpp"
// already included above
// #include "rosidl_typesupport_c/type_support_map.h"
// already included above
// #include "rosidl_typesupport_cpp/message_type_support_dispatch.hpp"
// already included above
// #include "rosidl_typesupport_cpp/visibility_control.h"
// already included above
// #include "rosidl_typesupport_interface/macros.h"

namespace arm_planning
{

namespace action
{

namespace rosidl_typesupport_cpp
{

typedef struct _PlanMotion_FeedbackMessage_type_support_ids_t
{
  const char * typesupport_identifier[2];
} _PlanMotion_FeedbackMessage_type_support_ids_t;

static const _PlanMotion_FeedbackMessage_type_support_ids_t _PlanMotion_FeedbackMessage_message_typesupport_ids = {
  {
    "rosidl_typesupport_fastrtps_cpp",  // ::rosidl_typesupport_fastrtps_cpp::typesupport_identifier,
    "rosidl_typesupport_introspection_cpp",  // ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  }
};

typedef struct _PlanMotion_FeedbackMessage_type_support_symbol_names_t
{
  const char * symbol_name[2];
} _PlanMotion_FeedbackMessage_type_support_symbol_names_t;

#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

static const _PlanMotion_FeedbackMessage_type_support_symbol_names_t _PlanMotion_FeedbackMessage_message_typesupport_symbol_names = {
  {
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, arm_planning, action, PlanMotion_FeedbackMessage)),
    STRINGIFY(ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, arm_planning, action, PlanMotion_FeedbackMessage)),
  }
};

typedef struct _PlanMotion_FeedbackMessage_type_support_data_t
{
  void * data[2];
} _PlanMotion_FeedbackMessage_type_support_data_t;

static _PlanMotion_FeedbackMessage_type_support_data_t _PlanMotion_FeedbackMessage_message_typesupport_data = {
  {
    0,  // will store the shared library later
    0,  // will store the shared library later
  }
};

static const type_support_map_t _PlanMotion_FeedbackMessage_message_typesupport_map = {
  2,
  "arm_planning",
  &_PlanMotion_FeedbackMessage_message_typesupport_ids.typesupport_identifier[0],
  &_PlanMotion_FeedbackMessage_message_typesupport_symbol_names.symbol_name[0],
  &_PlanMotion_FeedbackMessage_message_typesupport_data.data[0],
};

static const rosidl_message_type_support_t PlanMotion_FeedbackMessage_message_type_support_handle = {
  ::rosidl_typesupport_cpp::typesupport_identifier,
  reinterpret_cast<const type_support_map_t *>(&_PlanMotion_FeedbackMessage_message_typesupport_map),
  ::rosidl_typesupport_cpp::get_message_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_cpp

}  // namespace action

}  // namespace arm_planning

namespace rosidl_typesupport_cpp
{

template<>
ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<arm_planning::action::PlanMotion_FeedbackMessage>()
{
  return &::arm_planning::action::rosidl_typesupport_cpp::PlanMotion_FeedbackMessage_message_type_support_handle;
}

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_cpp, arm_planning, action, PlanMotion_FeedbackMessage)() {
  return get_message_type_support_handle<arm_planning::action::PlanMotion_FeedbackMessage>();
}

#ifdef __cplusplus
}
#endif
}  // namespace rosidl_typesupport_cpp

#include "action_msgs/msg/goal_status_array.hpp"
#include "action_msgs/srv/cancel_goal.hpp"
// already included above
// #include "arm_planning/action/detail/plan_motion__struct.hpp"
// already included above
// #include "rosidl_typesupport_cpp/visibility_control.h"
#include "rosidl_runtime_c/action_type_support_struct.h"
#include "rosidl_typesupport_cpp/action_type_support.hpp"
// already included above
// #include "rosidl_typesupport_cpp/message_type_support.hpp"
// already included above
// #include "rosidl_typesupport_cpp/service_type_support.hpp"

namespace arm_planning
{

namespace action
{

namespace rosidl_typesupport_cpp
{

static rosidl_action_type_support_t PlanMotion_action_type_support_handle = {
  NULL, NULL, NULL, NULL, NULL};

}  // namespace rosidl_typesupport_cpp

}  // namespace action

}  // namespace arm_planning

namespace rosidl_typesupport_cpp
{

template<>
ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_action_type_support_t *
get_action_type_support_handle<arm_planning::action::PlanMotion>()
{
  using ::arm_planning::action::rosidl_typesupport_cpp::PlanMotion_action_type_support_handle;
  // Thread-safe by always writing the same values to the static struct
  PlanMotion_action_type_support_handle.goal_service_type_support = get_service_type_support_handle<::arm_planning::action::PlanMotion::Impl::SendGoalService>();
  PlanMotion_action_type_support_handle.result_service_type_support = get_service_type_support_handle<::arm_planning::action::PlanMotion::Impl::GetResultService>();
  PlanMotion_action_type_support_handle.cancel_service_type_support = get_service_type_support_handle<::arm_planning::action::PlanMotion::Impl::CancelGoalService>();
  PlanMotion_action_type_support_handle.feedback_message_type_support = get_message_type_support_handle<::arm_planning::action::PlanMotion::Impl::FeedbackMessage>();
  PlanMotion_action_type_support_handle.status_message_type_support = get_message_type_support_handle<::arm_planning::action::PlanMotion::Impl::GoalStatusMessage>();
  return &PlanMotion_action_type_support_handle;
}

}  // namespace rosidl_typesupport_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_CPP_PUBLIC
const rosidl_action_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__ACTION_SYMBOL_NAME(rosidl_typesupport_cpp, arm_planning, action, PlanMotion)() {
  return ::rosidl_typesupport_cpp::get_action_type_support_handle<arm_planning::action::PlanMotion>();
}

#ifdef __cplusplus
}
#endif
