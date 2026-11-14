cmake_minimum_required(VERSION 3.8)
project(vision_nodes_pcl)
cmake_policy(SET CMP0074 NEW)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic)

find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(vision_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(message_filters REQUIRED)
find_package(PCL REQUIRED COMPONENTS common io filters registration kdtree)
find_package(OpenCV REQUIRED COMPONENTS core imgproc)
find_package(Eigen3 REQUIRED)

include_directories(include ${PCL_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
add_definitions(${PCL_DEFINITIONS})

# 计算、业务约束和 ROS 收发分别组织；main 只负责启动及错误退出。
add_library(vision_pcl_geometry src/registration.cpp src/pose_constraints.cpp)
ament_target_dependencies(vision_pcl_geometry geometry_msgs)
target_link_libraries(vision_pcl_geometry ${PCL_LIBRARIES} ${OpenCV_LIBRARIES} Eigen3::Eigen)

add_library(pcl_vision_runtime src/icp_registration_node.cpp src/model_cloud_publisher_node.cpp
  src/main_visualizer_node.cpp)
ament_target_dependencies(pcl_vision_runtime rclcpp ament_index_cpp sensor_msgs std_msgs
  geometry_msgs vision_msgs tf2 tf2_ros tf2_eigen pcl_conversions cv_bridge message_filters)
target_link_libraries(pcl_vision_runtime vision_pcl_geometry)

foreach(node icp_registration model_cloud_publisher main_visualizer)
  add_executable(${node} src/${node}_main.cpp)
  ament_target_dependencies(${node} rclcpp sensor_msgs std_msgs geometry_msgs vision_msgs
    tf2_ros cv_bridge message_filters)
  target_link_libraries(${node} pcl_vision_runtime)
  install(TARGETS ${node} DESTINATION lib/${PROJECT_NAME})
endforeach()
install(TARGETS vision_pcl_geometry pcl_vision_runtime DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY config launch DESTINATION share/${PROJECT_NAME})
install(FILES README.md DESTINATION share/${PROJECT_NAME})
# 源资产仍统一在 Engineer_27/assets；安装副本供任意工作目录启动使用。
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../../assets/energy.ply
              ${CMAKE_CURRENT_SOURCE_DIR}/../../../assets/best.pt
  DESTINATION share/${PROJECT_NAME}/assets)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_cmake_test REQUIRED)
  ament_add_gtest(test_registration test/test_registration.cpp)
  target_link_libraries(test_registration vision_pcl_geometry)
  target_compile_definitions(test_registration PRIVATE
    MODEL_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../../assets/energy.ply")
  ament_add_test(test_pipeline
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/test_pipeline.py
    GENERATE_RESULT_FOR_RETURN_CODE_ZERO TIMEOUT 90)
endif()
ament_package()
