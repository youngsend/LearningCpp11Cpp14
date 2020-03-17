# Chapter 7: Concepts and Generic Programming
## ConceptsありやConceptsなしのtemplateはどう違う？
- Conceptsのチェックタイミングや従来のTemplateのチェックは全部compile時でやる。
    - もっと厳密に言うと、pre-conceptのコードはtemplate instantiation timeでチェックが行う。
        - template instantiation time: when code is generated for the template and a set of template arguments.
    - conceptsはinstantiationの前は既にチェックできる。
- では、なぜConceptsが必要？
    - Conceptsがないと、templateをチェックするために、implementationを見ないと行けない。
        - このチェック方法は**duck typing**と言う。If it walks like a duck and it quacks like a duck, it's a duck.^^
	- Conceptsがあったら、interfaceだけ見れば、チェックできる。
- 例えば、Conceptsなしのtemplate関数：
	```c++
	template<typename Seq, typename Num>
	Num sum(Seq s, Num v){
        for (const auto& x : s)
            v+=x;
        return v;
    }
	```
    - SeqやNum template argumentsの正しさをチェックするために、実現コードを見ないと行けない。
    - 実現コードを見て、Seqは必ずsome kind of sequence,Numは必ずsome kind of numberが分かる。
- またConceptsありのtemplate関数：
    ```c++
	template<Sequence Seq, Number Num>
	Num sum(Seq s, Num v){
        for (const auto& x : s)
            v+=x;
        return v;
    }
	```
    - SequenceやNumberのConceptsがちゃんと定義されたら、この関数のinterfaceを見るだけで既にargumentsをチェックできる。

## Advice
- When designing a template, use a concrete version for initial implementation, debugging, and measurement.
 - つまり本当に必要な場合のみtemplate化する。そのそのある関数は１つtypeのargumentsを対応すればいい場合に、templateにするな。
- Use a lambda if you need a simple function object in one place only.
 - これはまだできていないところ。
- Use templates to express containers and ranges.
- Use variadic templates when you need a function that takes a variable number of **arguments of a variety of types**.
 - variadic templatesはまだ使ったことがない。
 - **Don't use variadic templates** for **homogeneous argument lists** (prefer initializer lists for that).
     - もしargumentsが同タイプだったら、initializer listsを使おう。https://en.cppreference.com/w/cpp/utility/initializer_list
     - `std::initializer_list<T>`
- Templates offer compile-time "duck typing".