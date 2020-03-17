## `s` suffix
- A **string literal** is by definition a **const char***.
- To get a literal of type std::string use a `s` suffix.
```c++
auto s = "Cat"s; // a std::string
auto p = "Dog";  // a C-style string: a const char*
```
- `std::literals::string_literals`

## `string` Implementation
- **short-string optimization**: short string values are kept in the `string` object itself and **only longer strings** are placed on **free store**.
- When a string's value changes from a short to a long string (and vice versa) its **representation adjusts appropriately**.
- shortの意味：That's implementation defined, but "about 14 characters" isn't a bad guess.
- short-string optimizationがはやっている理由：
  1. 特にmulti-threaded implementations, memory allocation can be relatively costly.
  2. When lots of strings of differing lengths are used, **memory fragmentation**が発生。 
- stringの実現：`using string = basic_string<char>`.

## `string_view`
- string_view: `{ begin(), size() }`
- a read-only view of its characters.
- いつ使う：when we want to pass a substring.

