#include "rclcpp/rclcpp.hpp" 
#include "example_interfaces/srv/add_two_ints.hpp"

class AddTwoIntsServer : public rclcpp::Node 
{
    public:
        AddTwoIntsServer() : Node("add_two_ints_server")
        {
            server_ = this->create_service<example_interfaces::srv::AddTwoInts>(
                "add_two_ints", std::bind(&AddTwoIntsServer::callbackAddTwoInts, this, std::placeholders::_1, std::placeholders::_2));
            RCLCPP_INFO(this->get_logger(), "Add Two Ints Server is ready.");
        }
    private:
        void callbackAddTwoInts(const example_interfaces::srv::AddTwoInts::Request::SharedPtr request,
                                      example_interfaces::srv::AddTwoInts::Response::SharedPtr response)
        {
            response->sum = request->a + request->b;
            RCLCPP_INFO(this->get_logger(), "%d + %d = %d", (int)request->a, (int)request->b, (int)response->sum);
        }

        rclcpp::Service<example_interfaces::srv::AddTwoInts>::SharedPtr server_;
};


int main(int argc, char **argv) 
{
    rclcpp::init(argc, argv); 
    auto node = std::make_shared<AddTwoIntsServer>(); 
    rclcpp::spin(node); 
    rclcpp::shutdown(); 
    return 0; 
}
