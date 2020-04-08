# Java 的 Thread 你真的了解吗

> ​	Java语言一共设置了10个级别的线程优先级（Thread.MIN_PRIORITY至Thread.MAX_PRIORITY）。在两个线程同时处于Ready状态时，优先级越高的线程越容易被系统选择执行。(这个是Java定义的, 但是操作系统另有实现, 不过是JVM的事)
>
> ​	关于Java的线程具体实现, 是根据虚拟机决定的, 虚拟机提供了怎样的实现, 那么就是如何实现的. 一般是内核态线程, 用户态线程, 还有两者相结合. 都有使用. 这都是看JVM如何实现的, 所以广义上来说, 多线程的实现并不受限. 所以没有统一,对于C语言多线程实现方式有很多种呢都 .  
>
> ​		但是对于我们主流的JVM都是内核态线程(问题 : 映射到操作系统上的线程天然的缺陷是切换、调度成本高昂，系统能容纳的线程数量也很有限), 与操作系统的线程是1:1的关系.
>
> ​		比如新兴的Golang和Kotlin使用的就是用户态, 这种基于用户态实现的线程一般称之为 协程(Routine)或者纤程(Fiber) , 他的做法是所有调度全部由用户实现.  也就是说举个例子, 操作系统的线程是死的,但是线程是基于CPU的(对于操作系统来说就是要实现一个CPU上如何实现多线程,所以线程数是有操作系统决定的,所以用户直接用就行了), 但是协程则是基于线程的, 一个道理, 一个线程上可以运行多个协程, 所以需要用户去设计如何在一个线程上调度多个协程,  同时线程stack默认值是1M(可以通过-Xss设置), 但是协程可以是几k不等由用户定义, 所以对于一个线程可以运行几百个协程 .  关于操作系统多线程的实现可以自行百度(时间片算法), 其实明白了协程自然也明白了. 
>
> ​	还有一种实现是两者的结合. 不多讲. 
>
> ​	所以对于开发者来说, 只有JVM规范和Java规范. 没有所谓的绝对的做法.

