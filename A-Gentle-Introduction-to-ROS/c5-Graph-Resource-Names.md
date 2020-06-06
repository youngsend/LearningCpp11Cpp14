## 5.1 Global names

- **Nodes, topics, services, and parameters** are collectively referred to as **graph resources**.
  - Each graph resource is **identified** by a **short string** called a **graph resource name**.  
  - Both `rosnode info` and `ros::init` expect node names.
  - Both `rostopic echo` and the constructor for `ros::Publisher` expect topic names.

- Global namesの構成
  - a leading slash /.
  - **a sequence of zero or more namespaces**, separated by slashes.
    - **Namespaces are used to group related graph resources together**.
    - **Global names that don't explicitly mention any namespace** are said to be in the **global namespace**.
  - a **base name** that describes the resource itself.
- The real advantage of this naming system comes from the use of relative names and private names. できるだけrelative namesやprivate namesを使いましょう！

## 5.2 Relative names

- 定義：The main alternative to providing a global name, which includes a **complete specification of the namespace in which the name lives**, is to allow ROS to supply a **default namespace**.
  - relative names cannot be matched to specific graph resources unless we know the default namespace that ROS is using to resolve them.

### Setting the default namespace

- This default namespace is tracked individually **for each node**, rather than being a system-wide setting.
  - If you don't set the default namespace, then ROS will use the global namespace (/).
- The **best and most common** method for choosing a different default namespace for a node or group of nodes is to **use `ns` attributes in a launch file**. 今まで僕一回もlaunch fileに`ns`を指定したことない！

relative namesのメリット：easily push that node and the topics it uses down into a namespace that the node's original designers might not necessarily anticipate.

- prevent name collisions when groups of nodes from different sources are combined.

良い実践：when writing nodes, it's recommended to **avoid using global names**, except in the unusual situations where there is a very good reason to use them. 改善点：コードの中のトピック名はほとんどglobal namesになっている！

## 5.3 Private names

- node name + private name -> global name.

- private nameは~で開始。これは確かにteleop_twist_keyboard.pyの中にみた。https://github.com/ros-teleop/teleop_twist_keyboard/blob/master/teleop_twist_keyboard.py

  ```python
  speed = rospy.get_param("~speed", 0.5)
  turn = rospy.get_param("~turn", 1.0)
  ```

- Private names are often used for **parameters** - **`roslaunch` has a specific feature for setting parameters that are accessible by private names** and services that govern the operation of a node.
  - serviceでの使い方はまだ分かっていない！
- でもnode nameが分かったら、誰もそのprivate nameの値をアクセスできる。だから本当のprivateじゃないよ。

## 5.4 Anonymous names

唯一node名を得るmechanism。

```c++
ros::init(argc, argv, base_name, ros::init_options::AnonymousName);
```

- free to run as many simultaneous copies of that program as we like, knowing that each will be assigned a unique name when it starts.