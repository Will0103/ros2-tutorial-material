#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/msg/int64.hpp"
#include "example_interfaces/srv/set_bool.hpp"

using namespace std::placeholders;

class NumberCounterNode : public rclcpp::Node
{
public:
    NumberCounterNode() : Node("number_counter"), count_(0)
    {
        subscriber_ = this->create_subscription<example_interfaces::msg::Int64>
        ("number", 10, std::bind(&NumberCounterNode::callbackNumber, this, std::placeholders::_1));
        publisher_ = this->create_publisher<example_interfaces::msg::Int64>("number_count", 10);
        server_ = this->create_service<example_interfaces::srv::SetBool>("reset_counter", std::bind(&NumberCounterNode::callbackReset, this, _1,_2));
    }
private:
    void callbackNumber(const example_interfaces::msg::Int64::SharedPtr msg)
    {
        count_ += msg->data;
        RCLCPP_INFO(this->get_logger(), "%d", count_);
        auto msg2 = example_interfaces::msg::Int64();
        msg2.data = count_;
        publisher_->publish(msg2);
    }

    void callbackReset(const example_interfaces::srv::SetBool::Request::SharedPtr request,
                       const example_interfaces::srv::SetBool::Response::SharedPtr response)
    {
        if (request->data)
        {
            count_ = 0;
            response->success = true;
            response->message = std::string("reset !");  
        }  
        else
        {
            response->success = false;
            response->message = std::string("what the hell...");  
        }
        RCLCPP_INFO(this->get_logger(), response->message.c_str());
    }

    rclcpp::Publisher<example_interfaces::msg::Int64>::SharedPtr publisher_;
    rclcpp::Subscription<example_interfaces::msg::Int64>::SharedPtr subscriber_;
    rclcpp::Service<example_interfaces::srv::SetBool>::SharedPtr server_;
    int count_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<NumberCounterNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}