# 5.1 Memory model basics

- objectsやmemory locationsの定義：Whatever its type, an object is stored in one or more *memory locations*. Each memory location is either an object (or sub-object) of a scalar type such as `unsigned short` or `my_class*` or a sequence of adjacent bit fields.
  - zero-length bit field (must be unnamed)の意味はstart a new byte.
- data raceを防ぐために、enforced ordering between accesses in two threadsが必要。一方mutex；また*atomic* operations。
- modification orders.

# 5.2 Atomic operations and types in C++

## 5.2.1 The standard atomic types

- atomic typesでも2種類の実現が存在する。`is_lock_free()`でどっちかをチェックできる。
  - `is_lock_free()`がtrue: directly with atomic instructions.
  - `is_lock_free()`がfalse: by using a lock internal to the compiler and library.
- C++17から`is_always_lock_free()`. trueの条件：the atomic type X is lock-free for all supported hardware that the output of the current compilation might run on.
- 唯一`is_lock_free()`関数を提供していないatomic typeは`std::atomic_flag`。
  - そもそも`std::atomic_flag`はrequired to be lock-free.
  - `std::atomic_flag`を使って、簡単なlockを実現できる。例はここ：https://en.cppreference.com/w/cpp/atomic/atomic_flag
- atomic typesの一切assignment operatorsやmember functionのreturn値は全部`std::atomic<T>`の`T`です。（either the value stored or the value prior to the operation）絶対`std::atomic<T>&`じゃない！

## 5.2.2 Operations on `std::atomic_flag`

- 唯一初期化が必要のatomicタイプ。`std::atomic_flag f=ATOMIC_FLAG_INIT;`。It's the only atomic type to require such special treatment for initialization; but it's also the **only type guaranteed to be lock-free**.
- 唯一3種類操作：destroy it; clear it; set it and query the previous value.

## 全てのatomicタイプがcopy-constructやcopy-assignmentがダメの原因

- copy constructやcopy assignmentは2つobjectが必要なので、atomic operationにならない。
  - All operations on an atomic type are defined as atomic, and assignment and copy-construction involve two objects. **A single operation on two distinct objects can't be atomic**.

## `std::atomic_flag`でspinlock mutexを実現

```c++
class spinlock_mutex{
    std::atomic_flag flag;
public:
    spinlock_mutex():flag(ATOMIC_FLAG_INIT){}
    void lock(){
        while(flag.test_and_set(std::memory_order_acquire));
    }
    void unlock(){
        flag.clear(std::memory_order_release);
    }
};
```

- `flag.test_and_set()`がtrueの時loop。`test_and_set()`は前回の状態を返すので、trueだったら、つまりflagは誰が設定した。つまりlockされている状態です。falseだったら、つまり誰が既に`clear()`したんだ。しかも今回の`test_and_set()`で自分がlockできた。
- これはbusy-waitですね。

# 5.2.3 Operations on `std::atomic<bool>`

- https://en.cppreference.com/w/cpp/atomic/atomic
- `std::atomic<>`の共通member functionの一部は
  - constructor: `atomic(T desired)`もしくは`atomic()`. copy-constructorはダメ。
  - `operator=`と`store()`は一緒。
  - **`operator T`と`load()`は一緒**。
  - `T exchange(T desired, order)`。

## `compare_exchange_weak()`や`compare_exchange_strong()`の動作

`bool compare_exchange_weak(T& expected, T desired, std::memory_order success, std::memory_order failure)`

`bool compare_exchange_strong(T& expected, T desired, std::memory_order success, std::memory_order failure)`

- **`expected`は`T&`**！

- `expected`と比較して、一緒だったら、`desired`でatomic variableを更新。

- **一緒じゃなかったら、value of the atomic variableで`expected`を更新**。

  - このやり方のメリットは、もし他のスレッドがvalueを更新してなければ、次の周期でcompare_exchangeは成功するはず。

- `compare_exchange_weak()`はvalue of the atomic variableとexpectedが同じの場合でも失敗する可能性がある。原因はlack a single compare-and-exchange instruction。

  - なので、`compare_exchange_weak()`はよくloopに使われる。（spurious failureを防ぐために）

    ```c++
    bool expected=false;
    extern atomic<bool> b; // set somewhere else
    // 下記のloopはspurious failureの場合のみ続く！下記の分析を参考
    while(!b.compare_exchange_weak(expected,true) && !expected);
    ```

    - `compare_exchange_weak()`がtrueだったら、成功なので、while loop終わり。
    - `compare_exchange_weak()`がfalseだったら、2可能性がある。
      - 本当の失敗（`b.value==true`）、expectedはbのvalueで更新されるはず、つまりtrueになる。while loop終わり。
      - spurious失敗（`b.value==false`）、expectedは更新されてもされなくてもfalse。while loop続く。もし更新されたら無駄です。

