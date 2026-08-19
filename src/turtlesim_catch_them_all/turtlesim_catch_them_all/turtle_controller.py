#!/usr/bin/env python3 
import rclpy 
from rclpy.node import Node 
import random
import math

from geometry_msgs.msg import Twist
from turtlesim.msg import Pose
from my_robot_interfaces.msg import Turtle
from my_robot_interfaces.srv import CatchTurtle

class TurtleControllerNode(Node):
    def __init__(self):
        super().__init__("turtle_controller")
        self.declare_parameter("catch_closest_turtle_first", False)
        self.catch_closest_turtle_ = self.get_parameter("catch_closest_turtle_first").value
        self.turtles_ = None
        self.target_ = None
        self.turtle_name_ = None
        self.Kp_ = 3.0
        self.waiting_for_response_ = False
        self.pose_subscrber_ = self.create_subscription(Pose, "turtle1/pose", self.callback_pose_subscriber, 10)
        self.alive_turtles_subscriber_ = self.create_subscription(Turtle, "alive_turtles", self.callback_alive_turtles, 10)
        self.cmd_vel_publisher_ = self.create_publisher(Twist, "turtle1/cmd_vel", 10)
        self.move_timer_ = self.create_timer(0.05, self.publish_cmd_vel)
        self.catch_turtle_client_ = self.create_client(CatchTurtle, "catch_turtle")

    def callback_pose_subscriber(self, msg: Pose):
        self.turtle1_X_ = msg.x
        self.turtle1_Y_ = msg.y
        self.turtle1_deg_ = msg.theta
        
    def callback_alive_turtles(self, msg:Turtle):
        self.turtles_ = msg.name
        self.turtles_x_ = msg.x
        self.turtles_y_ = msg.y
        if not self.turtle_name_:
            return
        if self.turtle_name_ not in self.turtles_:
            self.get_logger().info(self.turtle_name_ + " has been killed")
            self.target_ = None
            self.waiting_for_response_ = False

    def publish_cmd_vel(self):
        if self.waiting_for_response_:
            return        
        if not self.turtles_ :
            return
        if not self.target_ :
            if not self.catch_closest_turtle_:
                self.target_ = random.randint(1,len(self.turtles_))
                self.turtle_name_ = self.turtles_[self.target_-1]
            else:
                distance_closest = None
                for name, x, y in zip(self.turtles_, self.turtles_x_,self.turtles_y_):
                    dis_x = round(x - self.turtle1_X_, 1)
                    dis_y = round(y - self.turtle1_Y_, 1)
                    distance = math.sqrt(dis_x**2 + dis_y**2)
                    if distance_closest == None or distance < distance_closest:
                        distance_closest = distance
                        self.turtle_name_ = name
                self.get_logger().info("Target : " + str(self.turtle_name_))
        position = self.turtles_.index(self.turtle_name_)
        target_X = self.turtles_x_[position]
        target_Y = self.turtles_y_[position]
        delta_distance, delta_degree = self.VelocityCalculator(target_X, target_Y)        
        msg = Twist()
        if abs(delta_degree) < 0.5:
            msg.linear.x = self.Kp_ * delta_distance
            msg.angular.z = self. Kp_ * delta_degree
        else:
            msg.linear.x = 0.5 * self.Kp_ * delta_distance
            msg.angular.z = self. Kp_ * delta_degree
        self.cmd_vel_publisher_.publish(msg)

        if delta_distance < 0.5:
            request = CatchTurtle.Request()
            request.name = self.turtle_name_
            self.waiting_for_response_ = True
            future = self.catch_turtle_client_.call_async(request)
            future.add_done_callback(self.callback_catch_turtle)

    def callback_catch_turtle(self, future):
        response = future.result()
        if response:
            pass

    def VelocityCalculator(self, x, y):
        dis_x = round(x - self.turtle1_X_, 1)
        dis_y = round(y - self.turtle1_Y_, 1)
        delta_distance = math.sqrt(dis_x**2 + dis_y**2)
        delta_degree = math.atan2(dis_y, dis_x) - self.turtle1_deg_
        if delta_degree > math.pi:
            delta_degree -= 2 * math.pi
        elif delta_degree < -math.pi:
            delta_degree += 2 * math.pi
        return delta_distance, delta_degree



def main(args=None): 
    rclpy.init(args=args) 
    node = TurtleControllerNode()  
    rclpy.spin(node)    
    rclpy.shutdown() 

if __name__ == "__main__":  
    main()