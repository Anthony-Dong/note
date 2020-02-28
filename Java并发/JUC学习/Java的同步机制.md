# Java - 锁与同步操作

## 1. synchronized关键字 与 Lock锁

JUC中提供了伟大的Lock锁, 通过Java编程实现的的, 是著名的并发编程的作者 `Doug Lea` 实现的. 在当时JDK1.5时代之前, Java的synchronized效率太低了. 因此Java公司看不下去了, 当时Doug Lea写了个并发框架, 就是叫做 `dj.util.concurrent`  , 因此Java为了长久的发展, 和短暂的解决synchronize的问题 , 整合了 juc包. 所以随之而来就是JUC包的大量使用. 

但是Java并不会放弃 synchronized的优化. 他们不断的优化, 其实优化思想也是类似于JUC中Lock的思想. 利用自旋和其他多线程特点, 来减少线程直接进行挂起和等待的操作. 

我们来看看他们的性能对比吧. 在JDK1.5的时候, 现在都1.8了人人使用的是. 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/e5742761-acc8-41c5-bd83-611bf7974d84.png)



![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/7be9b607-3bd4-4396-b834-41096166b13b.png)



​		通过简单的对比. 发现差距十分的大. 但是 尽管在JDK 5时代ReentrantLock曾经在性能上领先过synchronized，但这已经是十多年之前的胜利了。从长远来看，Java虚拟机更容易针对synchronized来进行优化，因为Java虚拟机可以在线程和对象的元数据中记录synchronized中锁的相关信息，而使用J.U.C中的Lock的话，Java虚拟机是很难得知具体哪些锁对象是由特定线程锁持有的。

## 2. ReentrantLock 可重入锁

> ​	ReentrantLock 从字面可以看出是一把可重入锁，这点和 synchronized 一样，但实现原理也与 syncronized 有很大差别，它是基于经典的 AQS(AbstractQueueSyncronized) 实现的, AQS 是基于 volitale 和 CAS 实现的，其中 AQS 中维护一个 valitale 类型的变量 state 来做一个可重入锁的重入次数，加锁和释放锁也是围绕这个变量来进行的。 ReentrantLock 也提供了一些 synchronized 没有的特点，因此比 synchronized 好用。

锁的用处 : 当一个资源被多个线程共用时,如果执行修改操作, 那么就存在线程安全的问题. 有可能ab线程同时读取到的都是1,那么修改只能+1,那么两次计算结果就会出现少计算一次. 有兴趣的可以看看线程读取变量到写入变量的各种操作, 其实分为很多步.

```java
public class Reentrant {

    public static void main(String[] args) throws InterruptedException {
        ExecutorService pool = Executors.newFixedThreadPool(10);
        IntStream.range(0,10).forEach(e->{
            pool.execute(()->{
                for (int x = 0; x < 10000; x++) {
                    increase();
                }
            });
        });
        // 等到全部线程执行完毕输出
        pool.shutdown();
        pool.awaitTermination(Integer.MAX_VALUE, TimeUnit.DAYS);
        System.out.println("输出结果 : "+x);
    }

    private static int x = 0;

    private static void increase() {
        x += 1;
    }
}
```

每次执行结果,可能有些偏差,但是绝对会小于 100000的

```java
输出结果 : 67398
```

那如何处理呢, 这时候就需要加一把锁了 .  可以使用sync修饰 `increase` 方法

```java
private synchronized static void increase() {
    x += 1;
}
```

此时输出结果为 

```java
输出结果 : 100000
```

也可以使用我们说到的Lock , 我们这里使用的是 ReentrantLock , 是一个可重入锁. 

```java
private static final Lock LOCK = new ReentrantLock();

private  static void increase() {
    try {
        LOCK.lock();
        x += 1;
    } finally {
        LOCK.unlock();
    }
}
```

这样子输出结果也是对的. `LOCK.lock();` 会使得只有一个线程通过去, 其他的都在等待他前面的线程执行unlock操作. 

> ​	以下是可重入锁的特性. 

### **1、可重入**

 	ReentrantLock 和 syncronized 关键字一样，都是可重入锁，不过两者实现原理稍有差别， RetrantLock 利用 AQS 的的 state 状态来判断资源是否已锁，同一线程重入加锁， state 的状态 +1 ; 同一线程重入解锁, state 状态 -1 (解锁必须为当前独占线程，否则异常); 当 state 为 0 时解锁成功。

```java
new Thread(() -> {
    try {
        // 加锁
        LOCK.lock();
        try {
            // 加锁
            LOCK.lock();
            // 
        } finally {
            // 释放锁
            LOCK.unlock();
        }
    } finally {
        // 释放锁
        LOCK.unlock();
    }
}, "t1").start();
```