- `compare_exchange_strong()`は多分まさに上記のコードで実現されている。つまり`compare_exchange_strong()`の内部にloopがある。

## User-Defined Typeも`std::atomic`できる条件

- This type must have a ***trivial* copy-assignment operator**.
  - must not have any virtual functions or virtual base classes.
  - must use the **compiler-generated copy-assignment operator**.
    - つまり`T& T::operator=(const T&)=default`.
    - なぜdefault (compiler-generated)じゃないといけない？If user-defined copy-assignment or comparison operators were permitted, this would **require passing a reference to the protected data as an argument to a user-supplied function**. これは危険です。lockのscope以外にデータが渡されるかも。 
  - Every base class and non-static data member of a user-defined type must also have a trivial copy-assignment operator.

- つまり、例えば`std::atomic<std::vector<int>>`はダメ。because it has a non-trivial copy constructor and copy assignment operator.
  - この場合は、mutexを使ってください。
- 僕の理解だと、trivial copyできるobjectは、全てのdataが自分の中にある、heapに預けてない。また、atomicタイプのcompareは**bitwise comparison**なので、trivial copyできるobjectに相応しい。

# 5.3 Synchronizing operations and enforcing ordering

## 5.3.1 The *synchronize-with* relationship

- 定義：The *synchronizes-with* relationship is something that you can get **only between operations on atomic types**.
  - いつか反映されること（write -> read）だと思う。なので、反映されるまでreadの方でwhile loopでやるのが多い。
    - while loopを利用することで、write -> readの*synchronize-with*関係を保証するんだ。
  - ２つスレッドが、それぞれwrite, readする場合、必ずwrite->readではないが、もしwriteが発生したら、readしたvalueは必ず最新と言う意味だと思う。

## 5.3.2 The *happens-before* relationship

- 文字通りの意味だ。source code上の順番。
- 逆に*happens-before*関係がunspecifiedな例は、**If the operations occur in the same statement**. 例えば`foo(get_num(), get_num());`、どのcall先に発生するか分からない。
- *inter-thread happens-before*と*synchronizes-with*の関係：if operation A in one thread *synchronizes with* operation B in another thread, then A *inter-thread happens-before* B.

# 5.3.3 Memory ordering for atomic operations

- *sequentially consistent* ordering: `memory_order_seq_cst`, default。一番コスト高い。
- *acquire-release* ordering: `memory_order_consume, memory_order_acquire, memory_order_release, memory_order_acq_rel`。`memory_order_consume`はC++17から勧められてないので、無視しとく。
- *relaxed* ordering: `memory_order_relaxed`。完全自由。一番理解しやすい。

## Sequentially Consistent Ordering

- 僕の理解：あるatomic変数xに大して、writeがさえ終わったら、全てのthreadから見れるようになる、つまり全てのreadのvalueが最新になる。
  - そう言う意味だと、つまり全てのthreadがいつでも同じvalueを見る。
- **global synchronization between all threads**.

## Relaxed Ordering

- thread Aでxに書き込んでも、thread Bが見れるかどうか保証ない。つまり**visibility**の保証がない。
- relaxed orderingを理解できる図！![](/home/sen/senGit/LearningCpp11Cpp14/C++-Concurrency-in-Action_Second-Edition/img/relaxed-ordering-2020-05-17 14-15-44.png)
  - 5,10,23,3,1,2,42,67はxの編集履歴。Carl, Dave, Anne, Fred, Youはthread。書き込む場合、書き込むthreadだけは自動的に書き込んだ数字もしくはその以降の数字が見せられる。読む場合は、threadが前回見た数字もしくはその以降の数字が見せられる。thread間は特に関係ない、保証ない、同期ない。
- Anthony Williamsはstrongly recommend avoiding relaxed atomic operation.

## Acquire-Release Ordering

- thread間の**pairwise**同期

- ３つoperation typeと３つmemory order typeは対応関係がある。

  - atomic load: `memory_order_acquire`.
  - atomic store: `memory_order_release`.
  - atomic read-modify-write: *acquire*, *release* or `memory_order_acq_rel`.

