## 6.1 What does it mean to design for concurrency

- ２つ面の考えが必要：安全性、並行性。

- 安全性があるが、並行性はない：`mutex`。
  - 理由：only one thread can acquire a lock on the mutex at a time.
- 並行性の原則：**the smaller the protected region**, the fewer operations are serialized, and the greater the potential for concurrency.
- data structureのこの機能がよく必要である：support **concurrent access** from threads performing **different operations** while **serializing** threads that try to perform the **same operations**.

## 6.2 Lock-based concurrent data structures

- 設計目標：

  - **the right mutex** is locked when accessing the data.
  - the lock is held for the **minimum** amount of time.

- lock-based concurrent queueの設計：page 179 - 194。

  - a thread-safe queue using locks and condition variables.

  - a thread-safe queue using fine-grained locks and condition variables: std::queueベースでやると、queue全体をロックする対象になってしまうので、single-linked listまで改めてqueueを実現する。(in order to permit fine-grained locking you need to look carefully at the details of the data structure rather than wrapping a pre-existing container. p196)

    - Queueの操作は基本Head, Tailノードに対してやるので、この２ノードにそれぞれmutexを用意。

    - Push, Popが同時にHead, Tailを維持しないように、Dummy nodeを導入する。TailはいつもDummy nodeに指す。そうすると、PushはTailのみを維持する、PopはHeadのみを維持する。

      ```c++
      queue():head(new node),tail(head.get()){}
      ```

    - condition variableを使って、queueのempty状態中待つwait_and_pop()関数を実現。bounded queueの場合だったら、queueのsizeが最大状態中に待つ関数にも使う。

- 安全性の分析は難しい、因みに安全性の分析項目は以下の４つ：

  - Ensure that no thread can see a state where the **invariants** of the data structure have been **broken** by the actions of another thread.
  - Take care to avoid race conditions inherent in the interface to the data structure by providing functions for **complete operations** rather than for operation steps.
  - Pay attention to how the data structure behaves in the presence of **exceptions** to ensure that the invariants are not broken.
  - Minimize the opportunities for deadlock when using the data structure by **restricting the scope of locks** and **avoiding nested locks** where possible.

## Designing a map data structure for fine-grained locking

- lookupというのは、例えば各種mapだ。

  - STLのmapは、結構concurrency的に難しいのは、iterator。

- concurrencyのために、一番よい構造体は**hash table**だ。（binary tree, such as a red-black treeやsorted arrayと比べて）

- hash tableは固定数のbucketsによって構成される。buckets間は完全平行できる。

  - 原因：which bucket a key belongs to is purely a property of the key and its hash function.

  - The downside is that you need a **good hash function** for the key.

  - 具体的な構成：

    ```c++
    // 1. bucket vector
    std::vector<std::unique_ptr<bucket_type>> buckets;
    
    // 2. bucket_typeはstructです。listを持っている。
    std::list<bucket_value> bucket_data;
    
    // 3. bucket_valueはkey, value pairです。
    std::pair<Key, Value> bucket_value;
    ```

    - bucket_typeに対して、`std::shared_mutex`を使う。そうすると、multiple reader threads（`shared_lock`） or a single writer thread（`unique_lock`）ができるので、さらにconcurrencyを向上。
    - bucketの数について、hash table work best with a prime number of buckets.
    - このhash tableの内容を`std::map`に入れる時、先に全bucketsのmutexを取らなきゃ。

## 復習項目

- what it means to design a data structure for concurrency.
- some guidelines.
- common data structureの設計（stack, queue, hash map, linked list）
  - how to apply those guidelines to implement them in a way designed for concurrent access.
  - use locks to protect the data and prevent data race.