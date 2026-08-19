#include "rclcpp/rclcpp.hpp"
#include "Will_cpp_pkg/move_turtle_server_node.hpp"



int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<MoveTurtleServerNode>();
    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(node->get_node_base_interface());
    executor.spin();
    // rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}