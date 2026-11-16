cmake_minimum_required(VERSION 3.8)
project(moveit_config)
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
ament_python_install_package(${PROJECT_NAME})
find_package(Python3 REQUIRED COMPONENTS Interpreter)
# 安装资源来自 assets，归档原件不进入运行包
get_filename_component(ENGINEER_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/../../../assets/engineer" ABSOLUTE)
get_filename_component(ENERGY_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/../../../assets/energy" ABSOLUTE)
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
# 单块矿作为独立资源安装，不并入机械臂的关节树。
set(GENERATED_ENERGY "${CMAKE_CURRENT_BINARY_DIR}/energy/energy.urdf")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${ENERGY_ASSETS}/energy.urdf")
execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/prepare_model.py"
          --assets-dir "${ENERGY_ASSETS}" --urdf-name energy.urdf
          --package-prefix energy --output "${GENERATED_ENERGY}"
  RESULT_VARIABLE ENERGY_RESULT)
if(NOT ENERGY_RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to prepare energy model resources")
endif()
install(FILES "${GENERATED_ENERGY}" "${ENERGY_ASSETS}/energy.stl"
        DESTINATION share/${PROJECT_NAME}/energy)
install(DIRECTORY config launch DESTINATION share/${PROJECT_NAME} PATTERN "__pycache__" EXCLUDE)
install(FILES .setup_assistant DESTINATION share/${PROJECT_NAME})
ament_package()
