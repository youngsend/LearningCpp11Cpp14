# Chapter 13: Utilities

## exceptionの場合でもRAIIはResourceを解放できる、standard-library lockを例として解説

```c++
mutex m; // used to protect access to shared data
// ...
void f(){
    scoped_lock<mutex> lck {m}; // acquire the mutex m
    // ... manipulate shared data ...
}
```

- A `thread` will not proceed until `lck`'s **constructor** has acquired the `mutex`.
- The corresponding destructor releases the resources.
- In this example, `scoped_lock`'s destructor releases the `mutex` when the thread of control **leaves `f()`**.
  - `f()`から離れる方法１：through a `return`.
  - `f()`から離れる方法２：by "falling off the end of the function".例えば`void`関数。
  - `f()`から離れる方法３：through an **exception throw**.

## `unique_ptr`や`vector`が似てるところ

- A `unique_ptr` is a handle to an individual object (**or an array**) in much the same way that a `vector` is a handle to a sequence of objects.
- **Both control the lifetime of other objects** (using RAII) and both rely on move semantics to make `return` simple and efficient.

## `shared_ptr`はなんのdata raceも解決できないので、自分で解決しよう。あくまでsmart pointerはresource managementが目的だ

- In particular, `shared_ptr`s do not in themselves provide any rules for which of their owners can read and/or write the shared object.
- Data races and other forms of confusion are not addressed simply by eliminating the resource management issues.

## pointer semanticsが必要の場合のみsmart pointerを使う。ほとんどの場合はcontainerは全然対応できる

- We **do not** need to **use a pointer to return a collection of objects from a function**; a container that is a resource handle will do that simply and efficiently.

## moveにふさわしい場合

- A compiler will prefer to move when an object **is about to be destroyed (as in a `return`)** because that's assumed to be the simpler and more efficient operation.

- swap.

  ```c++
  template <typename T>
  void swap(T& a, T& b){
      T tmp {move(a)};	// the T constructor sees an rvalue and moves
      a = move(b);		// the T assignment sees an rvalue and moves
      b = move(tmp);		// the T assignment sees an rvalue and moves
  }
  ```

  - 明示的にmoveを使うのが避けるべき。It shoule have been called something like `rvalue_cast`. Like other casts, it's error-prone and **best avoided**. これは僕の認識と違うので、びっくりした。

## moveへの心配

- メインの心配は**moved-from object**をまた使っちゃうこと。でも僕Clionでの経験だと、こういう場合はwarningが出る。（多分エラーも？）
  - しかもmoved-from objectはassigned-toの立場で使われても全然大丈夫でしょう。上記のswapの場合はそうです。
- The state of a moved-from object is in general unspecified, but all standard-library types leave a **moved-from object in a state where it can be destroyed and assigned to**.

## Range Checkingの最終案：`gsl::span`

- `string_view`と同じように`span<int>`: `{ begin(), size() }`。

  - resembles a `string_view` and **an STL pair of iterators**.

- `span`の使用例：

  ```c++
  void fs(span<int> p){
      for(int x : p)
          x = 0;
  }
  
  void use(int x){
      int a[100];
      fs(a);			// implicitly creates a span<int>{a,100}
      fs(a,1000);		// error: span expected
      fs({a+10,100});	// a range error in fs
      fs({a,x});		// obviously suspect
  }
  ```

  - **Creating a `span` directly from an array, is now safe (the compiler computes the element count)** and notationally simple.
  - When used for subscripting (e.g., `r[i]`), range checking is done and a `gsl::fail_fast` is thrown in case of a range error.

## Specialized Containers大雑把

- なぜSTLがspecialized containersを提供している？If the standard library didn't provide them, many people would **have to design and implement their own**.
- `tuple<T...>`: A sequence of an **arbitrary number** of elements of **arbitrary types**. 以前要素数が3だと思っていた...
  - `pair` and `tuple` are heterogeneous（つまり要素は違うタイプでもOK。例えば`std::map`の`pair<key, value>`）; all other containers are homogeneous (all elements are of the same type).
  - **tuple** elements are **contiguously allocated**. だから`get<0>`のようなとり方だ。
