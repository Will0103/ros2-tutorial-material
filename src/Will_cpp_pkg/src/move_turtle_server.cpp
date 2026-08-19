#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "my_robot_interfaces/action/move_turtle.hpp"
#include "turtlesim/srv/kill.hpp"
#include "turtlesim/srv/spawn.hpp"
#include "turtlesim/msg/pose.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "lifecycle_msgs/msg/state.hpp"


using MoveTurtle = my_robot_interfaces::action::MoveTurtle;
using MoveTurtleGoalHandle = rclcpp_action::ServerGoalHandle<MoveTurtle>;
using Kill = turtlesim::srv::Kill;
using Spawn = turtlesim::srv::Spawn;
using Pose = turtlesim::msg::Pose;
using CmdVel = geometry_msgs::msg::Twist;
using LifecycleCallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;
using LifecycleState = lifecycle_msgs::msg::State;
using namespace std::placeholders;


class MoveTurtleServerNode : public rclcpp_lifecycle::LifecycleNode
{
public:
    MoveTurtleServerNode() : LifecycleNode("mover_turtle_server")
    {
        RCLCPP_INFO(this->get_logger(), "In constructor");
        cb_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
        
    }

private:
    LifecycleCallbackReturn on_configure(const rclcpp_lifecycle::State &previous_state)
    {
        (void)previous_state;
        RCLCPP_INFO(this->get_logger(), "In on_configure");

        //Kill initial turtle_1
        kill_client_ = this->create_client<Kill>("kill", 10);
        if (!kill_client_->wait_for_service(std::chrono::seconds(2)))
        {
            RCLCPP_INFO(this->get_logger(), "Server is not working yet...");
            return LifecycleCallbackReturn::FAILURE;
        }    
        auto kill_target = std::make_shared<Kill::Request>();
        kill_target->name = "turtle1";
        kill_client_->async_send_request(kill_target);
        RCLCPP_INFO(this->get_logger(), "Kill turtle_1");
        
        //Spawn dog_1
        spawn_client_ = this->create_client<Spawn>("spawn", 10);  
        if (!spawn_client_->wait_for_service(std::chrono::seconds(2)))
        {
            RCLCPP_INFO(this->get_logger(), "Server is not working yet...");
            return LifecycleCallbackReturn::FAILURE;
        }    
        this->declare_parameter("prefix", "dog_1");
        prefix_ = this->get_parameter("prefix").as_string();
        auto spawn_target = std::make_shared<Spawn::Request>();
        spawn_target->x = 5.5;
        spawn_target->y = 5.5;
        spawn_target->theta = 0.5*M_PI;
        spawn_target->name = prefix_;
        spawn_client_->async_send_request(spawn_target);
        RCLCPP_INFO(this->get_logger(), "Spawn dog_1");

        //Subscribe dog_1 position
        pose_subscriber_ = this->create_subscription<Pose>(prefix_ + "/pose", 10, std::bind(&MoveTurtleServerNode::pose_callback, this, _1));

        //Publish dog_1 velocity
        cmd_vel_pub_ = this->create_publisher<CmdVel>(prefix_ + "/cmd_vel", 10);

        //Create move_turtle server
        move_turtle_server_ = rclcpp_action::create_server<MoveTurtle>(this, "move_turtle_" + prefix_,
                    std::bind(&MoveTurtleServerNode::goal_callback, this, _1, _2),
                    std::bind(&MoveTurtleServerNode::cancel_response_callback, this, _1),
                    std::bind(&MoveTurtleServerNode::handle_accept_callback, this, _1),
                    rcl_action_server_get_default_options(),
                    cb_group_);

        return LifecycleCallbackReturn::SUCCESS;
    }
    
    LifecycleCallbackReturn on_cleanup(const rclcpp_lifecycle::State &previous_state)
    {
        (void)previous_state;
        RCLCPP_INFO(this->get_logger(), "In on_cleanup");
        kill_client_.reset();
        spawn_client_.reset();
        pose_subscriber_.reset();
        cmd_vel_pub_.reset();
        move_turtle_server_.reset();
        prefix_.clear();
        
        return LifecycleCallbackReturn::SUCCESS;
    }

    LifecycleCallbackReturn on_shutdown(const rclcpp_lifecycle::State &previous_state)
    {
        (void)previous_state;
        RCLCPP_INFO(this->get_logger(), "In on_shutdown");
        kill_client_.reset();
        spawn_client_.reset();
        pose_subscriber_.reset();
        cmd_vel_pub_.reset();
        move_turtle_server_.reset();
        prefix_.clear();
        return LifecycleCallbackReturn::SUCCESS;
    }

