# JUC - CompletableFuture 异步编程和流式编程

> ​	`CompletableFuture`类实现了`Future`接口和`CompletionStage` 接口, `Future`接口我们就不多介绍了, 他就是一个异步的回调,并且调用`get`方法是阻塞的 , 这个并不是这个类的特点 , 他的关键是 `CompletionStage`这个接口, 他更像是一个一段程序执行的各个阶段, 每个阶段执行结束都会返回一个阶段, 所以阶段可能相互依赖, 也可能不依赖 , 也有各自情况 ,他更符合流式编程的特点 ,特别像`JavaScript`的`Promise`对象 , 一种then , 就是这玩意 , 估计你听懂了我们就开始吧
>
> ​	还有这玩意是流式编程 ,代表根本没有尾, 只要你不想结束 ....



## 1. CompletionStage

> ​	A stage of a possibly asynchronous computation, that performs an action or computes a value when another CompletionStage completes.
>
> ​	就是代表一个阶段, 一种流式编程

> ​	The computation performed by a stage may be expressed as `a Function, Consumer, or Runnable` (using methods with names including apply, accept, or run, respectively) depending on whether it requires arguments and/or produces results. For example,` stage.thenApply(x -> square(x)).thenAccept(x -> System.out.print(x)).thenRun(() -> System.out.println())` .An additional form (compose) applies functions of stages themselves, rather than their results.

我感觉这一段就是他表述的精髓  : 根据是够需要参数和返回结果 , 一个阶段的计算可以表示为一个方法, 一个消费 , 一个 runnable , 

其中 `CompletableFuture`也是基于这个接口去实现的 

## 2. CompletableFuture

> ​	When two or more threads attempt to complete, completeExceptionally, or cancel a CompletableFuture, only one of them succeeds.  他保证了多线程的安全性,其实就是这段话表述的意思 , 



> ​	感觉如果你使用这玩意就不要使用 `Future`带的方法,比如`get`

### 1. 简单开始

比如举个例子 , 我们需要去查询一下数据库当前一共有多少数据 , 此时我们查询线程需要很耗时 ,我们需要做其他事情,然后查询完我们还要对他进行数据加工

```java
@Test
public void test3() throws IOException {
    CompletableFuture<Integer> future = new CompletableFuture<>();
    // 线程一
    future.thenApply((r) -> r * 10).whenComplete((r, e) -> System.out.println("拿到最终数据 : "+r));

    // 线程二
    new Thread(() -> {
        // 模拟耗时
        ThreadUtil.sleep(100);
        System.out.println("查询到数据 1000 条");
        future.complete(1000);
    }).start();

    // 阻塞你懂得
    System.in.read();
}

// 输出 : 
// 查询到数据 1000 条
// 拿到最终数据 : 10000
```

这就是异步 , 我们输入和输出是两个线程执行 ,

还有一种就是我们可能查询失败了,没有结果返回怎么办呢 ? 

```java
@Test
public void test3() throws IOException {
    CompletableFuture<Integer> future = new CompletableFuture<>();
    // 线程一
    future.thenApply((r) -> r * 10).whenComplete((r, e) -> System.out.println(e.getMessage()));


    // 线程二
    new Thread(() -> {
        // 模拟耗时
        ThreadUtil.sleep(100);
        System.out.println("查询失败 : "+Thread.currentThread().getName());
        future.completeExceptionally(new RuntimeException("查询失败"));
    }).start();

    // 阻塞你懂得
    System.in.read();
}
// 输出
// 查询失败 : Thread-0
// java.lang.RuntimeException: 查询失败
```

### 2. 静态方法-实例化

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-06/661a03c4-5ef4-4213-bfa1-de2d22a7aaa5.jpg?x-oss-process=style/template01)

我们大致看看这几个方法 . `supply` 是提供一个返回值 , `run`是提供一个无返回值的run方法, `allOf` 是将几个`CompletableFuture` 合并 , `completedFuture` 是将结果立马返回一个带有结果的对象 , 



### 3.立即完成  complete 和 completeExceptionally 方法

> ​	`complete ` : If not already completed, sets the value returned by get() and related methods to the given value. 

​	`complete `方法就是如果此阶段(`此阶段很重要,要懂得阶段指的是啥,因为每一个方法的调用都需要一个CompletionStage对象,这个就是指的此阶段`)没有完成 , 立马设置值完成 , 

> ​	`completeExceptionally ` : If not already completed, causes invocations of get() and related methods to throw the given exception. 
>
> ​		true if this invocation caused this CompletableFuture to transition to a completed state, else false

​	`completeExceptionally ` 方法就是没有完成, 立马抛出异常, 如果返回true , 表示成功将其此阶段完成, false表示失败 

