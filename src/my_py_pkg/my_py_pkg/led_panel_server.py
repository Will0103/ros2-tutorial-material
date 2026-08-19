#!/usr/bin/env python3 
import rclpy 
from rclpy.node import Node 

from my_robot_interfaces.msg import LedPanelState 
from my_robot_interfaces.srv import SetLed


class LedPanelNode(Node):
    def __init__(self):
        super().__init__("led_panel")
        self.declare_parameter("led_panel_state", [0, 0, 0])
        self. led_panel_state_ = self.get_parameter("led_panel_state").value
        # self.led_panel_state_ = [0, 0, 0]
        self.server_ = self.create_service (SetLed, "set_led", self.callback_set_led)
        self.get_logger().info("Led Panel Service is working! LED state is " + str(self.led_panel_state_))
        self.publisher_ = self.create_publisher (LedPanelState, "led_panel_state", 10)
        self.timer_ = self.create_timer (1.0, self.publisher_led_panel_state)
    
    def callback_set_led(self, request:SetLed.Request, response:SetLed.Response):
        number_ = request.led_number
        state_ = request.state
        response.success = True
        # self.get_logger().info(
        #     "Input 1 : " + str(number_-1) + ", input 2 :" +str(self.led_panel_state_[number_-1]) + ", output 1 : " +str(response.success))
        if state_:
            self.led_panel_state_[number_-1] = 1
            self.get_logger().info("LED " + str(number_)+" turn on ! " + str(self.led_panel_state_))
        else:
            self.led_panel_state_[number_-1] = 0
            self.get_logger().info("LED " + str(number_)+" turn off ! "+ str(self.led_panel_state_))
        return response
    
    def publisher_led_panel_state(self):
        msg = LedPanelState()
        msg.led_panel_state = self.led_panel_state_
        self.publisher_.publish(msg)
            

def main(args=None): 
    rclpy.init(args=args) 
    node = LedPanelNode()  
    rclpy.spin(node)    
    rclpy.shutdown() 

if __name__ == "__main__":  
    main()