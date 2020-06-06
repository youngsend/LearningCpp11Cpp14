Servicesとmessagesの２つ違うところ：

- service calls are bi-directional.
- service calls implement one-to-one communication.
  - each message is associated with a topic that might have many publishers and many subscribers.

## 8.1 Terminology for services

- The specific content of the request and response data is determined by the **service data type**, which is **analogous to the message types** that determine the content of messages.
  - The only difference is that a service data type is divided into two parts, representing the request and the response.

## 8.2 Finding and calling services from the command line

- Listing all services

  ```bash
  rosservice list
  ```

- 2種類service: 僕の理解だと、node service vs. general service.（自分がつけた名前）
  - node service: used to get information from or pass information to specific nodes.
    - usually **use their node's name as a namespace** to prevent name collisions.
    - allow their nodes to offer them via private names like `~get_loggers` or `~set_logger_level`.
  - general service: not **conceptually** tied to any particular node.
    - 例えば`/spawn`：when we call `/spawn`, we only care that a new turtle is created, not about the details of which node does that work.
    - These kinds of services typically have **names that describe their function**, but that do not mention any specific node.
    - これらのserviceはあるnodeが提供しているが。

- ```bash
  # Listing services by node
  rosnode info node-name
  
  # Finding the node offering a service
  rosservice node service-name
  
  # Finding the data type of a service（このサービスを使えるように大事）
  rosservice info service-name
  ```

  - package name + type name -> service data type.

- Inspecting service data types（サービスを使えるように大事）

  ```bash
  rossrv show turtlesim/Spawn
  ```

  - `rosservice` is for interacting with services that are currently offered by some node.
  - `rossrv` - whose name comes from the `.srv` extension - is for asking about service data types, whether or not any currently available service has that type.
  - The different is similar to the **difference between the `rostopic` and `rosmsg`** commands.

- request, response両方空でも可能。例えばin the `/reset` service offered by `turtlesim_node`, which has type `std_srvs/Empty`, both the request and response parts are empty.

- Calling services from the command line

  ```bash
  rosservice call service-name request-content
  
  # create a new turtle named "Mikey", at position (3, 3), facing angle 0.
  rosservice call /spawn 3 3 0 Mikey
  ```

## 8.3 A client program

```c++
ros::ServiceClient client = node_handle.serviceClient<service_type>(service_name);
```

- *service_name* **should be a relative name**, but can also be a global name. 改善点：*service_name*をrelative nameに変えましょう。

- Creating a `ros::ServiceClient` does not require a queue size, in contrast to the analogous `ros::Publisher`.
  - Because the client waits for the service call to complete, there is no need to maintain a queue of outgoing service calls.

- A common mistake is to fail to check the return value of `call`.

- By default, the process of finding and connecting to the server node occurs inside the `call` method.

  - This connection is used for that service call and then closed before `call` returns.

- **Declaring a dependency**.

  - To get `catkin_make` to correctly compile a client program, we must be sure that **the program's package declares a dependency on the package that owns the service type**.

  - Require edits to `CMakeLists.txt` and to the manifext, `package.xml`. 僕の経験だと、確かに`package.xml`に依頼する他のpackage名を指定しないと、コンパイルできない。

  - ```cmake
    find_package(catkin REQUIRED COMPONENTS roscpp turtlesim)
    ```

  - ```xml
    <build_depend>turtlesim</build_depend>
    <run_depend>turtlesim</run_depend>
    ```

## 8.4 A server program

- Writing a service callback

  ```c++
  bool function_name(
      package_name::service_type::Request &req,
  	package_name::service_type::Response &resp){...}
  ```

- Creating a server object

  ```c++
  ros::ServerServer server = node_handle.advertiseService(
      service_name, pointer_to_callback_function);
  ```

  - `ros::NodeHandle::advertiseService` refuses to accept private names.

  - We can create `ros::NodeHandle` objects with their own specific default namespaces.

    ```c++
    ros::NodeHandle nhPrivate("~");
    ```

    - The default namespace for any relative names we send to this NodeHandle would then be the same as the node's name.

- **Giving ROS control**: ROS will not execute **any callback** functions until we specifically ask it to, using `ros::spin()` or `ros::spinOnce()`. serviceの場合忘れそう。

 ### spinOnceのwhile loopのsleepによってservice callbackの実行が遅延することの対策

- multi-thread: We can use two separate threads: one to publish messages, and one to handle service callbacks.
  - Although ROS doesn't require programs to use threads explicitly, it is quite cooperative if they do.

- We can replace the `sleep/ros::spinOnce` loop with a `ros::spin`, and use a **timer callback** to publish messages.
  - つまりtimer callbackの周期はservice callbackに影響しない？