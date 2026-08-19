cmake_minimum_required(VERSION 3.8)
project(Will_cpp_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(lifecycle_msgs REQUIRED)
find_package(example_interfaces REQUIRED)
find_package(my_robot_interfaces REQUIRED)
find_package(turtlesim REQUIRED)
find_package(geometry_msgs REQUIRED)

include_directories(include)

add_executable(my_first_node src/my_first_node.cpp)
ament_target_dependencies(my_first_node rclcpp)
add_executable(robot_news_station src/robot_news_station.cpp)
ament_target_dependencies(robot_news_station rclcpp example_interfaces)
add_executable(smartphone src/smartphone.cpp)
ament_target_dependencies(smartphone rclcpp example_interfaces)
add_executable(number_publisher src/number_publisher.cpp)
ament_target_dependencies(number_publisher rclcpp example_interfaces)
add_executable(number_counter src/number_counter.cpp)
ament_target_dependencies(number_counter rclcpp example_interfaces)
add_executable(add_two_ints_server src/add_two_ints_server.cpp)
ament_target_dependencies(add_two_ints_server rclcpp example_interfaces)
add_executable(add_two_ints_client_no_oop src/add_two_ints_client_no_oop.cpp)
ament_target_dependencies(add_two_ints_client_no_oop rclcpp example_interfaces)
add_executable(add_two_ints_client src/add_two_ints_client.cpp)
ament_target_dependencies(add_two_ints_client rclcpp example_interfaces)
add_executable(reset_client src/reset_client.cpp)
ament_target_dependencies(reset_client rclcpp example_interfaces)
add_executable(turtle_spawner src/turtle_spawner.cpp)
ament_target_dependencies(turtle_spawner rclcpp turtlesim my_robot_interfaces)
add_executable(turtle_controller src/turtle_controller.cpp)
ament_target_dependencies(turtle_controller rclcpp turtlesim my_robot_interfaces geometry_msgs)
add_executable(move_turtle_server src/move_turtle_server.cpp)
ament_target_dependencies(move_turtle_server rclcpp rclcpp_action rclcpp_lifecycle lifecycle_msgs turtlesim my_robot_interfaces geometry_msgs )

add_executable(manual_composition src/manual_composition.cpp src/move_turtle_server_node.cpp)
ament_target_dependencies(manual_composition rclcpp rclcpp_action rclcpp_lifecycle lifecycle_msgs turtlesim my_robot_interfaces geometry_msgs)




install(TARGETS
  my_first_node 
  robot_news_station
  smartphone
  number_publisher
  number_counter
  add_two_ints_server
  add_two_ints_client_no_oop
  add_two_ints_client
  reset_client
  turtle_spawner
  turtle_controller
  move_turtle_server
  manual_composition
  DESTINATION lib/${PROJECT_NAME}
)


ament_package()
