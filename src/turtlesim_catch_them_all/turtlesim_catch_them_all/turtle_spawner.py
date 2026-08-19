#!/usr/bin/env python3 
import rclpy 
from rclpy.node import Node 
import random
from functools import partial

from turtlesim.srv import Spawn
from turtlesim.srv import Kill
from my_robot_interfaces.msg import Turtle
from my_robot_interfaces.srv import CatchTurtle

class TurtleSpawnerNode(Node):
    def __init__(self):
        super().__init__("turtle_spawner")
        self.declare_parameter("spawn_frequency", 1.0)
        self.spawn_frequency_ = self.get_parameter("spawn_frequency").value
        self.declare_parameter("object_name", "turtle_")
        self.object_name_ = self.get_parameter("object_name").value
        self.turtles_ = []
        self.turtles_x_ = []
        self.turtles_y_ = []
        self.counter_ = 0
        self.spawner_client_ = self.create_client(Spawn, "spawn")
        self.killer_client_ = self.create_client(Kill, "kill")
        self.spawner_timer_ = self.create_timer(self.spawn_frequency_, self.call_spwaner)
        self.alive_turtles_publisher_ = self.create_publisher(Turtle, "alive_turtles", 10)
        self.alive_turtles_timer_ = self.create_timer(0.3, self.publisher_alive_turtles)
        self.catch_turtle_server_ = self.create_service(CatchTurtle, "catch_turtle", self.callback_catch_turtle)

    def call_spwaner(self):
        spawner_request = Spawn.Request()
        spawner_request.x = round(random.uniform(0,11),1)
        spawner_request.y = round(random.uniform(0,11),1)
        self.counter_ += 1
        spawner_request.name = self.object_name_ + str(self.counter_)
        # self.get_logger().info(spawner_request.name)
        # self.spawn_x_ = spawner_request.x
        # self.spawn_y_ = spawner_request.y
        future = self.spawner_client_.call_async(spawner_request)
        future.add_done_callback(partial(self.callback_spwaner, spawner_request = spawner_request)) # Partial can bring parameter to callback

    def callback_spwaner(self, future, spawner_request: Spawn.Request):
        spawner_response = future.result()
        self.turtles_x_.append(spawner_request.x)
        self.turtles_y_.append(spawner_request.y)
        self.turtles_.append(spawner_response.name)
        self.get_logger().info("We have " + str(self.turtles_))
            
    def publisher_alive_turtles(self):
        msg = Turtle()
        msg.name = self.turtles_
        msg.x = self.turtles_x_
        msg.y = self.turtles_y_
        self.alive_turtles_publisher_.publish(msg)
            
    def callback_catch_turtle(self, request: CatchTurtle.Request, response: CatchTurtle.Response):
        self.turtle_name_ = request.name
        target_turtle_index = self.turtles_.index(self.turtle_name_)
        killer_request = Kill.Request()
        killer_request.name = self.turtle_name_
        future = self.killer_client_.call_async(killer_request)
        future.add_done_callback(partial(self.callback_killer, index = target_turtle_index))        
        response.success = True
        return response
     
    def callback_killer(self, future, index):
        del self.turtles_[index]
        del self.turtles_x_[index]
        del self.turtles_y_[index]
        
        # self.turtles_.remove(self.turtles_[self.target_turtle_index_])
        # self.turtles_x_.remove(self.turtles_x_[self.target_turtle_index_])
        # self.turtles_y_.remove(self.turtles_y_[self.target_turtle_index_])
        # self.get_logger().info(self.turtle_name_ + " has been killed")

        


def main(args=None): 
    rclpy.init(args=args) 
    node = TurtleSpawnerNode()  
    rclpy.spin(node)    
    rclpy.shutdown() 

if __name__ == "__main__":  
    main()