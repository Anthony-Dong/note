# AQS - ReentrantLock如何基于AQS实现

> ​	这个是著名的的Java并发包中同步阻塞队列

CLH锁也是一种基于链表的可扩展、高性能、公平的自旋锁，申请线程只在本地变量上自旋，它不断轮询前驱的状态，如果发现前驱释放了锁就结束自旋。网上有很多相关资料, 可以查阅一下. 

**AQS基本模型图 : ** 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/23f3c904-948e-4926-86d8-3a72e39508e5.webp)



> ​	下面我们就以大名鼎鼎的ReentrantLock为例子讲解AQS的实现. 

## ReentrantLock

> ​	`ReentrantLock`  厉害是因为他有一下特点 : `可重入` , `需要手动加锁、解锁(所以容易出现死锁)`   , `支持设置锁的超时时间(tryLock) `  , `支持公平/非公平锁`  , `可中断锁(可以interrupt)` . 



> ​	简单的使用一下可重入锁例子. 使用以一般如下. 

```java
public class Main {

    private static final ReentrantLock LOCK = new ReentrantLock();

    public static void main(String[] args) throws InterruptedException {
        // 线程一 绝对先执行
        new Thread(() -> {
            try {
                // 加锁
                LOCK.lock();
            } catch (Exception e) {
                // no
            } finally {
                // 释放锁
                LOCK.unlock();
            }
        },"t1").start();
    }
}
```

## lock

这里的 sync 继承与 `AbstractQueuedSynchronizer` 类 ,我们是非公平锁

`java.util.concurrent.locks.ReentrantLock#lock`

```java
public void lock() {
    sync.lock();
}
```

下一步到了 `java.util.concurrent.locks.ReentrantLock.NonfairSync#lock ` 这里

```java
final void lock() {
    // CAS设置状态变量成功,就设置当前线程为持有线程
    if (compareAndSetState(0, 1))
        setExclusiveOwnerThread(Thread.currentThread());
    else
        // 否者,尝试获取/可重入/其他线程
        acquire(1);
}
```

```java
compareAndSetState(0, 1)

protected final boolean compareAndSetState(int expect, int update) {
    // See below for intrinsics setup to support this
    return unsafe.compareAndSwapInt(this, stateOffset, expect, update);
}

stateOffset = unsafe.objectFieldOffset
                (AbstractQueuedSynchronizer.class.getDeclaredField("state"));

/**
 * The synchronization state.
*/
private volatile int state;                
```

就是将 state 以CAS的方式设置为1  , 然后将排它锁的拥有者设置为当前线程.    这里让大家知道的是 CAS操作本身就是线程安全的 . 关于CAS如何实现,不同的CPU架构实现方式不同, 对于开发者来说他只是一个接口, 我们直接使用就行了. 

然后就是第二个线程来加锁的问题

很显然需要执行 `acquire(1)`    , 到 `java.util.concurrent.locks.AbstractQueuedSynchronizer#acquire`

```java
public final void acquire(int arg) {
    // tryAcquire(arg) 成功, 则加锁成功, 否则继续执行下一步
    if (!tryAcquire(arg) &&
        acquireQueued(addWaiter(Node.EXCLUSIVE), arg))
        selfInterrupt();
}
```

`tryAcquire` ->  `java.util.concurrent.locks.ReentrantLock.Sync#nonfairTryAcquire`

```java
final boolean nonfairTryAcquire(int acquires) {
    // 当前线程
    final Thread current = Thread.currentThread();
    // 当前值
    int c = getState();
    if (c == 0) {
        if (compareAndSetState(0, acquires)) {
            setExclusiveOwnerThread(current);
            return true;
        }
    }
    // 如果是当前线程是当前的排他锁, 那么就+1 , 这就是可重入锁的原理 , 然后返回true.
    else if (current == getExclusiveOwnerThread()) {
        int nextc = c + acquires;
        if (nextc < 0) // overflow
            throw new Error("Maximum lock count exceeded");
        setState(nextc);
        return true;
    }
    return false;
}
```

