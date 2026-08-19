#!/usr/bin/env python3 
import rclpy 
from rclpy.node import Node 

from my_robot_interfaces.srv import SetLed


class BatteryNode(Node):
    def __init__(self):
        super().__init__("battery")
        self.client_ = self.create_client(SetLed, "set_led")
        self.timer_ = self.create_timer(1.0, self.callback_timer)
        self.counter_ = 0
        self.get_logger().info("Battery starts working !")

    def callback_timer(self):
        self.counter_ += 1
        if self.counter_ > 10:
            self.counter_ = 1
        if self.counter_ < 5:
            self.get_logger().info("Battery has been using for " + str(self.counter_) + " sec.")
        else:
            self.get_logger().info("Battery has been recharging for " + str(self.counter_-4) + " sec.")

        if self.counter_ == 4:
            self.call_set_led(3, True)
            self.get_logger().info("Battery is recharging !")
        elif self.counter_ == 10:
            self.call_set_led(3, False)
            self.get_logger().info("Battery is full !")

    def call_set_led(self, Led_number: int, state: bool):
        request = SetLed.Request()
        request.led_number = Led_number
        request.state = state
        future = self.client_.call_async(request)
        future.add_done_callback(self.callback_Led_panel_state)

    def callback_Led_panel_state(self, future):
        response = future.result()
        self.get_logger().info("LED panel state has been switched !")

        

def main(args=None): 
    rclpy.init(args=args) 
    node = BatteryNode()  
    rclpy.spin(node)    
    rclpy.shutdown() 

if __name__ == "__main__":  
    main()