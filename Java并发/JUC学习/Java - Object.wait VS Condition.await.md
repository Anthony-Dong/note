# Java - Object.wait VS Condition.await

​	有些人不知道这俩玩意有啥用, 其实大有用处, 对于有些同步操作, 需要使用大量的阻塞操作, 这个玩意可以很好的做到通知效果. 当你条件多时候, 你就可以使用 Condition了 . 

## Dubbo中使用Condition

我们拿个例子来举例子.  比如一个 Dubbo框架你想过没有. 客户端如何拿到返回值的 , 那么如何加入超时, 一系列的问题都在这 .  所以需要我们来思考 . 

我给大家展示一块代码 : `org.apache.dubbo.rpc.protocol.dubbo.DubboInvoker#doInvoke`

```java
@Override
protected Result doInvoke(final Invocation invocation) throws Throwable {
    RpcInvocation inv = (RpcInvocation) invocation;
    // 获取当前客户端
    ExchangeClient currentClient;
    try {
        // 一堆参数获取
        boolean isAsync = RpcUtils.isAsync(getUrl(), invocation);
        boolean isAsyncFuture = RpcUtils.isReturnTypeFuture(inv);
        boolean isOneway = RpcUtils.isOneway(getUrl(), invocation);
        int timeout = getUrl().getMethodParameter(methodName, Constants.TIMEOUT_KEY, Constants.DEFAULT_TIMEOUT);
        if (isOneway) {
			// 只执行一次
            return new RpcResult();
        } else if (isAsync) {
   			// 异步
            return result;
        } else {
            // 同步 , 默认其实是同步的
            RpcContext.getContext().setFuture(null);
            // 客户端.get()
            return (Result) currentClient.request(inv, timeout).get();
        }
    } catch (Exception e) {
		// .... 一堆异常 
    } 
}
```

我们继续往下走 : `org.apache.dubbo.remoting.exchange.support.DefaultFuture#get()`

```java
@Override
public Object get() throws RemotingException {
    return get(timeout);
}
// 获取结果
@Override
public Object get(int timeout) throws RemotingException {
	// 有没有结果
    if (!isDone()) {
        long start = System.currentTimeMillis();
        // 类似于sync obj.wait的写法
        lock.lock();
        try {
            // 核心地方. 就是这里. 不断的等待. 然后可以输入一个超时间,前提是你这个锁是没有其他线程拿到,这个超时时间才有效. 不然会一直等待到signal被唤醒,或者线程中断.
            while (!isDone()) {
                done.await(timeout, TimeUnit.MILLISECONDS);
                if (isDone() || System.currentTimeMillis() - start > timeout) {
                    break;
                }
            }
        } catch (InterruptedException e) {
            // 抛出异常
        } finally {
            lock.unlock();
        }
        if (!isDone()) {
           // 超时也是抛出异常
        }
    }
    return returnFromResponse();
}
```



那么现在简单了, 究竟是谁帮助他存入结果呢`org.apache.dubbo.remoting.exchange.support.DefaultFuture#doReceived`

```java
private void doReceived(Response res) {
    lock.lock();
    try {
        // 拿到结果
        response = res;
        if (done != null) {
            // 唤醒
            done.signal();
        }
    } finally {
        // 释放锁
        lock.unlock();
    }
    // 看看有木有回调.有回调就执行回调方法
    if (callback != null) {
        invokeCallback(callback);
    }
}
```

我们再往上走就到了 :   `org.apache.dubbo.remoting.transport.dispatcher.connection.ConnectionOrderedChannelHandler#received` 

由于Netty的worker线程有限, 而且worker线程,每个线程同步执行,比如一个客户端分配一个线程,那么这个线程可能就会因为处理消息而阻塞. 所以分配给其他线程池来执行. 他就可以只负责将消息转发给线程池执行了.

```java
executor.execute(new ChannelEventRunnable(channel, handler, ChannelState.RECEIVED, message));
```

其次就是 `org.apache.dubbo.remoting.transport.dispatcher.ChannelEventRunnable`  他实现了`java.lang.Runnable` 接口, , 所以调用就是靠这个. 

