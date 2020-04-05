## `join()`の意味

```c++
void f();	// function
struct F{	// function object
    void operator()();	// F's call operator
};

void user(){
    thread t1 {f};	// f() executes in separate thread
    thread t2 {F()};	// F()() executes in separate thread
    
    t1.join();	// wait for t1
    t2.join();	// wait for t2
}
```

- The `join()`s ensure that we **don't exit `user()` until the threads have completed**.
- To **"join" a `thread`** means to **"wait for the thread to terminate"**.
- ２つthreadの初期化ですが、`f`を渡す場合も実はfunction objectが作られる。
- やっぱりスレッドを作る時、function objectの方が綺麗です。
  - functionの場合は、`thread` variadic template constructorを利用する。例えば`thread t1 {f, cref(vec1), &res1};`。
  - function objectの場合は、`thread t2 {F{vec2, &res2}};`。

## threadはdataをshareできる。processは普段shareしない。

- 理由：**Threads of a program share a single address space**.
  - Since threads share an address space, they can **communicate through shared objects**.
  - Such communication is typically **controlled by locks** or other mechanisms to prevent **data races (uncontrolled concurrent access to a variable)**.
  - １つdata raceの例：２つスレッドが`cout`（`ostream`）を使う場合、アウトプットは乱れるかも。
- Processes **generally** do not **directly** share data.

## concurrent programのtaskを定義する時の目標

- taskの意味：a computation that can **potentially be executed concurrently** with other computations.
- 目標：keep tasks **completely separate** except where they communicate in **simple and obvious** ways.
- いいtaskの例（no sharing dataの例）：a function that happens to run concurrently with its caller.
  - どうやって：**pass arguments, get a result back** and make sure that there is **no use of shared data in between** (no data races).

## Returning Resultsの方法１: by reference. not elegant.

- pass the input data by `const` reference and to **pass the location of a place to deposit the result** as a separate argument.
- しかし、Bjarne Stroustrupは、doesn't consider returning results through references particularly elegant. 後ほどもっといい方法があるはず。

## Sharing Data

- `mutex`の意味：**mutual exclusion object**.

- Use of resource handles, such as `scoped_lock` and `unique_lock`, is **simpler and far safer than explicitly locking and unlocking `mutex`es**.

  ```c++
  mutex m;	// controlling mutex
  int sh;	//shared data
  
  void f(){
      scoped_lock lck {m};	// acquire mutex, construct through m.lock()
      sh += 7;	// manipulate shared data
  }	// release mutex implicitly, destruct through m.unlock()
  ```

- error-proneの1ヶ所：one has to know which `mutex` is supposed to correspond to which data.
  - 回避方法：`mutex`やdataを1つクラスにする。

## `scoped_lock`のdead lockを防ぐ方法

- dead lockの発生可能場合：need to simultaneously access several resources to perform some action.

- `scoped_lock`の対応法：**acquire several locks simultaneously**.

  ```c++
  void f(){
      scoped_lock lck {mutex1, mutex2, mutex3};	// acquire all three locks
      // ... manipulate shared data ...
  } // implicitly release all mutexes
  ```

- 原理：This `scoped_lock` will proceed only after acquiring all its `mutex`es arguments and **will never block ("go to sleep") while holding a `mutex`**. そうすると、全部の`mutex`esを取れる前、どの`mutex`も独占しない。

## copying arguments and returnsかsharing dataか？

- copying arguments and returnsの方が簡単。
- Modern machines are very good at copying data, especially compact data, such as `vector` elements.
- So don't  choose shared data for communication because of "efficiency" without thought and preferably not without measurement.

## reader-writer lockの実現：`shared_mutex`, `shared_lock`, `unique_lock`

- 状況：sharing data among many readers and a single writer.

- A reader will acquire the mutex "shared" so that other readers can still gain access, whereas a writer will demand exclusive access. つまり、readerとreaderは共有関係、readerとwriterは競争関係。

  ```c++
  shared_mutex mx;	// a mutex that can be shared
  
  void reader(){
      shared_lock lck {mx};	// willing to share access with other readers
      // ... read ...
  }
  
  void writer(){
      unique_lock lck {mx};	// needs exclusive (unique) access
      // ... write ...
  }
  ```

## Waiting for Events, consumer-producerの実現：`condition_variable`

- A `condition_variable` allows a `thread` to wait for some *condition* (often called an *event*) to occur as the result of work done by other `thread`s.

```c++
queue<Message> mqueue;		// the queue of messages
condition_variable mcond;	// the variable communicating events
mutex mmutex;				// for synchronizing access to mcond

void consumer(){
    while(true){
        unique_lock lck {mmutex};	// acquire mmutex
        mcond.wait(lck,[]{return !mqueue.empty();}); // release lck and wait.
        											// re-acquire lck upon wakeup.
        									// don't wake up unless mqueue is non-empty
        auto m = mqueue.front();
        mqueue.pop();
        lck.unlock();	// release lck
        // ... process m ...
    }
}

void producer(){
    while(true){
        Message m;
        // ... fill the message ...
        scoped_lock lck {mmutex};
        mqueue.push(m);
        mcond.notify_one(); // notify
    }	// release lock (at end of scope)
}
```

