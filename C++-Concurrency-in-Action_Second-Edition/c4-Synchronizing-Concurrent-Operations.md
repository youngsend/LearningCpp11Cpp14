# 4.1 Waiting for an event or other condition

# 4.1.1 Waiting for a condition with condition variables

## `std::condition_variable`の使用例及び流れ説明: waiting for data to process

```c++
std::mutex mut;
std::queue<data_chunk> data_queue;
std::condition_variable data_cond;

void data_preparation_thread(){
    while(more_data_to_prepare()){
        data_chunk const data = prepare_data();
        {
            std::lock_guard<std::mutex> lk(mut);
            data_queue.push(data);
        } // ここでunlock
        data_cond.notify_one(); // unlockした後、notify!
    }
}

void data_processing_thread(){
    while(true){
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, []{return !data_queue.empty();});
        data_chunk data = data_queue.front();
        data_queue.pop();
        lk.unlock();
        process(data);
        if(is_last_chunk(data))
            break;
    }
}
```

- unlock後notifyの理由：so that, if the waiting thread wakes immediately, it doesn't then have to block again, waiting for you to unlock the mutex. つまりスレッドAがlockのまま、notifyして、またunlockすると、待っていたスレッドBは、wakeしても、まだlockできなくて、また自分をblockしちゃうことが可能。
- `wait`の流れ：通知を受けて、lockして、lambda（**the condition being waited for**）をチェックする。
  - lambdaがtrueだったら、lockしたままwaitを出る。returnする。
  - lambdaがfalseだったら、unlockして、また自分をblockする。returnしない。

## *spurious（にせの） wake*の意味やアドバイス

- 普段は`std::condition_variable::wait()`が行ったスレッドは、ほかのスレッドのnotifyを受けて起きるが。*spurious wake*の場合は、notifyが来てないのに、自分が起きてしまうこと。。。

- 英語の定義：When the waiting thread reacquires the mutex and checks the condition, if it isn't in **direct response** to a notification from another thread, it's called a *spurious wake*.

- アドバイス：*spurious wake*が何回か起こるか分からないので、lambdaにはside effectがない（つまりlambda範囲外も使っているデータを変更しない）ほうがいいです。
  - **side effect (computer science)**の意味: https://en.wikipedia.org/wiki/Side_effect_(computer_science)
  - In computer science, an operation, function or expression is said to have a side effect if it modifies some state variable value(s) **outside its local environment**, that is to say has an **observable effect** besides returning a value (the main effect) to the invoker of the operation.
  - 例えば：modifying a non-local variable, modifying a static local variable, modifying a mutable argument passed by reference, performing I/O or calling other side-effect functions.

# 4.1.2 Building a thread-safe queue with condition variables

- `std::mutex`, `std::condition_variable`を`std::queue`とthread-safe queueクラスにする。
- 上記4.1.1の関数はそれぞれthread-safe queueの`push()`や`wait_and_pop()`にする。
- ソースコード：https://github.com/youngsend/ccia_code_samples/blob/master/listings/listing_4.5.cpp

## mutexに`mutable`キーワードの使用の理由

- thread-safe queueクラスのdata member mutexにmutableをつける必要がある。

```c++
private:
	mutable std::mutex mut;
	std::queue<T> data_queue;
	std::condition_variable data_cond;
public:
	// ...
	threadsafe_queue(threadsafe_queue const& other){ // copy constructor
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue = other.data_queue;
    }
	// ...
	bool empty() const { // const member function
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
```

- 主な理由は：**Locking a mutex is a mutating operation**.
  - copy constructor内：otherがconstなので、`mut`がmutableじゃないと、`other.mut`はlockできない。
  - `empty()`内：const member functionなので、基本的に関数内はdata memberを変更しちゃダメ。なので、data memberの`mut`をlockできるように（変更）、`mut`を`mutable`に指定しないといけない。

# 4.2 Waiting for one-off events with futures

- The **future objects** themselves don't provide synchronized accesses. If multiple threads need to access a single future object, they must protect access via a mutex or other synchronization mechanisms.

# 4.2.1 Returning values from background tasks: `std::async`

```c++
int find_the_answer();
void do_other_stuff();
int main(){
    std::future<int> the_answer = std::async(find_the_answer);
    do_other_stuff();
    std::cout << "The answer is " << the_answer.get();
}
```

## `std::async`を新規する時のargumentのルール