```java
@Override
public void run() {
    if (state == ChannelState.RECEIVED) {
        try {
            // 这里调用 `org.apache.dubbo.remoting.exchange.support.header.HeaderExchangeHandler`,类似以一种委派模型,其实也和包装模型也像, 一层层包装也属于一层层委派.
            handler.received(channel, message);
        } catch (Exception e) {
			// 异常
        }
    } else {
        switch (state) {
                /// 省略, 有兴趣的可以看看
        }
    }
}
```



基本流程就是这个. : 

当Reference对象调用一个RPC时 , 会被动态代理到执行`doiovoke`方法那去. 他会不断的等待.等到到拿去结果. (这里使用到了ReentrantLock+Condition) ,  此时服务器反馈回消息来了. 此时就会执行委派模型, 不断的分发给下一级 , 最后交给一个线程池处理 .然后再调用`DefaultFuture` , 帮助结果赋值,然后返回.   

但是这里有个问题, 我说过. 就是当一个锁被俩线程拿到. 那么 wait()的超时效果就没有了 . 形同虚设. 那么Dubbo采用了一个开启一个线程来处理这个事情. 将我们的`DefaultFuture`对象交给它  . 如果发现超时直接返回一个超时结果. 所以这些都是细节.  更多细节请看 `org.apache.dubbo.common.timer.HashedWheelTimer` , 他类似于一个表盘.每隔一段时间检查一次 . 有没有 `org.apache.dubbo.common.timer.TimerTask` 落入表盘. 如果有就执行了. 



## `Object.wait()` 和  `Object.notify()`

> ​	其中 Object.wait() 还有加入超时机制的. Object.notify() 还有一个 notifyAll(). 下面都会讲 , 你要理解了他的流程. 就明白了Condition了. 

我们来个简单的例子. 来体验一下 wait方法 .

```java
private static String rest = null;

public static void main(String[] args){
    Object lock = new Object();

    // 启动一个线程,等待,直到rest有结果.
    new Thread(() -> {
        synchronized (lock) {
            try {
                while (rest == null) {
                    // 这里会等待有人将他唤醒,当执行第一次执行wait方法时, 他会释放lock锁.代表其他线程可以拿到锁了
                    lock.wait();
                }
                System.out.println(String.format("res : %s",rest));
            } catch (InterruptedException e) {
                //
            }
        }
    }).start();

    // 启动一个线程, 来执行,然后告诉他结果.
    new Thread(() -> {
        synchronized (lock) {
            sleep(2000);
            rest = "辛苦你了,等了2S.";
            lock.notify();
        }
    }).start();
}
```

输出 : 

```java
res : 辛苦你了,等了2S.
```

通过上诉例子, 我们知道. 当调用了 `notify` 会释放一个等待线程. 那么我们再多一个等待线程呢.  我们将上面两个线程runnable封装一下. 省代码空间

```java
new Thread(waiter).start();
new Thread(waiter).start();
new Thread(notifier).start();
```

输出 : 

```java
thread : Thread-0, res : 辛苦你了,等了2S.
```

发现只有一个线程被唤醒. 其实这里也有先后顺序的. 

#### 执行流程

​		在执行`wait方法`之前，必须去争夺Synchorized锁,不然会抛出异常的 `IllegalMonitorStateException`，首先会执行JVM指令`monitorenter` ,如果当前线程获取到此对象(lock对象),那么lock对象的markword会记录该锁的线程持有者和计数器.同时在执行Lock.wait()的时候,此时已经是持有锁了. 此时就会执行释放锁操作 . 对于上诉那个例子来说,就是thread2可以执行了. 

​	  怎么说说这个wait线程去哪了 ? , 在JVM里, 有一个`_WaitSet ` 这个集合进行保存, 他是一个双链表结构(实际上是队列结构, 先进先出). 当一个线程执行wait时, 第一步将他封装成一个node节点, 类似于AQS, 然后将这个节点node添加到 waitset中, 然后调用`ObjectMonitor::exit` 释放当前的monitor对象(local对象).最终调用`park`将他挂起. 跟AQS如出一辙. 