```java
public static void main(String[] args) throws IOException {
    final long start = System.currentTimeMillis();

    final CompletableFuture<String> compute = CompletableFuture.supplyAsync(() -> {
        ThreadUtils.sleep(1000);
        System.out.println("我就是要拖拉 , 耗时 : " + (System.currentTimeMillis() - start));
        return "我就是要拖拉";
    });


    new Thread(() -> {
        CompletableFuture<Void> future = compute.thenAccept(v -> {
            System.out.println("接收到结果 : " + v + " , 耗时 : " + (System.currentTimeMillis() - start));
        });
    }).start();

    new Thread(() -> {
        boolean b = compute.complete("那个烂货,太磨叽了,我先将他任务感觉结算了");
        System.out.println("快速完成是否成功 : " + b);
    }).start();

	// 阻塞
    System.in.read();
}
```

如果你看懂我上面写的是什么 , 你就理解了这个 `CompletionStage` 究竟只做了些什么 , 试着猜一下输出结果  , 

输出 : 

```java
接收到结果 : 那个烂货,太磨叽了,我先将他任务感觉结算了 , 耗时 : 64
快速完成是否成功 : true
我就是要拖拉 , 耗时 : 1072
```

那个 `completeExceptionally ` 他是提供一个异常 , 可能我上面的`thenAccept`方法接收不了, 你可以换其他方法去处理,

### 4.  抓取异常 - exceptionally 方法

```java
@Test
public void testException() throws ExecutionException, InterruptedException {

    CompletableFuture<String> future = CompletableFuture.supplyAsync(() -> {
        error();
        return getName();
    }).exceptionally(new Function<Throwable, String>() {
        @Override
        public String apply(Throwable throwable) {
            return "获取失败";
        }
    }).whenComplete(new BiConsumer<String, Throwable>() {
        @Override
        public void accept(String s, Throwable throwable) {
            System.out.println(s);
        }
    });
}

// 输出 :  获取失败
```

### 5 加工 - supplyAsync & thenApply & thenAccept 方法

对于`CompletableFuture` 我感觉你理解成流式编程比较好 , 和`Stream`很相似, 这种编程

```java
@Test
public void testOperation() throws IOException {

    CompletableFuture.supplyAsync(new Supplier<String>() {
        @Override
        public String get() {
            // 获取名字
            return getName();
        }
    }).thenApply(new Function<String, Person>() {
        @Override
        public Person apply(String s) {
            // 封装对象
            return new Person(s, System.currentTimeMillis());
        }
    }).thenAccept(new Consumer<Person>() {
        @Override
        public void accept(Person person) {
            // 消费对象
            System.out.println(person);
        }
    });

    System.in.read();
}
// 输出 : Person{name='tom', timestamp=1575645761277}
```

### 6. 加工-  whenComplete 方法

`public CompletionStage<T> whenComplete    (BiConsumer<? super T, ? super Throwable> action);`

> ​	Returns a new CompletionStage with the same result or exception as this stage, that executes the given action when this stage completes.
>
> ​	返回上一阶段的结果或者异常信息, 

```java
@Test
public void testComplete() {

    CompletableFuture.supplyAsync(() -> "Hello CompletableFuture !")
            .whenComplete(new BiConsumer<String, Throwable>() {
                @Override
                public void accept(String s, Throwable throwable) {
                    System.out.println(s);
                }
            });
}
输出 : Hello CompletableFuture !
```

**我们发现个问题 , `accept`意思是接收(被动的)方法的意思,太自私了自己消费不管返回 ,  `apply` 意思是申请(主动的)方法比较好,我用了我还返回一个值 , 哎 ,知道这个就好吧了**

### 7. 加工 - handle 方法

`public <U> CompletionStage<U> handle    (BiFunction<? super T, Throwable, ? extends U> fn);`

> ​	Returns a new CompletionStage that, when this stage completes either normally or exceptionally, is executed with this stage's result and exception as arguments to the supplied function.
>
> ​	返回一个新的阶段,当此阶段完成可能是正常也可能是异常 , 将使用此阶段的结果和异常作为提供的函数的参数执行。同时可以有返回值

所以怎么说呢  , 我觉得他是 `whenComplete`的改进版本 , 他有返回值, 他绝对是一个 `apply`方法

```java
@Test
public void testHandle() throws ExecutionException, InterruptedException {

    String msg = CompletableFuture.supplyAsync(() -> "hello CompletableFuture")
            .handle(new BiFunction<String, Throwable, String>() {
                @Override
                public String apply(String s, Throwable throwable) {
                    return "我收到了 : " + s;
                }
            }).get();

   System.out.println(msg);
}

// 输出 :  我收到了 : hello CompletableFuture
```

