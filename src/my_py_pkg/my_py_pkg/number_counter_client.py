#!/usr/bin/env python3 
import rclpy 
from rclpy.node import Node 
from example_interfaces.srv import SetBool


class ResetNumberCounter(Node):
    def __init__(self):
        super().__init__("reset_number_counter")
        self.client_ = self.create_client(SetBool, "reset_counter")
    
    def call_reset_counter(self, reset:bool):
        while not self.client_.wait_for_service(1.0):
            self.get_logger().warn("waiting for Number Counter")
        request = SetBool.Request()
        self.get_logger().info(str(reset))
        request.data = reset
        future = self.client_.call_async(request)
        future.add_done_callback(self.callback_reset_number_counter)

    def callback_reset_number_counter(self, future):
        response = future.result()
        if response.success == True:
            self.get_logger().info(response.message)

def main(args=None): 
    rclpy.init(args=args) 
    node = ResetNumberCounter() 
    node.call_reset_counter(5>3) 
    rclpy.spin(node)    
    rclpy.shutdown() 

if __name__ == "__main__":  
    main()