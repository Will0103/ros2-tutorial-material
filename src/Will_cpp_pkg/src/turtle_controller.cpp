#include "rclcpp/rclcpp.hpp"
#include "my_robot_interfaces/msg/turtle2_array.hpp"
#include "my_robot_interfaces/srv/catch_turtle.hpp"
#include "turtlesim/msg/pose.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <random>
#include <cmath>

using namespace std::placeholders;


class TurtleControllerNode : public rclcpp::Node
{
public:
    TurtleControllerNode() : Node("turtle_controller") 
    {
        this->declare_parameter("catch_closest_turtle", true);
        catch_closest_turtle_first = this->get_parameter("catch_closest_turtle").as_bool();
        subscriber_pose_ = this->create_subscription<turtlesim::msg::Pose>("turtle1/pose", 10, std::bind(&TurtleControllerNode::callbackPose, this, _1));
        subscriber_alive_turtle_ = this->create_subscription<my_robot_interfaces::msg::Turtle2Array>(
            "alive_turtle", 10, std::bind(&TurtleControllerNode::turtleList, this, _1));
        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("turtle1/cmd_vel", 10);
        timer_ = this->create_wall_timer(std::chrono::milliseconds(100), std::bind(&TurtleControllerNode::chaseTurtle, this));
        catch_turtle_ = this->create_client<my_robot_interfaces::srv::CatchTurtle>("catch_turtle", 10);
    }
private:
    void callbackPose(const turtlesim::msg::Pose::SharedPtr msg)
    {
        turtle1_x = msg->x;
        turtle1_y = msg->y;
        turtle1_theta = msg->theta;
    }

    void turtleList(const my_robot_interfaces::msg::Turtle2Array::SharedPtr msg)
    {   
        alive_turtles_ = msg->turtles;
    }

    void chaseTurtle()
    {
        if (alive_turtles_.empty())
            return;
        if (catch_closest_turtle_first)
        {
            if (!target_exist) 
            {
                double min_dis = 999;
                int index = 0;
                for (size_t i = 0; i < alive_turtles_.size(); i++)
                {
                    distance = sqrt(pow((alive_turtles_[i].x-turtle1_x), 2.0) + pow((alive_turtles_[i].y-turtle1_y), 2.0));
                    if (distance < min_dis)
                    {    
                        min_dis = distance;
                        index = i;
                    }
                }
                target_x = alive_turtles_[index].x;
                target_y = alive_turtles_[index].y;
                target_name = alive_turtles_[index].name;
                RCLCPP_INFO(this->get_logger(),"%s !", target_name.c_str());
                target_exist = true; 
            }
        }
        else
        {
            if (!target_exist) 
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> dist(0, alive_turtles_.size()-1);
                int index = dist(gen);
                target_x = alive_turtles_[index].x;
                target_y = alive_turtles_[index].y;
                target_name = alive_turtles_[index].name;
                RCLCPP_INFO(this->get_logger(),"%s !", target_name.c_str());
                target_exist = true; 
            }
        }
        
        distance = sqrt(pow((target_x-turtle1_x), 2.0) + pow((target_y-turtle1_y), 2.0));
        dis_theta = atan2((target_y-turtle1_y), (target_x-turtle1_x))-turtle1_theta;
        if (dis_theta > M_PI)
            dis_theta -= 2 * M_PI;
        else if (dis_theta < -M_PI)
            dis_theta += 2 * M_PI;
        vx = 2.5 * distance;
        az = 4.5 * dis_theta;
        auto cmd_vel = geometry_msgs::msg::Twist();  
        cmd_vel.linear.x = vx;
        cmd_vel.angular.z = az;
        if (distance > 0.5)
            publisher_->publish(cmd_vel);
        else
        {
            RCLCPP_INFO(this->get_logger(),"Reach target !");
            cmd_vel.linear.x = 0;
            cmd_vel.angular.z = 0; 
            publisher_->publish(cmd_vel);
            auto request = std::make_shared<my_robot_interfaces::srv::CatchTurtle::Request>();
            request->name = target_name;
            catch_turtle_->async_send_request(
                        request,
                        [this, request](rclcpp::Client<my_robot_interfaces::srv::CatchTurtle>::SharedFuture future)
                    {
                        auto response = future.get();
                        if (response)
                            {
                                RCLCPP_INFO(this->get_logger(),"%s has been catched !", request->name.c_str());
                                target_exist= false;    
                            }    
                        
                    });

                  
        }
    }

    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr subscriber_pose_;
    rclcpp::Subscription<my_robot_interfaces::msg::Turtle2Array>::SharedPtr subscriber_alive_turtle_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    std::vector<my_robot_interfaces::msg::Turtle2> alive_turtles_;
    rclcpp::Client<my_robot_interfaces::srv::CatchTurtle>::SharedPtr catch_turtle_;
    bool target_exist= false;
    float turtle1_x, turtle1_y, turtle1_theta;
    double target_x, target_y, distance, dis_theta, vx, az;
    std::string target_name;
    bool catch_closest_turtle_first;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TurtleControllerNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}