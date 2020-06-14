- They are always attached to a specific entity, whether that can be a directory, target, source file, test case, cache variable or even the overall build process itself.

## variablesやpropertiesの区別

- A variable is not attached to any particular entity and it is very common for projects to define and use their own variables.
- Properties are typically **well defined** and **documented by CMake** and always apply to a specific entity.

## 9.1 General Property Commands

```cmake
set_property(entitySpecific
			[APPEND] [APPEND_STRING]
			PROPERTY propName [value1 [value2 [...]]])
			
# entitySpecificは下記から
GLOBAL
DIRECTORY [dir]
TARGET [target1 [target2 [...]]]
SOURCE [source1 [source2 [...]]]
INSTALL [file1 [file2 [...]]]
TEST [test1 [test2 [...]]]
CACHE [var1 [var2 [...]]]
```

- The `propName` would normally match one of the properties **defined in the CMake documentation**. でも新しいpropertyも作れる。（`define_property()`を使う。rarely used.）その時はuse some project-specific prefix on the property name.

  - CMake has a large number of pre-defined properties of each type. Developers should consult the CMake reference documentation for the available properties and their intended purpose.

- `APPEND`, `APPEND_STRING` control how the named property is updated if it already has a value.

  | Previous Value(s) | New Value(s) | No Keyword | APPEND  | APPEND_STRING |
  | ----------------- | ------------ | ---------- | ------- | ------------- |
  | foo               | bar          | bar        | foo;bar | foobar        |
  | a;b               | c;d          | c;d        | a;b;c;d | a;bc;d        |

```cmake
get_property(resultVar entitySpecific
			PROPERTY propName
			[DEFINED | SET | BRIEF_DOCS | FULL_DOCS])
			
# entitySpecificは下記から
GLOBAL
DIRECTORY [dir]
TARGET target
SOURCE source
INSTALL file
TEST test
CACHE var
VARIABLE
```

- `VARIABLE`の場合、variableの名前はpropNameで指定される。でもこのやり方はuncommon。`${}`で取りましょう！
- `SET` is usually what projects need rather than `DEFINED` in most scenarios.

## 9.2 Global Properties

```cmake
get_cmake_property(resultVar property)
```

- `property`はthe name of any global propertyもしくは下記のpseudo propertiesから：`VARIABLES, CACHE_VARIABLES, COMMANDS, MACROS, COMPONENTS`.
  - components defined by `install()` commands.

## 9.3 Directory Properties

- Directory properties mostly focus on setting **defaults for target properties** and overriding global properties or defaults for the current directory.

```cmake
set_directory_properties(PROPERTIES prop1 val1 [prop2 val2 ...])
```

- always apply to the current directories.

```cmake
get_directory_property(resultVar [DIRECTORY dir] property)
get_directory_property(resultVar [DIRECTORY dir] DEFINITION varName)
```

- 2番めはprovides a means of obtaining a variable's value from a different directory scope.
  - should rarely be needed and its use should be avoided for scenarios other than debugging the build or similar temporary tasks.

## 9.4 Target Properties

- Few things in CMake have such a strong and direct influence on how targets are built as target properties.
- target propertiesの仕事：
  - control and provide information about everything from the **flags** used to compile source files through to the **type and location** of the built binaries and intermediate files.
  - target properties are where most of the **details about how to actually turn source files into binaries** are collected and applied.

```cmake
set_target_properties(target1 [target2...]
					PROPERTIES
					propertyName1 value1
					[propertyName2 value2] ... )
get_target_property(resultVar target propertyName)
```

- if a list value needs to be provided for a given property, the `set_target_properties()` command requires that value to be specified in string form, e.g. `"this;is;a;list"`.

- the family of `target_...()` commands are a critical part of CMake and all but the most trivial of CMake projects would typically use them.
  - define the properties for a particular target.
  - define how that information might be propagated to other targets that link against it. *Chapter 14, Compiler and Linker Essentials*.

## 9.5 Source Properties

- Projects should rarely need to query or modify source file properties.

```cmake
set_source_files_properties(file1 [file2...]
							PROPERTIES
							propertyName1 value1
							[propertyName2 value2] ... )
get_source_file_property(resultVar sourceFile propertyName)
```

- 使わない方がいい。時々その実現はtargetの全部のsourceに影響してしまう。

## 9.6 Cache Variable Properties

- For the most part, cache variable properties are aimed more at how the cache variables are handled in the CMake GUI and the console-based `ccmake` tool rather than affecting the build in any tangible way. cache variableのpropertyはcache variableのcmake tool上のoperation、見た目などに影響する。
- cache variable propertyのname: `TYPE, ADVANCED, HELPSTRING, STRINGS `.

## 9.7 Other Property Types

```cmake
set_tests_properties(test1 [test2...]
					PROPERTIES
					propertyName1 value1
					[propertyName2 value2] ... )
get_test_property(resultVar test propertyName)
```

## 良い実践

- For target properties, use of the various `target_...()` commands is strongly recommended over manipulating the associated target properties directly.
  - These commands not only manipulate the properties on specific targets, they also set up **dependency relationships between targets** so that CMake can **propagate** some properties automatically.
- Source properties do have the potential for undesirable negative impacts on the build behavior of a project.
  - Projects should consider using other alternatives to source properties where available, such as the techniques given in *Section 19.2, Source Code Access to Version Details*.