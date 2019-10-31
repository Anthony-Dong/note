# JUC - 同步锁

## 1. 认识 synchronized 与 lock

### 两者的区别是什么 ?

- `synchronized`为 **java中的内置锁**：,特点是使用简单，一切交给JVM去处理,不需要显示释放(显式与隐式: 显式就是需要自己手动去调用 , 隐式就是不需要自己去执行,jvm内部给你执行) . 同时 其分为 **方法同步** ,与 **代码块同步**

- JDK 1.5 中加入了 `Lock`   , 其实需要 **手动加锁和释放锁** , 分为 两大类 主要是 `Lock`  和 `ReadWriteLock`

- 本节我们主要讲的是 `ReentrantReadWriteLock`  和 `ReentrantLock`

- 相比于`synchronized`，`ReentrantLock`在功能上更加丰富，它具有**可重入、可中断、可限时、公平锁**等特点。

- 在jdk1.5里面，`ReentrantLock`的性能是明显优于`synchronized`的，但是在jdk1.6里面，`synchronized`做了优化，他们之间的性能差别已经不明显了。可能在多核上表现的略显得较为明显,

  ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-22/2cc3bcf5-f8b0-4c65-ba63-2fb80b8b315e.jpg?x-oss-process=style/template01)



- 由于`ReentrantLock`是重入锁，所以可以**反复得到相同的一把锁**，它有一个与锁相关的获取计数器，如果拥有锁的某个线程再次得到锁，那么获取计数器就加1，然后锁需要被释放两次才能获得真正释放(重入锁)。



### 机制

**ReentrantLock**简单来说，它有一个与锁相关的获取计数器，如果拥有锁的某个线程再次得到锁，那么获取计数器就加1，然后锁需要被释放两次才能获得真正释放。

 `synchronized` 的语义；如果线程进入由线程已经拥有的监控器保护的 synchronized 块，就允许线程继续进行，当线程退出第二个（或者后续） `synchronized` 块的时候，不释放锁，只有线程退出它进入的监控器保护的第一个 `synchronized` 块时，才释放锁。



## 2. Lock -- ReentrantLock 可重入锁

### 简单使用

```java
public class TestSimpleUserLock {
    private static final Lock LOCK = new ReentrantLock();
	
    private static ExecutorService service = Executors.newFixedThreadPool(10);

    public static void main(String[] args) {
        //这种写法是 lambda 表达式
        service.execute(()->{
            TestSimpleUserLock.doLock();
        });
        //这种写法是 方法引用
        service.execute(TestSimpleUserLock::doLock);
        service.execute(TestSimpleUserLock::doLock);


        service.execute(TestSimpleUserLock::doUNLock);
        service.execute(TestSimpleUserLock::doUNLock);
        service.execute(TestSimpleUserLock::doUNLock);

        service.shutdown();

    }
	
    //加锁操作
    static void doLock() {
        // 加锁
        LOCK.lock();
        try {
            TimeUnit.SECONDS.sleep(1);
            System.out.println("当前线程 : " + Thread.currentThread().getName());
        } catch (InterruptedException e) {
            System.out.println("======sleep interrupted======" + Thread.currentThread().getName());
        } finally {
            // 释放锁
            LOCK.unlock();
        }
    }

    // 不加锁操作
    static void doUNLock() {

        try {
            TimeUnit.SECONDS.sleep(1);
            System.out.println("当前线程 : " + Thread.currentThread().getName());
        } catch (InterruptedException e) {
            System.out.println("======sleep interrupted======" + Thread.currentThread().getName());
        }
    }
}
```

**输出结果 : **

```java
当前线程 : pool-1-thread-1
当前线程 : pool-1-thread-4
当前线程 : pool-1-thread-5
当前线程 : pool-1-thread-6
当前线程 : pool-1-thread-2
当前线程 : pool-1-thread-3
```

​		很显然  加锁的是顺序执行 ,不加锁的是异步执行 , 1 结束了 456没有加锁的任务都直接结束了,我们发现 456线程并没有执行的时候被锁住



### 加锁的几种方式对比