- `bitset` and `vector<bool>` hold bits and access them through **proxy objects**; all other standard-library containers can hold a variety of types and access elements directly.
- `valarray` requires its elements to be **numbers** and to provide numerical operations.
- `T[N]`は**implicitly converts to a T***.
  - An `array` is best understood as a built-in array with its size firmly attached, without implicit, potentially surprising conversions to pointer types, and with a few convenience functions provided.
  - An `array` does not follow the "handle to elements" model of STL containers. Instead, an `array` **directly contains its elements**.組み込みの場合、Cスタイル配列を`vector`より`array`に変換した方がいいかな？いいえ。

## 組み込みの場合`vector`や`array`をどっち使う？

- `array`のデータは全部stackに置く。The stack is a limited resource (**especially on some embedded systems**), and **stack overflow is nasty**. なので、組み込みの場合も`vector`を使った方がいいでしょう。

- `array`はたまに`vector`よりperformance advantageがある。**Occasionally**, there is a significant performance advantage to be had by directly accessing elements allocated on the stack rather than allocating elements on the free store, accessing them indirectly through the `vector` (a handle), and then deallocating them.

- でも`array`は簡単にpointerを受けるC-style関数に利用される。

  ```c++
  void f(int* p, int sz);	// C-style interface
  
  void g(){
      array<int,10> a;
      f(a,a.size());			// error: no conversion
      f(&a[0],a.size());		// C-style use
      f(a.data(),a.size());	// C-style use
      
      auto p = find(a.begin(),a.end(),777);	// C++/STL-style use
      // ...
  }
  ```

  - こういう場合、もし`a`が`vector`だと、多分もう１つoverrideの`f`が必要だろう。（`vector`を受けるoverride）

- どうしてもbuilt-in arrayはやめる。Bjarne's main reason to prefer `array` is that it saves him from **surprising and nasty conversions to pointers**. 例えば`Circle[]`が`Shape*`に変換されちゃって、またsubscriptingで要素を取ると、disaster。`sizeof(Shape)<sizeof(Circle)`なので、**subscripting a `Circle[]` through a `Shape*` gives a wrong offset**.

- ちなみに**stack**や**heap**の比較は下記のページは凄く分かりやすい。https://gribblelab.org/CBootCamp/7_Memory_Stack_vs_Heap.html

## `std::equal_range`の`Compare comp`の使い方

- `std::equal_range`は下記のようなinterfaceです。

  ```c++
  template<class ForwardIt, class T, class Compare>
      std::pair<ForwardIt,ForwardIt>
      equal_range(ForwardIt first, ForwardIt last, const T& value, Compare comp);
  ```

  - for all elements, if `comp(element, value)` is true, then `!comp(value, element)` is also true. つまり`value`はいつも`comp`の**2番め引数**になるようにロジックを設計すべき。

  - `comp`のsignatureは必ず下記のようです。

    ```c++
    bool pred(const Type1 &a, const Type2 &b);
    ```

## `std::get(std::tuple)`

- `std::tuple`の要素のタイプが全部違う場合のみ、typeでgetできる。
- もし`std::tuple`に同じタイプの要素が存在していれば、typeでgetすると、エラーが出ている。下記のページで試した。https://en.cppreference.com/w/cpp/utility/tuple/get

## 3種類Alternatives

- `variant` to represent **one of** a specified set of alternatives.
  - If we try to access a `variant` holding a different type than the expected one, `bad_variant_access` is thrown.
- `optional` to represent a value of a specified type **or no value**.
- `any` to represent **one of** an **unbounded** set of alternative types.
  - If we try to access an `any` holding a different type than the expected one, `bad_any_access` is thrown.

## `std::variant`の面白い応用：`std::visit`と一緒に使う（type-matching visitor）

