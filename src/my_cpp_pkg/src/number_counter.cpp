#include "rclcpp/rclcpp.hpp" 
#include "example_interfaces/msg/int64.hpp"

using namespace std::placeholders;


class NumberCounterNode : public rclcpp::Node 
{
    public:
        NumberCounterNode() : Node("number_counter"), counter_(0), msg_number_(0)
        {
            subscriber_ = this->create_subscription<example_interfaces::msg::Int64>(
                "number", 10, std::bind(&NumberCounterNode::callbackNumber, this, _1));
            RCLCPP_INFO(this->get_logger(), "Number counter has been started!");
            publisher_ = this->create_publisher<example_interfaces::msg::Int64>("number_count", 10);    
            // timer_ = this->create_wall_timer(std::chrono::seconds(1), std::bind(&NumberCounterNode::publishCount, this));
        }

    private:
        void callbackNumber(const example_interfaces::msg::Int64::SharedPtr msg)
        {
            // RCLCPP_INFO(this->get_logger(), "Received number: %ld", msg->data); // Print the received message to the console
            counter_++;
            auto msg_number_ = example_interfaces::msg::Int64();
            msg_number_.data = msg->data * counter_;
            publisher_->publish(msg_number_);         
        }        

        /* void publishCount()
        {
            auto msg_mutilplied = example_interfaces::msg::Int64();
            // counter_++;
            msg_mutilplied.data = msg_number_*counter_;
            publisher_->publish(msg_mutilplied);
        } */

        rclcpp::Subscription<example_interfaces::msg::Int64>::SharedPtr subscriber_;
        rclcpp::Publisher<example_interfaces::msg::Int64>::SharedPtr publisher_;
        // rclcpp::TimerBase::SharedPtr timer_; 
        int counter_;   
        int msg_number_;
};


int main(int argc, char **argv) 
{
    rclcpp::init(argc, argv); 
    auto node = std::make_shared<NumberCounterNode>(); 
    rclcpp::spin(node); 
    rclcpp::shutdown(); 
    return 0; 
}
