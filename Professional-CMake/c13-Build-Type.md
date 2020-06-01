- build typeの仕事：high level control which **selects different sets of compiler and linker behavior**.
  - Each build type results in **a different set of compiler and linker flags**.
  - may also change other behaviors, such as **altering which source files get compiled or what libraries to link to**.
    - 確かにmap_managerはrelease時沢山のtest用コードはコンパイルする必要がない！

## 13.1 Build Type Basics

- 開発者は大体buildの内容がdebugもしくはreleaseのどっちかと思っている。
  - debug build: **compiler flags** are used to enable the **recording of information** that debuggers can use to **associate machine instructions with the source code**. つまりコンパイル後のmachine instructionsとsource codeの対応関係を記録して、そうすると、実行中あるmachine instructionにエラーが発生したら、すぐそのエラーが出るsource codeへ飛べる。
    - no optimizations.
  - release build: generally has full optimizations enabled and no debug information generated.
- 上の２つ以外：
  - `RelWithDebInfo`: compromise（妥協） of debug and release build.
    - Most optimizations for speed are typically applied, but most debug functionality is also enabled.
    - もっとも役に立つ場合：when the performance of a Debug build is not acceptable even for a debugging session.
    - 注意：the default settings for `RelWithDebInfo` will **disable assertions**.
  - `MinSizeRel`: typically only used for **constrained resource environments such as embedded devices**.
    - The code is optimized for size rather than speed and no debug information is created.

## 13.1.1 Single Configuration Generators

- Some, like **Makefiles and Ninja**, support only a single build type per build directory.

  - For these generators, the build type has to be chosen by setting the `CMAKE_BUILD_TYPE` **cache variable**. 例えば：

    ```bash
    cmake -G Ninja -DCMAKE_BUILD_TYPE:STRING=Debug ../source
    cmake --build .
    ```

- switching between different build typesの代わりに、**set up separate build directories for each build type**, all still using the same sources.![image-20200601103907803](img\separate-build-directories-20200601103907803.png)
  - It allows a single configuration generator to effectively act like a multi configuration generator.

## 13.1.2 Multiple Configuration Generators

- Some generators, notably **Xcode and Visual Studio**, support multiple configurations in a single build directory.

  - These generators **ignore the `CMAKE_BUILD_TYPE`** cache variable and instead require the developer to choose the build type within the IDE or with a command line option at build time. 例えば：

    ```bash
    cmake -G Xcode ../source
    cmake --build . --config Debug
    ```

    - そういえば、CLionのCMakeには`CMAKE_BUILD_TYPE` cache variableを使っているので、**CLionはsingle configuration generatorを使っているんだ、Unix Makefilesを使っている（Makefilesの一種だ）**。

  - Both environments keep separate directories for the different build types, so switching between builds doesn't cause constant rebuilds.

## 13.2 Common Errors

- 注意：singleとmultiple configuration generatorのbash例を比較すると、

  - single configuration generators, the build type is specified at **configure time**.

  - multi configuration generators, the build type is specified at **build time**.

  - 意味：the build type is **not always known when CMake is processing a project's CMakeLists.txt** file. 例えば下記のCMake codeはmulti configuration generatorsに対して意味ない：

    ```cmake
    # WARNING: Do not do this!
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    	# Do something only for debug builds
    endif()
    ```

    - work fine for **Makefile-based generators and Ninja**, but **not for Xcode or Visual Studio**.
    - 注意：**any logic based on `CMAKE_BUILD_TYPE` within a project is questionable unless it is protected by a check to confirm a single configuration generator is being used.**
    - Rather than referring to CMAKE_BUILD_TYPE in the CMakeLists.txt file, projects should instead use other more robust alternative techniques, such as **generator expressions based on `$<CONFIG:...>`**. *Generator Expressions*は第十章。

- scripting buildsの時generatorのタイプは想定しない！両方対応できるように、例えば：

  ```bash
  mkdir build
  cd build
  cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ../source
  cmake --build . --config Release
  ```

  - メリット：a developer could **simply change the generator name** given to the -G parameter and the rest of the script would work unchanged.

- single configuration generatorにはもしCMAKE_BUILD_TYPEを設定しなかったら、build typeは**empty**、これは必ずDebugとは限らない。
- 結論というと、ただCMakeLists.txtに`CMAKE_BUILD_TYPE`を設定するのが、single configuration generatorのみに有効。もっといいのはbuild scriptに設定するんだ！
  - でもcatkinの場合、特にbuild scriptは使ってないが、どうする？

## 13.3 Custom Build Types

