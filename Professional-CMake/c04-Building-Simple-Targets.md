- `add_executable(myApp main.cpp)`はassumes the developer wants a basic **console executable** to be built, but CMake also allows the developer to define other types of executables, such as **app bundles on Apple platforms** and **Windows GUI applications**.
- CMakeの他のtargetに関する機能：（targetは大体executableとlibraryの2種類でしょう）
  - **build and link libraries**.
    - CMakeがsupportするlibraries種類：**static, shared, modules, and frameworks**.
  - **manage dependencies between targets** and **how libraries are linked**.

## 4.1 Executables

```cmake
add_executable(targetName [WIN32] [MACOSX_BUNDLE]
				[EXCLUDE_FROM_ALL]
				source1 [source2 ...])
```

- `WIN32`: it will be created with a `WinMain()` **entry point** instead of just `main()` and it will be linked with the `/SUBSYSTEM:WINDOWS` option.
  - Windows以外のplatforms: the WIN32 option is ignored.

- `MACOSX_BUNDLE`: it applies not just to macOS, but also to other Apple platforms like iOS. *Section 22.2 Application Bundles*
  - On non-Apple platforms, the MACOSX_BUNDLE keyword is ignored.
- other forms of `add_executable()`: produce a kind of reference to an **existing executable or target** rather than defining a new one to be built. *Chapter 16, Target Types*.
- `EXCLUDE_FROM_ALL`: When no target is specified at build time (`cmake --build . --config Release --target MyApp`, つまり`--target MyApp`がない場合), the default ALL target is built.

## 4.2 Defining Libraries（大事）

- For many **larger projects**, the ability to create and work with libraries is also essential to **keep the project manageable**.

```cmake
add_library(targetName [STATIC | SHARED | MODULE]
			[EXCLUDE_FROM_ALL]
			source1 [source2 ...])
```

- `STATIC`: a static library or **archive**.
  - Windows: `targetName.lib`
  - **Unix-like: `targetName.a`**

- `SHARED`: a shared or **dynamically linked library**.

  - Windows: `targetName.dll`
  - Apple: `libtargetName.dylib`
  - Unix-like: `libtargetName.so`
  - On Apple platforms, shared libraries can also be marked as frameworks, *Section 22.3, Frameworks*.

- `MODULE`: somewhat like a shared library, but intended to be **loaded dynamically at run-time** rather than being linked directly to a library or executable.

  - Pythonのimportみたい。

- Preferred practice is to not specify the library type and leave the choice up to the developer **when building the project**.

  - `BUILD_SHARED_LIBS`というCMake変数で指定する。trueだったら、libraryは`SHARED`になる。じゃなかったら、`STATIC`になる。

  - 設定方法１：**including a `-D` option** on the cmake command line:

    ```bash
    cmake -DBUILD_SHARED_LIBS=YES /path/to/source
    ```

  - 設定方法２(less flexible)：set in CMakeLists.txt, placed **before any `add_library()` commands**.

    ```cmake
    set(BUILD_SHARED_LIBS YES)
    ```

## 4.3 Linking Targets

- library間のdependency関係の3種類：
  - `PRIVATE`: library A uses library B in its own **internal implementation**.
    - **Anything else that links to library A** doesn't need to know about B because it is an internal implementation detail of A.
    - 誰がlibrary Aにlinkしても、library Bにはlinkしない。
  - `PUBLIC`: not only does library A use library B internally, it also **uses B in its interface**. （内部実現、interface両方library Bを使っている）
    - **A cannot be used without B**, so anything that uses A will also have a direct dependency on B.
    - 例えば、a function defined in library A which has **at least one parameter of a type** defined and implemented in library B, so code cannot call the function from A without providing a parameter whose type comes from B. Library Aのinterfaceの引数のタイプはLibrary Bに定義されている。
    - 誰がlibrary Aにlinkしたら、自動的にlibrary Bにもlinkする。
  - `INTERFACE`: library A doesn't require B internally, it **only uses B in its interface**. (*Chapter 16, Target Types*)

```cmake
target_link_libraries(targetName
		<PRIVATE|PUBLIC|INTERFACE> item1 [item2 ...]
		[<PRIVATE|PUBLIC|INTERFACE> item3 [item4 ...]]
		...)
```

例：（`target_link_libraries`のtargetNameは既に作ったlibraryもしくはexecutableの名前だ）

```cmake
add_library(collector src1.cpp)
add_library(algo src2.cpp)
add_library(engine src3.cpp)
add_library(ui src4.cpp)
add_executable(myApp main.cpp)

target_link_libraries(collector
	PUBLIC ui
	PRIVATE algo engine)
target_link_libraries(myApp PRIVATE collector)	
```

- 説明：
  - `myApp` will also be linked to `ui` because of the `PUBLIC` relationship between `collector` and `ui`.
  - `myApp` will not be directly linked to `algo` and `engine`.

## 4.4 Linking Non-targets

- つまりexisting **CMake targets**以外のlibraryもlinkできる。（exisiting CMake targetsは、`add_library`で作ったlibrary）2種link方法：
  - Full path to a library file.
  - Plain library name.
    - the linker command will **search** for the library. This would be **common** for libraries **provided by the system**.

- The `debug`, `optimized`, `general` keywords should be **avoided** for new projects as there are clearer, more flexible and more robust ways to achieve the same thing with today's CMake features.

## 良い実践

- Target names need not be related to the project name.

  - 悪い実践：

    ```cmake
    set(projectName MyExample)
    project(${projectName})
    add_executable(${projectName} ...)
    ```

  - **Set the project name directly** rather than via a variable, choose a target name according to what the target does rather than the project it is part of and assume the project will eventually need to define more than one target.

- When naming targets for libraries, **resist** the temptation to start or end the name with *lib*.

- The `BUILD_SHARED_LIBS` variable can be used to change the default **in one place** instead of having to modify every call to `add_library()`. 前提：`STATIC/SHARED` keywordを指定しない。
- いつも`PRIVATE/PUBLIC/INTERFACE`を指定する（`target_link_libraries()`）。