1. `LOCK.lock();`方法 , 其实就是去获取一个可用的锁, 上面代码就是lock() 代码的效果

```java
我们不难发现  lock , 当线程执行lock() 方法时, 只能有一个线程执行.

1. Acquires the lock if it is not held by another thread and returns immediately, setting the lock hold count to one.
2. If the current thread already holds the lock then the hold count is incremented by one and the method returns immediately.
3. If the lock is held by another thread then the current thread becomes disabled for thread scheduling purposes and lies dormant until the lock has been acquired, at which time the lock hold count is set to one.    


源码中 lock 方法 有三种情况

1. 如果需求的锁没有被其他线程所持有,那么久立刻返回并且设置持有锁的数量为1
2. 如果当前线程已经持有锁,那么持有的数量加一,然后返回,  就是一个多次执行 lock 方法() 
3. 如果这个锁 被其他线程持有,那么当前线程则不能调度线程,然后休眠,止到有可用的锁,同时将锁的持有数量设置为1

所以呢,只能有一个线程获取锁,达到同步执行的效果
```



2. 我们上面发现一个问题就是 如果我的线程被中断了会如何 ? 加锁过程被中断了是否执行? ------ 这就引出了我们的 `LOCK.lockInterruptibly();`  我们看看下面这段代码对比一下

```java
public class TestSimpleUserLock {
    private static final Lock LOCK = new ReentrantLock();

    private static ExecutorService service = Executors.newFixedThreadPool(10);



    public static void main(String[] args) {

        // 执行第一段
//        service.execute(TestSimpleUserLock::doInterruptibleLock);
//        service.execute(TestSimpleUserLock::doInterruptibleLock);
//        service.execute(TestSimpleUserLock::doInterruptibleLock);
		 // 执行第二段
        service.execute(TestSimpleUserLock::doLock);
        service.execute(TestSimpleUserLock::doLock);
        service.execute(TestSimpleUserLock::doLock);
        
        service.shutdownNow();

    }

    static void doLock() {
        LOCK.lock();
        try {
            System.out.println("我执行了 : "+Thread.currentThread().getName());
            TimeUnit.SECONDS.sleep(1);
            System.out.println("当前线程 : " + Thread.currentThread().getName());
        } catch (InterruptedException e) {
            throw new RuntimeException("sleep InterruptedException in "+Thread.currentThread().getName());
        } finally {
            LOCK.unlock();
        }
    }


    static void doInterruptibleLock() {
        try {
            LOCK.lockInterruptibly();
        } catch (InterruptedException e) {
            throw new RuntimeException("InterruptedException in "+Thread.currentThread().getName());
        }
        try {
            System.out.println("我执行了 : "+Thread.currentThread().getName());
            TimeUnit.SECONDS.sleep(1);
            System.out.println("当前线程 : " + Thread.currentThread().getName());
        } catch (InterruptedException e) {
            throw new RuntimeException("sleep Exception in "+Thread.currentThread().getName());
        } finally {
            LOCK.unlock();
        }
    }
}

```

我们知道 `service.shutdownNow();` 是强制中断线程的,所以可以模拟线程中断情况

执行我们的 `doInterruptibleLock `方法时 输出结果 : 

```java
我执行了 : pool-1-thread-1
Exception in thread "pool-1-thread-2" Exception in thread "pool-1-thread-1" Exception in thread "pool-1-thread-3" 
    // 线程一
java.lang.RuntimeException: sleep Exception in pool-1-thread-1
	at com.lock.TestSimpleUserLock.doInterruptibleLock(TestSimpleUserLock.java:64)
	at java.util.concurrent.ThreadPoolExecutor.runWorker(ThreadPoolExecutor.java:1149)
	at java.util.concurrent.ThreadPoolExecutor$Worker.run(ThreadPoolExecutor.java:624)
	at java.lang.Thread.run(Thread.java:748)
    // 线程二
java.lang.RuntimeException: InterruptedException in pool-1-thread-2
	at com.lock.TestSimpleUserLock.doInterruptibleLock(TestSimpleUserLock.java:57)
	at java.util.concurrent.ThreadPoolExecutor.runWorker(ThreadPoolExecutor.java:1149)
	at java.util.concurrent.ThreadPoolExecutor$Worker.run(ThreadPoolExecutor.java:624)
	at java.lang.Thread.run(Thread.java:748)
    // 线程三
java.lang.RuntimeException: InterruptedException in pool-1-thread-3
	at com.lock.TestSimpleUserLock.doInterruptibleLock(TestSimpleUserLock.java:57)
	at java.util.concurrent.ThreadPoolExecutor.runWorker(ThreadPoolExecutor.java:1149)
	at java.util.concurrent.ThreadPoolExecutor$Worker.run(ThreadPoolExecutor.java:624)
	at java.lang.Thread.run(Thread.java:748)

```