### 8. 加工 - thenCompose

`public <U> CompletionStage<U> thenCompose (Function<? super T, ? extends CompletionStage<U>> fn)`

> ​	Returns a new CompletionStage that, when this stage completes normally, is executed with this stage as the argument to the supplied function. 

就是上一阶段正常执行完毕会执行这个方法 返回一个新的阶段 , 不过这个阶段提供了一个 function 函数 , 可以对上一阶段的返回进行处理  , 并且function函数返回一个新的 `CompletionStage`对象得

```java
@Test
public void testCompose() throws IOException {

    CompletableFuture.supplyAsync(() -> "tony")
            .thenCompose(s -> {
                System.out.println("name : " + s);
                return CompletableFuture.supplyAsync(() -> "hello world");
            });

	// 不使用lambda 有的时候类型推断需要自己改,如果你敲你绝对会遇见的, 还是推荐大家用lambda
    CompletableFuture.supplyAsync(() -> "tony")
            .thenCompose(new Function<String, CompletionStage<Object>>() {
                @Override
                public CompletionStage<Object> apply(String s) {
                    return CompletableFuture.supplyAsync(() -> "hello world");
                }
            });

    System.in.read();
}

// 输出 : 
// name : tony
```

由于他不能处理异常信息 , 会出现以下情况

```java
@Test
public void testCompose() throws IOException {

    CompletableFuture.supplyAsync(() -> {
        int i = 1 / 0;
        return "tony";
    }).thenCompose(s -> {
        System.out.println("name : " + s);
        return CompletableFuture.supplyAsync(() -> "hello world");
    });


    System.in.read();
}

// 输出 :  就没有输出结果 .. . ..
```



### 9.  取消 - cancel

`public boolean cancel(boolean mayInterruptIfRunning) `

> ​	If not already completed, completes this CompletableFuture with a CancellationException. Dependent CompletableFutures that have not already completed will also complete exceptionally, with a CompletionException caused by this CancellationException.

> ​	参数 : mayInterruptIfRunning – this value has no effect in this implementation because interrupts are not used to control processing.
>
> ​	意思就是`取消该阶段执行`, 取消失败返回异常, true 与 false 对取消没有影响 , 

```java
@Test
public void testCompose() throws IOException {

    CompletableFuture.runAsync(() -> {
        ThreadUtil.sleep(100);
        System.out.println("我执行1");
    }).thenRun(() -> {
        System.out.println("我执行2");
    });

    System.in.read();

}

输出 : 
// 我执行1
```



### 10. 顺序合并

#### 1. thenCombine

`public <U,V> CompletionStage<V> thenCombine (CompletionStage<? extends U> other,BiFunction<? super T,? super U,? extends V> fn);`

> ​	Returns a new CompletionStage that, when this and the other given stage both complete normally, is executed with the two results as arguments to the supplied function.
>
> ​	返回一个新的stage , 当这个和另一个给定的阶段都正常完成执行 , 将结果提供给fn方法

```java
@Test
public void testCombine() throws IOException {

    CompletableFuture.supplyAsync(() -> getName())
            .thenCombine(CompletableFuture.completedFuture("tony")
                    , new BiFunction<String, String, Object>() {
                        @Override
                        public Object apply(String s, String s2) {
                            System.out.println("s1  : " + s);
                            System.out.println("s2 : " + s2);
                            return "完成";
                        }
                    });

    System.in.read();
}

// 输出 : 
// s1  : tom
// s2 : hello
```

#### 2. thenAcceptBoth

`public <U> CompletionStage<Void> thenAcceptBoth  (CompletionStage<? extends U> other,BiConsumer<? super T, ? super U> action);`

> ​	Returns a new CompletionStage that, when this and the other given stage both complete normally, is executed with the two results as arguments to the supplied action.

这个和上面 `thenCombine` 的区别就是 , 他不提供返回值 ,  所以就不写了 , 都需要两个任务同时完成



#### 3. runAfterBoth

`public CompletionStage<Void> runAfterBoth(CompletionStage<?> other,Runnable action);`

> ​	Returns a new CompletionStage that, when this and the other given stage both complete normally, executes the given action. 

这个是前面俩阶段完成后 ,执行这个方法 , 不提供两个阶段的结果 , 所以不展示了 



#### 4. applyToEither

`public <U> CompletionStage<U> applyToEither (CompletionStage<? extends T> other,Function<? super T, U> fn);`

> ​	Returns a new CompletionStage that, when either this or the other given stage complete normally, is executed with the corresponding result as argument to the supplied function.

他呢就是 当两个任务有一个完成的时候就执行这个 , 将完成那个阶段的返回值提供给这个方法 fn

