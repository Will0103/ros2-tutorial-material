cmake_minimum_required(VERSION 3.8)
project(actions_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(my_robot_interfaces REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(lifecycle_msgs REQUIRED)

add_executable(count_until_server src/count_until_server.cpp)
ament_target_dependencies(count_until_server rclcpp rclcpp_action my_robot_interfaces)
add_executable(count_until_server_queue_goals src/count_until_server_queue_goals.cpp)
ament_target_dependencies(count_until_server_queue_goals rclcpp rclcpp_action my_robot_interfaces)
add_executable(count_until_client src/count_until_client.cpp)
ament_target_dependencies(count_until_client rclcpp rclcpp_action my_robot_interfaces)
add_executable(robot_move_server src/robot_move_server.cpp)
ament_target_dependencies(robot_move_server rclcpp rclcpp_action my_robot_interfaces)
add_executable(robot_move_client src/robot_move_client.cpp)
ament_target_dependencies(robot_move_client rclcpp rclcpp_action my_robot_interfaces)
add_executable(robot_move_server_lifecycle src/robot_move_server_lifecycle.cpp)
ament_target_dependencies(robot_move_server_lifecycle rclcpp rclcpp_action rclcpp_lifecycle my_robot_interfaces lifecycle_msgs)




install(TARGETS
  count_until_server
  count_until_server_queue_goals
  count_until_client
  robot_move_server
  robot_move_client
  robot_move_server_lifecycle
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