我们发现线程一执行了输出语句,其他两个个都被打断了, 直接在获取锁那里就抛出去了



那么当我们改成执行`doLock` 方法时 , 结果是什么呢? 

```java
我执行了 : pool-1-thread-1
我执行了 : pool-1-thread-3
 我执行了 : pool-1-thread-2    
Exception in thread "pool-1-thread-3"Exception in thread "pool-1-thread-1"  Exception in thread "pool-1-thread-2" 
    // 线程三
java.lang.RuntimeException: sleep InterruptedException in pool-1-thread-3
	at com.lock.TestSimpleUserLock.doLock(TestSimpleUserLock.java:46)
	at java.util.concurrent.ThreadPoolExecutor.runWorker(ThreadPoolExecutor.java:1149)
	at java.util.concurrent.ThreadPoolExecutor$Worker.run(ThreadPoolExecutor.java:624)
	at java.lang.Thread.run(Thread.java:748)
    // 线程二
java.lang.RuntimeException: sleep InterruptedException in pool-1-thread-2
	at com.lock.TestSimpleUserLock.doLock(TestSimpleUserLock.java:46)
	at java.util.concurrent.ThreadPoolExecutor.runWorker(ThreadPoolExecutor.java:1149)
	at java.util.concurrent.ThreadPoolExecutor$Worker.run(ThreadPoolExecutor.java:624)
	at java.lang.Thread.run(Thread.java:748)
    // 线程一
java.lang.RuntimeException: sleep InterruptedException in pool-1-thread-1
	at com.lock.TestSimpleUserLock.doLock(TestSimpleUserLock.java:46)
	at java.util.concurrent.ThreadPoolExecutor.runWorker(ThreadPoolExecutor.java:1149)
	at java.util.concurrent.ThreadPoolExecutor$Worker.run(ThreadPoolExecutor.java:624)
	at java.lang.Thread.run(Thread.java:748)
```



我们发现他们三个都执行了输出语句,只是 sleep 方法被打断了 ,所以 对于加锁我们可以使用 `LOCK.lockInterruptibly();`  这样子可以更加安全的使用锁 , 如果获取锁的时候被打断了,那么我们是不是必须不让他执行呢 ? 



3. `LOCK.tryLock()` 方法 ,返回一个 `Bollean`值,我们看看官方说法 

```java
Acquires the lock only if it is free at the time of invocation. 只有在一把锁处于空闲的时候才能调用

Acquires the lock if it is available and returns immediately with the value {@code true}.If the lock is not available then this method will return immediately with the value {@code false}.

如果这把锁可以用立马返回true , 不能用立马返回false ,    

官方提供的通用写法
* <p>A typical usage idiom for this method would be:
     *  <pre> {@code
     * Lock lock = ...;
     * if (lock.tryLock()) {
     *   try {
     *     // manipulate protected state
     *   } finally {
     *     lock.unlock();
     *   }
     * } else {
     *   // perform alternative actions
     * }}</pre>
```



所以我们照着官方的意思 去使用

