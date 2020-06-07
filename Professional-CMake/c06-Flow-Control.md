# 6.1 The `if()` Command

## 6.1.1 Basic Expressions

```cmake
if(value)
```

- true: valueがconstant: ON, YES, TRUE, Y. クォーテーションマークがあるか（quoted）構わない。下記全部同様。
- false: valueがconstant: OFF, NO, FALSE, N, IGNORE, NOTFOUND, empty string, a string that ends in -NOTFOUND.
- 数字の場合：converted to a bool following usual C rules.
- その他：treated as a variable name (or possibly as a string（quoted場合のみ）).
  - an unquoted name of a (possibly undefined) variable.
    - the variable's value is compared **against the false constants**. If none of those match the value, true.
    - An undefined variable: false.
  - a quoted string.
    - CMake 3.1以降：if not match any of the defined **true** constants: false.
      - can be overridden with a policy setting. *Chapter 12, Policies*.
    - CMake 3.1以前：if the value of the string matched the name of an existing variable, then the quoted string is effectively **replaced by that variable name (unquoted)**.
  - It is generally advisable to **avoid using quoted arguments** with the `if(something)` form. quoted stringの場合はcomparison expressions（Comparison Tests）を使おう。

## 6.1.3 Comparison Tests

3種類：*numeric, string, **version numbers***.

```cmake
if(value1 OPERATOR value2)
```

- value1, value2はvariable names or (possibly quoted) values.
  - if a value is the same as the name of a defined variable: variable.
  - otherwise: string or value.

| Numeric       | String           | Version numbers       |
| ------------- | ---------------- | --------------------- |
| LESS          | STRLESS          | VERSION_LESS          |
| GREATER       | STRGREATER       | VERSION_GREATER       |
| EQUAL         | STREQUAL         | VERSION_EQUAL         |
| LESS_EQUAL    | STRLESS_EQUAL    | VERSION_LESS_EQUAL    |
| GREATER_EQUAL | STRGREATER_EQUAL | VERSION_GREATER_EQUAL |

- Numeric

  ```cmake
  # true
  if(2 GREATER 1)
  if("23" EQUAL 23)
  set(val 42)
  if(${val} EQUAL 42)
  if("${val}" EQUAL 42)
  ```

- Version numbers: Each version component is expected to be an integer, but the comparison result is essentially undefined if this restriction does not hold.
- String: Be mindful of the potential for the variable/string substitution situation.
  - String comparisons are one of the most common situations where such unexpected substitutions occur.

- **Parentheses can be used to capture parts of the matched value**.

  ```cmake
  if("Hi from ${who}" MATCHES "Hi from (Fred|Barney).*")
  	message("{CMAKE_MATCH_1} says hello")
  endif()
  ```

  - `CMAKE_MATCH_<n>`.
  - The entire matched string is stored in group 0.

## 6.1.4 File System Tests

```cmake
if(EXISTS pathToFileOrDir)
if(IS_DIRECTORY pathToDir)
if(IS_SYMLINK fileName)
if(IS_ABSOLUTE path)
if(file1 IS_NEWER_THAN file2)
```

- It would not be unusual to test for the existence of file1 and file2 before performing the actual IS_NEWER_THAN test, since IS_NEWER_THAN operator returns true if either file is missing or if both files have the same timestamp.
  - Full paths should also be given when using IS_NEWER_THAN.
- None of the file system operators perform any variable/string substitution without ${}, regardless of any quoting.

## 6.1.5 Existence Tests

```cmake
if(DEFINED name)
if(COMMAND name)
if(POLICY name)
if(TARGET name)
if(TEST name)
```

- defined:

  ```cmake
  if(DEFINED SOMEVAR) # Checks for a CMake variable (regular or cache)
  if(DEFINED CACHE{SOMEVAR})
  if(DEFINED ENV{SOMEVAR})
  ```

- command: whether a CMake command, **function or macro** with the specified name exists.
- target: if a CMake target of the specified name has been specified by one of the commands `add_executable(), add_library(), add_custom_target()`.
  - can be used to check if a target is already defined before trying to create it.
- test: if a CMake test has been defined by `add_test()` command. *Chapter 24, Testing*.

```cmake
if(value IN_LIST listVar)
```

## 6.1.6 Common Examples

- 注意すべきところ：cases of testing the platform instead of the entity the constraint actually relates to.
  - Using platform instead of the more accurate constraint can unnecessarily limit the generator choices available to developers, or it may result in the wrong behavior entirely.

- **conditional inclusion of a target** based on whether or not a particular CMake option has been set. **改善点：テスト用のtargetをconditional inclusionにしよう！**

  ```cmake
  option(BUILD_MYLIB "Enable building the myLib target")
  if(BUILD_MYLIB)
  	add_library(myLib src1.cpp src2.cpp)
  endif()
  ```

  - More complex projects often use the above pattern to conditionally include subdirectories or perform a variety of other tasks based on a CMake option or cache variable.
    - Developers can then turn that option on/off（どうやって？） or set the variable to non-default values without having to edit the CMakeLists.txt file directly.

## `foreach()`, `while()`, `break()`, `continue()`

```cmake
foreach(loopVar arg1 arg2 ...) # for each argN value, loopVar is set to that argument.
	# ...
endforeach()

foreach(loopVar IN [LISTS listVar1 ...] [ITEMS item1 ...]) # ITEMSは上記と同じ
	# ...
endforeach()
```

```cmake
set(list1 A B)
set(list2)
set(foo WillNotBeShown)
foreach(loopVar IN LISTS list1 list2 ITEMS foo bar)
	message("Iteration for: ${loopVar}")
endforeach()

# Iteration for: A
# Iteration for: B
# Iteration for: foo
# Iteration for: bar
```

```cmake
foreach(loopVar RANGE start stop [step])
```

## 良い実践

- Minimize opportunities for strings to be unintentionally interpreted as variables in if(), foreach() and while() commands.
  - Strongly prefer to **set a minimu CMake version of at least 3.1** to **disable the old behavior that allowed implicit conversion of quoted string values to variable names**.

- If iterating over items, consider whether using the IN LISTS or IN ITEMS forms communicate more clearly what is being done rather than a bare `foreach(loopVar item1 item2 ...)` form.