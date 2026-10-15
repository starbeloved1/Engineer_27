# generated from rosidl_cmake/cmake/rosidl_cmake_aggregate_target-extras.cmake.in

# Create a convenience aggregate target arm_planning::arm_planning
# that links all generated interface targets, so downstream packages can use
# a single modern CMake target name instead of ${arm_planning_TARGETS}.
if(arm_planning_TARGETS AND NOT TARGET arm_planning::arm_planning)
  add_library(arm_planning::arm_planning INTERFACE IMPORTED)
  set_target_properties(arm_planning::arm_planning PROPERTIES
    INTERFACE_LINK_LIBRARIES "${arm_planning_TARGETS}")
endif()