我强烈不推荐这种火箭代表. 他难看了. 

### 2.需要手动加锁、解锁

> ​	上诉例子. 已经展示了



### 3. 支持设置锁的超时时间

​	 synchronized 关键字无法设置锁的超时时间，如果一个获得锁的线程内部发生死锁，那么其他线程就会一直进入阻塞状态，而 ReentrantLock 提供 tryLock 方法，允许设置线程获取锁的超时时间，如果超时，则跳过，不进行任何操作，避免死锁(DeadLock)的发生。

```java
// 一刻也不等待
boolean flag = LOCK.tryLock();


// 等待1000ms
boolean flag = LOCK.tryLock(1000, TimeUnit.MILLISECONDS);
```

### 4. 支持公平/非公平锁

​		synchronized 关键字是一种非公平锁，先抢到锁的线程先执行。而 ReentrantLock 的构造方法中允许设置 true/false 来实现公平、非公平锁，如果设置为 true ，则线程获取锁要遵循"先来后到"的规则，每次都会构造一个线程 Node ，然后到双向链表的"尾巴"后面排队，等待前面的 Node 释放锁资源。

```java
// 参数为true 则是公平, 不加或者写false为非公平
private static final ReentrantLock FAIR_LOCK = new ReentrantLock(true);
private static final ReentrantLock UNFAIR_LOCK = new ReentrantLock(false);
```

### 5. 可中断锁

 	ReentrantLock 中的 lockInterruptibly() 方法使得线程可以在被阻塞时响应中断，比如一个线程 t1 通过 lockInterruptibly() 方法获取到一个可重入锁，并执行一个长时间的任务，另一个线程通过 interrupt() 方法就可以立刻打断 t1 线程的执行，来获取t1持有的那个可重入锁。而通过 ReentrantLock 的 lock() 方法或者 Synchronized 持有锁的线程是不会响应其他线程的 interrupt() 方法的，直到该方法主动释放锁之后才会响应 interrupt() 方法。

```java
public class Main {
    private static final ReentrantLock LOCK = new ReentrantLock(false);

    public static void main(String[] args) throws InterruptedException {
        // 线程一 绝对先执行
        Thread thread1 = new Thread(() -> {
            try {
                LOCK.lockInterruptibly();
                TimeUnit.SECONDS.sleep(20000);
            } catch (InterruptedException e) {
                System.out.println(String.format("%s 被打断了", Thread.currentThread().getName()));
            } finally {
                LOCK.unlock();
            }
        }, "t1");
        thread1.start();

        Thread thread2 = new Thread(() -> {
            try {
                // 打断它
                thread1.interrupt();
                LOCK.lock();
                System.out.println(String.format("%s 打断了 %s", Thread.currentThread().getName(), thread1.getName()));
            } finally {
                // 释放锁
                LOCK.unlock();
            }
        }, "t2");
        thread2.start();
    }
}
```

输出 : 

```java
t1 被打断了
t2 打断了 t1
```

## 3. ReentrantReadWriteLock 读写锁

> ​	一个读写锁**只能同时存在一个写锁 , 但是可以存在多个读锁**，**但不能同时存在写锁和读锁**。  这就是读写锁的核心. 
>
> 对于`ReentrantReadWriteLock` 内部其实维护了俩Lock , 一个是 `ReadLock` 一个是`WriteLock` 

​	**ReentrantReadWriteLock (读写锁)其实是两把锁，一把是 WriteLock (写锁)，一把是读锁， ReadLock 。**读写锁的规则是：`读读不互斥、读写互斥、写写互斥`。在一些实际的场景中，**读操作的频率远远高于写操作，如果直接用一般的锁进行并发控制的话，就会读读互斥、读写互斥、写写互斥，效率低下，读写锁的产生就是为了优化这种场景的操作效率。**一般情况下独占锁的效率低来源于高并发下对临界区的激烈竞争导致线程上下文切换。因此当并发不是很高的情况下，读写锁由于需要额外维护读锁的状态，可能还不如独占锁的效率高，因此需要根据实际情况选择使用。 

​		ReentrantReadWriteLock 的原理也是基于 AQS 进行实现的，与 ReentrantLock 的差别在于 ReentrantReadWriteLock 锁拥有共享锁、排他锁属性。读写锁中的加锁、释放锁也是基于 Sync (继承于 AQS )，并且主要使用 AQS 中的 state 和 node 中的 waitState 变量进行实现的。实现读写锁与实现普通互斥锁的主要区别在于需要分别记录读锁状态及写锁状态，并且等待队列中需要区别处理两种加锁操作。 ReentrantReadWriteLock 中将 AQS 中的 int 类型的 state 分为高 16 位与第 16 位分别记录读锁和写锁的状态，如下图所示：

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/e79ad658-a8b0-4929-9198-d4cc448d0ff5.webp)