- 参考例：https://en.cppreference.com/w/cpp/utility/variant/visit

  ```c++
  using var_t = std::variant<int, long, double, std::string>;
  
  // helper type for the visitor
  template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>; //not needed as of C++20
  
  int main(){
      std::vector<var_t> vec = {10, 15l, 1.5, "hello"};
      for(auto& v : vec){
          // type-matching visitor: a class with 3 overloaded operator()'s
          std::visit(overloaded {
              [](auto arg) { std::cout << arg << ' '; },
              [](double arg) { std::cout << std::fixed << arg << ' '; },
              [](const std::string& arg) { std::cout << std::quoted(arg) << ' '; },
          }, v);
      }
  }
  ```

  - 多分`using Ts::operator()...`の意味は、overloadsに全部`()` operatorを適用するよ！って言うこと。
  - 一番目のoverloadの引数タイプは`auto`ですが、`10`や`15l`のみこのoverloadに適用された、つまり一番appropriate lambdaを呼び出す。
  - `overloaded(Ts...) -> overloaded<Ts...>`は**deduction guide**という。良く分かっていないが、A deduction guide is a mechanism for resolving **subtle ambiguities**, primarily for **constructors of class templates in foundation libraries**.

- もちろんif文でチェックしてもいいし。`std::holds_alternative`を使えるし。`std::is_same_v<>`も使える。

  ```c++
  using Node = variant<Expression,Statement,Declaration,Type>;
  
  void check(Node* p){
      if(holds_alternative<Expression>(*p)){
          Expression& e = get<Expression>(*p);
          // ...
      }else if(holds_alternative<Statement>(*p)){
          Statement& s = get<Statement>(*p);
          // ...
      }
      // ... Declaration and Type ...
  }
  ```

  - 明らかにvisit patternよりは不便でしょう。

- ToDo：**Visitor**設計パターンを理解しよう。carlaにも使っている！

  - carlaにvisitorを使っているところは、`RoadInfoVisitor.h, RoadInfo.h, RoadInfoIterator.h `や各種RoadInfo子クラス。

## `optional`への理解

- An `optional<A>` can be seen as a special kind of `variant` (like a `variant<A,nothing>`) or as a **generalization of the idea of an `A*` either pointing to an object or being `nullptr`**.

- **The `optional` equivalent to `nullptr` is the empty object, `{}`**.

- `optional`の可能な使う場合：exceptionの代わりとして

  ```c++
  optional<string> compose_message(istream& s){
      string mess;
      // ... read from s and compose message ...
      if(no_problems)
          return mess;
      return {};	// the empty optional.
  }
  
  // ...
  if(auto m = compose_message(cin))
      cout << *m;	// note the dereference (*)
  else{
      // ... handle error ...
  }
  ```

  - An `optional` is treated as a **pointer to its object rather than the object itself**.

## new以外のAllocator：pool allocatorを例として

- 長時間運行するプログラムで、複数のproducerや複数のconsumerが`list<shared_ptr<Event>> q`をいじる。１つずつ`new`や`delete`だけのallocationだと、深刻なfragmentation問題が発生する。

- fragmentation問題を解決するため、pool allocatorを利用する。A pool allocator is an allocator that manages **objects of a single fixed size** and **allocates space for many objects at a time**, rather than using individual allocations.

- **standard container**はoptionally **take allocator arguments**.

- pool allocatorの対応前後の一部コードは下記のようです。

  ```c++
  // individual allocation
  q.push_back(make_shared<Event>());
  
  // pool allocation
  q.push_back(allocate_shared<Event,pmr::polymorphic_allocator<Event>>{&pool});
  ```

- 難しすぎる。説明できない。

## Time

- Bjarne Stroustrupは何度も何度もefficiencyについて測らないといけないと強調している。**Guesses about performance are most unreliable**. これは僕はまだ全然できてない。
- 測ることは`<chrono>`で超便利でしょう。
  - `duration_cast<milliseconds>`があって便利。
  - time-unit suffixesがあって便利。
  - Subtracting two `time_point`s gives a `duration`も便利。

