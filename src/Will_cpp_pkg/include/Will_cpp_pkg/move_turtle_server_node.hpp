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
    MoveTurtleServerNode();

private:
    LifecycleCallbackReturn on_configure(const rclcpp_lifecycle::State &previous_state);
    
    LifecycleCallbackReturn on_cleanup(const rclcpp_lifecycle::State &previous_state);
    
    LifecycleCallbackReturn on_shutdown(const rclcpp_lifecycle::State &previous_state);
    
    LifecycleCallbackReturn on_activate(const rclcpp_lifecycle::State &previous_state);

    LifecycleCallbackReturn on_deactivate(const rclcpp_lifecycle::State &previous_state);
    
    ////////////////////////////////////////////////////////////////////////

    void pose_callback(const std::shared_ptr<const Pose> &msg);
    
    rclcpp_action::GoalResponse goal_callback(const rclcpp_action::GoalUUID &uuid, const std::shared_ptr<const MoveTurtle::Goal> &goal);
    
    rclcpp_action::CancelResponse cancel_response_callback(const std::shared_ptr<MoveTurtleGoalHandle> &goal_handle);

    void handle_accept_callback(const std::shared_ptr<MoveTurtleGoalHandle> &goal_handle);

    void execute_callback(const std::shared_ptr<MoveTurtleGoalHandle> &goal_handle);

    /////////////////////////////////////////////////////////////////////////

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