- multi configuration generatorsの場合：

  - `CMAKE_CONFIGURATION_TYPES` cache variableでcontrolしている。

  - The first encountered `project()` command populates the cache variable with a default list if it has not been defined, but projects may **modify** the **non-cache variable of the same name** after that point (注意：**modifying the cache variable is unsafe since it may discard changes made by the developer**、普段non-cache variableの優先度高いから).

  - 注意：avoid setting CMAKE_CONFIGURATION_TYPES if it is not already defined. 使う前に、check if CMAKE_CONFIGURATION_TYPES was non-empty.

    - CMake 3.9 added a new **GENERATOR_IS_MULTI_CONFIG** global property which is set to true when a multi configuration generator is being used, providing a definitive way to obtain that information instead of relying on inferring it from CMAKE_CONFIGURATION_TYPES.
    - For better robustness, it is still recommended to use **at least CMake 3.11 if custom build types are going to be defined**.
      - prior to CMake 3.11, certain parts of CMake only accounted for the default build typesから。

  - 開発者が`CMAKE_CONFIGURATION_TYPES`をいじれるから、projects should therefore **not make any assumptions about what configuration types are or are not defined**.

  - multi configuration generatorsのcustom build typesの良い実践：

    ```cmake
    cmake_minimum_required(3.11)
    project(Foo)
    
    get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(isMultiConfig)
    	if(NOT "Profile" IN_LIST CMAKE_CONFIGURATION_TYPES)
    		list(APPEND CMAKE_CONFIGURATION_TYPES Profile)
    	endif()
    endif()
    
    # Set relevant Profile-specific flag variables if not already set...
    ```

- single configuration generatorsの場合：

  - 機能：cache variables can **have their STRINGS property defined** to **hold a set of valid values**. *第九章Properties*

    - The CMake GUI application will then present that variable as a combo box containing the valid values instead of as a text edit field.

    ```cmake
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
    			STRINGS Debug Release Profile)
    ```

    - **Properties can only be changed from within the project's CMakeLists.txt** files, なので、この設定は安全安心。
    - しかしこの設定は何の選択範囲の制限もない。Developers can still set CMAKE_BUILD_TYPE to any value at the cmake command line or edit the CMakeLists.txt file manually.

- single, multi両方対応できるように、例えば

  ```cmake
  cmake_minimum_required(3.11)
  project(Foo)
  
  get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if(isMultiConfig)
  	if(NOT "Profile" IN_LIST CMAKE_CONFIGURATION_TYPES)
  		list(APPEND CMAKE_CONFIGURATION_TYPES Profile)
  	endif()
  else()
  	set(allowableBuildTypes Debug Release Profile)
  	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
  				STRINGS "${allowableBuildTypes}")
  	if(NOT CMAKE_BUILD_TYPE)
  		set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
  	elseif(NOT CMAKE_BUILD_TYPE IN_LIST allowableBuildTypes)
  		message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
  	endif()
  endif()
  
  # Set relevant Profile-specific flag variables if not already set...
  ```

- build typeの選定の効果：it specifies which **configuration-specific variables** CMake should use and is also affects any **generator expressions** whose logic depends on the current configuration (i.e. `$<CONFIG>` and `$<CONFIG:...>`).

  - 具体的に下記の２種類variables：

    - `CMAKE_<LANG>_FLAGS_<CONFIG>`.
    - `CMAKE_<TARGETTYPE>_LINKER_FLAGS_<CONFIG>`.

  - 例えば：

    ```cmake
    set(CMAKE_C_FLAGS_PROFILE				"-p -g -02" CACHE STRING "")
    set(CMAKE_CXX_FLAGS_PROFILE				"-p -g -02" CACHE STRING "")
    set(CMAKE_EXE_LINKER_FLAGS_PROFILE		"-p -g -02" CACHE STRING "")
    set(CMAKE_SHARED_LINKER_FLAGS_PROFILE	"-p -g -02" CACHE STRING "")
    set(CMAKE_STATIC_LINKER_FLAGS_PROFILE	"-p -g -02" CACHE STRING "")
    set(CMAKE_MODULE_LINKER_FLAGS_PROFILE	"-p -g -02" CACHE STRING "")
    ```

    - `"-p -g -02"`の意味はturn on profiling as well as enabling debugging symbols and most optimizations.

    - もう１つ設定方法：base the compiler and linker flags on one of the other build types and add the extra flags needed. 例えば、base on RelWithDebInfo build type（profilingの部分）:

      ```cmake
      set(CMAKE_C_FLAGS_PROFILE
      	"${CMAKE_C_FLAGS_RELWITHDEBINFO} -p" CACHE STRING "")
      ```

- **Each custom configuration should have the associated compiler and linker flag variables defined**. これは難しそう。

  - もう１つよくcustom build typeに含まれるvariable: `CMAKE_<CONFIG>_POSTFIX`.

    - initialize the `<CONFIG>_POSTFIX` property of each **library target**. 名前の通りpostfixだ。

    ```cmake
    set(CMAKE_PROFILE_POSTFIX _profile)
    ```

    - 効果：allows **libraries from multiple build types to be put in the same directory **without overwriting each other.
    - `CMAKE_DEBUG_POSTFIX` is often set to values like `d` or `_debug`.
    - By convention, the postfix for Release builds is typically empty.

## 良い実践

- 特定のCMake generatorを想定しない。
- single configuration generatorの場合、build type毎にbuild directoryを用意する。
- CMAKE_BUILD_TYPEに基づくロジックを避ける。代わりに、**generator expressionsでやる**。
- Generator expressions like `${TARGET_FILE:...}` should be used to provide the required path for all generators, whether they be single or multi configuration.