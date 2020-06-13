## 8.1 The Basics

- **Macros effectively paste their body into the point of the call** and the macro arguments are substituted as simple string replacements. 

  ```cmake
  function(name [arg1 [arg2 [...]]])
  	# Function body
  endfunction()
  
  macro(name [arg1 [arg2 [...]]])
  	# Macro body
  endmacro()
  ```

  ```cmake
  function(print_me)
  	message("Hello from inside a function")
  	message("All done")
  endfunction()
  
  print_me()
  ```

  - functionやmacro nameのスタイル：all lowercase with words separated by underscores.

## 8.2 Argument Handling Essentials

- Macro arguments are **string replacements**, so whatever was used as the argument to the macro call is essentially **pasted into wherever that argument appears in the macro body**.

  - If a macro argument is used in an `if()` statement, it would be treated as a string rather than a variable.

  ```cmake
  function(func arg)
  	if(DEFINED arg)
  		message("Function arg is a defined variable")
  	else()
  		message("Function arg is NOT a defined variable")
  	endif()
  endfunction()
  
  macro(macr arg)
  	if(DEFINED arg)
  		message("Macro arg is a defined variable")
  	else()
  		message("Macro arg is NOT a defined variable")
  	endif()
  endmacro()
  
  func(foobar)
  macr(foobar)
  
  # 結果
  # Function arg is a defined variable
  # Macro arg is NOT a defined variable
  ```

- The value of that argument can be accessed in the function or macro body using the usual variable notation, even though **macro arguments are not technically variables**.

  ```cmake
  macro(macr myArg)
  	message("myArg = ${myArg}")
  endmacro()
  
  macr(foobar)
  # myArg = foobar
  ```

- C/C++と同じように、`ARGC, ARGV`も提供されている。named arguments and **any additional unnamed arguments** that were givenが含まれる。

  - また`ARGN`、only contains arguments beyond the named ones.
  - また`ARGVx`の形でx番目argumentを取れる。

- `ARG...`を使う場合：supporting optional arguments and implementing a command which can take an arbitrary number of items to be processed. 例えば、a function that defines an executable target, links that target to some library and defines a test case for it. (**frequently encountered when writing test cases**) 改善点：関数を利用して、CMakeLists.txtにある重複の内容を整理しよう！

  ```cmake
  # Use a named argument for the target and treat all remaining (unnamed) arguments as the source files for the test case
  function(add_mytest targetName)
  	add_executable(${targetName} ${ARGN})
  	target_link_libraries(${targetName} PRIVATE foobar)
  	add_test(NAME	${targetName}
  			COMMAND	${targetName})
  endfunction()
  
  # Define some test cases using the above function
  add_mytest(smallTest small.cpp)
  add_mytest(bigTest	big.cpp algo.cpp net.cpp)
  ```

- macroのargumentはvariableじゃないので、決してvariableとして使わないで！間違い無いように、いつも${argument}にしましょう。例えばmacroの中に`foreach(arg IN LISTS ARGN)`じゃなく、`foreach(arg IN ITEMS ${ARGN})`ように利用しましょう。

## 8.3 Keyword Arguments（面白い！）

**keyword-based arguments and flexible argument ordering**の例：

```cmake
target_link_libraries(targetName
	<PRIVATE|PUBLIC|INTERFACE> item1 [item2 ...]
	[<PRIVATE|PUBLIC|INTERFACE> item3 [item4 ...]]
	...)
```

user-defined関数もkeyword-based argumentsを受けるように`cmake_parse_arguments()`コマンドを利用する：

```cmake
include(CMakeParseArguments) # Needed only for CMake 3.4 and earlier
cmake_parse_arguments(prefix
					  noValueKeywords
					  singleValueKeywords
					  multiValueKeywords
					  argsToParse)
```

- Typically, `argsToParse` is given as `${ARGN}`.
- Each of the keyword arguments is a list of keyword names supported by that function or macro, so they should **each be surrounded by quotes** to ensure they are parsed correctly. ここのロジックは分かっていないが、下記の例から使い方は分かる。
- When `cmake_parse_arguments()` returns, for every **keyword**, a **corresponding variable** will be available **whose name** consists of the specified **`prefix`, an underscore and the keyword name**. 