```java
public class TestSimpleUserLock {
    private static final Lock LOCK = new ReentrantLock();

    private static ExecutorService service = Executors.newFixedThreadPool(10);

    public static void main(String[] args) {
        service.execute(TestSimpleUserLock::doTryLock);
        service.execute(TestSimpleUserLock::doTryLock);
        service.execute(TestSimpleUserLock::doTryLock);
    }

    static void doTryLock() {
        if (LOCK.tryLock()) {
            try {
                System.out.println("我执行了 : " + Thread.currentThread().getName());
                TimeUnit.SECONDS.sleep(2);
                System.out.println("当前线程 : " + Thread.currentThread().getName());
            } catch (InterruptedException e) {
                e.printStackTrace();
            } finally {
                LOCK.unlock();
            }

        } else {
            System.out.println("没拿到锁,我很慌, "+Thread.currentThread().getName());
        }
    }
}
```

**输出结果:** 

```java
我执行了 : pool-1-thread-1
没拿到锁,我很慌, pool-1-thread-2
没拿到锁,我很慌, pool-1-thread-3
当前线程 : pool-1-thread-1
```

结果表明 , 获取锁的人执行 ,没获取锁的人直接去浪了,这么做可以有个好处就是判断是否获取锁,这样子其实我们可以进行递归操作,自旋锁,但是发现会出现栈溢出 `java.lang.StackOverflowError` ,这个 溢出是啥呢 ?

`Thrown when a stack overflow occurs because an application recurses too deeply.`  `recurses `是递归的意思,所以就是因为当前应用递归太深导致栈溢出.



4. `LOCK.tryLock(1, TimeUnit.SECONDS)`  就是上面的方法加了一个超时时间.



## 3. ReadWriteLock -- ReentrantReadWriteLock



## 4. synchronized 讲解

### 1. 同步代码块

为了解决并发操作可能造成的异常，java的多线程支持引入了同步监视器来解决这个问题，使用同步监视器的通用方法就是同步代码块。其语法如下：

```java
synchronized(obj){

//同步代码块

}
```



**其中obj就是同步监视器**，它的含义是：线程开始执行同步代码块之前，**必须先获得对同步监视器的锁定**。任何时刻**只能有一个线程可以获得对同步监视器的锁定**，当同步代码块执行完成后，**该线程会释放对该同步监视器的锁定**。虽然java程序允许使用任何对象作为同步监视器，但 是同步监视器的目的就是为了阻止两个线程对同一个共享资源进行并发访问，因此通常推荐使用可能被并发访问的共享资源充当同步监视器。

### 2. 同步方法

​		对于关键字synchronized修饰的方法，不需要再指定同步监视器，这个同步方法**（非static方法）**无需显式地指定同步监视器，**同步方法的同步监视器就是this**，也就是**调用该方法的对象**。**static方法**的锁为 "对应的Class实例";

​		**注意，synchronized可以修饰方法，修饰代码块，但是不能修饰构造器、成员变量等。**

```java
public synchronized void testThread()
{
    //需要被同步的代码块
}
```







## 5. volatile 关键字

> - volatile 关键字: 当多个线程进行操作共享数据时,可以保证内存中的数据是可见的;相较于 synchronized 是一种
>   较为轻量级的同步策略;
> - volatile 不具备"互斥性";
> - volatile 不能保证变量的"原子性"; ()

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-23/2dd664d4-5044-468b-9ab2-a321387ace71.png?x-oss-process=style/template01)

```java
package com.volatile_test;

public class TestVolatile{

    public static void main(String[] args){
        ThreadDemo td = new ThreadDemo();
        // 子线程
        Thread thread = new Thread(td);

        thread.start();

        // 主线程  对于 子线程的改变并不知情, 主要是 由于主线程先执行 ,他不知道 子线程的改变 如果我们让子线程 join一下就发生改变 了
        while(true){
            if(td.isFlag()){
                System.out.println(td.isFlag());
                break;
            }
        }
    }
}

class ThreadDemo implements Runnable{
    //  加与不加的区别  (不加 由于子线程改变了 flag ,主线程不知道他改变 ,所以会无限循环下去 上面的while代码)
    private   boolean flag = false;

    public void run(){
        try{
            // 该线程 sleep(200), 导致了程序无法执行成功
            Thread.sleep(200);
        }catch(Exception e){
            e.printStackTrace();
        }

        flag = true;

        System.out.println("flag: "+flag);
    }

    public boolean isFlag(){
        return flag;
    }

    public void setFlag(boolean flag){
        this.flag = flag;
    }
}
```





