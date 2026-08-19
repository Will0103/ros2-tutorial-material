#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/srv/set_bool.hpp"

using namespace std::chrono_literals;

class ResetNode : public rclcpp::Node
{
public:
    ResetNode() : Node("reset")
    {
        client_ = this->create_client<example_interfaces::srv::SetBool>("reset_counter",10);
    }

    void resetCounter(bool a)
    {
        while (!client_->wait_for_service(2s))
        {
            RCLCPP_INFO(this->get_logger(), "Waiting for server...");
        }

        auto request = std::make_shared<example_interfaces::srv::SetBool::Request>();
        request->data = a;

        client_->async_send_request(request, std::bind(&ResetNode::callbackReset, this, std::placeholders::_1));
    }

private:
    void callbackReset(rclcpp::Client<example_interfaces::srv::SetBool>::SharedFuture future)
    {
        auto response = future.get();
        if (response->success)
            RCLCPP_INFO(this->get_logger(), "Number counter is reset !");
        else
            RCLCPP_INFO(this->get_logger(), response->message.c_str());
    }    


    rclcpp::Client<example_interfaces::srv::SetBool>::SharedPtr client_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ResetNode>();
    node->resetCounter(true);
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}