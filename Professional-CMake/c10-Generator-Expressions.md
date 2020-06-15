## なぜConfigurationやGenerationの2ステップに分ける？

- Consider a project processed for a **multi configuration** CMake generator like Xcode or Visual Studio.
- When the CMakeLists.txt files are being read, CMake **doesn't know which configuration a target will be built for**.
- It is a **multi configuration setup**, so there's **more than one choice** (e.g. Debug, Release, etc.)
- The developer selects the configuration at *build* time, well after CMake has finished.
- Some kind of placeholder is needed to tell CMake "For whichever configuration is being built, use the directory of the final executable".
  - このplaceholderはgenerator expressions。
- Generator expressionsの仕事：provide a way to **encode some logic** which is **not evaluated at configure time**, the evaluation is instead **delayed until the generation phase** when the project files are being written.

## 10.1 Simple Boolean Logic

| Expression                    | Result                                             | Notes                        |
| ----------------------------- | -------------------------------------------------- | ---------------------------- |
| `$<1:foo>`                    | foo                                                |                              |
| `$<0:foo>`                    |                                                    |                              |
| `$<true:foo>`                 |                                                    | Error, not a 1 or 0          |
| `$<$<BOOL:true>:foo>`         | foo                                                |                              |
| `$<$<NOT:0>:foo>`             | foo                                                |                              |
| `$<$<NOT:1>:foo>`             |                                                    |                              |
| `$<$<NOT:true>:foo>`          |                                                    | Error, NOT requires a 1 or 0 |
| `$<$<AND:1,0>:foo>`           |                                                    |                              |
| `$<$<OR:1,0>:foo>`            | foo                                                |                              |
| `$<1:$<$<BOOL:false>:foo>>`   |                                                    |                              |
| `$<IF:$<BOOL:${foo}>,yes,no>` | Result will be `yes` or `no` depending on `${foo}` |                              |

- 最終的にboolean generator expressionsは2つしかない：`$<1:foo>`、`$<0:foo>`。前者の結果はfoo、後者の結果は空。その他のexpressionは全部1, 0の代わりとして使う。上記以外の1か0に評価されるexpressionsはまた：

  ```cmake
  $<STREQUAL:string1,string2>
  $<EQUAL:number1,number2>
  $<VERSION_EQUAL:version1,version2>
  $<VERSION_GREATER:version1,version2>
  $<VERSION_LESS:version1,version2>
  ```

- **`$<CONFIG:arg>`**: This will evaluate to 1 if `arg` corresponds to the build type actually being built.

  - Common uses of this would be to **provide compiler flags only for debug builds** or to select different implementations for different build types. 改善点：Debug modeのやり方にこれを利用できそう！

    ```cmake
    add_executable(myApp src1.cpp src2.cpp)
    
    # Before CMake 3.8
    target_link_libraries(myApp PRIVATE
    	$<$<CONFIG:Debug>:checkedAlgo>
    	$<$<NOT:$<CONFIG:Debug>>:fastAlgo>)
    	
    # CMake 3.8 or later allows a more concise form
    target_link_libraries(myApp PRIVATE $<IF:$<CONFIG:Debug>,checkedAlgo,fastAlgo>)
    ```

    - link the executable to the `checkedAlgo` library for `Debug` builds and to the `fastAlgo` library for all other build types.
    - The `$<CONFIG:...>` generator expression is **the only way** to robustly provide such functionality which works for all CMake project generators.

## 10.2 Target Details

- 一般的なgenerator expressions:

  ```cmake
  $<TARGET_PROPERTY:target,property>
  $<TARGET_PROPERTY:property>
  ```

- もっと具体的なgenerator expressions: `TARGET_FILE, TARGET_FILE_NAME, TARGET_FILE_DIR`.
  - `TARGET_FILE_DIR` is the most robust way to obtain the directory in which the final executable or library is built. 特にmulti configuration generatorの場合意味ある。
  - especially useful when defining custome build rules for copying files around in post build steps.

## object library

- 意味：It isn't a library in the usual sense, it is just a collection of object files that CMake associates with a target but **doesn't actually result in a final library file being created**. 確かにpath visualizerはobject libraryになっているそうです。

  ```cmake
  # Define an object library
  add_library(objLib OBJECT src1.cpp src2.cpp)
  
  # Define two executables which each have their own source
  # file as well as the object files from objLib
  add_executable(app1 app1.cpp $<TARGET_OBJECTS:objLib>)
  add_executable(app2 app2.cpp $<TARGET_OBJECTS:objLib>)
  ```

  - no separate library is created for `objLib`（object libraryだから）, but the `src1.cpp` and `src2.cpp` source files are still only compiled once.

## 10.3 General Information

- `$<CONFIG>`: build type.
  - Use this **in preference to the `CMAKE_BUILD_TYPE`** variable since that variable is **not used on multi configuration project generators** like Xcode or Visual Studio.

- `$<PLATFORM_ID>`: platform.
  - useful in **cross-compiling** situations.
  - 似てる機能の`CMAKE_SYSTEM_NAME`.

- `$<C_COMPILER_VERSION>, $<CXX_COMPILER_VERSION>`.

  ```cmake
  # produce the string OLD_COMPILER if the C++ compiler version is less than 4.2.0
  $<$<VERSION_LESS:$<CXX_COMPILER_VERSION>,4.2.0>:OLD_COMPILER>
  ```

## 10.4 Utility Expressions

- `$<COMMA>, $<SEMICOLON>, $<LOWER_CASE:...>, $<UPPER_CASE:...>`.
- `$<JOIN:list,...>`: replace each semicolon in `list` with the `...` content, effectively joining the list items with `...` between each other.

## 良い実践

- generator expressionを利用するシーン、例えば、project logic which tries to do different things for different build types.
- また、conditionally including a source file depending on the build type can be done relatively concisely, with `$<CONFIG>`.
- As always, developers should favor clarity over cleverness and this is especially true with generator expressions.