- 同じスレッドのstore（例えばstore x, store y、順番あり）がbatchになって、他のスレッドが、もしこのbatchの中のstore yを見れたら、必ずstore xも（もしくはその以降のbatchのxへの修正）見れてる。これで、**２つスレッド間は同期できる**。例：

  ```c++
  std::atomic<bool> x,y;
  std::atomic<int> z;
  void write_x_then_y(){
      x.store(true, std::memory_order_relaxed); // 1
      y.store(true, std::memory_order_release); // 2
  }
  void read_y_then_x(){
      while(!y.load(std::memory_order_acquire)); // 3
      if(x.load(std::memory_order_relaxed)) // 4
          ++z;
  }
  int main(){
      x=false;
      y=false;
      z=0;
      std::thread a(write_x_then_y);
      std::thread b(read_y_then_x);
      a.join();
      b.join();
      assert(z.load()!=0);
  }
  ```

  - 結果として、zは必ず1になる。1,2は同じbatchになる。3のwhile loopで2が反映されることを保証する。つまりbatchの2を見れてる。なので、同じbatch、しかも2の前の行ったbatchの1も反映されている。なので、`x.load()`で一回だけだけど、必ずtrueを見れる。
  - でも1,2順番違ったら、保証ない。
  - また、2,3が`memory_order_relaxed`になったら、保証ない。

- acquire-releaseは２つスレッド間しか同期できないので、注意。acquireとreleaseのペアだから。

- **transitive synchronization with acquire-release ordering**: 上記の２つスレッドacquire-releaseを理解したら、下記の例も分かりやすい。thread AとBがacquire-release, thread BとCがacquire-release関係を持ってる。thread AとCは全然シェアデータがないけど、同期できる。https://github.com/anthonywilliams/ccia_code_samples/blob/master/listings/listing_5.9.cpp

## 5.3.4 Release sequence and synchronizes-with

僕の理解は、１つスレッドのreleaseが、複数スレッドのacquireと同期できること。間違っているかも。

## 5.3.5 Fences

relaxedからrelease-acquire関係にするもう１つ方法。例から分かりやすい。上記の例と似てる。

```c++
std::atomic<bool> x,y;
std::atomic<int> z;
void write_x_then_y(){
    x.store(true, std::memory_order_relaxed); // 1
    std::atomic_thread_fence(std::memory_order_release); // 5
    y.store(true, std::memory_order_relaxed); // 2
}
void read_y_then_x(){
    while(!y.load(std::memory_order_relaxed)); // 3
    std::atomic_thread_fence(std::memory_order_acquire); // 6
    if(x.load(std::memory_order_relaxed)) // 4
        ++z;
}
int main(){
    x=false;
    y=false;
    z=0;
    std::thread a(write_x_then_y);
    std::thread b(read_y_then_x);
    a.join();
    b.join();
    assert(z.load()!=0);
}
```

- 5が1,2に順番をつけた。6が3,4に順番をつけた。3が2の発生を保証するので、4も必ず1のあとで行うので、zは1になる。
- It's only when the fence comes *between* the store to x and the store to y that it imposes an ordering.

## 5.3.6 Ordering non-atomic operations with atomics

改めて注意喚起：**the real benefit of using atomic operations to enforce an ordering is that they can enforce an ordering on non-atomic operations** and avoid the undefined behavior of a data race. non-atomic operationの間に順番をつけるのがatomic operationを利用する初心だ。例えば上記の例の変体：行動が変わらない。

```c++
bool x;
std::atomic<bool> y;
std::atomic<int> z;
void write_x_then_y(){
    x = true; // 1
    std::atomic_thread_fence(std::memory_order_release); // 5
    y.store(true, std::memory_order_relaxed); // 2
}
void read_y_then_x(){
    while(!y.load(std::memory_order_relaxed)); // 3
    std::atomic_thread_fence(std::memory_order_acquire); // 6
    if(x) // 4
        ++z;
}
int main(){
    x=false;
    y=false;
    z=0;
    std::thread a(write_x_then_y);
    std::thread b(read_y_then_x);
    a.join();
    b.join();
    assert(z.load()!=0);
}
```

## 5.3.7 Ordering non-atomic operations

- mutexの実現は大体atomic operationのrelease-acquire関係を利用している：`lock()` is an acquire operation on an internal memory location, and `unlock()` is a release operation on that same memory location.
- その他の機能も全部いろんなsynchronize-with関係。`std::thread; std::mutex; std::shared_mutex; std::promise; std::future; std::shared_future; std::packaged_task; std::async;`など
  - Condition variables do not provide any synchronization relationships. They are optimizations over busy-wait loops, and all the synchronization is provided by the operations on the associated mutex.

# 復習項目

- the basic atomic types provided by specializations of the `std::atomic<>` class template.
- generic atomic interface provided by the primary `std::atomic<>` template.
- operations on these types.
- complex details of the various memory-ordering options.
- fences and how they can be paired with operations on atomic types to enforce an ordering.
- how the atomic operations can be used to enforce an ordering between non-atomic operations on separate threads.
- synchronization relationships provided by the higher-level facilities.