## Function Adaption

- 例で見ると、分かりやすい。

  ```c++
  void draw_all(vector<Shape*>& v){
      for_each(v.begin(),v.end(),[](Shape* p){ p->draw(); });
  }
  ```

  - `for_each()`の3番め引数の形は`f(x)`ですが、`draw()`の呼び出す形は`x->f()`です。`x->f()`から`f(x)`のように渡すために、**lambda**で解決している。

- 解決方法2番め：`mem_fn()`: make a function object from a **member function**.

  ```c++
  void draw_all(vector<Shape*>& v){
      for_each(v.begin(),v.end(),mem_fn(&Shape::draw));
  }
  ```

- 解決方法3番め：Define the function. lambdaと同じじゃない？

  ```c++
  function fct = [](Shape* p){ p->draw(); };
  ```

  - a `function`, being an object, **does not participate in overloading**.
    - If you need to overload function objects (including lambdas), consider `overloaded`.

## Type Functions: `iterator_traits`

- C++のIteratorは、必ず下記の5種類propertiesを提供すべき。

  - difference_type, **value_type**, pointer, reference, **iterator_type**.

  - `iterator_traits`はIteratorからこの5種類の情報を取得するutilities。

  - そうすると、僕等はIteratorの関数だけ実現できる（Containerのタイプはどうでもいい）。**This makes it possible to implement algorithms only in terms of iterators**. さらにアルゴリズムやcontainerを分離できる！

  - 例

    ```c++
    template<typename Iter>
    using Iterator_category = typename std::iterator_traits<Iter>::iterator_category
    ```

- 自分でIteratorを定義するときも、この5種類情報を提供すべきでしょう！

  - 例えばcarlaのRoadInfoIterator.h。https://github.com/carla-simulator/carla/blob/master/LibCarla/source/carla/road/element/RoadInfoIterator.h
  - RoadInfoIterator.hにはiterator_categoryは用意してないけど。。。

- **standard-library container**の場合は、`iterator_traits`を使わなくても、簡単に`value_type`などを取れる。（`iterator_type`以外、でも`iterator`は取れるので、また`iterator_traits`を使えばOK）

  - 例えば`std::vector`は下記のmember typeをすぐ取れる：value_type, allocator_type, size_type, difference_type, reference, const_reference, pointer, const_pointer, iterator, const_iterator, reverse_iterator, const_reverse_iterator。例えば：

    ```c++
    template<typename C>
    using Value_type = typename C::value_type;
    using Iterator_type = typename C::iterator;
    ```

- `iterator_category`でiteratorに対して、overloadingができる！`iterator_category`は下記の6種類がある：input_iterator_tag, output_iterator_tag, forward_iterator_tag, bidirectional_iterator_tag, random_access_iterator_tag, contiguous_iterator_tag。https://en.cppreference.com/w/cpp/iterator/iterator_tags
  - 各種Iteratorに適するアルゴリズムがあるので、`iterator_category`で最適なoverloadingを適用する。

## Advice

- Prefer resource handles with specific semantics to smart pointers.
- **Don't use `std::move()`**. Why?? 多分メインの心配は下でしょう？
- Never read from an object after `std::move()`ing or `std::forward()`ing it.
- Use `array` where you need a sequence with a `constexpr` size.
- Don't overuse `pair` and `tuple`; named `struct`s often lead to more readable code. 確かに！
- Time your programs before making claims about efficiency.
- Use `mem_fn()` or a lambda to create function objects that can **invoke a member function** when called using the **traditional function call notation**.
- **Use `function` when you need to store something that can be called**. 2回同じlambdaを書かないといけない時はちょうどいいでしょう。
- Prefer concepts over traits and `enable_if` whenever you can. conceptsはまだ使えないでしょう！