- 多分これはどこでも通用すると思う。例えばrosのsubscriberのargumentの中のcallback。
- 2種類の新規
  - If the first argument is **a pointer to a member function**, the second argument **provides the object** on which to apply the member function (either directly（コピー）, or via a pointer（例えば`this`）, or wrapped in `std::ref`). タスクはクラスのmember functionです。
    - And the remaining arguments are passed as arguments to the member function.
  - Otherwise, the second and subsequent arguments are passed as arguments to the **function or callable object** specified as the first argument. タスクは普通の関数またはcallable object（operator()を定義しているクラスまたは構造体）。

## `std::async`のタスクの実行タイミング

- `std::launch::async`: 新規後すぐ新しいスレッドで実行する。
- `std::launch::deferred`: futureの`get()`または`wait()`が呼び出された時からタスクを実行する。
  - `get()`は`wait()`を使っている。https://en.cppreference.com/w/cpp/thread/future/get
- default: `std::launch::deferred | std::launch::async`. 意味：Implementation chooses.

# 4.2.2 Associating a task with a future: `std::packaged_task<>`

僕の考え：（`std::async`と比べて）

- `std::async`は非常に関数っぽいです。なので、一番使いやすいでしょう。`std::async`を使うときは、僕は全然スレッドの管理など触ってない。
- `std::packaged_task`も使いやすいが、自分で明示的に`std::future`を取らないといけない。また、自分でスレッドの管理をしないといけないでしょう。どこで実行させるかを指定しないといけない。例えば、`std::packaged_task`を他のスレッドに渡して、実行させるとか。

## `std::packaged_task`オブジェクトはcallableオブジェクト

なので、

- It can be wrapped in a `std::function` object;
- **passed to a `std::thread` as the thread function**; もう１つ言い方：passing **tasks** between threads.
- passed to another function that requires a callable object;
- or even **invoked directly**. 

僕がよく見たのは第2、第4です。

## `std::packaged_task`の普段の使い方

- Wrap a task in a `std::packaged_task`.
- Retrieve the future.
- Pass the `std::packaged_task` object elsewhere to be invoked in due course. 上記の第2、第4。
- When you need the result, you can wait for the future to become ready.

cppreferenceの例を見れば、すぐ分かる。https://en.cppreference.com/w/cpp/thread/packaged_task

# 4.2.3 Making (std::)promises: スレッド間のデータ通信

- 送信側は`std::promise`の`set_value()`でデータを入れる。
- 受信側は`std::future`でデータを取る。まだなければ、自分をblockして待つ。

# 4.2.4 Saving an exception for the future

- `std::async`や`std::packaged_task`は自動的にfunction callのexceptionを`future`に渡す。なので、普通の関数と全然一緒。
  - that exception is stored in the future in place of a stored value, the future becomes *ready*, and a call to `get()` rethrows that stored exception.
- `std::promise`の場合は、自分でexceptionを入れる。`set_exception()`で。
- もう1つ方法：使う前にオブジェクトを壊す。
  - destroy the `std::promise` or `std::packaged_task` associated with the future without calling either of the set function on the promise or invoking the packaged task.

# 4.2.5 Waiting from multiple threads: `std::shared_future`

- `std::shared_future`の初期化：`std::future`をmove in。
  - Since `std::future` objects don't share ownership of the asynchronous state with any other object, **the ownership must be transferred into the `std::shared_future` using `std::move`**, leaving `std::future` in an empty state, as if it were a default constructor.

## multiple threadsから１つ`std::future`をアクセスすることはやめて、１つ`std::shared_future`も同様

- ちゃんとlockしてもこのやり方はやめてください。`std::future`の`get()`は一回しか呼び出せないので、2回め以降のcallはundefined behavior。
  - `std::future`の設計：`std::future` models unique ownership of the asynchronous result, and the **one-shot nature** of `get()` makes such concurrent access pointless anyway - only one thread can retrieve the value, because after the first call to `get()` there's no value left to retrieve.

- 僕の考え：move-onlyタイプの考え方は大体同じでしょう。
  - `std::thread`の`join()`もしくは`detach()`が一回しか呼び出せないことと同様です。`std::thread`のresourceはthreadなので、当然一回しか実行させない。同じく、`std::future`のresourceはスレッド間通信したデータです（もっといい言い方は、その一発のスレッド間のデータ通信channel？）。`get()`が一回発生したら、一発のデータ通信は必ず終わるので、この`std::future`の存在理由は既になくなった（threadが終わったあとの`std::thread`）。

## 複数`std::shared_future`のコピーがそれぞれ同じデータをgetできる

