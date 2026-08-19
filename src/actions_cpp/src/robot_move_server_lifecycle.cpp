#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "my_robot_interfaces/action/move_robot.hpp"
#include "lifecycle_msgs/msg/state.hpp"

using MoveRobot = my_robot_interfaces::action::MoveRobot;
using LifectcleCallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;
using LifecycleState = lifecycle_msgs::msg::State;
using namespace std::placeholders;

class RobotMoveServerNode : public rclcpp_lifecycle::LifecycleNode
{
public:
    RobotMoveServerNode() : LifecycleNode("robot_move_server"), current_position(50)
    {
        RCLCPP_INFO(this->get_logger(), "In constructor");
        cb_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
    }

private:
    LifectcleCallbackReturn on_configure(const rclcpp_lifecycle::State &previous_state)
    {
        (void)previous_state;
        RCLCPP_INFO(this->get_logger(), "In on_configure");
        this->declare_parameter("robot_name", "");
        robot_name_ = this->get_parameter("robot_name").as_string();
        server_ = rclcpp_action::create_server<MoveRobot>(this, "robot_move_" + robot_name_,
                std::bind(&RobotMoveServerNode::goal_callback, this, _1, _2),
                std::bind(&RobotMoveServerNode::cancel_callback, this, _1),
                std::bind(&RobotMoveServerNode::handle_accept_callback, this, _1),
                rcl_action_server_get_default_options(),
                cb_group_
                );
        return LifectcleCallbackReturn::SUCCESS;
    }    
    
    LifectcleCallbackReturn on_cleanup(const rclcpp_lifecycle::State &previous_state)
    {
        (void)previous_state;
        RCLCPP_INFO(this->get_logger(), "In on_cleanup");
        robot_name_ = "";
        this->undeclare_parameter("robot_name");
        server_.reset();
        return LifectcleCallbackReturn::SUCCESS;
    }   
    
    LifectcleCallbackReturn on_activate(const rclcpp_lifecycle::State &previous_state)
    {
        RCLCPP_INFO(this->get_logger(), "In on_activate");
        rclcpp_lifecycle::LifecycleNode::on_activate(previous_state);
        return LifectcleCallbackReturn::SUCCESS;
    }   
    
    LifectcleCallbackReturn on_deactivate(const rclcpp_lifecycle::State &previous_state)
    {
        RCLCPP_INFO(this->get_logger(), "In on_deactivate");
        rclcpp_lifecycle::LifecycleNode::on_deactivate(previous_state);
        return LifectcleCallbackReturn::SUCCESS;
    }   
    
    LifectcleCallbackReturn on_shutdown(const rclcpp_lifecycle::State &previous_state)
    {
        (void)previous_state;
        RCLCPP_INFO(this->get_logger(), "In on_shutdown");
        robot_name_ = "";
        this->undeclare_parameter("robot_name");
        server_.reset();
        return LifectcleCallbackReturn::SUCCESS;
    }   






    rclcpp_action::GoalResponse goal_callback(const rclcpp_action::GoalUUID &uuid, std::shared_ptr<const MoveRobot::Goal> goal)
    {
        (void)uuid;
 

        auto state = this->get_current_state();
        if (state.id() != LifecycleState::PRIMARY_STATE_ACTIVE){
            RCLCPP_INFO(this->get_logger(), "Sever is not activated yet!");
            return rclcpp_action::GoalResponse::REJECT;
        }

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
    std::string robot_name_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RobotMoveServerNode>();
    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(node->get_node_base_interface());
    executor.spin();
    rclcpp::shutdown();
    return 0;
}