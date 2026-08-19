#!/usr/bin/env python3 
import rclpy 
from rclpy.node import Node 
import time
import threading

from rclpy.action.server import ServerGoalHandle
from rclpy.action import ActionServer, GoalResponse, CancelResponse
from my_robot_interfaces.action import MoveRobot

from rclpy.executors import MultiThreadedExecutor
from rclpy.callback_groups import ReentrantCallbackGroup


class RobotMoveServerNode(Node):
    def __init__(self):
        super().__init__("robot_move_server")
        self.robot_move_server_ = ActionServer(self, MoveRobot, "robot_move", 
                                               goal_callback= self.goal_callback,
                                               handle_accepted_callback= self.handle_callback, 
                                               cancel_callback=self.cancel_callback,
                                               execute_callback=self.execute_callback,
                                               callback_group=ReentrantCallbackGroup())
        self.lock_ = threading.Lock()
        self.goal_handle_ = None
        self.robot_position = 50
        self.get_logger().info("Server up")

    def goal_callback(self, goal_request: MoveRobot.Goal):
        if goal_request.velocity > 0:
            return GoalResponse.ACCEPT
        else:
            return GoalResponse.REJECT

    def handle_callback(self, new_goal_handle: ServerGoalHandle):
        with self.lock_:
            old_goal_handle = self.goal_handle_
            if old_goal_handle is not None and old_goal_handle.is_active:
                old_goal_handle.abort()
                self.get_logger().info("Abort previous job")
            self.goal_handle_ = new_goal_handle
        self.goal_handle_.execute()

    def cancel_callback(self, goal_handle: ServerGoalHandle):
        self.get_logger().info("Goal is canceling...")
        return CancelResponse.ACCEPT

    def execute_callback(self, goal_handle: ServerGoalHandle):
        self.get_logger().info("Start executing !")
        self.goal_handle_ = goal_handle
        position = goal_handle.request.position
        velocity = goal_handle.request.velocity
        feedback = MoveRobot.Feedback()
        result = MoveRobot.Result()
        self.get_logger().info("Receive goal !")
        while(position > self.robot_position):
            self.get_logger().info("Target position : " + str(position) + ", Robot position : " + str(self.robot_position))
            if not goal_handle.is_active:
                result.position = self.robot_position
                result.message = "Abort the job !"
                return result
            if goal_handle.is_cancel_requested:
                result.position = self.robot_position
                goal_handle.canceled()
                result.message = "The job is canceled!"
                return result
            feedback.current_position = self.robot_position
            goal_handle.publish_feedback(feedback)
            distance = position - self.robot_position
            if distance >= velocity:
                self.robot_position += velocity
            elif distance < velocity:
                self.robot_position += distance
            time.sleep(1.0)
        while(position < self.robot_position):
            self.get_logger().info("Target position : " + str(position) + ", Robot position : " + str(self.robot_position))
            if not goal_handle.is_active:
                result.position = self.robot_position
                result.message = "Abort the job !"
                return result
            if goal_handle.is_cancel_requested:
                result.position = self.robot_position
                goal_handle.canceled()
                result.message = "The job is canceled!"
                return result
            feedback.current_position = self.robot_position
            goal_handle.publish_feedback(feedback)
            distance = self.robot_position -position
            if distance >= velocity:
                    self.robot_position -= velocity
            elif distance < velocity:
                    self.robot_position -= distance
            time.sleep(1.0)
        self.get_logger().info("Target position : " + str(position) + ", Robot position : " + str(self.robot_position))
        feedback.current_position = self.robot_position
        goal_handle.publish_feedback(feedback)
        if self.robot_position == position:
            goal_handle.succeed()
            result.position = self.robot_position
            result.message = "Reach targe position !"
            self.get_logger().info("Reach target position !")
            return result
                

        

def main(args=None): 
    rclpy.init(args=args) 
    node = RobotMoveServerNode()  
    rclpy.spin(node,MultiThreadedExecutor())    
    rclpy.shutdown() 

if __name__ == "__main__":  
    main()