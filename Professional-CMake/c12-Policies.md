Policiesが出る時：Policies are not something that developers are exposed to all that often, mostly just when CMake issues a **warning** about the project **relying on an older version's behavior**.

## 12.1 Policy Control

- CMake's policy functionality is **closely tied to the `cmake_minimum_required()`** command.

  - Set **CMake's behavior** to **match** that of the version given.
  - When a project starts with `cmake_minimum_required(VERSION 3.2)`, it says that at least CMake 3.2 is needed and also that the **project expects CMake to behave like the 3.2 release**.

- しかし、a project may want more fine-grained control than what the `cmake_minimum_required()` command provides.

- More specific control over policies is enabled through the `cmake_policy()` command.

- coarsest level:

  ```cmake
  cmake_policy(VERSION major[.minor[.patch[.tweak]]])
  ```

  - The `cmake_minimum_required()` command **implicitly call this form** to set CMake's behavior.

  - Enforce a particular version's behavior for a section of the project:

    ```cmake
    cmake_minimum_required(VERSION 3.7)
    project(WithLegacy)
    
    # Uses recent CMake features
    add_subdirectory(modernDir)
    
    # Imported from another project, relies on old behavior
    cmake_policy(VERSION 2.8.11)
    add_subdirectory(legacyDir)
    ```

- CMake 3.12からrange basedもできる

  ```cmake
  cmake_minimum_required(VERSION 3.7...3.12)
  cmake_policy(VERSION 3.7...3.12)
  ```

  - 意味：The CMake version in use must be at least the minimum and the behavior to use should be the lesser of the specified maximum and the running CMake version.

- CMakeが定義しているpolicyの選択: NEW / OLD

  ```cmake
  # Allow non-existent target with get_target_property()
  cmake_policy(SET CMP0045 OLD)
  
  # Would halt with an error without the above policy change
  get_target_property(outVar doesNotExist COMPILE_DEFINITIONS)
  ```

  ```cmake
  cmake_minimum_required(VERSION 3.0)
  project(PolicyExample)
  
  if(CMAKE_VERSION VERSION_GREATER 3.1)
  	# Enable stronger checking of break() command usage
  	cmake_policy(SET CMP0055 NEW)
  endif()
  ```

  - もっと便利な実現：

    ```cmake
    cmake_minimum_required(VERSION 3.0)
    project(PolicyExample)
    
    # Only set the policy if the version of CMake being used knows about that policy number
    if(POLICY CMP0055)
    	cmake_policy(SET CMP0055 NEW)
    endif()
    ```

## 12.2 Policy Scope

- またstackのやり方：policy stack. resetがないよね、modulesのCMAKE_REQUIRED_...変数と違うところ。でも同じくよくmodule fileに使われる：

  ```cmake
  # Save existing policy state
  cmake_policy(PUSH)
  
  # Set some policies to OLD to preserve a few old behaviors
  cmake_policy(SET CMP0060 OLD) # Library path linking behavior
  cmake_policy(SET CMP0021 OLD) # Tolerate relative INCLUDE_DIRECTORIES
  
  # Do various processing here...
  
  # Restore earlier policy settings
  cmake_policy(POP)
  ```

-  他の裏にpolicyのpush, popをやっているコマンド：`add_subdirectory(), include(), find_package()`.
  - `find_package()`: push and pop upon starting and finishing processing of its associated `FindXXX.cmake` module file respectively.

## 良い実践

- Where possible, projects should prefer to work with policies at the CMake version level rather than manipulating specific policies.

- In cases where a project does need to manipulate **a specific policy**, it should check whether the policy is available using `if(POLICY...)` rather than testing the `CMAKE_VERSION` variable. `CMAKE_VERSION`を利用する場合は、本当にCMake versionに触りたい時だ。

  ```cmake
  # Version-level policy enforcement
  if(NOT CMAKE_VERSION VERSION_LESS 3.4)
  	cmake_policy(VERSION 3.4)
  endif()
  
  # Individual policy-level enforcement
  if(POLICY CMP0055)
  	cmake_policy(SET CMP0055 NEW)
  endif()
  ```

- If a project needs to manipulate **multiple individual policies locally**, surround that section with calls to `cmake_policy(PUSH)` and `cmake_policy(POP)` to ensure that the rest of the scope is isolated from the changes.
  - 間に`return()`を使わないで！