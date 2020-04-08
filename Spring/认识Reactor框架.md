# 认识Spring - Reactor框架

我们都知道 reactor模式的优缺点， 也就是基于异步实现的， 但是这只是模式， 那么框架如何运作， 如何优美，则是另一回事。 

最近在看 spring-cloud-gateway ， 我被老外写的代码吸引了， 一路到底就是 reactor框架整合netty的那段代码。 确实很优美。  主要是 reactor-netty 那个包。 确实写的不赖， 写了tcp ，udp 客户端和服务器端， 用户可以基于这个实现很多基于这个传输层的框架实现。



废话不多说， 先入门个 reactor吧，它是spring开发的一个子项目，spring的代码水平相当之高，尤其是封装和架构设计这块， github地址是 [https://github.com/reactor/reactor-core](https://github.com/reactor/reactor-core)   需要掌握它的写法和思想。



其中例子的地址是 [https://www.infoq.com/articles/reactor-by-example/](https://www.infoq.com/articles/reactor-by-example/)  ，我基本就是看的这个， 对于我们开发者而言， 其实不需要关注订阅， 对于reactor模式， 他是基于observer模式， 一个观察者模式， 发布订阅模式吧 ， 其中让人大家不理解的是 ，Flux 和 Mono ， 这俩概念， 其实他俩都是发布者， 而订阅者我们接触不到，是因为spring框架帮我们订阅了。 也就是这个思想。  他的实现是基于 RxJava 2思想的。

我们开始吧。 

>Reactor's two main types are the `Flux<T>` and `Mono<T>`. A Flux is the equivalent of an RxJava `Observable`, capable of emitting 0 or more items, and then optionally either completing or erroring.
>
>A Mono on the other hand can emit **at most** once. It corresponds to both `Single` and `Maybe` types on the RxJava side. Thus an asynchronous task that just wants to signal completion can use a `Mono<Void>`.
>
>This simple distinction between two types makes things easy to grasp while providing meaningful semantics in a reactive API: by just looking at the returned reactive type, one can know if a method is more of a "fire-and-forget" or "request-response" (`Mono`) kind of thing or is really dealing with multiple data items as a stream (`Flux`).
>
>Both Flux and Mono make use of this semantic by coercing to the relevant type when using some operators. For instance, calling `single()` on a `Flux<T>` will return a `Mono<T>`, whereas concatenating two monos together using `concatWith` will produce a `Flux`. Similarly, some operators will make no sense on a `Mono` (for example `take(n)`, which produces n > 1 results), whereas other operators will *only* make sense on a `Mono` (e.g. `or(otherMono)`).

上面这个就讲述了 Moon 和 Flux的区别, 其实就是 one / more 的区别. many one = more , 同时也可以分解. 



快速开始前, 我们只需要加入 maven依赖

```java
<dependency>
    <groupId>io.projectreactor</groupId>
    <artifactId>reactor-core</artifactId>
    <version>3.1.8.RELEASE</version>
</dependency>
```



简单例子

```java
public class ReactorSnippets {

    private static List<String> words = Arrays.asList(
            "the",
            "quick",
            "brown",
            "fox",
            "jumped",
            "over",
            "the",
            "lazy",
            "dog"
    );

    @Test
    public void simpleCreation() {
        Flux<String> fewWords = Flux.just("Hello", "World");
        Flux<String> manyWords = Flux.fromIterable(words);
	
        // 第一个订阅者
        fewWords.subscribe(System.out::println);
        System.out.println("==============);
        // 第二个                   
        manyWords.subscribe(System.out::println);
    }
}
```

上面输出 : 

```java
Hello
World

the
quick
brown
fox
jumped
over
the
lazy
dog
```





## reactor线程模型 / 观察者设计模式

reactor线程模型是, 指的是主线程发现的监听到的事件 , 执行事件(可能是worker线程执行). 



观察者模式. 其思想就是发布和订阅. 