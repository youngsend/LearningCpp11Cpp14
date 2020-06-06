## 7.1 Accessing parameters from the command line

- All parameters are "owned" by the parameter server rather than by any particular node.
  - This means that parameters - even those created with private names - will continue to exist even after the node they're intended for has terminated.

- ```bash
  rosparam list
  
  rosparam get parameter_name
  rosparam get namespace # retrieve the values of every parameter in a namespace
  
  # asking about the global namespace, see the values of every parameter
  rosparam get /
  ```

- ```bash
  # setting parameters: modify the values of existing parameters or create new ones.
  rosparam set parameter_name parameter_value
  ```

- ```bash
  # set several parameters in the same namespace at once
  rosparam set namespace values
  # values should be specified as a YAML dictionary, newline characterは必須。spaces after the colonsも重要。
  rosparam set /duck_colors "huey: red
  dewey: blue
  louie: green
  webby: pink"	# 改善点：launch fileに設定しているパラメータをYAMLファイルにしてみよう！
  
  # 上記は以下と等価
  rosparam set /duck_colors/huey red
  rosparam set /duck_colors/dewey blue
  rosparam set /duck_colors/louie green
  rosparam set /duck_colors/webby pink
  ```

- Creating and loading parameter files

  ```bash
  # store all of the parameters from a namespace to a file
  rosparam dump filename namespace
  
  rosparam load filename namespace
  ```

  - the combination of `dump` and `load` can be useful for testing.

- Updated parameter values are **not automatically "pushed" to nodes**.
  - we must be aware of how (or if) that node **re-queries** its parameters.
  - Quite often, but not for `turtlesim`, the answer is based on a subsystem called `dynamic_reconfigure`.

## 7.3 Accessing parameters from C++

```c++
void ros::param::set(parameter_name, input_value);
bool ros::param::get(parameter_name, output_value);
```

- *parameter_name* can be a **global, relative, or private name**.
- It is technically possible (but somewhat messy) to assign a private parameter to a node on its command line using a remap-like syntax, by prepending the name with an underscore: `_param-name:=param-value`. これはroslaunchで良く使っている。

## 7.4 Setting parameters in launch files

- Setting parameters:

  ```xml
  <group ns="duck_colors">
  	<param name="huey" value="red" />
      <param name="dewey" value="blue" />
      <param name="louie" value="green" />
      <param name="webby" value="pink" />
  </group>
  ```

- setting private parameters:

  ```xml
  <node ...>
  	<param name="param-name" value="param-value" />
  </node>
  ```

  - Parameter names given in `param` elements that are children of `node` elements are always resolved as private names, regardless of whether they begin with ~ or even /. 改善点：コードの中にprivate parameterをgetする時`~param-name`に変えましょう！

- Reading parameters from a file: 改善点：launch fileにある多数のパラメータをfileに移しましょう！

  ```xml
  <rosparam command="load" file="$(find package-name)/param-file" />
  ```