关于以上这段话不懂的, 可以看看深入理解Java虚拟机这本书. 关于线程,协程,纤程如果还是不懂的可以看看[这篇文章](https://www.cnblogs.com/Leo_wl/p/10943969.html) .



一下就是Java的Thread了. 带你进入Java的世界. 你是否有些没有领略过呢. 

## 1. Thread

> ​	这个是Java线程唯一入口类, 其他都是封装, 不管线程池, 还是其他. 都是继承了Thread . runnable实际上是一个接口方法, 线程启动会调用罢了. 可以说是一个回调函数. 

### 实例化

#### 1. 无参 

```java
public Thread() {
        init(null, null, "Thread-" + nextThreadNum(), 0);
}
```

#### 2. 最常用的

> ​	target – the object whose run method is invoked when this thread is started. If null, this classes run method does nothing.
>
> **target 这个对象的run方法当线程开始是被运行 , 如果没有 , 这个类运行方法什么也不做 ,**
>
> **这种也是我们最常用的一种了**

```java
public Thread(Runnable target) {
    init(null, target, "Thread-" + nextThreadNum(), 0);
}
```

#### 3. 参数最全的

```java
public Thread(ThreadGroup group, Runnable target, String name,
                  long stackSize) {
        init(group, target, name, stackSize);
}
```

> group :  the thread group. If null and there is a security manager, the group is determined by SecurityManager.getThreadGroup(). If there is not a security manager or SecurityManager.getThreadGroup() returns null, the group is set to the current thread's thread group.
>
> name – the name of the new thread
>
> stackSize – the desired stack size for the new thread, or zero to indicate that this parameter is to be ignored.
>
> group - 线程组 , 如果没有的话,这里有一个security manager , 则由SecurityManager的`getThreadGroup()`方法确定组 , 如果没有security manager 等 ,则将该组设置为当前线程的线程组。
>
> target -  Runnable对象的run方法当线程开始是被运行 , 如果没有 , 这个类运行方法什么也不做 ,
>
> name  - 当前线程的名字
>
> stackSize - 新线程所需的堆栈大小，或者0表示忽略该参数。这里设置0可以让JVM自动管理 ,如果我们手动设置 ,会造成 ,栈溢出`StackOverflowError`或者栈太小了,内存溢出了 `OutOfMemoryError `或者 `内部 error`

### 核心 - start 

对于Java来说, 启动一个线程只有一个途径就是 ,  拿到一个Thread实例, 然后start , Java的线程和系统线程是等价的. 

对于一些人说创建一个线程的途径, 也是 所有继承或者直接实例化Thread都可以创建一个线程. 但是启动就一个方法入口就是start . 跟golang的 go关键字一样. 

这个是 start()方法的JavaDOC . 

> Causes this thread to begin execution; the Java Virtual Machine calls the run method of this thread.
> 	The result is that two threads are running concurrently: the current thread (which returns from the call to the start method) and the other thread (which executes its run method).
> 	It is never legal to start a thread more than once. In particular, a thread may not be restarted once it has completed execution.

当前线程开始执行, JVM会用调用当前线程的run方法. 

结果是两个线程并发执行(这两个线程, 一个是执行start方法的线程(可能是主线程,执行完start方法后, 会立马返回), 一个就是你实例化的线程(他会执行run方法)

我们不允许start方法被调用两次 ,这样是不合理的. 线程执行完毕大概是不会重启的 . (其实是靠线程内部一个状态量控制的,启动后就修改了,你调用start绝对会失败,而且start方法是sync修饰的)

## 2. 守护线程

`final void setDaemon(boolean on)`

> Marks this thread as either a daemon thread or a user thread. The Java Virtual Machine exits when the only threads running are all daemon threads.
> This method must be invoked before the thread is started.	
>
> **描述的是当JVM中的运行的线程就省下守护线程了, JVM就退出了  , 守护线程这个方法 必须在线程开始前调用,也就是说在start() 方法前调用的.**

```java
public final void setDaemon(boolean on) {
    //  the current thread cannot modify this thread
    checkAccess();
    // strat 运行后 ,线程状态会改成 isAlive ,此时会抛出异常
    if (isAlive()) {
        throw new IllegalThreadStateException();
    }
    // 默认是非守护线程
    daemon = on;
}
```

所谓守护线程,是指在程序运行的时候在后台提供一种通用服务的线程，比如垃圾回收线程就是一个很称职的守护者，并且这种线程并不属于程序中不可或缺的部分。

​		**其实对于后期开发, 对于服务线程, 尤其是线程池一般都是设置成deamon , 除了主线程尽可能的使用守护线程, 好处就是好控制JVM进程的关闭.  对于Golang来说,GoRoutine其实就是一个守护线程.  所以一般你运行,不加锁或者不等待,一般直接进程关闭了.**

## 3. 线程状态	

Java的 线程一共有六种状态  , 其实对于 new , runnable , terminated 大家都懂 , 就是其他三个不知道处于啥场景. 

### 种类 :

NEW  : Thread state for a thread which has not yet started.(执行实例化一个线程的操作后的状态)

RUNNABLE  : Thread state for a runnable thread. A thread in the runnable state is executing in the Java virtual machine but it may be waiting for other resources from the operating system such as processor. (这里其实是你执行了start方法后的 , 但是启动线程其实分为两步, 第一步就是去准备一些资源吧, 此时处于就绪状态, 等真正分配好了,才会进入真正的启动过程, 所以启动一个线程的代价比较大)

`BLOCKED  `:  Thread state for a thread blocked waiting for a monitor lock.(sync)

`WAITING `:  Thread state for a waiting thread. (Object.wait)

`TIMED_WAITING `:  Thread state for a waiting thread with a specified waiting time. (Sleep)

TERMINATED :  Thread state for a terminated thread. The thread has completed execution.

#### 首先说这几个状态如何查看吧, 

利用JDK自带的工具  , jstack 和  jconsole 都可以. .不然你手动咋查看状态. 开一个线程传入当前线程, 然后.... 太麻烦了 . 

#### 1. Thread.sleep(time)

这个其实是线程处于了 `TIMED_WAITING `  , 所以上面解释到 `Thread state for a waiting thread with a specified waiting time.`  , 是一致的.  

#### 2. Object.wait()

这个是处于 `WAITING ` 状态. 处于对象锁, 所以处于wait中.  

对于加了 timeout参数的则是出于 `TIMED_WAITING` 状态 

```java
名称: anthony
状态: java.lang.Object@3f462b58上的TIMED_WAITING
总阻止数: 0, 总等待数: 1
```

#### 3. Unsafe.park()

> ​	同理LockSupport也是一样的.  其实他就是基于Unsafe实现的. 

当你没有指定时间则是 `WAITING`  , 指定了等待时间则是 `TIMED_WAITING` 

```java
名称: anthony2
状态: TIMED_WAITING
总阻止数: 0, 总等待数: 1
```

但是这个为啥和上面那个不同 , 他没有等待对象, 是因为每一个线程内部都会有一个Blocker对象. 你需要设置这个对象. 那么调用park操作才会显示的展示出来 

```java
LockSupport.parkUntil(new Object(), System.currentTimeMillis() + 1111111L);
```

```java
volatile Object parkBlocker;
```

#### 4. synchronize

同步代码快,还是同步方法, 都是处于一个 `BLOCKED  ` 状态 . 

我们可以通过 jconsole 查看anthony线程这个对象被哪个线程拿着 . 显然是anthony-2这个线程. 

```java
名称: anthony
状态: java.lang.Object@18961ed8上的BLOCKED, 拥有者: anthony-2
总阻止数: 1, 总等待数: 0
```

总结一下, 就这么一张图, 引用自实战Java虚拟机. 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/e19e449c-46c4-4989-bf71-96e3628f4ae8.png)



### 阻塞与等待的区别 

**“阻塞状态”与“等待状态”的区别 : **

**“阻塞状态”在等待着获取到一个排它锁，这个事件将在另外一个线程放弃这个锁的时候发生；**

**“等待状态”则是在等待一段时间，或者唤醒动作的发生。在程序等待进入同步区域的时候，线程将进入这种状态。**

## 4. 如何关闭线程

### 1. interrupt() 方法

```java
public static void main(String[] args) throws InterruptedException {

    Thread thread = new Thread(() -> {
        // 这个方法只是测试是否当前时刻被打断.记住是某一刻.所以`thread.interrupt`并不能处理这里.
        while (!Thread.currentThread().isInterrupted()) {
            try {
                say();
                // 必须强制抓取InterruptedException异常,核心在这里.
            } catch (InterruptedException e) {
                // throw抛出去, 停止当前线程
                throw new RuntimeException(e.getMessage());
            }
        }
    });
    thread.start();
    // 运行1S ,然后发送一个打断命令.
    TimeUnit.MILLISECONDS.sleep(1000);
    thread.interrupt();
}

static void say() throws InterruptedException {
    TimeUnit.MILLISECONDS.sleep(100);
    System.out.printf("Thread : %s in %s.\n", Thread.currentThread().getName(), Thread.currentThread().getState());
}
```

输出 : 

```java
Thread : Thread-0 in RUNNABLE.
Thread : Thread-0 in RUNNABLE.
Thread : Thread-0 in RUNNABLE.
Thread : Thread-0 in RUNNABLE.
Thread : Thread-0 in RUNNABLE.
Thread : Thread-0 in RUNNABLE.
Thread : Thread-0 in RUNNABLE.
Thread : Thread-0 in RUNNABLE.
Thread : Thread-0 in RUNNABLE.
Exception in thread "Thread-0" java.lang.RuntimeException: sleep interrupted
	at com.guava.TestCondition.lambda$main$0(TestCondition.java:23)
	at java.lang.Thread.run(Thread.java:748)
```

### 2. throw new Exception()

```java
public static void main(String[] args) {
    new Thread(() -> {
        int x = 1;
        while (true) {
            System.out.println(Thread.currentThread().getName() + " : " + x++);
            if (x == 5) {
                // 抛出异常
                throw new RuntimeException("exception");
            }
        }
    }).start();
}
```

输出

```java
Thread-0 : 1
Thread-0 : 2
Thread-0 : 3
Thread-0 : 4
Exception in thread "Thread-0" java.lang.RuntimeException: exception
	at com.threadstatus.StopThreadByException.lambda$main$0(StopThreadByException.java:20)
	at java.lang.Thread.run(Thread.java:748)
```

### 3. 设置标识符

> 注意可见性.

```java
public class StopThreadWithFlag {
    private static volatile boolean flag = true;
    public static void main(String[] args) throws InterruptedException {
        Thread thread = new Thread(() -> {
            int x = 1;
            while (flag) {

                try {
                    TimeUnit.MILLISECONDS.sleep(500);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                System.out.println(Thread.currentThread().getName() + " : " + x++);
            }
        });
        thread.start();
        thread.join();

        flag = false;
    }
}
```

结果呢 ? 不会停止 , 因为主线程根本不执行 , 此时thread.join ,其实是一种阻塞状态 , 根本不会让主线程去执行 ,怎么办 .所以这种情况下 ,我们需要互换角色 , 此时主线程执行,子线程也会执行.

```java
public class StopThreadWithFlag {
    private static volatile boolean flag = true;

    public static void main(String[] args) throws InterruptedException {
        int x = 1;
        Thread thread = new Thread(() -> {
            flag = false;
        });
        thread.start();

        while (flag) {
            System.out.println(Thread.currentThread().getName() + " : " + x++);
        }
    }
}
```





参考文章 : 深入理解Java虚拟机.