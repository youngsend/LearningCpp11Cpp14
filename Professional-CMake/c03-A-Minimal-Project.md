```cmake
cmake_minimum_required(VERSION 3.2)
project(MyApp)
add_executable(myExe main.cpp)
```

- command names are case insensitive.
  - but the more common convention these days is to use all lowercase for command names.

## 3.1 Managing CMake Versions

```cmake
cmake_minimum_required(VERSION major.minor[.patch[.tweak]])
```

- The developers behind CMake are very careful to maintain **backwards compatibility** with each new release, so when users update to a newer version of CMake, projects should continue to build as they did before.
- `cmake_minimum_required()` commandの２つ仕事：
  - ensures that a particular minimum set of **CMake functionality** is available before proceeding.
  - enforces **policy settings** to match CMake behavior to the specified version. policy settingsの意味はまだ分かってない。*Chapter 12. Policies*
- In most projects, specifying the `patch` and `tweak` parts is not necessary, since **new features** typically **only appear in minor version updates**.
- Versionの決定：
  - 場合１：The greatest difficulty is typically experienced by projects that need to support older platforms where the **system-provided version of CMake** may be quite old.
    - For such cases, if at all possible, developers should consider **installing a more recent release** rather than restricting themselves to very old CMake version.
  - 場合２：If the project will itself be a **dependency for other projects**.
    - It may be beneficial to instead require the oldest CMake version that still provides the minimum CMake features needed, but make use of features from later CMake versions if available（やり方はChapter 12. Policies）.
- oldest workable versionを使うデメリット：more deprecation warnings.

## 3.2 The `project()` Command

```cmake
project(projectName
		[VERSION major[.minor[.patch[.tweak]]]]
		[LANGUAGES languageName ...])
```

- This name is used for the top level of a project with some project generators and it is also used in various other parts of the project, such as to act as defaults for packaging and documentation metadata, to provide project-specific variables and so on.
- A good habit to establish is to **define the project's version here** so that other parts of the project can refer to it.（詳細はChapter 19, Specifying Version Details）
- If no LANGUAGES option is provided, CMake will **default to C and CXX**.
  - programming languages that should be enabled for the project. Supported values include C, CXX, Fortran, ASM, Java and others.
- `project()`の仕事：
  - check the compilers for each enabled language and ensure they are able to compile and link successfully.
    - Problems with the compiler and linker setup are then caught very early.
  - set up a number of variables and properties which control the build for the enabled languages.
- When the **compiler and linker checks** performed by CMake are successful, their results are **cached** so that they do not have to be repeated in subsequent CMake runs (CMakeCache.txt).
  - Developers would typically only need to look CMakeCache.txt if working with a new or unusual compiler or when **setting up toolchain files for cross-compiling**. cross-compilingのやり方もっと知りたい！

## 3.3 Building A Basic Executable

```cmake
add_executable(targetName source1 [source2 ...])
```

- By default, the name of the executable would be myApp.exe on Windows and myApp on Unix-based platforms like macOS, Linux, etc. (platform-dependent name)
- The executable name can be customized with **target properties**: Chapter 9, Properties.

### 良い実践

- force thinking about **project version** numbers early and start incorporating version numbering into the `project()` command as soon as possible.
  - Consider popular practices such as Semantic Versioning when deciding on a **versioning strategy**.