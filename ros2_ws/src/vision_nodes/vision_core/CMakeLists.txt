cmake_minimum_required(VERSION 3.8)
project(vision_core)
cmake_policy(SET CMP0074 NEW)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(PCL REQUIRED COMPONENTS common io filters registration kdtree)
find_package(OpenCV REQUIRED COMPONENTS core)
find_package(Eigen3 REQUIRED CONFIG)

add_library(vision_core src/registration.cpp src/pose_constraints.cpp)
target_compile_features(vision_core PUBLIC cxx_std_17)
target_include_directories(vision_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${PCL_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
ament_target_dependencies(vision_core PUBLIC geometry_msgs rclcpp)
target_link_libraries(vision_core PUBLIC ${PCL_LIBRARIES} ${OpenCV_LIBRARIES} Eigen3::Eigen)
install(TARGETS vision_core EXPORT export_vision_core
  ARCHIVE DESTINATION lib LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)
ament_export_targets(export_vision_core HAS_LIBRARY_TARGET)
ament_export_dependencies(geometry_msgs rclcpp PCL OpenCV Eigen3)
ament_package(CONFIG_EXTRAS cmake/vision_core-extras.cmake)
