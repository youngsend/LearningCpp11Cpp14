## 5.1 Variable Basics

```cmake
set(varName value... [PARENT_SCOPE])
```

- とりあえずglobal scopeの変数を紹介。*Chapter 7, Using Subdirectories* and *Chapter 8, Functions and Macros* introduce the situations where **local scopes** arises and show how the `PARENT_SCOPE` keyword is used to promote the visibility of a variable into the enclosing scope.

- CMake treats **all variables as strings**.

  ```cmake
  set(myVar a b c)	# myVar = "a;b;c"
  set(myVar a;b;c)	# myVar = "a;b;c"
  set(myVar "a b c")	# myVar = "a b c"
  set(myVar a b;c)	# myVar = "a;b;c"
  set(myVar a "b c")	# myVar = "a;b c"
  ```

  - spaceはsemicolon（lists）。quoted spaceはspace。

- CMake is particularly flexible in that it is possible to use `${myVar}`  **recursively** or to specify the name of another variable to set.

  ```cmake
  set(foo ab)			# foo = "ab"
  set(bar ${foo}cd)	# bar = "abcd"
  set(baz ${foo} cd)	# baz = "ab;cd"
  set(myVar ba)		# myVar = "ba"
  set(big "${${myVar}r}ef")	# big = "${bar}ef" = "abcdef"
  set(${foo} xyz)		# ab = "xyz"
  set(bar ${notSetVar})	# bar = ""
  ```

  - Use of an undefined variable simply results in an empty string being substituted, similar to the way Unix shell scripts behave.

- quotesの中にescapingを使わないように、lua-inspired **bracket syntax**を利用できる。
  - the start of the content is marked by `[=[` and the end with `]=]`.
  - Any number of `=` characters can appear between the square brackets, including none at all, but the **same number** of `=` characters must be used at the start and the end.
  - No further transformation of the bracketed content is performed (i.e. **no variable substitution**（代用、代入） or escaping). つまり`${myVar}`も効かなくなる。
    - なので、particularly well suited to defining content like **Unix shell scripts**. Such content uses the `${...}` syntax for its own purpose and frequently contains quotes.
  - The flexibility to use any number of = characters between the [ and ] markers also means embedded square brackets do not get misinterpreted as markers.
  - 僕の考え：C++ regexのRと同じ意図だ。つまりraw string。

## 5.2 Environment Variables

- `$Env{varName}`で取る。
- `set(ENV{PATH} "$ENV{PATH}:/opt/myDir")`もできるが、あんまり意味ない：the change to the environment variable will not be visible at build time.

## 5.3 Cache Variables

- CMakeCache.txtに保存され、persist between CMake runs.

  ```cmake
  set(varName value... CACHE type "docstring" [FORCE])
  ```

  - `type`は下記から：`type`の目的はimprove the user experience in GUI tools.
    - BOOL, FILEPATH, PATH, STRING, INTERNAL.

- **Setting a boolean cache variable** is such a common need:

  ```cmake
  option(optVar helpString [initialValue])
  ```

  - if *initialValue* is omitted, the default value OFF will be used.

  - このコマンドは下記と大体同じ（意味が違う場合もある）：

    ```cmake
    set(optVar initialValue CACHE BOOL helpString)
    ```

- `FORCE`: the `set()` command will **only overwrite** a cache variable if the `FORCE` keyword is present.
  - `option()` commandはFORCEできない。つまりoverwriteできない。
  - なぜFORCE？（なぜcache variable？）
    - Rather than hard-coding the value in the CMakeLists.txt file as a normal variable, a cache variable can be used so that the developer can **override the value without having to edit the CMakeLists.txt** file.
    - The variable can be modified by interactive GUI tools or by **scripts** without having to change anything in the project itself.
- **cache variableの意図：customization points**. 例えば：
  - **different parts of the build can be turned on or off**.
  - **paths to external packages can be set**.
  - **flags for compilers and linkers can be modified**.

## 5.4 Potentially Surprising Behavior of Variables

- **normal variables take precedence over cache variables**. しかし、cache variableを設定する時、同じ名前のnormal variablesが消される２つ場合がある：

  - The cache variable did not exist before the call to set().
  - The cache variable existed before the call to set(), but it did not have a defined type.

- でも、基本はnormal variableの優先度高いから、2回目からまたnormal variableの値が取られる。

  - In the first run, the cache variable won't exist or won't have a defined type, but in subsequent runs it will. Therefore, in the first run, a normal variable would be hidden, but in subsequent runs it would not.

    ```cmake
    set(myVar foo)		# Local myVar
    set(result ${myVar}) # result = foo
    set(myVar bar CACHE STRING "") # Cache myVar
    
    set(result ${myVar}) 	# First run: result = bar
    						# Subsequent runs: result = foo
    ```

- `option()`の場合：CMake 3.13以降、if a normal variable already exists with the same name, the command **does nothing**. 以前なら、set()と同じ。
- Both `unset(foo)` and `set(foo)` remove a **non-cache variable** from the current scope. なので、cache variableには意味ない。
- cache variableを取りたい場合：`$CACHE{someVar}`(CMake 3.13から)
  - でもtemporary debugging以外の場合使わないほうがいい：since it breaks the long-established expectation that normal variables will override values set in the cache.

## 5.5 Manipulating Cache Variables

## 方法１：Setting Cache Values on the Command Line

- `-D` option:

  ```bash
  cmake -D myVar:type=someValue ...
  ```

- the entire value given with the -D option should be quoted if setting a cache variable with a value containing spaces.

  ```bash
  cmake -D "bar:STRING=This contains spaces" ...
  ```

## 方法２：CMake GUI Tools

