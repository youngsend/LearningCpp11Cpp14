# 2.1.1 Launching a thread

## threadに簡単なタスク vs. 複雑なタスク

- 結構簡単なcase: task is a plain, ordinary **`void`-returning** function that **take no parameters**.
- 結構複雑なcase: the task could be a function object that 
  - takes additional parameters.
  - performs a series of independent operations that are **specified through some kind of messaging system** while it's running. 
  - the thread **stops only when it's signaled to do so**, again via some kind of messaging system.

## `std::thread` works with any callable type

- 種類１：関数。

- 種類２：an instance of a **class with a function call operator**を`std::thread` constructorに渡してもOK！

  ```c++
  class background_task {
      public:
      void operator()() const {
          do_something();
          do_something_else();
      }
  };
  background_task f;
  std::thread my_thread(f);
  ```

  - つまり、function objectを渡す。ここはclassですが、structも同様。
  - the supplied function object is **copied** into the storage belonging to the newly created thread of execution and invoked from there.

- 種類３：*lambda expression*

  ```c++
  std::thread my_thread([]{
      do_something();
      do_something_else();
  })
  ```

## `std::thread my_thread(background_task());`がダメである原因や回避方法

- この見出しのコードは**関数宣言(function declaration)**になってしまう。
  - **declares a `my_thread` function** that takes a single parameter (of type `pointer-to-a-function-taking-no-parameters-and-returning-a-background_task-object`) and **returns a `std::thread` object**, rather than launching a new thread.
  - 今回はbackground_taskはクラスですが、普通の関数でも（）はつけちゃダメ。Pythonでもそうです。
- ３種回避方法：
  - by naming your function object.
  - by using an extra set of parentheses. `std::thread my_thread((background_task()));`
  - by **using `{}`**. `std::thread my_thread{background_task()};` これは一番いいでしょう。the uniform initialization syntax.

## 必ず`std::thread`を`join()`する、もしくは`detach()`する。`std::thread`とthreadを区別しよう。

- 両方しないと、エラーが発生する。If you don't decide before the `std::thread` object is destroyed, then your program is terminated (the `std::thread` destructor calls `std::terminate()`).
- `std::thread`とthreadを区別しよう！**`join()`や`detach()`はあんまりthreadの動きのタイミング）と関係なさそう**。
  - You only have to make this decision before the `std::thread` object is destroyed - **the thread itself may well have finished long before** you join with it or detach it, and **if you detach it**, then if the thread is still running, it will continue to do so, and **may continue running long after the `std::thread` object is destroyed**; it will **only stop running when it finally returns from the thread function**.

## `detach()`の場合、threadが終わるまでアクセスするデータの有効性の確保

- If you don't wait for your thread to finish, you need to ensure that the data accessed by the thread is valid until the thread has finished with it. 違反の例（醜い）：

  ```c++
  struct func{
      int& i;
      func(int& i_):i(i_){}
      void operator()(){
          for(unsigned j=0; j<1000000;j++){
              do_something(i); // potential access to dangling reference.
          }
      }
  };
  
  void oops(){
      int some_local_state=0;
      func my_func(some_local_state);
      std::thread my_thread(my_func);
      my_thread.detach(); // don't wait for thread to finish.
  } // new thread might still be running.
  ```

  - 回避方法１：make the thread function self-contained and **copy** the data into the thread.
  - 回避方法２：ensure that the thread has completed execution before the function exits by **joining** with the thread.

- It's a **bad idea** to **create a thread within a function that has access to the local variables in that function**, unless the thread is guaranteed to finish before the function exits.

# 2.1.2 Waiting for a thread to complete

## `join()`よりfine-grainedコントロールが必要な場合

- fine-grainedコントロールの例：
  - check whether a thread is finished.
  - wait only a certain period of time.
- 対策：`join()`以外の方法、例えばcondition variables, futures.

## `std::thread`オブジェクトに対して、`join()`は1回のみ呼び出せる。その後`joinable()`はずっとfalse。

- 理由：The act of calling `join()` **cleans up any storage associated with the thread**, so the `std::thread` object is no longer associated with the now-finished thread; **it isn't associated with any thread**.
- 結果：once you've called `join()`, the `std::thread` object is **no longer joinable**, and `joinable()` will return `false`.

# 2.1.3 Waiting in exceptional circumstances

## `detach()`の場合は心配なし

- 理由：`detach()`を呼び出すと、`std::thread`とthreadは既に関係がなくなった。Exceptionがあってもなくてもthreadは実行し続く。
  - This **breaks the association of the thread with the `std::thread` object** and ensures that `std::terminate()` won't be called when the `std::thread` object is destroyed, even though the thread is still running in the background.

## `join()`の場合、RAIIでExceptionの状況も対応できる

- `join()`は、`std::thread`のスタートと離れるので、その間にExceptionが発生すると、`join()`を呼び出せなくて、`std::terminate()`が呼び出される。プログラムが落ちる。これを防ぐために、全パスに`join()`を入れないといけない！

  - 実はpointerのdeleteと同じ課題だ。

- 対策は、RAIIです！

  ```c++
  class thread_guard{
      std::thread& t;
  public:
      explicit thread_guard(std::thread& t_):t(t_){}
      ~thread_guard(){
          if(t.joinable()){
              t.join();
          }
      }
      thread_guard(thread_guard const&)=delete;
      thread_guard& operator=(thread_guard const&)=delete;
  };
  
  struct func;
  void f(){
      int some_local_state=0;
      func my_func(some_local_state);
      std::thread t(my_func);
      thread_guard g(t);
      do_something_in_current_thread();
  }
  ```

