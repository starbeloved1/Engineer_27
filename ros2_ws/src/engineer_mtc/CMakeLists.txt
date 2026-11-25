cmake_minimum_required(VERSION 3.8)
project(engineer_mtc)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra -Wpedantic)
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(moveit_msgs REQUIRED)
find_package(moveit_core REQUIRED)
find_package(moveit_task_constructor_msgs REQUIRED)
find_package(moveit_task_constructor_core REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(std_msgs REQUIRED)
find_package(serial_driver REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(urdf REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rosidl_default_generators REQUIRED)
rosidl_generate_interfaces(${PROJECT_NAME} "msg/PickStatus.msg" DEPENDENCIES std_msgs)

add_library(grasp_geometry src/shared/grasp_geometry.cpp)
target_include_directories(grasp_geometry PUBLIC include)
target_link_libraries(grasp_geometry PUBLIC Eigen3::Eigen)
ament_target_dependencies(grasp_geometry PUBLIC geometry_msgs)
add_executable(engineer_mtc_node
  src/main.cpp
  src/node/mtc_node.cpp
  src/shared/vision_target_buffer.cpp
  src/pick/task.cpp
  src/place/task.cpp
  src/shared/motion_stages.cpp
  src/node/task_executor.cpp
  src/pick/stage_1.cpp
  src/pick/stage_2.cpp
  src/pick/stage_3.cpp
  src/place/stage_4.cpp
  src/place/stage_5.cpp
  src/place/stage_6.cpp
  src/shared/energy_scene.cpp
  src/node/preview_markers.cpp
)
target_include_directories(engineer_mtc_node PRIVATE include)
ament_target_dependencies(engineer_mtc_node rclcpp geometry_msgs visualization_msgs sensor_msgs std_srvs moveit_msgs
  moveit_core moveit_task_constructor_msgs moveit_task_constructor_core tf2_ros tf2_eigen std_msgs rclcpp_action
  moveit_ros_planning_interface urdf ament_index_cpp serial_driver)
rosidl_get_typesupport_target(pick_typesupport ${PROJECT_NAME} rosidl_typesupport_cpp)
target_link_libraries(engineer_mtc_node grasp_geometry "${pick_typesupport}")
ament_export_dependencies(rosidl_default_runtime)
install(TARGETS engineer_mtc_node DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY config launch DESTINATION share/${PROJECT_NAME})
if(BUILD_TESTING)
  enable_testing()
  add_executable(vision_target_test test/test_vision_target.cpp src/shared/vision_target_buffer.cpp)
  target_link_libraries(vision_target_test grasp_geometry)
  add_test(NAME vision_target_test COMMAND vision_target_test)
endif()
ament_package()
