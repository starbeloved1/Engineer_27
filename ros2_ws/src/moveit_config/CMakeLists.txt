cmake_minimum_required(VERSION 3.8)
project(moveit_config)
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
ament_python_install_package(${PROJECT_NAME})
find_package(Python3 REQUIRED COMPONENTS Interpreter)
# 安装资源来自 assets，归档原件不进入运行包
get_filename_component(ENGINEER_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/../../../assets" ABSOLUTE)
set(GENERATED_URDF "${CMAKE_CURRENT_BINARY_DIR}/urdf/arm.urdf")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${ENGINEER_ASSETS}/arm.urdf")
execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/prepare_model.py"
          --assets-dir "${ENGINEER_ASSETS}" --output "${GENERATED_URDF}"
  RESULT_VARIABLE MODEL_RESULT)
if(NOT MODEL_RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to prepare model resources from assets")
endif()
install(FILES "${GENERATED_URDF}" DESTINATION share/${PROJECT_NAME}/urdf)
install(DIRECTORY "${ENGINEER_ASSETS}/meshes/visual" "${ENGINEER_ASSETS}/meshes/collision"
        DESTINATION share/${PROJECT_NAME}/meshes FILES_MATCHING PATTERN "*.stl")
install(DIRECTORY config launch DESTINATION share/${PROJECT_NAME} PATTERN "__pycache__" EXCLUDE)
install(FILES .setup_assistant DESTINATION share/${PROJECT_NAME})
ament_package()
