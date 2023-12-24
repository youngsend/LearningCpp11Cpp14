# Guideline 6: adhere to the expected behavior of abstractions

- violation of expectation in an abstractionの一例：Retangleクラスがwidthやheightを独立でセットできる想定(the result of `getWidth()` does not change after `setHeight()` is called)があるが、Squareはwidthやheightが一緒にさせないといけない（widthやheightを独立で設定できない）

- LSP (Liskov substitution principle): **the expectations in an abstraction, must be adhered to in a subtype**.

  - **preconditions cannot be strengthened in a subtype: a subtype cannot expect more in a function than what the super type expresses.** That would violate the expectations in the abstraction.

  - **postconditions cannot be weakened in a subtype: a subtype cannot promise less when leaving a function than the super type promises**.

  - function return types in a subtype must be covariant:

    ```c++
    struct Base { /* ...some virtual functins, including destructor... */ }
    struct Derived : public Base { /* ... */ }
    
    struct X {
      virtual ~X() = default;
      virtual Base* f();
    };
    
    struct Y: public X {
      Derived* f() override;  // covariant return type
    }
    ```

- The `Square` class doesn't fulfill the expectations in the `Rectangle` class, and **the hierarchy in this example doesn't express an IS-A relationship**.

- The example also demonstrates that **inheritance is not a natural or intuitive feature, but a hard feature**. 継承を使うのは慎重！！

  - 難しさ：**Whenever you use inheritance, you must make sure that all expectations in the base class are fulfilled and that the derived type behaves as expected**.

- `if(dynamic_cast<Derived const*>(&b)) {...}`のように無理やり汎用関数を作るのはやめましょう！！
- Takeaways:
  - understand that an abstraction represents a set of requirements and expectations.
  - make sure that derived classes adhere to the expected behavior of their base classes.
  - **communicate the expectations of an abstraction**.

# Guideline 7: understand the similarities between base classes and concepts

- concepts can be considered the equivalent, the static counterpart, of base classes.
- Takeaways:
  - consider concepts (both the C++20 feature and pre-C++20 named template arguments) as the static equivalent of base classes.

# Guideline 8: understand the semantic requirements of overload sets

- **free functions**! 
- One part of the STL philosophy is to loosely couple the different pieces of functionality and promote reuse by separating concerns as free functions. That's why containers and algorithms are two separate concepts within the STL: conceptually, containers don't know about the algorithms, and algorithms don't know about containers. **The abstraction between them is accomplished via iterators that allow you to combine the two in seemingly endless ways**.

- > Overload operations that are roughly equivalent. --- Code Guideline C.162
  >
  > Overload only for operations that are roughly equivalent. --- Code Guideline C.163

  - 162: the advantages of having the same name for semantically equivalent functions.
  - 163: the problem of having the same name for semantically different functions.

- Takeaways:
  - be aware that function overloading is a compile-time abstraction mechanism.
  - keep in mind that there are expectations on the behavior of functions within an overload set.
  - **pay attention to existing names and conventions**.

# Guideline 9: pay attention to the ownership of abstractions