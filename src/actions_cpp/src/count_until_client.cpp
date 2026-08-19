#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "my_robot_interfaces/action/count_until.hpp"

using CountUntil = my_robot_interfaces::action::CountUntil;
using CountUntilGoalHandle = rclcpp_action::ClientGoalHandle<CountUntil>;
using namespace std::placeholders;

class CountUntilClientNode : public rclcpp::Node
{
public:
    CountUntilClientNode() : Node("count_until_client")
    {
        count_until_client_ = rclcpp_action::create_client<CountUntil>(this, "count_until");
    }

    void send_goal(int target_number, double period)
    {
        count_until_client_->wait_for_action_server();

        auto goal = CountUntil::Goal();
        goal.target_number = target_number;
        goal.period = period;

        // Add callbacks
        auto options = rclcpp_action::Client<CountUntil>::SendGoalOptions();
        options.result_callback = std::bind(&CountUntilClientNode::goal_result_callback, this, _1);
        options.goal_response_callback = std::bind(&CountUntilClientNode::goal_response_callback, this, _1);
        options.feedback_callback = std::bind(&CountUntilClientNode::goal_feedback_callback, this, _1, _2);


        RCLCPP_INFO(this->get_logger(), "Sending a goal");
        count_until_client_->async_send_goal(goal, options);
        // timer_ = this->create_wall_timer(std::chrono::seconds(2), 
        //                                     [this]()
        //                                 {
        //                                     timer_->cancel();
        //                                     RCLCPP_INFO(this->get_logger(), "Cancel the goal");
        //                                     count_until_client_->async_cancel_goal(goal_handle_);
        //                                 });
    }

private:


    // Callback to know if the gola was accepted or rejected
    void goal_response_callback(const CountUntilGoalHandle::SharedPtr &goal_handle)
    {
        if (!goal_handle){
            RCLCPP_INFO(this->get_logger(), "Goal got rejected");
        }
        else{
            RCLCPP_INFO(this->get_logger(), "Goal got accepted");
            this->goal_handle_ = goal_handle; // Saved for cancel request
        }
    }


    // Callback to receive the result once the goal is done
    void goal_result_callback(const CountUntilGoalHandle::WrappedResult &result)
    {
        auto status = result.code;
        if (status == rclcpp_action::ResultCode::SUCCEEDED){
            RCLCPP_INFO(this->get_logger(), "Succeed");
        }
        else if (status == rclcpp_action::ResultCode::ABORTED){
            RCLCPP_ERROR(this->get_logger(), "Aborted");
        }
        else if (status == rclcpp_action::ResultCode::CANCELED){
            RCLCPP_WARN(this->get_logger(), "Canceled");
        }
        
        int reached_number = result.result->reached_number;
        RCLCPP_INFO(this->get_logger(), "Result: %d", reached_number);
    }

    // Callback to received feedback
    void goal_feedback_callback(const CountUntilGoalHandle::SharedPtr &goal_handle,
                                const std::shared_ptr<const CountUntil::Feedback> feedback)
    {
        (void)goal_handle;
        int number = feedback->current_number;
        RCLCPP_INFO(this->get_logger(), "Got feedback: %d", number);
    }


    rclcpp_action::Client<CountUntil>::SharedPtr count_until_client_;
    rclcpp::TimerBase::SharedPtr timer_;
    CountUntilGoalHandle::SharedPtr goal_handle_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CountUntilClientNode>();
    node->send_goal(6,0.8);
    // node->send_goal(9,1.2);
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}