# 2.1.4 Running threads in the background

## `detach()`したら、`std::thread` objectとthreadは分離する

- With no direct means of communicating with this thread.
- If a thread become detached, it **isn't possible to obtain a `std::thread` object that references it** , so it can no longer be joined.
- Ownership and control of this thread are passed over to the C++ Runtime Library, which ensures that the resources associated with the thread are correctly reclaimed when the thread exits.
- Detached threads are often called *daemon threads*.
- When `detach()`, there must be a thread to detach. その判定は`joinable()`でできる。

## `detach()`にぴったりの応用例：同時にmultiple documentsを編集できるword processor

- 設計：Run each document-editing window in its own thread; **each thread runs the same code but with different data** relating to the document being edited and the corresponding window properties.

- The thread handling the request **isn't going to care about waiting for that other thread to finish, because it's working on an unrelated document**, so this makes it a prime candidate for running a detached thread.

  ```c++
  void edit_document(std::string const& filename){
      open_document_and_display_gui(filename);
      while(!done_editing()){
          user_command cmd = get_user_input();
          if(cmd.type == open_new_document){ // reuse the same function: edit_document
              std::string const new_name = get_filename_from_user();
              std::thread t(edit_document, new_name); // 関数自分をnew threadに渡している
              t.detach();
          } else {
              process_user_input(cmd);
          }
      }
  }
  ```

# 2.2 Passing arguments to a thread function

## constructorは基本argumentsをコピー！

- By default, the **arguments are copied into internal storage**, where they can be accessed by the newly created thread of execution, and **then passed to the callable object or function as rvalues** as if they were temporaries.

  - だから、implicit conversionが発生しても、遅いかも。
  - なので、例えばchar*じゃなく、`std::string`オブジェクトを引数として`std::thread`に渡す！

- なぜrvalueでcallable object or functionに渡す？

  - The internal code passes copied arguments as rvalues in order to work with move-only types.

- ではどうやってreferenceを`std::thread`に渡す？：**`std::ref`**

  ```c++
  void update_data_for_widget(widget_id w, widget_data& data);
  void not_oops(widget_id w){
      widget_data data;
      std::thread t(update_data_for_widget,w,std::ref(data));
      display_status();
      t.join();
      process_widget_data(data);
  }
  ```

## move-onlyタイプargumentを渡す場合：moveする

```c++
void process_big_object(std::unique_ptr<big_object>);
std::unique_ptr<big_object> p(new big_object); // make_uniqueを使った方がいいでしょう
p->prepare_data(42);
std::thread t(process_big_object,std::move(p));
```

- Where the source object is temporary, the move is automatic, but where the source is a named value, the transfer must be requested directly by invoking `std::move()`.
  - temporary objectはつまり名前なしのobjectでしょう。例えば関数のreturn値、string literalなど。
- moveされているのは、source objectが指している**resourceのownership**だ！
  - move-onlyというのは、このresourceのownershipは独占するしかない。シェアできない！
- `std::thread`もそもそもmove-onlyタイプです。
  - `std::thread`が指しているresourceはthreadだ。なので、`std::thread`をmoveすることは、threadのownershipが変わることだ。
- move-onlyタイプのもう1つ例：`std::ifstream`。みんなresource-owningタイプです。

# 2.3 Transferring ownership of a thread

- 既にthreadを持っている`std::thread`にassignするのがダメ。しかし、持っていたthreadがmoveされちゃった`std::thread`にはassignできる。
  - また、`join()`された`std::thread`にもassignできる。（`detach()`された`std::thread`も同様だと思う）

## move-onlyのため、containers of `std::thread`が便利

- 良く使われるcontainers of `std::unique_ptr`と似てる。

```c++
void do_work(unsigned id);
void f(){
    std::vector<std::thread> threads;
    for(unsigned i=0;i<20;++i){
        // Spawn threads.
        threads.emplace_back(do_work,i); // 直接にstd::threadのconstructorに渡す
    }
    for(auto& entry : threads)
        entry.join();
}
```

# 2.4 Choosing the number of threads at runtime

- `std::thread::hardware_concurrency()`で実際平行実行できるスレッド数が分かるので、その数分を越えないように。分からない場合は、0がreturnされる。
- また、スレッドが暇過ぎることを防ぐために、最小仕事量（例えば処理する最小要素数）を決めた方がいいでしょう。

# 2.5 Identifying threads

- `std::thread::id` can be used as keys in associative containers, or sorted...
- `std::hash<std::thread::id>`もあるので、unordered containersにも使える。
- thread IDの唯一保証：（std::coutの場合）thread IDs that compare as equal should produce the same output, and those aren't equal should give different output. thread IDのvalueは特にsemantic意味はなし。
- メインの使う場合：Identifying threads in order to **associate data or behavior with specific threads** that's inconvenient to associate through alternative means.

# 復習項目

- Basics of thread management.
  - starting threads.
  - waiting for them to finish.
  - *not* waiting for them to finish because you want them to run in the background.
- How to **pass arguments** into the thread function when a thread is started.
- How to **transfer the responsibility for managing a thread** from one part of the code to another.
- How **groups of threads** can be used to divide work.
- Identifying threads.