很显然第二把锁加锁是失败的,

 `acquireQueued(addWaiter(Node.EXCLUSIVE), arg)` ->  `addWaiter(Node.EXCLUSIVE)`  -`java.util.concurrent.locks.AbstractQueuedSynchronizer#addWaiter`  加一个排它锁(互斥锁),   很简单其实就是将 , 头节点的下一个节点设置为当前等待线程,

```java
private Node addWaiter(Node mode) {
    // 新建一个节点,做入队操作
    Node node = new Node(Thread.currentThread(), mode);
    Node pred = tail;
    if (pred != null) {
        node.prev = pred;
        // 放入队尾, 这里是CAS线程安全,只会有一个尾true
        if (compareAndSetTail(pred, node)) {
            pred.next = node;
            return node;
        }
    }
    // 队列的初始化过程,或者如果入队失败了就使用自旋锁
    enq(node);
    // 返回节点, 此时绝对已经入队成功了.
    return node;
}
```

` enq(node)`->`java.util.concurrent.locks.AbstractQueuedSynchronizer#enq`这里主要是 一个自旋锁的过程, 基于CAS实现. 入队操作线程安全的.  

```java
private Node enq(final Node node) {
    // 自旋, 直到入队成功,其实就是放入队尾
    for (;;) {
        Node t = tail;
        if (t == null) { // Must initialize , 初始化, 因为单个线程获取锁是不会初始化的.
            if (compareAndSetHead(new Node()))
                tail = head;
        } else {
            node.prev = t;
            // CAS操作
            if (compareAndSetTail(t, node)) {
                t.next = node;
                return t;
            }
        }
    }
}
```

入队操作也很简单, 就是先让节点指向尾节点, 再让尾节点指向当前节点 , 然后将原来的尾节点指向当前节点, (双向链表,所以又pre和next)

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-01-13/4a7078e3-e6fb-4246-a858-b4699625e42b.jpg?x-oss-process=style/template01)

`acquireQueued(addWaiter(Node.EXCLUSIVE), arg)`   -> `java.util.concurrent.locks.AbstractQueuedSynchronizer#acquireQueued`

```java
final boolean acquireQueued(final Node node, int arg) {
    boolean failed = true;
    try {
        boolean interrupted = false;
        for (;;) {
            // 获取此节点的前置节点,
            final Node p = node.predecessor();
            // 其实目的是为了防止在这个执行过程中,A线程拿到了锁,但是他执行了unlock,此时B恰好到这里.
            if (p == head && tryAcquire(arg)) {
                setHead(node);
                p.next = null; // help GC
                failed = false;
                return interrupted;
            }
            // AfterFailedAcquire之后会执行这个.设置等待状态waitStatus.这里状态量可以看文档说明
            // 设置成功状态量就执行 park操作
            if (shouldParkAfterFailedAcquire(p, node) &&
                parkAndCheckInterrupt())
                interrupted = true;
        }
    } finally {
        if (failed)
            cancelAcquire(node);
    }
}
```

`java.util.concurrent.locks.AbstractQueuedSynchronizer#parkAndCheckInterrupt`

```java
private final boolean parkAndCheckInterrupt() {
    // 这个就是核心地方,被阻塞的线程都会在这里等待.
    LockSupport.park(this);
    return Thread.interrupted();
}
```



## unlock

其实这里的操作时线程安全, 解锁过程,为什么呢,第一是因为持有锁的线程都在等待中,只有一个解锁的人才能执行此操作, 所以不需要考虑多线程问题

`java.util.concurrent.locks.ReentrantLock#unlock` 

```java
public void unlock() {
    sync.release(1);
}
```

`java.util.concurrent.locks.AbstractQueuedSynchronizer#release`

```java
public final boolean release(int arg) {
    // 先去释放自己
    if (tryRelease(arg)) {
        // 头节点
        Node h = head;
        // 判断空
        if (h != null && h.waitStatus != 0)
            // 如果过头节不为空就unpark , 其实是下一个节点
            unparkSuccessor(h);
        return true;
    }
    return false;
}
```

