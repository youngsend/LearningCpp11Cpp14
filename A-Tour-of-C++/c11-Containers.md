# Chapter 11: Containers

## vectorの`push_back`の実現

```c++
template<typename T>
void Vector<T>::push_back(const T& t){
    if(capacity()<size()+1)				// make sure we have space for t
        reserve(size()==0?8:2*size());	// double the capacity
    new(space) T{t};					// initialize *space to t
    ++space;
}
```

- Vectorクラスの３つpointers.
  - `T* elem`: pointer to first element.
  - `T* space`: pointer to first unused (and uninitialized) slot.つまりone-past-the-last elementを指す。
  - `T* last`: pointer to last slot.つまりone-past-the-last allocated spaceを指す。
  - `elem`から`space`まではelements。`space`から`last`はextra spaceです。
    - `int size()`: number of elements. `space-elem`.
    - `int capacity()`: number of slots available for elements. `last-elem`.
    - `void reserve(int newsz)`: increase **capacity** to newsz. It may have to allocate new memory and when it does, it moves the elements to the new allocation.
- double the capacityのメリット：allocation and relocation of elements happen **only infrequently**. 
- Bjarne Stroustrupはonly explicitly use `reserve()` to avoid reallocation of elements when he wants to **use pointers to elements**. 多分pointers to elementsを使う場合は特にelementsが移動されたくないでしょう。

## vectorのElements

- The element is **not a reference or a pointer to some object**. This makes for nice, compact containers with fast access.
- If you have a **class hierarchy** that relies on **virtual functions** to get **polymorphic behavior**, do not store objects directly in a container. Instead store a **smart pointer**.
  - raw pointerはまだleakの可能性がある。deleteを忘れるか、deleteの前returnしちゃうか、deleteの前exceptionが出ちゃうか。
  - `vector<unique_ptr<Shape>> vups;`.
  - `vector<Shape> vs;`: No, don't - there is no room for a Circle or a Smiley.

## exceptionのcatchについて

- If the user doesn't catch an exception, the program will **terminate in a well-defined manner** rather than proceeding or failing in an undefined manner.

- One way to minimize surprises from uncaught exceptions is to use a `main()` with a `try`-block as its body.

  ```c++
  int main(){
      try{
          // your code
      }
      catch(out_of_range&){
          cerr << "range error\n";
      }
      catch(...){
          cerr << "unknown exception thrown\n";
      }
  }
  ```

## vectorやlist、どっち？

- When all we want is a sequence of elements, we have a choice between using a vector or a list.
- **Unless you have a reason not to, use a vector**.
- A vector performs better for traversal (e.g., `find()` and `count()`) and for sorting and searching (e.g., `sort()` and `equal_range()`).
- A vector is usually more efficient than a list for short sequences of small elements (even for `insert()` and `erase()`).

## unordered containerのhash functionの実現方法

- A hash function is often provided as a function object.

  ```c++
  struct Record {
      string name;
      int product_code;
  };
  
  struct Rhash {
      size_t operator()(const Record& r) const {
          return hash<string>()(r.name) ^ hash<int>()(r.product_code);
      }
  };
  
  unordered_set<Record,Rhash> my_set; // set of Records using Rhash for lookup
  ```

  - Creating a new hash function by **combining existing hash functions using exclusive-or (`^`) is simple and often very effective**.

- We can avoid explicitly passing the hash operation by defining it as a specialization of the **standard-library hash**. 例えばcarlaのWaypointのhashの例：https://github.com/carla-simulator/carla/blob/master/LibCarla/source/carla/road/element/Waypoint.h

  ```c++
  namespace std {
      template<>
      struct hash<carla::road::element::Waypoint> {
          using argument_type = carla::road::element::Waypoint;
          using result_type = uint64_t;
          result_type operator()(const argument_type &waypoint) const;
      };
  }
  ```

- Given a good hash function, an unordered_map is much faster than a map for large containers. However, the worst-case behavior of an unordered_map with a poor hash function is far worse than that of a map.

## Advice

- Consider the singly-linked list, `forward_list`, a container **optimized for the empty sequence**. An empty forward_list occupies just one word, whereas an empty vector occupies three. Empty sequences, and sequences with only an element or two, are surprisingly common and useful.

- **Use `reserve()` to avoid invalidating pointers and iterators to elements**.

  - Don't assume performance benefits from `reserve()` without measurement.

- **Don't use iterators into a resized vector**.

  - こういうことをしているかをチェック！！

  - resized vectorの意味は、elementsが増えたり減ったりすること。

  - `push_back`の場合（https://en.cppreference.com/w/cpp/container/vector/push_back）：**If the new `size()` is greater than `capacity()` then all iterators and references (including the past-the-end iterator) are invalidated. Otherwise only the past-the-end iterator is invalidated**.

  - `iterator erase( iterator pos )`の場合はiteratorを使っても問題なさそう。

    - **更新したiterator**を返しているから。**C++11 and later**.https://en.cppreference.com/w/cpp/container/vector/erase
    - `erase()`はきっと`space`ポインターを更新するから。`end()`はきっと`space`ポインターを参照しているから。本当？

    ```c++
    std::vector<int> c{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    // つまりerase()がiteratorを更新するだけじゃなく、end()も更新している？
    for (auto it = c.begin();it != c.end(); ){
        if (*it % 2 == 0) {
            // erase() invalidates the iterator, use returned iterator.
            it = c.erase(it);
        } else {
            ++it;
        }
    }
    ```

- Use `forward_list` for sequences that are usually empty. 対向車線vectorの変わりに使えそう。
- Prefer compact and contiguous data structures.
  - Use unordered containers if you need fast lookup for large amounts of data.
  - Use unordered containers for element types with no natural order (e.g., no reasonable `<`).
  - When it comes to performance, don't trust your intuition: measure.