- CMakeが提供しているGUI: `cmake-gui`.（CLionのCMake設定画面見たい）
- **Using a toolchain file is typical when cross-compiling**. *Chapter 21, Toolchains and Cross Compiling*.
- もう１つCMakeが提供しているGUI：`ccmake`.
  - text-based interface. （BIOS画面見たい）
  - `ccmake`のちょうどいい場合：when the full `cmake-gui` application is not practical or not available, such as over a terminal connection that cannot support UI forwarding.

## 5.6 Debugging Variables and Diagnostics

```cmake
message([mode] msg1 [msg2]...)
```

- mode: affects how the message is output and in some cases stops further processing. つまりLoggingやAssert両方できるんだ。下記はレベル降順のはず。
  - FATAL_ERROR
  - SEND_ERROR: continue until the configure stage completes, but generation will not be performed.
    - Projects should generally prefer to use FATAL_ERROR to avoid spurious errors that are the result of an earlier error rather than identifying a genuine problem.
  - WARNING
  - AUTHOR_WARNING: only shown if developer warnings are enabled (use the `-Wno-dev` option on the `cmake` command line to disable them).
    - Projects do not often use this particular type of message, they are usually generated by CMake itself.
  - DEPRECATION, NOTICE
  - STATUS: concise status information.
    - Projects should this to be the **preferred message mode for messages that users will be interested in**.
    - Messages will be printed to `stdout` and may **have two hyphens automatically prepended** in some contexts. よくみてる！
  - VERBOSE, DEBUG, TRACE.

- logging levelの設定：共通考え方の１つだ、ROSも最低logging levelを設定できる。

  ```bash
  cmake --log-level=VERBOSE ...
  ```

  - defaultはSTATUS

- ```cmake
  variable_watch(myVar [command])
  ```

  - When a variable is watched, **all attempts to read or modify it are logged**.
  - 利用場合：how a variable ended up with a particular value.
  - command: will be executed every time the variable is read or modified. 普段はいらない(very uncommon)。`std::experimental::future`のcontinuationみたい。

## 5.7 String Handling: `string()`

```cmake
string(FIND inputString subString outVar [REVERSE])
```

- outVar: since CMake commands cannot return a value.
- the first occurrence is found unless REVERSE is specified.

```cmake
string(REPLACE matchString replaceWith outVar input [input...])
```

```cmake
string(REGEX MATCH		"[ace]"				matchOne abcdefabcdef)
string(REGEX MATCHALL	"[ace]"				matchAll abcdefabcdef)
string(REGEX REPLACE	"([de])" "X\\1Y"	replVar1 abc def abcdef)
string(REGEX REPLACE	"([de])" [[X\1Y]]	replVar2 abcdefabcdef)
```

- 結果：

  ```
  matchOne = a
  matchAll = a;c;e;a;c;e
  replVar1 = abcXdYXeYfabcXdYXeY
  replVar2 = abcXdYXeYfabcXdYXeY
  ```

```cmake
string(SUBSTRING input index length outVar)
```

```cmake
string(LENGTH	input outVar)
string(TOLOWER	input outVar)
string(TOUPPER	input outVar)
string(STRIP	input outVar)
```

## 5.8 Lists

- **Lists are used heavily in CMake**.
- `list()`コマンドが必要である理由：Lists are just a **single string** with list items separated by semicolons, which can make it less convenient to manipulate individual list items.
  - なので、CMake provides the `list()` command to facilitate such tasks.

```cmake
set(myList a b c)	# Creates the list "a;b;c"
list(LENGTH myList len)	# len = 3
list(GET myList 2 1 letters) # letters = c;b

list(INSERT myList 2 X Y Z)	# myList = a;b;X;Y;Z;c
list(APPEND myList d e f)	# myList = a;b;X;Y;Z;c;d;e;f
list(PREPEND myList P Q R)	# myList = P;Q;R;a;b;X;Y;Z;c;d;e;f

list(FIND myList d index)	# index = 9
```

```cmake
list(REMOVE_ITEM		myList value [value...])
list(REMOVE_AT			myList index [index...])
list(REMOVE_DUPLICATES	myList)

list(POP_FRONT 	myList [outVar1 [outVar2...]])
list(POP_BACK	myList [outVar1 [outVar2...]])

list(REVERSE 	myList)
list(SORT		myList [COMPARE method] [CASE case] [ORDER order])
```

- method: STRING or FILE_BASENAME
  - basenameの意味は、pathのfilenameの部分
- case: SENSITIVE or INSENSITIVE

## 5.9 Math

```cmake
set(x 3)
set(y 7)
math(EXPR zDec "(${x}+${y}) * 2")	# zDec = 20
math(EXPR zHex "(${x}+${y}) * 2" OUTPUT_FORMAT HEXADECIMAL)	# zHex = 0x14
```

## 良い実践

- CMake GUI toolを使ってみよう。
  - A little bit of time spent getting familiar with it will simplify working with more complex projects later.
- Prefer to provide **cache variables** for controlling **whether to enable optional parts** of the build instead of encoding the logic in build scripts outside of CMake.
- Try to avoid relying on environment variables being defined, apart from perhaps the ubiquitous PATH or similar operating system level variables.
  - pass information directly to CMake through cache variables instead whether possible. なので、CMakeLists.txtにROSに関する環境変数を直接に使わないほうがいいでしょう！
- 名前付ける方法：
  - For cache variables, consider grouping related variables under **a common prefix followed by an underscore**.
    - CMake GUI groups variables based on the same prefix automatically.
  - Consider that the project **may one day become a sub-part of some larger project**, so a name **beginning with the project name** or something closely associated with the project may be desirable.
- 同じ名前のnon-cache変数とcache変数はやめろう。
- CMakeが提供しているpre-defined variablesをみてみよう。
  - Some of these variables are heavily used by projects.