```java
@Test
public void testEither() throws IOException {
    CompletableFuture.supplyAsync(() -> {
        ThreadUtil.sleep(100);
        System.out.println("stage 1 执行");
        return "stage 1";
    }).applyToEither(CompletableFuture.supplyAsync(() -> {
        ThreadUtil.sleep(200);
        System.out.println("stage 2 执行");
        return "stage 2";
    }), new Function<String, Object>() {
        @Override
        public Object apply(String s) {
            System.out.println("收到最快完成的任务 : " + s);
            return null;
        }
    });

    System.in.read();
}

// 输出 : 
// stage 1 执行
// 收到最快完成的任务 : stage 1
// stage 2 执行
```

说明就算是有一个任务后执行, 也会执行的, 只不过这个方法是只要有一个任务有返回值就会执行 ...



#### 5. acceptEither

`public CompletionStage<Void> acceptEither (CompletionStage<? extends T> other, Consumer<? super T> action);`

> ​	Returns a new CompletionStage that, when either this or the other given stage complete normally, is executed with the corresponding result as argument to the supplied action.

当有一个任务正常完成就会执行他 , 他是一个消费方法, 所以不会提供返回值 , 不展示了



#### 6. runAfterEither

`public CompletionStage<Void> runAfterEither(CompletionStage<?> other, Runnable action);`

> Returns a new CompletionStage that, when either this or the other given stage complete normally, executes the given action.

这个和前两个一样 , 都是有一个正常完成就会执行后面的方法 , 不过他只是一个runnable 方法, 只是一个执行任务 , 

所以不展示了 ....



到这里基本上 `CompletionStage` 的方法都讲完了 , 基本上大家看JavaDoc 就可以随便写了 , 他主要是就是三个方法一个是 只提供, 一个是又消费又提供 ,一个只消费 ,一个我也不消费我也不提供, 就是这几个. 





### 11. 并行合并

> allOf方法是当所有的`CompletableFuture`都执行完完毕后才执行下一阶段。
>
> anyOf方法是当任意一个`CompletableFuture`执行完后执行下一阶段。

#### 1. allOf  

allOf 会将添加的阶段全部执行完毕后才执行下一阶段的任务 , 其中所有阶段都是`并行执行` ;

```java
@Test
public void testAddAll() throws IOException {

    CompletableFuture<Void> fun1 = CompletableFuture.runAsync(() -> {
        ThreadUtil.sleep(100);
        System.out.println("fun1");
    });

    CompletableFuture<Void> fun2 = CompletableFuture.runAsync(() -> {
        System.out.println("fun2");
    });


    CompletableFuture.allOf(fun1, fun2).thenRun(() -> {
        System.out.println("添加完毕");
    });

    System.in.read();
}

// 输出 : 
// fun2
// fun1
// 添加完毕	
```

#### 2.  anyOf 

anyOf 会其中一个执行完毕就会执行下一阶段,其中添加的任务都是`并行执行` ;

```java
@Test
public void testAddAll() throws IOException {

    CompletableFuture<Void> fun1 = CompletableFuture.runAsync(() -> {
        ThreadUtil.sleep(100);
        System.out.println("fun1");
    });

    CompletableFuture<Void> fun2 = CompletableFuture.runAsync(() -> {
        System.out.println("fun2");
    });


    CompletableFuture.anyOf(fun1, fun2).thenRun(() -> {
        System.out.println("添加完毕");
    });

    System.in.read();
}

输出 :
// fun2
// 添加完毕
// fun1
```



#### 3.测试并行执行

```java
@Test
public void testAddAll() throws IOException {

    long start = System.currentTimeMillis();

    CompletableFuture<Void> fun1 = CompletableFuture.runAsync(() -> {
        ThreadUtil.sleep(100);
        System.out.println("fun1");
    });

    CompletableFuture<Void> fun2 = CompletableFuture.runAsync(() -> {
        ThreadUtil.sleep(200);
        System.out.println("fun2");
    });

    CompletableFuture.allOf(fun1, fun2).thenRun(() -> {
        System.out.println("添加完毕");
        System.out.println(System.currentTimeMillis() - start);
    });

    System.in.read();
}
```

如果输出时间小于300ms 就表示是并行执行的 , 输出

```java
fun1
fun2
添加完毕
265  // 确实是并行执行 ...
```



## 3. 总结

希望我上面的总结你可以对于 `CompletionStage` 这个思想有新的认识 , 这种思想很重要, 流式编程 , 想异步想同步, 全在自己把握中 ,  第二节主要是讲了  `CompletionStage` 中的全部方法, 如果不想看我写的, 可以看看文档中的表述 , 其实Java编程中提供了 很多这种的例子, 可以学习一下 `guava` 的编程,或者基于 `scala`的编程