## 6. 原子性问题

举例 :

>  1. java中  i++ 主要是 分为了  三步 
>
>     ​		初始化 (不算)  int i = 10;
>
>     ​	　　int temp = i;
>     ​			i = i + 1;
>     ​			a = temp;
>
>  2. 其中 **" 读-改-写"**是不可分割的三个步骤;
>
>  3. 原子变量 : JDK1.5 以后,  `java.util.concurrent.atomic`包下,提供了常用的原子变量; 
>
> 	4. 原子变量中的值,使用 `volatile` 修饰,保证了内存可见性;
>
> 	5. CAS(Compare-And-Swap) 算法保证数据的原子性;

```java

import java.util.concurrent.atomic.AtomicInteger;

public class TestAtomicDemo{
    public static void main(String[] args){

        AtomicDemo ad = new AtomicDemo();

        for(int i=0; i < 10; i++){
            new Thread(ad).start();
        }

        SolveAtomicDemo sad = new SolveAtomicDemo();
        for(int i=0; i < 10; i++){
            new Thread(sad).start();
        }

    }
}

// 没有原子操作
class AtomicDemo implements Runnable{
    // 由于我们就只创建了一个 实例对象, 加不加static是效果 一样的
    private  int serialNumber = 0;

    public void run(){

        try{
            Thread.sleep(20);
        }catch(InterruptedException e){

        }

        System.out.println(Thread.currentThread().getName() + ":" + getSerialNumber());
    }

    private int getSerialNumber(){
        return serialNumber++;
    }
}
// 原子操作
class  SolveAtomicDemo implements Runnable{

    //  创建一个原子变量
    private AtomicInteger serialNumber = new AtomicInteger();


    //
    @Override
    public void run() {
        try{
            Thread.sleep(20);
        }catch(InterruptedException e){

        }

        System.out.println(Thread.currentThread().getName() + ":" + getSerialNumber());

    }

    private int getSerialNumber(){
        // 自增 ==  i++
        return serialNumber.getAndIncrement();
    }
}
```

我们 看一下 atomic 包下的 对象 , 包含了常见的操作对象  boolean integer long  (第三方集成的 double 等)

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-23/14292bf9-c851-4931-8519-67231c8c1aad.jpg?x-oss-process=style/template01)



![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-23/8f25c5d8-4f68-4ffd-9ca4-2a2c39b3d4a3.jpg?x-oss-process=style/template01)



## 7.如何选择

1. 我们什么时候才应该使用 `ReentrantLock` 呢？答案非常简单 —— 在确实需要一些 synchronized 所没有的特性的时候，比如时间锁等候、可中断锁等候、无块结构锁、多个条件变量或者锁投票。
2. 我建议用 synchronized 开发，直到确实证明 synchronized 不合适，而不要仅仅是假设如果使用 `ReentrantLock` “性能会更好”。请记住，这些是供高级用户使用的高级工具。（而且，真正的高级用户喜欢选择能够找到的最简单工具，直到他们认为简单的工具不适用为止。）。一如既往，首先要把事情做好，然后再考虑是不是有必要做得更快。
3. `Lock` 框架是同步的兼容替代品，它提供了 `synchronized` 没有提供的许多特性，它的实现在争用下提供了更好的性能。但是，这些明显存在的好处，还不足以成为用 `ReentrantLock` 代替 `synchronized` 的理由。相反，应当根据您是否 *需要* `ReentrantLock` 的能力来作出选择。大多数情况下，您不应当选择它 —— synchronized 工作得很好，可以在所有 JVM 上工作，更多的开发人员了解它，而且不太容易出错。只有在真正需要 `Lock` 的时候才用它。在这些情况下，您会很高兴拥有这款工具。

[java基础练习](https://github.com/Anthony-Dong/javapractice/tree/master/JavaSeBasics)