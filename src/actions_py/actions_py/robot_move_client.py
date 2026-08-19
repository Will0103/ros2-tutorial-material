#!/usr/bin/env python3 
import rclpy 
from rclpy.node import Node 

from rclpy.action import ActionClient
from rclpy.action.client import ClientGoalHandle, GoalStatus
from my_robot_interfaces.action import MoveRobot
from my_robot_interfaces.msg import CancelRequest


class RobotMoveClient(Node):
    def __init__(self):
        super().__init__("robot_move_client")
        self.robot_move_client_ = ActionClient(self, MoveRobot, "robot_move")
        self.goal_handle_ = None
        self.cancel_subscriber_ = self.create_subscription(CancelRequest, "cancel_status", self.cancel_request, 10)
        

    def cancel_request(self, msg_bool: CancelRequest):
        if msg_bool.cancel:
            self.get_logger().info("Send cancel request")
            self.goal_handle_.cancel_goal_async()
        
    def send_goal(self, position, velocity):
        self.robot_move_client_.wait_for_server()
        goal = MoveRobot.Goal()
        goal.position = position
        goal.velocity = velocity
        self.get_logger().info("Sending goal")
        self.robot_move_client_.send_goal_async(goal, feedback_callback=self.feedback_callback).add_done_callback(self.goal_response_callback)

    def goal_response_callback(self, future):
        self.goal_handle_: ClientGoalHandle = future.result()
        if self.goal_handle_.accepted:
            self.get_logger().info("goal got accepted")
            self.goal_handle_.get_result_async().add_done_callback(self.goal_result_callback)
        else:
            self.get_logger().info("goal got rejected")
        
    def goal_result_callback(self, future):
        robot_position = future.result().result.position    
        msg = future.result().result.message
        status = future.result().status
        if status == GoalStatus.STATUS_SUCCEEDED:
            self.get_logger().info("Success")
        elif status == GoalStatus.STATUS_ABORTED:
            self.get_logger().error("Aborted")
        elif status == GoalStatus.STATUS_CANCELED:
            self.get_logger().warn("Canceled")
        self.get_logger().info(msg + " Robot position: " + str(robot_position))

    def feedback_callback(self, feedback_position):
        robot_position = feedback_position.feedback.current_position
        self.get_logger().info("Robot is at : " + str(robot_position))
        

def main(args=None): 
    rclpy.init(args=args) 
    node = RobotMoveClient()  
    node.send_goal(84, 4)
    rclpy.spin(node)    
    rclpy.shutdown() 

if __name__ == "__main__":  
    main()