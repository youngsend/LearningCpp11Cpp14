## run-time polymorphismに関するcontroversy（論争）

- Of these features （1985年Bjarne Stroustrupが開発した機能のなかに）, support for run-time polymorphism in the form of virtual functions was by far the most controversial.
- Systems programmers tended to view indirect function calls with suspicion, and people acquainted with other languages supporting object-oriented programming had a hard time believing that `virtual` functions could be fast enough to be useful in systems code.
- The resistance to virtual functions may be related to a resistance to the idea that you can get better systems through **more regular structure of code** supported by a programming language.
- **My view was (and is) that we need every bit of help we can get from languages and tools: the inherent complexity of the systems we are trying to build is always at the edge of what we can express**.

## C++成長の危険について

- The dangers are "Design by committee", feature bloat, lack of consistent style, and short-sighted decisions.

## C++ Core Guidelinesの目標

- **complete type-safety** and **complete resource-safety** as a base for simpler, faster, and more maintainable code. だから師匠は全然`std::move`を勧めない（type-safety, resource-safety両方）でしょう。

## C風からC++風に

- Macro substitution is almost never necessary in C++. Use `const`, `constexpr`, `enum` or `enum class` to define manifest constants, `inline` to avoid function-calling overhead, `template`s to specify families of functions and types, and `namespace`s to avoid name clashes.
- Avoid `void*`, unions, and **casts**, except deep within the implementation of some function or class. Their use limits the support you can get from the type system and can harm performance. **In most cases, a cast is an indication of a design error**.

- **Minimize the use of arrays and C-style strings**. C++ standard-library `string`s, `array`s, and `vector`s can often be used to write simpler and more maintainable code compared to the traditional C style. In general, try not to build yourself what has already been provided by the standard library.

## Linkage

- To give a C++ function C linkage (so that it can be called from a C program fragment) or to allow a C function to be called from a C++ program fragment, declare it `extern "C"`. for example, `extern "C" double sqrt(double);`.

## Advice

- Don't get stuck with decades-old language-feature sets and design techniques.
- Prefer named casts, such as `static_cast` over C-style casts.
- When converting from `malloc()` and `free()` to `new` and `delete`, consider using `vector`, `push_back()`, and `reserve()` instead of `realloc()`.
- For each standard C header `<X.h>` that places names in the global namespace, the header `<cX>` places the names in namespace `std`.
- Use `extern "C"` when declaring C functions.
- Prefer `string` over C-style strings (direct manipulation of zero-terminated arrays of `char`).
- **Prefer `iostream`s over `stdio`**.
- Prefer containers (e.g., `vector`) over built-in arrays.