- `std::shared_future`は１つしかなければあんまり`std::future`と変わりがない。
- 正しい使い方：pass a **copy** of the `shared_future` object to each thread, so each thread can access its **own** local `shared_future` object safely, as the internals are now correctly synchronized by the library. 
- 正しい使い方（下の方）を説明する図：![](/home/sen/senGit/LearningCpp11Cpp14/C++-Concurrency-in-Action_Second-Edition/img/shared-future-2020-04-23 19-52-35.png)

# 4.3 Waiting with a time limit

- 2種類timeout: duration-based timeout, absolute timeout.
- 合わせて2種類の**waiting function**:
  - duration-based timeout waiting: `_for` suffix.
  - absolute timeout waiting: `_until` suffix.

## Clocks, Durations, Time points

- https://en.cppreference.com/w/cpp/chrono
- Clock: a clock consists of a starting point (or *epoch*) and a **tick rate**. 時間の計測担当。
  - clockは`now()`を提供する。`now()`のタイプは`time_point`。
  - Typically, `std::chrono::system_clock` will not be steady, because the clock can be adjusted, even if such adjustment is done automatically to take account of local clock drift.
  - **Steady clock**s are important for **timeout calculation**s, so C++が`std::chrono::steady_clock`を提供している。
- Duration: A duration consists of a span of time, defined as **some number of ticks of some time unit**.
  - floating-point literalsの場合、例えば`2.5min`、durationのタイプは`std::chrono::duration<some-floating-point-type,std::ratio<60,1>>`になる。`some-floating-point-type`は実現によって変わるかも。
  - Conversion between durations is implicit where it does not require **truncation** of the value (so converting hours to seconds is OK, but converting seconds to hours is not（この場合Explicitキャストが必要）).
  - The time for a duration-based wait is measured using a steady clock internal to the library.
- Time point: A time point is a duration of time that has passed **since the epoch of a specific clock**. 時刻担当。

## `future`の`wait_for`の3種類return値：

```c++
std::future<int> f=std::async(some_task);
if(f.wait_for(std::chrono::milliseconds(35))==std::future_status::ready)
    do_something_with(f.get());
```

- `std::future_status::timeout` if the wait times out.
- `std::future_status::ready` if the future is ready.
- `std::future_status::deferred` if the future's task is deferred. つまり、タスクは`get()`がcallされなければ実行しないので、`wait_for()`はどう待っても意味ないよ。

## timeoutができるSTL関数

- `std::this_thread`の`sleep_for`, `sleep_until`。
- `std::condition_variable`, `std::future`の`wait_for`, `wait_until`。
- `std::timed_mutex`, `std::shared_timed_mutex`, `std::unique_lock`, `std::shared_lock`の`try_lock_for`, `try_lock_until`。

# 4.4 Using synchronization of operations to simplify code

# 4.4.1 Functional programming with futures

## Functional programmingの考え方

- functional programmingはprogrammingスタイルです。
- *functional programming* (FP) refers to a style of programming where **the result of a function call depends solely on the parameters to that function and doesn't depend on any external state**.
  - 関数は全く外部状態に依存しなくて、渡されたparameterのみに依存する。なので、渡されたparameterは全部copyです。
  - また、A *pure* function doesn't modify any external state either; the effects of the function are entirely limited to the return value.
- メリット：shared dataは使わないので、mutexなどいらない。また、天然concurrencyがしやすい。
- functional programmingがdefaultのHaskell。**all functions are pure by default**, increasingly popular for programming concurrent systems.

## Function programmingスタイルのQuick Sort実現

![](/home/sen/senGit/LearningCpp11Cpp14/C++-Concurrency-in-Action_Second-Edition/img/quicksort-2020-04-25 18-08-02.png)

```c++
template<typename T>
std::list<T> sequential_quick_sort(std::list<T> input){ // parameterはcopy
    if(input.empty())
        return input;
    std::list<T> result;
    result.splice(result.begin(), input, input.begin()); // inputの先頭要素をresultにmove
    T const& pivot = *result.begin();
    
    auto divide_point = std::partition(input.begin(), input.end(), [&](T const& t){return t<pivot;}); // divide_pointはiterator,partition後の>=pivotの初めての要素に指す
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point); // [first,last)
    auto new_lower(sequential_quick_sort(std::move(lower_part))); // 前半再帰
    auto new_higher(sequential_quick_sort(std::move(input))); // 後半再帰
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower);
    return result;
}
```

## `future`や`async`でFP quick sortをconcurrency化する

```c++
// 2行だけ変更
std::future<std::list<T>> new_lower(std::async(&parallel_quick_sort<T>, std::move(lower_part)));

result.splice(result.begin(), new_lower.get());
```

