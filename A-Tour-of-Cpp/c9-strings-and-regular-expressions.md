# Chapter 9: Strings and Regular Expressions
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

## subpatternの定義
- **A part of a pattern** is considered a subpattern (which **can be extracted separately from an `smatch`**) if it is **enclosed in parentheses**.
  - 小括弧に囲まれたら、subpatternと言う。
- 例
  - `\d+-\d+`: subpatternなし
  - `\d+(-\d+)`: １つsubpattern
  - `(\d+)(-\d+)`: ２つsubpattern
- なぜsubpatternが大事？
  - subpatternにマッチした文字列はsmatchに保存される。
    - subpatternは大体僕等が興味を持っている部分。
    - 例えばXMLをパースする時、`<(.*?)>(.*?)</^1>`でタグごとに探している。（２つsubpattern）
      - `<b>bright</b>`をパースする時、2番目のsubpatternのため、`bright`のみを取れる。
      - 2番めsubpatternがないと、`bright`を取るために更に処理が必要。
  - groupingのため。

## smatchの定義

- an smatch is a vector of **submatches** of type string.
- The first element is the complete match.
- `<b>bright</b>`の例というと、`smatch[0]`は`<b>bright</b>`です。`smatch[2]`は`bright`です。なので、`bright`の部分のみ処理したい時も簡単に取れる！

## Regular Expression Special Characters

`[` Begin **character class**.

`]` End **character class**.

`{` Begin count. 

`}` End count.

`(` Begin grouping.

`)` End grouping.

## Repetition

`{n}`	Exactly n times.

`{n,}`	n or more times.

`{n,m}`	At least n and at most m times.

## non-greedy pattern matcher

-  A suffix **?** after any of the **repetition notations** (**?, *, +**, and **{}**) makes the pattern matcher "lazy" or "non-greedy".