- mmutexは`queue`や`condition_variable`両方保護している。以下は自分の考え：
  - なぜ`condition_variable`を保護する必要がある？例えば複数スレッドが待っていれば（`mcond.wait()`で）、１つのみlockを取れて、`[]{return !mqueue.empty();}`を実行できる。
    - ***lock* can be used to guard access to `pred()`（上記のlambda）**. https://en.cppreference.com/w/cpp/thread/condition_variable/wait
  - また`[]{return !mqueue.empty();}`に`queue`もアクセスしているので、`queue`と同じ`mutex`を使っている！
  - `mcond.notify_one()`は全然lockと関係ないでしょう。いつでも`notify_one()`できるでしょう。

- `consumer()`に`scoped_lock`じゃなく、`unique_lock`を使っている。
  - 理由１は**std::condition_variable works only with `std::unique_block<std::mutex>`**. https://en.cppreference.com/w/cpp/thread/condition_variable
  - 理由２、`scoped_lock`はscopeを出る時lockを解放する。しかし`consumer()`にはscopeを出る前に既にlockを解放したい（process mの前に解放したい）。だから`unique_lock`を使うしかない。
    - でも`unique_lock`は１つ`mutex`しか持てない。`scoped_lock`は複数`mutex`es同時に持てる。

## `future`や`promise`の繋がり

- `std::future<R> = std::promise<R>::get_future()`. https://en.cppreference.com/w/cpp/thread/promise
- The important point about `future` and `promise` is that they enable a transfer of a value between two tasks without explicit use of a lock; "the system" implements the transfer efficiently.

## `packaged_task`：taskと`promise`を合併する

- 自動的にtaskのreturn値を`promise`に保存する。
- `future`や`packaged_task`の繋がり：`std::future<R> = std::packaged_task<R(Args...)>::get_future()`. https://en.cppreference.com/w/cpp/thread/packaged_task

## `async()`

- Bjarne Stroustrupの拘り：（上にも既に書いていた）The one he believes to be the simplest yet still among the most powerful: **treat a task as a function that may happen to run concurrently with other tasks**. そう言えば、多分`async()`も師匠が一番勧めるでしょう。

  - Adviceから見ると、師匠は`packaged_task`のことが一番すきでしょう。

- **To launch tasks to potentially run asynchronously**, we can use `async()`.

  ```c++
  double comp4(vector<double>& v){ // spawn many tasks if v is large enough
      if(v.size()<10000)	// is it worth using concurrency?
          				// 10000 is only a simple and probably poor guess.
          return accum(v.begin(),v.end(),0.0);
      
      auto v0 = &v[0];
      auto sz = v.size();
      
      auto f0 = async(accum,v0,v0+sz/4,0.0);	// first quarter
      auto f1 = async(accum,v0+sz/4,v0+sz/2,0.0);	// second quarter
      auto f2 = async(accum,v0+sz/2,v0+sz*3/4,0.0);	// third quarter
      auto f3 = async(accum,v0+sz*3/4,v0+sz,0.0);	// fourth quarter
      
      return f0.get()+f1.get()+f2.get()+f3.get();	// collect and combine the results
  }
  ```

- Don't even think of using `async()` for tasks that share resources needing locking. つまり`async()`は全然Sharing data用ではないね！
- `async()`のperformance以外の用途の例：be used to spawn a task for getting information from a user, leaving the "main program" active with something else.

## Advice

- Use concurrency to improve **responsiveness** or to improve throughput.
- Work at the highest level of abstraction that you can afford. 例えば、上記の紹介の中に、`future, promise, packaged_task, async()`がconceptual levelです。だから優先でしょう！
- Consider processes as an alternative to threads. これはもっと説明してくれないと良く分からない。
- Prefer parallel algorithms to direct use of concurrency.
- Use `join()` to wait for a `thread` to complete.
- Avoid explicitly shared data whenever you can.
- Prefer RAII to explicit lock/unlock.
- Use `condition_variable`s to manage communication among `thread`s.
- **Don't wait without a condition**.
- Minimize time spent in a critical section. つまりlockを持つ時間を短縮することでしょう。
- Think in terms of tasks that can be executed concurrently, rather than directly in terms of `thread`s.
- **Prefer `packaged_task` and `future`s over direct use of `thread`s and `mutex`es**.
- Return a result using a `promise` and get a result from a `future`.
- **Use `packaged_task`s to handle exceptions thrown by tasks and to arrange for value return**.
- **Use a `packaged_task` and a `future` to express a request to an external service and wait for its response**.
- Use `async()` to launch **simple tasks**.