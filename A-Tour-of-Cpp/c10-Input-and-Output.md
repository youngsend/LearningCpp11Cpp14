# Chapter 10: Input and Output

## Output

- In <ostream>, the I/O stream library defines output for every built-in type. 基本タイプは全部cout/cerrできる。
  - `cerr` is the standard stream for reporting errors. どうしてわざわざcerrを用意する？多分filterのためだけ。普段は違う色がつけられるでしょう。
  - 例えば普通のfile redirection:`command > file`は、**redirects standard output, but not standard error**.
    - ROSのLoggingの中に、DEBUG and INFO messages are printed on standard output, WARN, ERROR, FATAL messages are sent to standard error.
    - 参考：https://www.cse.sc.edu/~jokane/agitr/agitr-letter.pdf
      - page 68. Chapter 4 Log Messages, 4.4.1 Console.
- `cout << "the value of i is " << i << '\n'`が可能である理由：the result of an output expression can itself be used for further output.

## I/O State

```c++
vector<int> read_ints(istream& is) {
    vector<int> res;
    for (int i; is>>i;)
        res.push_back(i);
    return res;
}
```

- the operation `is>>i` returns  a reference to `is`, and testing an `iostream` yields `true` if the stream is ready for another operation. `iostream`がテストされているのはI/O stateだ。`false`が返される場合は、２つ可能性：`is.eof()`/`is.fail()`.
- `is.fail()`の場合、`is.unget()`で**put the non-digit back into the stream**。また`string`に`cin`して、失敗原因を分析できる。
- File Streamsも同様にstateをチェックできる。
  - Assuming that the tests succeeded, `ofs` can be used as an ordinary `ostream` (just like `cout`) and `ifs` can be used as an ordinary `istream` (just like `cin`).

## I/O of User-Defined Types

- output operator: `ostream& operator<<(ostream& os, const Entry& e)`.

- input operator: `istream& operator>>(istream& is, Entry& e)`.

  - ちょっと複雑。has to check for correct formatting and deal with errors. 失敗した場合は`is.setstate(ios_base::failbit)`も必要。

  - 例えばこのフォーマット：`{"name",number}`.

  - ```c++
    istream& operator>>(istream& is, Entry& e) {
        char c,c2;
        if(is>>c && c=='{' && is>>c2 && c2=='"'){
            string name;
            while(is.get(c) && c!='"') // anything before a " is part of the name
                name+=c;
            
            if(is>>c && c==','){
                int number = 0;
                if (is>>number>>c && c=='}'){ // read the number and a }
                    e = {name,number};
                    return is;
                }
            }
        }
        is.setstate(ios_base::failbit); // 失敗した場合
        return is;
    }
    ```

  - **The `is>>c` skips whitespace by default, but `is.get(c)` does not**, so this Entry-input operator ignores (skips) whitespace outside the name string, but not within it.

## Formatting

- `fixed`は小数点のあとの部分のprecision（maximum number of digits）を指定する。
- `precision()` doesn't affect integer output. 整数部分は制限されない。小数部分だけ整数部分のprecisionを除いたあとのprecisionに制限される。

## String Streams

- `istringstream>>` for reading from a string.

- `ostringstream<<` for writing to a string. （for writing to a stringstream。stringstreamの中身を取るときは`oss.str()`）

  - One common use of an `ostringstream` is to format before giving the resulting string to a GUI.
  - `>>`/`<<`はdataの流れる向きだ。

- 例：Convert Source to Target

  ```c++
  template<typename Target =string, typename Source =string>
  Target to(Source arg) {
      stringstream interpreter;
      Target result;
      
      if(!(interpreter<<arg)				// write arg into stream
        ||!(interpreter>>result)			// read result from stream
        ||!(interpreter>>std::ws).eof())	// stuff left in stream?
          throw runtime_error{"to<>() failed"};
      
      return result;
  }
  ```

  - `std::ws`: Discards leading whitespace from an Input stream. `(interpreter>>std::ws).eof()`だったらつまり全部うまくTargetに変換できた。
  - If all function template arguments are defaulted, the `<>` can be left out.

- Use a **stringstream** or a generic value extraction function （上記の関数`to<X>`）for numeric conversion of strings.

## Path Operations（一部）

- `p/=p2`: `p` and `p2` concatenated using the file-name separator (by default `/`).
- **native format**: 環境依存path format。例えばWindowsでは`/`が自動的に`\`にされる。
- **generic format**: 環境非依存path format。常に`/`。
- `p.extension()`: The file extension part of path.

## File System Operations（一部）

- 豊富なAPI（普通Ubuntu terminalのコマンド全部できる、しかし**C++17から**です）：https://en.cppreference.com/w/cpp/filesystem
- `exists(p)`: Does `p` refer to an existing file system object?
- `bool=create_directory(p)`: Create new directory named `p`; **all intermediate directories on `p` must exist**.
- `bool=create_directories(p)`: **create all intermediate directories** on `p`.
- **`p=current_path()`**: `p` is the current working directory.
- **`current_path(p)`**: Make `p` the current working directory.

## Advice

- If you don't use C-style I/O and care about I/O performance, call `ios_base::sync_with_stdio(false);` to avoid significant overhead.
- Use character-level input only when you have to.
- **Avoid `endl`**. びっくりした！
- Using `stringstream`s for in-memory formatting.
- Prefer `<filesystem>` to direct use of a specific operating system interfaces.