#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "my_robot_interfaces/action/move_robot.hpp"
#include "my_robot_interfaces/msg/cancel_request.hpp"

using MoveRobot = my_robot_interfaces::action::MoveRobot;
using namespace std::placeholders;



class RobotMoveClientNode : public rclcpp::Node
{
public:
    RobotMoveClientNode() : Node("robot_move_client")
    {
        client_ = rclcpp_action::create_client<MoveRobot>(this, "robot_move");
        cancel_subscriber_ = this->create_subscription<my_robot_interfaces::msg::CancelRequest>("cancel_move", 10,
                                                                                        std::bind(&RobotMoveClientNode::cancel_request, this, _1));
        // timer_ = this->create_wall_timer(std::chrono::seconds(3), std::bind(&RobotMoveClientNode::cancel_Timer, this));
    }
    
    void send_goal(int position, int velocity)
    {
        client_->wait_for_action_server();
        auto goal = MoveRobot::Goal();
        goal.position = position;
        goal.velocity = velocity;
        RCLCPP_INFO(this->get_logger(), "position: %d, velocity: %d", position, velocity);
        
        auto options = rclcpp_action::Client<MoveRobot>::SendGoalOptions();
        
        options.result_callback = std::bind(&RobotMoveClientNode::goal_result_callback, this, _1);
        options.goal_response_callback = std::bind(&RobotMoveClientNode::goal_response_callback, this, _1);
        options.feedback_callback = std::bind(&RobotMoveClientNode::goal_feedback_callback, this , _1, _2);

        
        client_->async_send_goal(goal, options);
    }

private:
    // void cancel_Timer()
    // {
    //     timer_->cancel();
    //     client_->async_cancel_goal(goal_handel_);
    // }
    void cancel_request(const std::shared_ptr<my_robot_interfaces::msg::CancelRequest> msg)
    {
        if (msg->cancel){
            if (goal_handel_){
                RCLCPP_INFO(this->get_logger(), "Please cancel the action");
                client_->async_cancel_goal(goal_handel_);   
                goal_handel_.reset();
            }
            
        }
    }

    void goal_feedback_callback (const std::shared_ptr<rclcpp_action::ClientGoalHandle<MoveRobot>> &goal_handle,
                                 const std::shared_ptr<const MoveRobot::Feedback> feedback)
    {
        goal_handel_ = goal_handle;
        RCLCPP_INFO(this->get_logger(), "Position: %d", (int)feedback->current_position);
    }

    void goal_response_callback (const std::shared_ptr<rclcpp_action::ClientGoalHandle<MoveRobot>> &goal_handle)
    {
        if (goal_handle){
            RCLCPP_INFO(this->get_logger(), "Goal is accepted!");
        }else{
            RCLCPP_INFO(this->get_logger(), "Goal is rejected!");
        }
    }

    void goal_result_callback (const rclcpp_action::ClientGoalHandle<MoveRobot>::WrappedResult &result)
    {
        auto status = result.code;
        int final_position = result.result->position;
        auto msg = result.result->message;
        if (status == rclcpp_action::ResultCode::SUCCEEDED){
            RCLCPP_INFO(this->get_logger(), "Success");
        }else if (status == rclcpp_action::ResultCode::CANCELED){
            RCLCPP_ERROR(this->get_logger(), "Canceled");
        }else if (status == rclcpp_action::ResultCode::ABORTED){
            RCLCPP_WARN(this->get_logger(), "Aborted");
        }
        RCLCPP_INFO(this->get_logger(), "%s Final postion: %d", msg.c_str(), final_position);

    }

    rclcpp_action::Client<MoveRobot>::SharedPtr client_;
    // rclcpp::TimerBase::SharedPtr timer_;
    std::shared_ptr<rclcpp_action::ClientGoalHandle<MoveRobot>> goal_handel_;
    rclcpp::Subscription<my_robot_interfaces::msg::CancelRequest>::SharedPtr cancel_subscriber_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RobotMoveClientNode>();
    node->send_goal(83,4);
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}