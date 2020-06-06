## Running ROS over a network

To use ROS across a network of multiple computers requires configuration **both at the network level and at the ROS level**.

-  network level: all of the computers can talk to each other.
- ROS level: all of nodes can communicate with the master.
- http://wiki.ros.org/ROS/NetworkSetup
- http://wiki.ros.org/ROS/Tutorials/MultipleMachines
- http://wiki.ros.org/ROS/EnvironmentVariables
- Once you have configured things correctly, ROS will take care of the details of network communication.

## Writing cleaner programs

- use of **`ros::Timer` callbacks instead of `ros::Rate` objects**. 改善点：`ros::Rate`を`ros::Timer`に変更しよう。
- encapsulate all or part of a node's data in a class, using methods of that class as callbacks. これは実践中。

## Visualizing data with `rviz`

## Creating message and service types

## Managing coordinate frames with `tf`

## Simulating with Gazebo

Gazeboを試してみよう！