```java
private static final ReadWriteLock RW_LOCK = new ReentrantReadWriteLock();

try {
    RW_LOCK.readLock().lock();

    // do , 基本就这个
} finally {
    RW_LOCK.readLock().unlock();
}
```



## 4. volatile 保证数据可见性

> ​	当一个变量被定义成volatile之后，它将具备两项特性:第一项是保证此变量对所有线程的可见性，**这里的”可见性”是指当一条线程修改了这个变量的值，新值对于其他线程来说是可以立即得知的**。而普通变量并不能做到这一点，普通变量的值在线程间传递时均需要通过主内存来完成。比如，线程A修改一-个普通变量的值，然后向主内存进行回写，另外一条线程B在线程A回写完成了之后再对主内存进行读取操作,新变量值才会对线程B可见。 --- 引用自深入理解Java虚拟机

我们以一个例子为讲解对象. 

```java
// 申明一个变量 
static boolean flag = true;
public static void main(String[] args) {
    new Thread(() -> {
        sleep(1000);
        flag = false;
    }).start();

    // 等待他变为false就不执行了.(这里主线程一直空转)
    while (flag) {
		// do nothing
    }
    System.out.println("shutdown");
}
```

我们发现1000ms后并不会输出shutdown. 会一直阻塞. 

但是我们将 `flag` 加入 `volatile`  ,再次运行.  会发现1000ms后输出结束. 这就是volatile的作用. 保证数据的多线程中可见性.  

如果你有兴趣的话 ,可以看看juc包中的 `FutureTask`   , 其他他也是这么实现的. 

Volatile还有另外一个特点就是内存屏障 . 他会对于指令重排进行限制. 我们看看下面这个例子. 

```java
public class Singleton {
    // 加了volatile修饰
    private volatile static Singleton instance;

    public static Singleton getInstance() {
        // 双重检测
        if (instance == null) {
            synchronized (Singleton.class) {
                if (instance == null) {
                    instance = new Singleton();
                }
            }
        }
        return instance;
    }

    public static void main(String[] args) {
        Singleton.getInstance();
    }
}
```

他进行JIT编译后的代码是 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/ea7bbc4a-5537-4cbd-b689-9c11a140f4fa.png)

​	通过对比发现，关键变化在于有volatile修饰的变量， 赋值后(前面mov%eax, 0x 150(%esi)这句便是赋值操作)多执行了-个"lock addl$0x0, (%esp)" 操作，这个操作的作用相当于-一个**内存屏障(Memory Barrier或Memory Fence,指重排序时不能把后面的指令重排序到内存屏障之前的位置)**，只有一个处理器访问内存时，并不需要内存屏障;但如果有两个或更多处理器访问同- -块内存，且其中有一一个在观测另一个，就需要内存屏障来保证一致性了。 但是指令重排是属于处理器采用了允许多条指令不按程序规定的顺序分法给各个相应的电路单元进行处理 . 但不是说指令是任意重排的 , 如果有依赖的就不会重排, 比如 A+2 然后再将 A*2, 显然不能指令重排, 但是 我们引入B变量 . 可能执行在A前面 A后面 中间都可以哇. 因为AB变量无关. 

总结一下 :  volatile第一可以保证数据的可见性, 第二他可以制止指令重排 .  对与volition变量我们要知道他的读取消耗和普通变量一样, 但是写会引起很大的消耗. (这里的涉及到多线程对于volatile变量的执行顺序, 叫做`happen-before` , 其实就是先行先发生的意思,做啥事不可能没有先后,只是精度不够) ,  还有涉及到volatile变量的大量读写, 因为每次修改都需要立马刷新主存. 那么他会引起其他变量受难 , 这就是 false-sharing , 叫做伪共享 , 可以加入注解,或则引入额外变量进行加快读写. 



## 5. 原子性问题

举例 :

>  Java中执行 x=x+1 操作是,  将读取变量x,然后入栈, 将1入栈, 将栈顶两个数相加, 然后结果入栈, 将结果保存在变量x中'
>
>  所以结果就是分为  读取变量x  , 将x+1 得到的结果再赋值给x  . 
>
>  这就是一个原子操作, 他不是一步, 而是几步是一个原子  , 可以说是一个事务.  ​

所以引入了 atomic类. 里面都是原子操作. 基于CAS实现的,同时还有自旋操作. 