​	 那么当有线程调用`notify`时(前提也是此线程得先拿到锁,就是加同步代码块了), 然后反向操作了, 他会将waitset中第一个节点拿出来. 然后将第一个节点封装的线程执行`unpark`操作. 并且将它移到 `EntrySet`中, 也就是等待线程的集合中. 当你退出`sync(obj){}` 同步代码块时(就会调用了`monitorexit`), 会释放当前持有的锁. 

此时刚刚等待的线程会停止等待 . 所以就是这么一个过程. 

​	那么说说 `notifyAll`  , 他的做法就是将 WaitSet集合全部遍历一遍全部释放. 

#### `Object.wait(long timeout)`

> ​	为什么我要单独拿出来, 超时有啥好多的 , 因为这个超时是假的(其实上面那段话很明白了) . 并不是超时了一定会超时.  我们看看例子. 

我们将原来的waiter改成一个加了超时的, 这个单位是ms, 我们再次执行. 

```java
Runnable waiter = () -> {
    synchronized (lock) {
        long start = System.currentTimeMillis();
        try {
            while (rest == null) {
                // 这里会等待有人将他唤醒,当执行第一次执行wait方法时, 他会释放lock锁.代表其他线程可以拿到锁了
                lock.wait(1000);
                System.out.println(String.format("thread : %s wait : %dms", Thread.currentThread().getName(), System.currentTimeMillis() - start));
            }
            System.out.println(String.format("thread : %s, res : %s , cost : %dms", Thread.currentThread().getName(), rest, System.currentTimeMillis() - start));
        } catch (InterruptedException e) {
            //
        }
    }
};
```

然后再次运行, 发现什么?  花费了

```java
thread : Thread-0 wait : 2002ms
thread : Thread-0, res : 辛苦你了,等了2S. , cost : 2016ms
```

所以结果是啥, 超时有用吗. 没有, 那么是为什么呢? 

#### 原因

因为当你执行 `lock.wait(1000)` 当你超时结束. 这里是JVM里的, JVM里你超时结束了. 那么首先你需要去拿到锁. 你拿不到锁. 会继续执行 `lock.wait(1000) `   ,AQS的源码和JVM思想基本一致 . 直到你拿到锁. 你才会wait结束, 所以也就是上面为啥 你的花费了2000ms, 而只等待了1000ms. 



## Condition 

其实condition跟 java的wait-notify思想是一模一样的. 我将其对应的方法关联一下. 源码也不是太难, 设计到大量的CAS同步操作, 一次park和unpack操作, 基本和JVM内部思想一致.  不过他的灵活性高点. (随着JVM的进步, 其实这种性能差距会不断缩小, 而且代码更加简便, JVM控制不容易出现问题. 所以我推荐没有特殊需求就用wait和notify足够了)

那么Condition怎么来呢 . 我们看看 Lock接口他有一个方法是 `Condition newCondition();` 

**对 , 一把锁可以new 很多个Condition. 这点是Java内置的wait-notify做不到的. 这个优点就是一把锁可以控制多个Condition, 不是一把锁一个Condition . 懂了吧.** 

其次是Condition有一个方法是 `awaitUninterruptibly` , 是不可中断的等待. 我们知道Java内置的wait-notify,wait方法可以被打断的. 

我们来拿BlockStack为例子.  是一个后进先出的数组 .  么办法懒得写队列, 因为一个变量便可以维护好栈.

```java
public class BlockStack {

    final Lock lock = new ReentrantLock();
    // 当满了.就阻塞
    final Condition full = lock.newCondition();

    // 当空了, 也阻塞.
    final Condition empty = lock.newCondition();

    final Object[] arr;

    public BlockStack(int size) {
        arr = new Object[size];
    }

    // 当前索引位置
    int count = 0;

    public void push(Object val) throws InterruptedException {
        // lock获取锁
        lock.lock();
        try {
            // 如果满了.就一直等待
            if (count == arr.length) {
                // await-> park()..(等别人unpark).. ->tryacq()获取锁-> 拿到锁成功->结束wait
                full.await();
            }
            arr[count] = val;
            count++;
            empty.signal();
        } finally {
            // unlock -> release(1)->释放锁
            lock.unlock();
        }
    }

    // 剔除栈顶
    public Object pop() throws InterruptedException {
        lock.lock();
        try {
            while (count == 0) {
                empty.await();
            }
            // 此时count已经+1了,所以我们拿不到刚刚插进去的
            Object obj = arr[--count];
            // 找到一个Condition队列的头节点->将await线程从condition队列放入到阻塞队列中(核心方法在AQS的eq方法中)->unpark(await线程)
            full.signal();
            return obj;
        } finally {
            lock.unlock();
        }
    }
}
```



