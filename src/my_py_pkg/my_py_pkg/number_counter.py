#!/usr/bin/env python3 
import rclpy 
from rclpy.node import Node 
from example_interfaces.msg import Int64, String
from example_interfaces.srv import SetBool

class NumberCounterNode(Node):
    def __init__(self):
        super().__init__("number_counter")
        self.subscriber_ = self.create_subscription(Int64, "number", self.callback_number, 10)
        self.get_logger().info("Number counter is up and running!")
        self.publishers_ = self.create_publisher(Int64, "number_count", 10)
        self.counter_ = 0
        self.server_ = self.create_service(SetBool, "reset_counter", self.callback_reset_counter)
    
    def callback_number(self, msg: Int64):
        self.get_logger().info("I received: " + str(msg.data) + " ! ")
        self.counter_ += 1
        msg_count = Int64()
        msg_count.data = msg.data * self.counter_
        self.publishers_.publish(msg_count)

    def callback_reset_counter(self, request: SetBool.Request, response: SetBool.Response):
        if request.data == True:
            self.counter_ = 0
            response.success = True
            response.message = "Counter reset to 0"
        else:
            response.success = False
            response.message = "No reset!"
        return response

def main(args=None): 
    rclpy.init(args=args) 
    node = NumberCounterNode()  
    rclpy.spin(node)    
    rclpy.shutdown() 

if __name__ == "__main__":  
    main() 