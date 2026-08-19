#!/usr/bin/env python3 
#這個檔案是 Python 3 程式，Linux 執行時請自動找 Python 3 來跑

import rclpy # import the ROS2 Python library
from rclpy.node import Node #  import the Node class from the ROS2 Python library


class Mynode(Node):
    def __init__(self):
        super().__init__("py_test")
        self.counter_ = 0 
        self.get_logger().info("Hellow worldddd") # print "Hello world" in the terminal
        self.create_timer(1.0, self.timer_callback) # create a timer, call the function "timer_callback" every 1 second

    def timer_callback(self):
        self.get_logger().info("YOOOO" + str(self.counter_))
        self.counter_ += 1
def main(args=None): # create input var "args" which intial value is "none"
    rclpy.init(args=args) # initialize the ROS2 communication
    node = Mynode()  # create a node object
    rclpy.spin(node)    # do not exit the program, keep it running until user press Ctrl+C
    rclpy.shutdown() # shutdown the ROS2 communication

if __name__ == "__main__":  #if this file is the main program, then run the main function
    main()