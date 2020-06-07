Two fundamental CMake commands in any multi-directory project are `add_subdirectory()` and `include()`: bring content from another file or directory into the build. メリット：

- Build logic is localized, meaning that characteristics of the build can be defined in the **directory where they have the most relevance**. 他のどこのdirectoryもaddできるから。

- Builds can be composed of subcomponents which are defined **independently** from the top level project consuming them.
  - This is especially important if a project makes use of things like **git submodules** or **embeds third party source trees**.
- Turn parts of the build on or off.

## 7.1 `add_subdirectory()`

- The `add_subdirectory()` command allows a project to **bring another directory into the build**.
  - That directory **must have its own CMakeLists.txt** file which will be **processed at the point where `add_subdirectory()` is called** and **a corresponding directory** will be created for it **in the project's build tree**.

```cmake
add_subdirectory(sourceDir [binaryDir] [EXCLUDE_FROM_ALL])
```

- The sourceDir **does not have to be a subdirectory within the source tree**, although it usually is.
  - Any directory can be added, with sourceDir being specified as either an absolute or **relative path**, the latter being **relative to the current source directory**.
  - Absolute paths are typically only needed when adding directories that are outside the main source tree.
- If sourceDir is a path outside the source tree, CMake requires the binaryDir to be specified since a corresponding relative path can no longer be constructed automatically.

## 7.1.1 Source and Binary Directory Variables

- `CMAKE_SOURCE_DIR`: the top-most directory of the source tree.
  - where the top-most CMakeLists.txt file resides.
- `CMAKE_BINARY_DIR`.
- `CMAKE_CURRENT_SOURCE_DIR, CMAKE_CURRENT_BINARY_DIR`: changes for every call to `add_subdirectory()` and is restored again when `add_subdirectory()` returns.

## 7.1.2 Scope

- One of the effects of calling `add_subdirectory()` is that CMake creates a new scope for processing that directory's CMakeLists.txt file. 

  - The new scope acts **like a child of the calling scope**.
  - Any new variable created in the child scope will not be visible to the calling scope. 普通の関数の考え方でしょう。
  - Any change to a variable in the child scope **is local to that child scope**. つまりcopyですね。referenceじゃない。
    - メリット：**It allows the added directory to change whatever variables it wants without affecting variables in the calling scope**.
    - This helps keep the calling scope isolated from potentially unwanted changes.

- child scopeでcalling scopeの変数を変更したい場合：

  ```cmake
  set(myVar value PARENT_SCOPE)
  ```

  - child scopeのmyVarに影響ない。

  - clearer set of commands:

    ```cmake
    set(localVar bar)
    set(myVar ${localVar} PARENT_SCOPE)
    ```

- It's not just variables that are affected by scope, **policies and some properties** also have similar behavior to variables in this regard.

## 7.2 `include()`

```cmake
include(fileName 	[OPTIONAL] [RESULT_VARIABLE myVar] [NO_POLICY_SCOPE])
```

`add_subdirectory()`と比較：

- `include()` expects the name of a file to read in, whereas `add_subdirectory()` expects a directory and will look for a CMakeLists.txt file within that directory.
  - The filename typically has the extension `.cmake`, but it can be anything.
- `include()` does not introduce a new variable scope.
- Both commands introduce a new policy scope by default, but the `include()` command can be told not to do so with the NO_POLICY_SCOPE option.

```cmake
include(module 		[OPTIONAL] [RESULT_VARIABLE myVar] [NO_POLICY_SCOPE])
```

Used to load the named module. *Chapter 11, Modules*.

`include()`に影響される変数：

- CMAKE_CURRENT_LIST_DIR: Analogous to CMAKE_CURRENT_SOURCE_DIR except it will be updated when processing the included file. `include()`中、included fileのdirectoryになる。absolute path.
- CMAKE_CURRENT_LIST_FILE: absolute path to the file.
- CMAKE_CURRENT_LIST_LINE: may prove useful in some debugging scenarios.
- これらの変数はどのファイルに対しても有効：the above three variables work for any file being processed by CMake, not just those pulled in by an `include()` command.
  - even for a CMakeLists.txt file pulled in via `add_subdirectory()`.

## 7.3 Project-relative Variables

Source and Binary Directory変数（一番topの変数）を使って、fileのpathを築く場合のtrouble：`${CMAKE_SOURCE_DIR}/someFile`は一時的に使えるが、if the project is later incorporated into another parent project by bringing it into the parent build via `add_subdirectory()`、`${CMAKE_SOURCE_DIR}`が変わるので、エラーになる。

- A similar trap exists for CMAKE_BINARY_DIR.

なので、project-relative変数を使う。`project()`コマンドが提供している。

- PROJECT_SOURCE_DIR: the source directory of the **most recent call to `project()`** in the current scope or any parent scope.
- PROJECT_BINARY_DIR.
- projectName_SOURCE_DIR: `project(projectName)`.
- projectName_BINARY_DIR.
- They can be used from any part of the directory hierarchy to robustly refer to any other directory in the project.

`${CMAKE_SOURCE_DIR}/someFile`の修正：`${PROJECT_SOURCE_DIR}/someFile` or `${projectName_SOURCE_DIR}/someFile`。

- regardless whether the project is being built stand-alone or being incorporated into a larger project hierarchy.

## 7.4 Ending Processing Early

- include the same file一回のみするように。included fileの中に

  ```cmake
  if(DEFINED cool_stuff_include_guard)
  	return()
  endif()
  
  set(cool_stuff_include_guard 1)
  # ...
  ```

  ```cmake
  # CMake 3.10以降
  include_guard()
  ```

  - **analogous to the #pragma once of C/C++**. なるほど！よくみてる！

## 良い実践

- As a general guide, most simple projects are probably better off preferring to use `add_subdirectory()` over `include()`.

- Irrespective of whether using `add_subdirectory(), include()` or a combination of both, the **CMAKE_CURRENT_LIST_DIR variable is generally going to be a better choice than CMAKE_CURRENT_SOURCE_DIR**.
- できるだけCMAKE_SOURCE_DIR, CMAKE_BINARY_DIRを使わない：these typically break the ability of the project to be incorporated into a larger project hierarchy.