#!/bin/bash
echo "Stopping old Gazebo processes..."

pkill -9 gz
pkill -9 ruby

echo "Updating SDF..."
cd ~/ros2_ws

ros2 run xacro xacro \
src/my_robot_description/urdf/my_robot.urdf.xacro \
-o /tmp/my_robot.urdf

gz sdf -p /tmp/my_robot.urdf \
> src/my_robot_description/sdf/my_robot.sdf

echo "SDF update finished"
