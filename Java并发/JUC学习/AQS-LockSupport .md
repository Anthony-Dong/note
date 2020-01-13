# AQS-LockSupport 

> Basic thread blocking primitives for creating locks and other synchronization classes. 

`LockSupport `是创建锁和其他同步工具的基本阻塞工具.   锁代表什么, 对于线程来说就是阻塞呗, 一个线程没有拿到锁, 就相当于处于阻塞中, 那怎么阻塞, 就是这玩意.  这个就是AQS的核心. 虽然他自己实现了一个队列, 但是基本线程阻塞都是基于这个的. 

## 核心API

### park

加锁的一种方式 , 无脑的阻塞. 

```java
public static void park(Object blocker) {
    Thread t = Thread.currentThread();
    setBlocker(t, blocker);
    // 无脑的等待, 直到unpark(thread)
    UNSAFE.park(false, 0L);
    setBlocker(t, null);
}

public static void park() {
    //无脑的等待, 直到unpark(thread)
    UNSAFE.park(false, 0L);
}
```

其实这两种加锁的方式时一样的, 但是区别就是 , 一个多了个一步

其中这个偏移量是 `java.lang.Thread#parkBlocker` 这个对象的. 

```java
// 当前park的线程, parkBlocker对象,类似于sync(o).
private static void setBlocker(Thread t, Object arg) {
    // Even though volatile, hotspot doesn't need a write barrier here.
    UNSAFE.putObject(t, parkBlockerOffset, arg);
}
```

目的是为了什么 ? 我也不知道 .

还有几个其他的方法 , 比如等待多久哇. 就不展示, 因为那个和sleep么啥区别了.

### unpark

很简单就是将park中的线程解除, 不等待了. 

```java
public static void unpark(Thread thread) {
    if (thread != null)
        UNSAFE.unpark(thread);
}
```

所以用这个 `LockSupport ` 至少需要维护一个 thread的集合, 不然park后无法unpark.



## 简单实现一个互斥锁

用它来做一个先进先出互斥锁. 公平锁. `FIFOMutex` , 这个摘抄与`java.util.concurrent.locks.LockSupport`

```java
public class Block {

    // 标识
    private final AtomicBoolean locked = new AtomicBoolean(false);

    // 等待队列
    private final Queue<Thread> waiters = new ConcurrentLinkedQueue<Thread>();

    // 加锁
    public void lock() {
        boolean wasInterrupted = false;
        Thread current = Thread.currentThread();
        // 线程安全的
        waiters.add(current);

        // 头节点是当前线程,且lock为true ,则代表不需要阻塞.循环的目的就是防止用户执行unpark
        while (waiters.peek() != current || !locked.compareAndSet(false, true)) {
            // 执行到这里会阻塞/等待
            LockSupport.park(this);
            if (Thread.interrupted()) // ignore interrupts while waiting
                wasInterrupted = true;
        }

        // 移除头节点,因为当前线程不需要等待.
        waiters.remove();
        if (wasInterrupted)          // reassert interrupt status on exit
            current.interrupt();
    }

    // 解锁
    public void unlock() {
        // 设置,这里由于就一个线程操作, 所以不需要cas操作.
        locked.set(false);
        // unPark下一个等待的线程.
        LockSupport.unpark(waiters.peek());
    }
}
```



