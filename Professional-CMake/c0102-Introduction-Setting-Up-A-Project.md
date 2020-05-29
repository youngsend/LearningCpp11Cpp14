## Chapter 1. Introduction

- **Compilers, linkers, testing frameworks, packaging systems** and more all contribute to the complexity of deploying high quality, robust software.

- CMake is a **suite of tools** which covers everything from setting up a build right through to producing packages ready for distribution.

  ![](/home/sen/senGit/LearningCpp11Cpp14/Professional-CMake/img/start-to-end-process-2020-05-29 20-09-49.png)
  - CTest, CPackもCMakeに含まれる。

## Chapter 2. Setting Up A Project

- CMake brings some order to this, starting with a human-readable file called **CMakeLists.txt** that defines what should be built and how, **what tests to run** and what package(s) to create.
  - This file is a **platform independent description of the whole project**, which CMake then turns into platform specific build tool project files.
  - This is what **controls everything** that CMake will do in setting up and performing the build.

### projectの構成

- source directory: **under version control**.
  - CMakeLists.txt
  - the project's source files and all other files needed for the build.
- binary directory (**build directory**)
  - CMake, the chosen build tool (例えばmake, Visual Studio), CTest, CPack will all create various files.
  - Executables, libraries, **test output** and packages.
  - CMakeCache.txt
    - CMake uses this file to save details so that if it is run again, it can re-use information computed the first time and **speed up** the project generation.
  - The build tool's project files (例えばXcode or Visual Studio project files, Makefiles).
- source directoryとbuild directoryは2種類の関係がある：**in-source** and **out-of-source** builds.
  - in-source buildは**discouraged**. ごちゃごちゃ。

![](/home/sen/senGit/LearningCpp11Cpp14/Professional-CMake/img/project-directory-2020-05-29 21-55-48.png)

### 2.2 Out-of-source Builds

- メリット：developer can create **multiple build directories** for the same source directory, which allows builds to be set up with different sets of options, such as **debug and release versions**, etc.

### 2.3 Generating Project Files

- The developer selects the type of project file to be created by choosing a particular **project file *generator***.

![](/home/sen/senGit/LearningCpp11Cpp14/Professional-CMake/img/project-file-generators-2020-05-29 22-03-48.png)

- Multi-config、例えばDebug, Release. re-run CMakeの必要がない。

- Single-configはsimpler and often have good support in IDE environments not so closely associated with a particular compiler (**Qt Creator**, KDevelop, etc.). DebugとReleaseに切り替える時CMakeをre-runする必要がある。

- 基本のCMakeのrun方法：

  ```bash
  mkdir build
  cd build
  cmake -G "Unix Makefiles" ../source
  ```

  - `cmake`後、Generatorのタイプやsource directoryのパスを指定する。

- 2 steps: Configuring and Generating.

  - Configuring: CMake reads in the CMakeLists.txt file and builds up an **internal representation of the entire project**.
  - ConfiguringとGeneratingの分離の意味はChapter 10, *Generator Expressions*へ.

### 2.4 Running The Build Tool

- cmakeでbuilt toolを呼び出す：**`cmake --build /some/path/build --config Debug --target MyApp`**.

  - build toolは特に指定してない、2.3のGeneratorによって決まったらしい。（the correct build tool will be automatically invoked.）
  - CLionは確かにUnix Makefilesを使っている。

- 2番めパラメータ`--config`：For multi configuration generators, the `--config` specifies which configuration to build, whereas **single configuration generators will ignore the `--config` option and rely instead on information provided when the CMake project generation step was performed.**

  - single configuration generatorは、ここの`--config`を無視、2.3のProject Generationステップで使ったConfigを使う。

- 2.3, 2.4を合わせて、簡単な**scripted** build:

  ```bash
  mkdir build
  cd build
  cmake -G "Unix Makefiles" ../source
  cmake --build . --config Release --target MyApp
  ```

### Recommended Practices

- Use different project generators for the different build directories, such as Unix Makefiles and Xcode.
  - This can help to catch any **unintended dependencies on a particular build tool** or to check for differing compiler settings between generator types.
- **Periodically checking the build with a different project generator** than the one a developer usually uses can save considerable future pain by **discouraging generator-specific code** where it isn't required.
- A good strategy is to ensure the project builds with the default generator type on each **platform** of interest, plus one other type.
  - The **Ninja generator** is an excellent choice for the latter, since it has the **broadest platform support of all the generators** and it also creates very efficient builds.