`java.util.concurrent.locks.ReentrantLock.Sync#tryRelease`  -> 如果加锁多次就要解锁多次

```java
protected final boolean tryRelease(int releases) {
    // state减一
    int c = getState() - releases;
    // 是否是持有锁的线程
    if (Thread.currentThread() != getExclusiveOwnerThread())
        throw new IllegalMonitorStateException();
    boolean free = false;
    // 到0就代表释放成功,代表解锁成功
    if (c == 0) {
        free = true;
        setExclusiveOwnerThread(null);
    }
    //这是状态量
    setState(c);
    return free;
}
```

如果解锁成功  `java.util.concurrent.locks.AbstractQueuedSynchronizer#unparkSuccessor`

```java
private void unparkSuccessor(Node node) {
    // 这些都是一些状态量.所以无关紧要
    int ws = node.waitStatus;
    if (ws < 0)
        compareAndSetWaitStatus(node, ws, 0);
    // 下一个节点才是等待的线程,我也不清楚waitstatus的值是如何算的, 
    Node s = node.next;
    if (s == null || s.waitStatus > 0) {
        s = null;
        for (Node t = tail; t != null && t != node; t = t.prev)
            if (t.waitStatus <= 0)
                s = t;
    }
    if (s != null)
        // 这里就是真正的unpark操作,  到这里头部的park线程会unpark
        LockSupport.unpark(s.thread);
}
```



## 总结

总结一下, 他维护了一个线程队列 , 当前持有锁的线程为 排他状态或者

需要使用的API就那么几个

```java
public final void acquire(int arg) {
    if (!tryAcquire(arg) &&
        acquireQueued(addWaiter(Node.EXCLUSIVE), arg))
        selfInterrupt();
}
```



```java
public final boolean release(int arg) {
    if (tryRelease(arg)) {
        Node h = head;
        if (h != null && h.waitStatus != 0)
            unparkSuccessor(h);
        return true;
    }
    return false;
}
```



对于开发者需要实现这俩未实现的方法, 

```java
protected boolean tryRelease(int arg) {
    throw new UnsupportedOperationException();
}
```



```java
protected boolean tryRelease(int arg) {
    throw new UnsupportedOperationException();
}
```



这里简单实现了一个非公平的实现  -- > 抄的 reentranlock的

```java
public class AsyncQ extends AbstractQueuedSynchronizer {

    @Override
    protected boolean tryAcquire(int arg) {
        // 当前线程
        final Thread current = Thread.currentThread();
        // 状态量
        int c = getState();
        if (c == 0) {
            // 0 就设置为 1 ,成功就设置当前线程为ExclusiveOwnerThread,这里是线程安全的
            if (compareAndSetState(0, 1)) {
                setExclusiveOwnerThread(current);
                return true;
            }
            // 可重入
        } else if (current == getExclusiveOwnerThread()) {
            int nextc = c + 1;
            if (nextc < 0) // overflow
                throw new Error("Maximum lock count exceeded");
            setState(nextc);
            return true;
        }
        // 失败返回false
        return false;
    }


    @Override
    protected boolean tryRelease(int arg) {
        // 状态量
        int c = getState() - 1;
        // 不是持有线程, 绝对有问题, 所以直接异常
        if (Thread.currentThread() != getExclusiveOwnerThread())
            throw new IllegalMonitorStateException();
        // 如果状态是0就释放
        boolean free = false;
        if (c == 0) {
            free = true;
            setExclusiveOwnerThread(null);
        }
        setState(c);
        return free;
    }


    public static void main(String[] args) throws InterruptedException {
        AsyncQ asyncQ = new AsyncQ();
        Thread t1 = new Thread(() -> {
            // 加锁
            asyncQ.acquire(1);
 			
            // 解锁
            asyncQ.release(1);
        }, "t1");
        t1.start();
    }
}
```