    LifecycleCallbackReturn on_activate(const rclcpp_lifecycle::State &previous_state)
    {
        RCLCPP_INFO(this->get_logger(), "In on_activate");
        LifecycleNode::on_activate(previous_state);
        return LifecycleCallbackReturn::SUCCESS;
    }

    LifecycleCallbackReturn on_deactivate(const rclcpp_lifecycle::State &previous_state)
    {
        RCLCPP_INFO(this->get_logger(), "In on_deactivate");
        LifecycleNode::on_deactivate(previous_state);
        return LifecycleCallbackReturn::SUCCESS;
    }


    ////////////////////////////////////////////////////////////////////////

    void pose_callback(const std::shared_ptr<const Pose> &msg)
    {
        Pose_x_ = msg->x;
        Pose_y_ = msg->y;
    }

    rclcpp_action::GoalResponse goal_callback(const rclcpp_action::GoalUUID &uuid, const std::shared_ptr<const MoveTurtle::Goal> &goal)
    {
        (void)uuid;
        (void)goal;
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (goal_handle_){
                if (goal_handle_->is_active()){
                    RCLCPP_INFO(this->get_logger(), "Current goal is executing, new goal rejected");
                    return rclcpp_action::GoalResponse::REJECT;
                }
            }
        }
        
        auto state = this->get_current_state();
        if (state.id() == LifecycleState::PRIMARY_STATE_ACTIVE){
            RCLCPP_INFO(this->get_logger(), "Goal accepted");
            return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
        }
        RCLCPP_INFO(this->get_logger(), "Goal rejected");
        return rclcpp_action::GoalResponse::REJECT;
    }
    
    rclcpp_action::CancelResponse cancel_response_callback(const std::shared_ptr<MoveTurtleGoalHandle> &goal_handle)
    {
        (void)goal_handle;
        RCLCPP_INFO(this->get_logger(), "Cancel accept");
        return rclcpp_action::CancelResponse::ACCEPT;
    }

    void handle_accept_callback(const std::shared_ptr<MoveTurtleGoalHandle> &goal_handle)
    {
        RCLCPP_INFO(this->get_logger(), "Execute the goal");
        execute_callback(goal_handle);
    }

    void execute_callback(const std::shared_ptr<MoveTurtleGoalHandle> &goal_handle)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            goal_handle_ = goal_handle;
        }
        
        auto duration = goal_handle->get_goal()->duration;
        auto cmd_msg = CmdVel();
        cmd_msg.linear.x = goal_handle->get_goal()->linear_vel_x;
        cmd_msg.angular.z = goal_handle->get_goal()->angular_vel_z;
        start_time_ = this->now();
        cmd_vel_timer_ = this->create_wall_timer(std::chrono::milliseconds(100),
                                        [this, cmd_msg, duration, goal_handle]()
                                    {
                                        auto feedback = std::make_shared<MoveTurtle::Feedback>();
                                        feedback->position_x = Pose_x_;
                                        feedback->position_y = Pose_y_;
                                        goal_handle->publish_feedback(feedback);

                                        cmd_vel_pub_->publish(cmd_msg);
                                        if ((this->now()-start_time_).seconds() >= duration){
                                            cmd_vel_timer_->cancel();
                                            auto cmd_msg_stop = CmdVel();
                                            cmd_msg_stop.linear.x = 0;
                                            cmd_msg_stop.angular.z = 0; 
                                            cmd_vel_pub_->publish(cmd_msg_stop);

                                            auto result = std::make_shared<MoveTurtle::Result>();
                                            result->position_x = Pose_x_;
                                            result->position_y = Pose_y_;
                                            result->message = "success";
                                            RCLCPP_INFO(this->get_logger(), "Goal has been done !");
                                            goal_handle->succeed(result);
                                            {
                                                std::lock_guard<std::mutex> lock(mutex_);
                                                goal_handle_.reset();
                                            }
                                        }
                                    });
    }

    rclcpp_action::Server<MoveTurtle>::SharedPtr move_turtle_server_;
    rclcpp::Client<Kill>::SharedPtr kill_client_;
    rclcpp::Client<Spawn>::SharedPtr spawn_client_;
    rclcpp::Publisher<CmdVel>::SharedPtr cmd_vel_pub_;
    rclcpp::Subscription<Pose>::SharedPtr pose_subscriber_;
    std::string prefix_;
    double Pose_x_, Pose_y_;
    rclcpp::TimerBase::SharedPtr cmd_vel_timer_;
    rclcpp::Time start_time_;
    rclcpp::CallbackGroup::SharedPtr cb_group_;
    std::mutex mutex_;
    std::shared_ptr<MoveTurtleGoalHandle> goal_handle_;
};

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