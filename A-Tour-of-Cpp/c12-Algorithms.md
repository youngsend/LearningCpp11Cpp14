# Chapter 12: Algorithms

## Iteratorのメリット：data storageやalgorithmの分離

- An algorithm operates on its data through iterators and knows nothing about the container in which the elements are stored.
- Conversely, a container knows nothing about the algorithms operating on its elements; all it does is to supply iterator upon request (e.g., `begin()` and `end()`).

## container template関数にiteratorを使う

```c++
template<typename C, typename V>
vector<typename C::iterator> find_all(C& c, V v) {
    vector<typename C::iterator> res;
    for(auto p=c.begin(); p!=c.end(); ++p)
        if(*p==v)
            res.push_back(p);
    return res;
}
```

- 上記の実装だと、`<typename C::iterator>`の`typename`はいつも必要です。To inform the compiler that `C`'s `iterator` is supposed to be a type and not a value of some type.

- だから**type alias**を使いましょう。

  ```c++
  template<typename T>
  using Iterator = typename T::iterator;
  
  template<typename C, typename V>
  vector<Iterator<C>> find_all(C& c, V v) {
      vector<Iterator<C>> res;
      for(auto p=c.begin(); p!=c.end(); ++p)
          if(*p==v)
              res.push_back(p);
      return res;
  }
  ```

- 多分template化しやすいのが、各containerに同じようなinterfaceを用意する目的の１つでしょう。

## Iterator is a concept

- What is common for all iterators is their semantics and the naming of their operations. For example, applying `++` to any iterator yields an iterator that refers to the next element. Similarly, `*` yields the element to which the iterator refers.
- Iterator types can be as different as the containers, and the specialized needs they serve.
  - 例えばvectorのiteratorは、普通のpointerでもいいし、pointer to vector + index（allow range checking）でもいい。

## `istream_iterator`や`ostream_iterator`

- Stream iteratorを使うために、which stream to use and the type of objectの指定が必要。

- 例えば`fstream`の場合：

  ```c++
  string from, to;
  // ...
  ifstream is {from};
  ofstream os {to};
  
  set<string> b {istream_iterator<string>{is},istream_iterator<string>{}};
  copy(b.begin(), b.end(), ostream_iterator<string>{os,"\n"});
  ```
  - `istream_iterator`はいつも**used in pairs**. その2番めはused to **indicate the end of input**.
  - `ostream_iterator`の2番めargumentはused to delimit output values.
  - ファイルの読み込みには本当に便利です！
  - csvのパースも便利になるかな？試そう！もしdelimiterを指定できれば！

- もう1つ例：istream_iteratorはstringstream、ostream_iteratorはcoutを使う。

  ```c++
  std::istringstream str("0.1 0.2 0.3 0.4");
  std::partial_sum(std::istream_iterator<double>(str),
                  std::istream_iterator<double>(),
                  std::ostream_iterator<double>(std::cout, " "));
  ```

## Predicates

- make an action a parameter to an algorithm.

- 例えば`find_if()`の3番めargument。Lambdas are very common as operations passed as arguments. 

  ```c++
  std::istringstream str("1 3 5 7 8 9 10");
  std::cout << "\nThe first even number is " <<
      *std::find_if(std::istream_iterator<int>(str),
                   std::istream_iterator<int>(),
                   [](int i){return i%2 == 0;})
      << ".\n";
  ```

- function objectを使ってもいい。structはよく使われてますね！

  ```c++
  void f(map<string,int>& m){
      auto p = find_if(m.begin(),m.end(),Greater_than{42});
      // ...
  }
  
  struct Greater_than {
      int val;
      Greater_than(int v):val{v}{} // constructor
      bool operator()(const pair<string,int>& r)const{return r.second>val;}
  }
  ```

- `p=find_if(b,e,f)`: `p` is the first `p` in `[b:e)` so that `f(*p)`.

- `f=for_each(b,e,f)`: For each element `x` in `[b:e)` do `f(x)`.

  ```c++
  vector<int> v = {0,1,2,3,4,5};
  for_each(v.begin(),v.end(),[](int& x){x=x*x;});
  ```

## Core language conceptsの中のCommon<T,U>

- 定義：`T` and `U` share a common type. 違うタイプを比較できるかをチェック用。

- 例えば、`std::string`やC-style string (`char*`)は比較したい。`int`や`double`も比較したい。

  - そういう場合、`Common`の中にこの2タイプの`common_type_t`を定義しないといけない（もしまだ定義されてなければ）。

  ```c++
  using common_type_t<std::string,char*> = std::string;
  using common_type_t<double,int> = double;
  ```

- `Common` or `CommonReference` is used in the definitions of most concepts and algorithms that can compare values of different types.

## Object conceptsの中のRegular<T>, SemiRegular<T>

- `Regular<T>`: `SemiRegular<T>` and `EqualityComparable<T>`.
- `Regular` is the ideal for types.
  - **A `Regular` type works roughly like an `int` and simplifies much of our thinking about how to use a type**.
- The lack of default `==` for classes means that most classes start out as `SemiRegular` even though **most could and should** be `Regular`.

## Iterator Typesの中のSentinel<S,I>

- 定義：An `S` is a sentinel for an `Iterator` type; that is, `S` is a predicate on `I`'s value type.

## Conceptsの他の種類

- Comparison concepts.
- Callable concepts.
- Range concepts.

## Vectorized execution, vectorization, SIMD ("Single Instruction, Multiple Data")

- *vectorized execution*: tasks are done on a single thread using vectorization, also known as *SIMD*.

- vectorizationの例：https://stackoverflow.com/questions/1516622/what-does-vectorization-mean

  - 元々のタスク：

    ```c++
    for(i=0; i<N; i++){
        a[i]=a[i]+b[i];
    }
    ```

  - vectorization後：

    ```c++
    for(i=0; i<(N-N%VF); i+=VF){
        a[i:i+VF]=a[i:i+VF]+b[i:i+VF];
    }
    ```

## Advice

- Use predicates and other function objects to give standard algorithms a wider range of meanings.
- **A predicate must not modify its argument**.
- Know your standard-library algorithms and prefer them to hand-crafted loops.