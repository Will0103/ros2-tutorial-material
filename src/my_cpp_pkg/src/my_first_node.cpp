#include "rclcpp/rclcpp.hpp" // Include the ROS 2 C++ client library

class MyNode : public rclcpp::Node // Define a new class that inherits from rclcpp::Node
{
    public:
        MyNode() : Node("cpp_test"), counter_(0) //  Constructor that initializes the node with the name "cpp_test"
        {
            RCLCPP_INFO(this->get_logger(), "Hello world"); // Log a message when the node is created
            timer_ = this->create_wall_timer(std::chrono::seconds(1), std::bind(&MyNode::timer_call_back, this)); // Create a timer that calls the timer_call_back function every second
        }
    private:
        void timer_call_back()
        {
                RCLCPP_INFO(this->get_logger(), "YOOOO %d", counter_);
                counter_++;
        }
        rclcpp::TimerBase::SharedPtr timer_; // A shared pointer to a timer object
        int counter_;   //  A counter variable to keep track of the number of times the timer callback has been called
};


int main(int argc, char **argv) // The main function is the entry point of the program
{
    rclcpp::init(argc, argv); // Initialize the ROS 2 client library
    auto node = std::make_shared<MyNode>(); // Create a new node
    rclcpp::spin(node); // Keep the node alive until it is shut down
    rclcpp::shutdown(); // Shutdown the ROS 2 client library

    return 0; // Return success
}
