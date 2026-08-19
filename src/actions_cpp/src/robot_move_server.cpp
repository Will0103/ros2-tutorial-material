#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "my_robot_interfaces/action/move_robot.hpp"

using MoveRobot = my_robot_interfaces::action::MoveRobot;
using namespace std::placeholders;

class RobotMoveServerNode : public rclcpp::Node
{
public:
    RobotMoveServerNode() : Node("robot_move_server"), current_position(50)
    {
        cb_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
        server_ = rclcpp_action::create_server<MoveRobot>(this, "robot_move",
                std::bind(&RobotMoveServerNode::goal_callback, this, _1, _2),
                std::bind(&RobotMoveServerNode::cancel_callback, this, _1),
                std::bind(&RobotMoveServerNode::handle_accept_callback, this, _1),
                rcl_action_server_get_default_options(),
                cb_group_
                );
        RCLCPP_INFO(this->get_logger(), "Robot Move Server is on !");

    }
private:
    
    rclcpp_action::GoalResponse goal_callback(const rclcpp_action::GoalUUID &uuid, std::shared_ptr<const MoveRobot::Goal> goal)
    {
        (void)uuid;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (goal_handle_){
                if (goal_handle_->is_active()){
                    preempt_goal_UUID = goal_handle_->get_goal_id();
                }
            }
        }
        if ((goal->position <= 100 && goal->position >= 0) && goal->velocity > 0){
            RCLCPP_INFO(this->get_logger(), "Goal is accepted !");
            return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
        }
        

        RCLCPP_INFO(this->get_logger(), "Goal is rejected !");
        return rclcpp_action::GoalResponse::REJECT;
        
    }

    rclcpp_action::CancelResponse cancel_callback(const std::shared_ptr<rclcpp_action::ServerGoalHandle<MoveRobot>> goal_handle)
    {
        (void)goal_handle;
        RCLCPP_INFO(this->get_logger(), "Cancel request is accepted !");
        return rclcpp_action::CancelResponse::ACCEPT;
    }

    void handle_accept_callback(const std::shared_ptr<rclcpp_action::ServerGoalHandle<MoveRobot>> goal_handle)
    {
        RCLCPP_INFO(this->get_logger(), "Executing the goal...");
        execute_callback(goal_handle);
    }

    void execute_callback(const std::shared_ptr<rclcpp_action::ServerGoalHandle<MoveRobot>> goal_handle)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            goal_handle_ = goal_handle;
        }
        
        auto result = std::make_shared<MoveRobot::Result>();
        auto feed_back = std::make_shared<MoveRobot::Feedback>();
        auto position = goal_handle->get_goal()->position;
        auto velocity = goal_handle->get_goal()->velocity;
        RCLCPP_INFO(this->get_logger(), "Target: %d", (int)position);
        rclcpp::Rate loop_rate(1);
        while (position != current_position)
        {
            distance = std::abs(position - current_position);
            
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (goal_handle->get_goal_id() == preempt_goal_UUID){
                    result->position = current_position; 
                    result->message = std::string("Aborted !");   
                    goal_handle->abort(result);
                    return;
                }
            }
            if (goal_handle->is_canceling()){
                result->position = current_position; 
                result->message = std::string("Canceled !");   
                goal_handle->canceled(result);
                return;
            }

            if (position > current_position){
                if (distance < velocity){
                    current_position += distance;
                }else{
                    current_position += velocity;
                }
            }
            if (position < current_position){
                if (distance < velocity){
                    current_position -= distance;
                }else{
                    current_position -= velocity;
                }
            }
            feed_back->current_position = current_position;
            goal_handle->publish_feedback(feed_back);
            RCLCPP_INFO(this->get_logger(), "%d", current_position);
            loop_rate.sleep();
        }
        result->position = current_position; 
        result->message = std::string("Reach target postion !");   
        goal_handle->succeed(result);
        
    }

    rclcpp_action::Server<MoveRobot>::SharedPtr server_;
    int current_position, distance;
    std::mutex mutex_;
    std::shared_ptr<rclcpp_action::ServerGoalHandle<MoveRobot>> goal_handle_;
    rclcpp_action::GoalUUID preempt_goal_UUID;
    rclcpp::CallbackGroup::SharedPtr cb_group_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RobotMoveServerNode>();
    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
}