### Chapter 2: The principal structure of a program

- 3 semantic aspects: declarative parts (what things are), **definitions of objects (where things are)** and statements.

- A declaration only describes a feature but **does not create it**, so repeating a declaration does not do much harm but adds redundancy.
  - Identifiers may have several **consistent** declarations.

- The scope, as used for `main`, which is not inside a `{...}` pair, is called ***file scope***. **Identifiers in file scope are often referred to as *globals***.

- Declarations specify identifiers, whereas definitions specify objects.

- Missing elements in initializers default to 0. だから少なくとも`={}`を使っていたのだ。

- A process-startup routine that is provided by our platform calls the user-provided function `main`.

- **Use array notation for pointer parameters**:

  ```c
  /* These emphasize that the arguments cannot be null. */
  size_t strlen(char const string[static 1]);
  int main(int args, char* argv[argc+1]);
  
  /* Compatible declarations for the same functions. */
  size_t strlen(const char *string);
  int main(int argc, char **argv);
  ```

  - `main` receives an **array of pointers to `char`**: the program name, `argc-1` program arguments, and one  null pointer that terminates the array.

- **Use function notation for function pointer parameters**:

  ```c
  /* This emphasizes that the hander argument cannot be null */
  int atexit(void handler(void));
  
  /* Compatible declaration for the same function. */
  int atexit(void (*handler)(void));
  ```

### Chapter 3: Everything is about control

- Any value different from 0 represents logical true. つまりマイナスでもtrueとなる！実験済み。
- **Don't compare to 0, false, or true**.
- **All scalars have a truth value**.