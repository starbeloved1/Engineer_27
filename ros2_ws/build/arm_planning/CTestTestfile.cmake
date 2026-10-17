# CMake generated Testfile for 
# Source directory: /home/starbeloved/code/Robomaster/Engineer_27/ros2_ws/src/arm_planning
# Build directory: /home/starbeloved/code/Robomaster/Engineer_27/ros2_ws/build/arm_planning
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_planning_math "/usr/bin/python3" "-u" "/opt/ros/humble/share/ament_cmake_test/cmake/run_test.py" "/home/starbeloved/code/Robomaster/Engineer_27/ros2_ws/build/arm_planning/test_results/arm_planning/test_planning_math.gtest.xml" "--package-name" "arm_planning" "--output-file" "/home/starbeloved/code/Robomaster/Engineer_27/ros2_ws/build/arm_planning/ament_cmake_gtest/test_planning_math.txt" "--command" "/home/starbeloved/code/Robomaster/Engineer_27/ros2_ws/build/arm_planning/test_planning_math" "--gtest_output=xml:/home/starbeloved/code/Robomaster/Engineer_27/ros2_ws/build/arm_planning/test_results/arm_planning/test_planning_math.gtest.xml")
set_tests_properties(test_planning_math PROPERTIES  LABELS "gtest" REQUIRED_FILES "/home/starbeloved/code/Robomaster/Engineer_27/ros2_ws/build/arm_planning/test_planning_math" TIMEOUT "60" WORKING_DIRECTORY "/home/starbeloved/code/Robomaster/Engineer_27/ros2_ws/build/arm_planning" _BACKTRACE_TRIPLES "/opt/ros/humble/share/ament_cmake_test/cmake/ament_add_test.cmake;125;add_test;/opt/ros/humble/share/ament_cmake_gtest/cmake/ament_add_gtest_test.cmake;86;ament_add_test;/opt/ros/humble/share/ament_cmake_gtest/cmake/ament_add_gtest.cmake;93;ament_add_gtest_test;/home/starbeloved/code/Robomaster/Engineer_27/ros2_ws/src/arm_planning/CMakeLists.txt;35;ament_add_gtest;/home/starbeloved/code/Robomaster/Engineer_27/ros2_ws/src/arm_planning/CMakeLists.txt;0;")
subdirs("gtest")
