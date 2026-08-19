#!/usr/bin/env python3 
import rclpy 
from rclpy.node import Node 
from example_interfaces.msg import String

class RobotNewsStation(Node):
    def __init__(self):
        super().__init__("robot_news_station")
        self.declare_parameter("robot_name","C651")
        self.robot_name_ = self.get_parameter("robot_name").value
        # self.robot_name_ = "CCOP3587999"
        self.publishers_= self.create_publisher(String, "robot_news", 10) #robot_news is the topic name, 10 is the queue size
        self.timer_= self.create_timer(2, self.publish_news)
        self.get_logger().info("Robot news station is up and running!")

    def publish_news(self):
        msg=String()
        msg.data="Hi, this is " + self.robot_name_ + " reporting!"
        self.publishers_.publish(msg)

def main(args=None): 
    rclpy.init(args=args) 
    node = RobotNewsStation()  
    rclpy.spin(node)    
    rclpy.shutdown()

if __name__ == "__main__":  
    main()