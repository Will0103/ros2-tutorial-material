#include "rclcpp/rclcpp.hpp"
#include "turtlesim/srv/spawn.hpp"
#include "turtlesim/srv/kill.hpp"
#include <random>
#include "my_robot_interfaces/msg/turtle2.hpp"
#include "my_robot_interfaces/msg/turtle2_array.hpp"
#include "my_robot_interfaces/srv/catch_turtle.hpp"

using namespace std::chrono;
using namespace std::placeholders;

// struct Turtle
// {
//     float x;
//     float y;
//     std::string name;
// };

class TurtleSpawnerNode : public rclcpp::Node
{
public:
    TurtleSpawnerNode() : Node("turtle_spawner"), counter_(0)
    {
        this->declare_parameter("prefix", "frog");
        prefix_ = this->get_parameter("prefix").as_string();
        this->declare_parameter("spawn_rate", 2);
        int spawn_frequency = this->get_parameter("spawn_rate").as_int();
        spawn_ = this->create_client<turtlesim::srv::Spawn>("spawn", 10);
        kill_ = this->create_client<turtlesim::srv::Kill>("kill",10);
        timer_ = this->create_wall_timer(seconds(spawn_frequency), std::bind(&TurtleSpawnerNode::spwanTurtle, this));
        turtle_pub_ = this->create_publisher<my_robot_interfaces::msg::Turtle2Array>("alive_turtle", 10);
        catch_it_ = this->create_service<my_robot_interfaces::srv::CatchTurtle>("catch_turtle", std::bind(&TurtleSpawnerNode::killTurtle, this, _1, _2));

    }
private:
    void spwanTurtle()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist_xy(0.0, 11.0);
        std::uniform_real_distribution<double> dist_theta(- M_PI, M_PI);
        
        auto request = std::make_shared<turtlesim::srv::Spawn::Request>();
        request->x = dist_xy(gen);
        request->y = dist_xy(gen);
        request->theta = dist_theta(gen);
        request->name = prefix_ + "_" + std::to_string(++counter_);
        spawn_->async_send_request(request, 
            [this, request](rclcpp::Client<turtlesim::srv::Spawn>::SharedFuture future)
            {
            my_robot_interfaces::msg::Turtle2 turtle;
            auto response = future.get();
            turtle.x = request->x;
            turtle.y = request->y;
            turtle.name = response->name;
            turtles.push_back(turtle);
            auto msg = my_robot_interfaces::msg::Turtle2Array();
            msg.turtles = turtles;
            turtle_pub_->publish(msg);
            });
    }

    void killTurtle(const my_robot_interfaces::srv::CatchTurtle::Request::SharedPtr request,
                          my_robot_interfaces::srv::CatchTurtle::Response::SharedPtr response)
    {
        int index = -1;
        for (size_t i = 0; i < turtles.size(); i++)
        {
            if (turtles[i].name == request->name)
                {
                    index = i;
                    break;
                }
        }
        if (index == -1)
            {
                RCLCPP_INFO(this->get_logger(),"index = -1 ");
            }
        auto request2 = std::make_shared<turtlesim::srv::Kill::Request>();
        request2->name = turtles[index].name;
        kill_->async_send_request(request2,
            [this, index](rclcpp::Client<turtlesim::srv::Kill>::SharedFuture future)
            {
                auto response2 = future.get();
                RCLCPP_INFO(this->get_logger(),"%s has been killed !", turtles[index].name.c_str());
                turtles.erase(turtles.begin() + index);
                auto msg = my_robot_interfaces::msg::Turtle2Array();
                msg.turtles = turtles;
                turtle_pub_->publish(msg);
            });
        response->success = true;
    }


    rclcpp::Client<turtlesim::srv::Spawn>::SharedPtr spawn_;
    rclcpp::Client<turtlesim::srv::Kill>::SharedPtr kill_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::TimerBase::SharedPtr timer_test_;
    rclcpp::Publisher<my_robot_interfaces::msg::Turtle2Array>::SharedPtr turtle_pub_;
    std::vector<my_robot_interfaces::msg::Turtle2> turtles;
    rclcpp::Service<my_robot_interfaces::srv::CatchTurtle>::SharedPtr catch_it_;
    std::string prefix_;
    int counter_;
    
    
        
};



int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TurtleSpawnerNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}