- FP-styleなので、超`future,async`に合う。
- If `std::async()` starts a new thread every time, then if you recurse down 3 times, you'll have 8 threads running; if you recurse down 10 times (for ~1000 elements), you'll have 1024 threads running if the hardware can handle it.
- もしthreadが足りなかったら、新しいスレッドではなく、`get()`をcallするスレッドでタスクが実行される。
- まだ改善できるところ：`std::partition` does a lot of work, and that's still a sequential call.

# 4.4.4 Chaining continuations

- continuationはConcurrency TSの増強です。
- continuationは`future`がreadyになった時のcallbackみたい。
- continuationは`future.then()`に渡す。continuationの引数は`future`のみ。`.then(continuation)`は`future`をreturnする。
- continuationがそもそも`future`をreturnする場合も大丈夫、`.then()`はそもままの`future`をreturnする！
- むしろcontinuationが`future`をreturnしてほしい！そうすると、continuationはcallをblockしない。
- 僕の疑問：chaining continuationでもsequential処理になりそうじゃない？
- continuation chainで３つタスクをasynchronously実行: A function to process user login with fully asynchronous operations.

  ```c++
  std::experimental::future<void> process_login(std::string const& username, std::string const& password){
    return backend.async_authenticate_user(username, password).then(
    	[](auto id){
            return backend.async_request_current_info(id.get());
        }).then([](auto info_to_display){
        	try{
                update_display(info_to_display.get());
            } catch(auto& e){
                display_error(e);
            }
    	});
  } 
  ```

  - 最後は`future<void>`をreturnするので、main threadは別に`get()`する必要もないので、全然blockしない。（でもこれはchaining continuationなくても、ただ１つ`async()`でできる）
    - `get()/wait()`しないと、この関数本当に実行されるの？
  - `async_authenticate_user()`, `async_request_current_info()`はそれぞれ`future`をreturnしている！

## 4.4.5 Waiting for more than one future: `when_all`

- `when_all`がないと、すべての`future`に対して、１つずつ`get()`をcall。まだunavailableなら、current threadをblockして、待つしかない。

- `when_all`があれば、全部の`future`がreadyなので、`get()`をcallするときは必ず待たない。

- `when_all`の使用例：

  ```c++
  std::vector<std::experimental::future<ChunkResult>> results;
  // ... spawn async
  std::experimental::when_all(results.begin(), results.end()).then(
  	[](std::future<std::vector<
         std::experimental::future<ChunkResult>>> ready_results){ // when_allがreturnするfutureのタイプ
          auto all_results = ready_results.get();
          ...
          for (auto& f : all_results)
              v.push_back(f.get()); // ここで絶対待たない。
      });
  ```

- **Waiting for the first future in a set with `when_any`**.

## `std::experimental::latch`: スレッド達共有するcounter

- スレッド達が`count_down()`でlatch counterを減らす。
- latchが0になるまでは、`wait()`はずっとblock。
- `latch` is a synchronization object, so changes visible to a thread that call `count_down` are guaranteed to be visible to a thread that returns from a call to `wait` on the same latch object.
  - つまりthe call to `count_down` synchronizes with the call to `wait`.

## `std::experimental::barrier`: スレッド達が全部`barrier`に到達するまで、待つ！

- スレッド数で`barrier`をconstructする。
- スレッドがbarrierで`arrive_and_wait()`で、全員揃うまで待つ。全員揃ったら、全員が解放される。
- その待つgroupを退会：`arrive_and_drop()`。
- 使用例：Thread 0だけデータを取って、N分に分けて（`barrier`!）、N個スレッドがそれぞれ自分のデータを使ってタスクを実施し、結果をresultに詰める。全員がタスク終わったら（`barrier`!）、Thread 0がresultを外に書き込む。
- シナリオ：Each thread can do its processing on the data independently of the others, so no synchronization is needed during the processing, but all the threads must have completed their processing before the next data item can be processed, or before the subsequent processing can be done.

## `std::experimental::flex_barrier`: 全員揃った後のcallbackができる、次に揃うスレッド数も自由に変える（callbackのreturn値）

- 上記`barrier`の使用例の中のThread 0がやったことは全部このcallbackに入れた方がいい。callbackはスレッド達の中の１つで実行される。

# 復習項目

- various ways of synchronizing operations from the basic condition variables, through futures, promises, packaged tasks, latches, and barriers.
- synchronization問題を解決する3種類の設計パターン
  - functional-style programming.
    - Each task produces a result entirely dependent on its input rather than on the external environment.
  - message passing.
    - Communication between threads is via asynchronous messages sent through a messaging subsystem that acts as an intermediary.
  - continuation style.
    - **Follow-on tasks** for each operation are specified, and the system takes care of the scheduling.