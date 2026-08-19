#!/usr/bin/env python3 
import rclpy 
from rclpy.node import Node 
from example_interfaces.msg import Int64

class numberStation(Node):
    def __init__(self):
        super().__init__("number_publisher")
        self.declare_parameter("number", 2)
        self.declare_parameter("timer_period", 1.0)
        self.number_ = self.get_parameter("number").value
        self.timer_period_ = self.get_parameter("timer_period").value
        # self.get_logger().info(str(self.number_) + "  " + str(self.timer_period_))

        self.publishers_= self.create_publisher(Int64, "number", 10) 
        self.timer_= self.create_timer(self.timer_period_, self.publish_news)
        self.get_logger().info("number_publisher is up!")

    def publish_news(self):
        msg=Int64()
        msg.data= self.number_
        self.publishers_.publish(msg)

def main(args=None): 
    rclpy.init(args=args) 
    node = numberStation()  
    rclpy.spin(node)    
    rclpy.shutdown()

if __name__ == "__main__":  
    main()