```cmake
function(func)
	# Define the supported set of keywords
	set(prefix			ARG)
	set(noValues		ENABLE_NET COOL_STUFF)
	set(singleValues	TARGET)
	set(multiValues		SOURCES IMAGES)
	
	# Process the arguments passed in
	include(CMakeParseArguments)
	cmake_parse_arguments(${prefix}
						  "${noValues}"
						  "${singleValues}"
						  "${multiValues}"
						  ${ARGN})
						  
	# Log details for each supported keyword
	message("Option summary:")
	foreach(arg IN LISTS noValues)
		if(${prefix}_${arg})
			message(" ${arg} enabled")
		else()
			message(" ${arg} disabled")
		endif()
	endforeach()
	
	foreach(arg IN LISTS singleValues multiValues)
		# Single argument values will print as a simple string
		# Multiple argument values will print as a list
		message(" ${arg} = ${${prefix}_${arg}}")
	endforeach()
endfunction()

func(SOURCES foo.cpp bar.cpp TARGET myApp ENABLE_NET)
func(COOL_STUFF TARGET dummy IMAGES here.png there.png gone.png)

# 結果
# Option summary
#  ENABLE_NET enabled
#  COOL_STUFF disabled
#  TARGET = myApp
#  SOURCES = foo.cpp;bar.cpp
#  IMAGES = 
# Option summary
#  ENABLE_NET disabled
#  COOL_STUFF enabled
#  TARGET = dummy
#  SOURCES = 
#  IMAGES = here.png;there.png;gone.png
```

- **leftover arguments**のアクセス: `<prefix>_UNPARSED_ARGUMENTS`

  ```cmake
  # leftover arguments are not associated with any keyword
  function(demoArgs)
  	set(noValues		"")
  	set(singleValues	SPECIAL)
  	set(multiValues		EXTRAS)
  	cmake_parse_arguments(ARG "${noValues}" "${singleValues}" "${multiValues}" ${ARGN})
  	
  	message("Left-over args: ${ARG_UNPARSED_ARGUMENTS}")
  endfunction()
  
  demoArgs(burger fries SPECIAL secretSauce)
  # 結果
  # Left-over args: burger;fries
  ```

- single- or multi-valueの後、何のargumentsも付いてない場合の検知：`<prefix>_KEYWORDS_MISSING?VALUES`。
  - 注意：multi-valueキーワードの後ろにargumentsが付いてなくても可能です、間違いではない。＃

## 8.4 Scope

- functionの中でcaller側の変数を変更する：（call側が変数名を決められる）

  ```cmake
  function(func resultVar1 resultVar2)
  	set(${resultVar1} "First result" PARENT_SCOPE)
  	set(${resultVar2} "Second result" PARENT_SCOPE)
  endfunction()
  
  func(myVar otherVar)
  message("myVar: ${myVar}")
  message("otherVar: ${otherVar}")
  
  # 結果
  # myVar: First result
  # otherVar: Second result
  ```

- About the only reason one would use a macro instead of a function is if many variables need to be set in the calling scope.

- macroのscopeはcalling側なので、`return()`、`PARENT_SCOPE`を使わないはず。危ないです。
- **A macro effectively pastes its commands at the call site**.

## 8.5 Overriding Commands

- 名前一緒のcommand (function, macro)があれば、古い方を使う方法：an underscore + command name. 時々あるcommandのwrapperを作る時この手法を使う。でも危ない！だから止めた方がいい！

  ```cmake
  function(someFunc)
  	# Do something...
  endfunction()
  
  # Later in the project...
  function(someFunc)
  	if(...)
  		# Override the behavior with something else...
  	else()
  		# WARNING: Intended to call the original command, but it is not safe
  		_someFunc()
  	endif()
  endfunction()
  ```

  - If the command is only ever overridden like this once, it appears to work, but if it is overridden again, then the original command is **no longer accessible**. **無限loopにもなる！**

    ```cmake
    function(printme)
    	message("Hello from first")
    endfunction()
    
    function(printme)
    	message("Hello from second")
    	_printme()
    endfunction()
    
    function(printme)
    	message("Hello from third")
    	_printme()
    endfunction()
    
    printme()
    # 結果
    # Hello from firstメッセージは出ない。2番めprintmeはずっと自分を呼び出す。無限loopになる。
    ```

  - In general, it is fine to override a function or macro as long as it does not try to call the previous implementation.

## 良い実践

- In general, prefer to use functions rather than macros.
- Prefer to pass all values a function or macro needs as command arguments rather than relying on variables being set in the calling scope.
- For all but very trivial functions or macros, it is highly recommended to use the **keyword-based argument handling** provided by `cmake_parse_arguments()`.
  - This leads to better usability and improved robustness of calling code (e.g. **little chance of getting arguments mixed up**).

- functions, macrosを特定なフォルダに置く：a common practice is to nominate a particular directory (usually just below the top level of the project) where various `XXX.cmake` files can be collected. utilityフォルダ見たい。
  - Using a `.cmake` file name suffix allows the `include()` command to find the files as modules.

- Do not override any builtin CMake command, consider those to be off-limits（立入禁止） so that projects will always be able to assume the builtin commands behave as per the official documentation and there will be no opportunity for the original command to become inaccessible.