我们来分析一下这个执行流程.  前提是T1 比 T2先执行

```java
public class AQS {

    public static void main(String[] args) {
        final Lock lock = new ReentrantLock();
        final Condition full = lock.newCondition();
        // t1线程
        new Thread(() -> {
            lock.lock();
            try {
                full.await();
            } catch (InterruptedException e) {
                //
            } finally {
                lock.unlock();
            }
        },"t1").start();
        // t2线程
         new Thread(() -> {
            lock.lock();
            try {
                // 
                full.signal();
                
            } finally {
                lock.unlock();
            }
       },"t2").start();
    }
}
```

假设T1拿锁的时间 优于 T2执行. 如果执行满了. 就会无脑的await下去/ 

park 操作是 当前线程将自己执行waiting , 你说难不难,看过科幻片没有就是自己冰封自己,无休止的睡眠下去. 跟sleep一样. 只是他没有指定时间. 这时候就需要一个人,就是另一个人,把他唤醒.就是unpark操作了..

---

`T1.Lock() `  ->` tryaq()` ->`t1拿到锁 ` ->`T1.FULL.await()` -> ....

---

`T2.Lock()`  -> `tryaq() `  -> 由于T1已经拿到锁了,自己只能长眠了 `  -> (睡哇睡.....)    ` -> `(TAG1) 将自己唤醒了`-> 继续操作, 也就是走到了 `T2.FULL.signal` .

---

`T1.FULL.await()` -> `将自己加入到从condition队列` -> `释放锁(TAG1)` ->  `自己park(),长眠下去`  -> (睡哇睡) -> `(TAG2)将自己唤醒 ` -> `第一件事检查自己是否被中端`   ->`(判断head是不是自己),再尝试去获取锁` -> `拿到锁自然就结束了`(中间过程unpark后 , 不一定完事大吉了,可能是被推到了后面,也就是继续park,具体实现在 `AQS.acquireQueued` )   ----> 真正拿到锁才会继续执行...... 

---

 `T2.FULL.signal` -> `找到condition队列中的第一个节点,移除(也就是T1线程)` ->  `将T1线程加入到阻塞队列中(核心方法在AQS的eq方法中,线程安全操作,所以不保证T1加入到等待队列的head部分)` , 执行完毕了就.

---

`T2.unlock ()` -> `release()` -> `释放锁(TAG2)`  执行完毕.

---

`T1.unlock`  结束完成. 



**拿锁的过程 : **

> ​	拿锁过程很简单, 状态量如果可以修改, 就成功拿锁, 将AQS的占用线程设置为当前线程. 
>
> ​	如果状态量修改失败. 加入AQS等待队列. 然后调用park操作, 将自己睡眠. 

**释放锁的流程 :**

> ​	注意这个全部操作是安全的, 也就是不需要CAS , 将AQS占用线程设置为NULL , 重置状态量 ,  然后去看等待队列, 看看里有没有人 , 有人简单, 直接把第一个人叫起来, (unpack那个人) , 就好了. 完成工作返回. 





> ​	总结 : AQS其实不难, 难的是思想, 如何安全的插入队列,在不使用锁的情况下, 如何实现公平 , 如何高效的维护队列. 这些都是难题.  当你懂了AQS 其实Condition并不难. 



## 注意

**注意一点就是  :  ** 就算是 `Object.wait()`和` Condition.await()`   操作他们都是需要拿到锁, 才能继续执行, 懂了吗, 加入超时也是, 都是需要再次获取锁, 这样保证了安全性.   所以这俩方式都需要操作中获取两次锁. 所以效率可能而知. 所以一般用于同步获取数据时用到. 

这里吐槽一下` FutureTask.get() 方法` , 竟然使用的是空转 ,这个玩意至少不比空转好吗. 