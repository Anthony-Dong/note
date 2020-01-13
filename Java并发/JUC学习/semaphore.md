# JUC - Semaphore 与 Lock的区别

## 介绍

> ​	A counting semaphore.  Conceptually, a semaphore maintains a set of permits. 
>
> ```
> Semaphores are often used to restrict the number of threads than can access some (physical or logical) resource. For example, here is a class that uses a semaphore to control access to a pool of items:
> ```
>
> ​	一个计数信号量.概念上将,信号量维护了一系列的许可证 . 文档里还说了semaphore 通常被用来限制访问一些资源的线程数量. 例如 这里有一个类,它使用`semaphore ` 去控制访问items的池子
>
> 



## Semaphore  使用

### 简单看看使用情况

```java
public class TestSemaphore {

    public static void main(String[] args) {
        final Semaphore semaphore = new Semaphore(2);
        System.out.println("----------Semaphore-----------");
        IntStream.range(0, 4).forEach(i -> new Thread(() -> {
            System.out.println(Thread.currentThread().getName() + "  开始");
            try {
                semaphore.acquire(); // 获取所有许可证，抽干

                System.out.println(Thread.currentThread().getName() + "  获取许可证");
                TimeUnit.SECONDS.sleep(3);
                System.out.println(Thread.currentThread().getName() + "  等待完毕");
            } catch (InterruptedException e) {
                e.printStackTrace();
            } finally {
                System.out.println(Thread.currentThread().getName() + "  释放许可证");
                semaphore.release(1);
            }
            System.out.println(Thread.currentThread().getName() + " 结束");
        }, "thread" + (i + 1)).start());
    }
}
```

**执行结果**

```java
----------Semaphore-----------
thread1  开始
thread1  获取许可证
thread2  开始
thread2  获取许可证
thread3  开始
thread4  开始
thread1  等待完毕
thread1  释放许可证
thread3  获取许可证
thread1  结束
thread2  等待完毕
thread2  释放许可证
thread4  获取许可证
thread2  结束
thread3  等待完毕
thread3  释放许可证
thread3  结束
thread4  等待完毕
thread4  释放许可证
thread4  结束
```

我们发现只有获取许可证的线程才可以去执行线程任务,当释放许可证的时候,其他线程可以去获取这个许可证

### Semaphore  的构造方法

```
Semaphore semaphore = new Semaphore(2,false);
```

我们发现他是可以穿两个参数的 ,第一个是许可证的数量,第二个是 是否公平, false是不公平

### Semaphore  的常用方法

- semaphore.acquire()

```java
// Acquires a permit from this semaphore, blocking until one is available, or the thread is {@linkplain Thread#interrupt interrupted}.
semaphore.acquire(); 
```

`semaphore.acquire() `这个方法主要是用来获取一个permit , 直到等到一个可用的permit,才会阻塞结束,或者这个线程被打断.

- semaphore.release(1);

```java
// Releases the given number of permits, returning them to the semaphore.
semaphore.release(1);
semaphore.release();  // 不传参数默认是1 
```

`semaphore.release(1);`  这个方法可以指定一permit的大小, 释放给定数量的permit,返回给 smaphore 

- semaphore.drainPermits()

```java
// Acquires and returns all permits that are immediately available.
semaphore.drainPermits();
```

`semaphore.drainPermits()` 这个方法就是 获取并且并且立马返回所以可用的permit . 所以这个方法不需要release

- semaphore.acquireUninterruptibly(5)

```java
// Acquires the given number of permits from this semaphore,blocking until all are available.
semaphore.acquireUninterruptibly(5);
```

`semaphore.acquireUninterruptibly(5)` 这个方法的字面意思就是获取不间断的 `Uninterruptibly`是不间断的意思. 所以解释就是 需要获取给定数量的permit,止到这些permit都是可用的才阻塞结束.

- semaphore.availablePermits()

```
// Returns the current number of permits available in this semaphore.
semaphore.availablePermits()
```

`semaphore.availablePermits()` 这个方法会立刻返回所有可用的permit 



### 上面讲的的` permit` 对象是什么呢 ? 

我翻阅 源码发现 

```java
/**
 * The synchronization state.
 */
private volatile int state;
/**
 * Returns the current value of synchronization state.
 * This operation has memory semantics of a {@code volatile} read.
 * @return current state value
 */
protected final int getState() {
    return state;
}
```

这状态 表示 当前permit的数量 ,他用`volatile ` 修饰的目的就是为了 让多线程之间可以去共享这个变量 , 保证其可见性 . 根据数量进行 reduce 和 increase , 





## 对比一下 lock



```java
public class TestLock {

    public static void main(String[] args) {

        final ReentrantLock lock = new ReentrantLock();

        System.out.println("--------ReentrantLock-----------");

        IntStream.range(0, 4).forEach(e->{
            new Thread(()->{
                System.out.println(Thread.currentThread().getName() + "  开始");
                lock.lock(); // 一次拿一把锁
                try {
                    System.out.println(Thread.currentThread().getName() + "  拿到锁");
                    TimeUnit.SECONDS.sleep(3);
                    System.out.println(Thread.currentThread().getName() + "  等待完毕");
                } catch (InterruptedException ex) {
                    ex.printStackTrace();
                } finally {
                    System.out.println(Thread.currentThread().getName() + "  释放锁");
                    lock.unlock();
                }
                System.out.println(Thread.currentThread().getName() + "  结束");
            }, "thread" + (e + 1)
            ).start();
        });
    }
}
```

**执行结果**

```java
--------ReentrantLock-----------
thread1  开始
thread2  开始
thread1  拿到锁
thread3  开始
thread4  开始
thread1  等待完毕
thread1  释放锁
thread1  结束
thread2  拿到锁
thread2  等待完毕
thread2  释放锁
thread2  结束
thread3  拿到锁
thread3  等待完毕
thread3  释放锁
thread3  结束
thread4  拿到锁
thread4  等待完毕
thread4  释放锁
thread4  结束
```

我们不难发现  lock , 当线程执行lock() 方法时, 只能有一个线程执行.

```java
1. Acquires the lock if it is not held by another thread and returns immediately, setting the lock hold count to one.
2. If the current thread already holds the lock then the hold count is incremented by one and the method returns immediately.
3. If the lock is held by another thread then the current thread becomes disabled for thread scheduling purposes and lies dormant until the lock has been acquired, at which time the lock hold count is set to one.    
```

源码中 lock 方法 有三种情况

1. 如果需求的锁没有被其他线程所持有,那么久立刻返回并且设置持有锁的数量为1
2. 如果当前线程已经持有锁,那么持有的数量加一,然后返回,  就是一个多次执行 lock 方法() 
3. 如果这个锁 被其他线程持有,那么当前线程则不能调度线程,然后休眠,止到有可用的锁,同时将锁的持有数量设置为1.

所以呢,只能有一个线程获取锁,达到同步执行的效果