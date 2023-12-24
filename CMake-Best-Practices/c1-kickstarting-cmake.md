- toolchainの定義: a toolchain typically consists of **a series of programs that can compile and link libraries, as well as creating archives** and similar.
- toolchainの検知や指定：CMake will **automatically detect** the toolchain to use by **inspecting the system**, but if needed, this can be configured by environment variables or, **in the case of cross-compiling, by providing a toolchain file**.
- toolchainの指定はCMakelists.txtに書かないで！：「CMake files should be platform- and compiler-agnostic」の説に違反している！
  - toolchainの指定は、例えばcompilerの指定。linkerはcompilerによって自動的に選ばれるが、指定もできる。

- ４つビルドタイプ：
  - Debug: `GCC, Clang`に`-00 -g`をつけることと同じ
  - Release: `-03 -DNDEBUG`と同じ。speedを上げるために。
  - RelWithDebInfo: debug symbolあるが、assertsはない。`-02 -g -DNDEBUG`と同じ
  - MinSizeRel: Releaseと同じが、small binary sizeの方目指している。`-0s -DNDEBUG`と同じ