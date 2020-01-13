# JUC - CountDownLatch多线程同步工具

> ​	CountDownLatch是一个同步工具类，它允许一个或多个线程一直等待，直到其他线程执行完后再执行。例如，应用程序的主线程希望在负责启动框架服务的线程已经启动所有框架服务之后执行。

其实这个玩意跟 `java.util.concurrent.Semaphore` 很相似 , 看情况使用

#### 构造方法 

`java.util.concurrent.CountDownLatch#CountDownLatch` 方法

参数count 可以理解为`countDown` 调用的次数  , 然后当次数够了, `await` 就可以停止等待了

```java
//the number of times {@link #countDown} must be invoked before threads can pass through {@link #await}
public CountDownLatch(int count) {
        if (count < 0) throw new IllegalArgumentException("count < 0");
        this.sync = new Sync(count);
}
```

他涉及到 `AQS` 相关的知识 , 对于简单的计数器, 其实还不是他的主要难点, 计数器就是一个统计的方式, 所以引入了 `java.util.concurrent.locks.AbstractQueuedSynchronizer`  , 这个AQS的概念 , 对于线程的竞争以及公平与否, 这个玩意很难的, 我就不做过多的解释了 . 

#### 核心方法

`CountDownLatch`就俩方法指的我们注意 , 一个是`await`, 一个是`countDown`  ,

`await `是等到 0 才能向下执行 (就是当前线程阻塞了) , `countDown`是每次减一

#### 简单使用一下 

```java
public static void main(String[] args) {
    // 1. 初始化
    final CountDownLatch lock = new CountDownLatch(1);

    // 2. 等待线程
    new Thread(() -> {
        System.out.println(Thread.currentThread().getName() + " : 开始等待 . ");
        long start = System.currentTimeMillis();
        try {
            // 等待
            lock.await();
        } catch (InterruptedException e) {
           //
        }
        System.out.println(Thread.currentThread().getName() + " : 我等待结束 , 耗时 : " + (System.currentTimeMillis() - start));
    }).start();

    // 3. 解锁线程
    new Thread(() -> {
        try {
            // 1000ms 以后再解锁
            TimeUnit.MILLISECONDS.sleep(1000);
        } catch (InterruptedException e) {
            //
        }

        // -1 操作
        lock.countDown();
        System.out.println(Thread.currentThread().getName() + " : 我给你解锁 . ");
    }).start();
}
```

输出

```java
Thread-0 : 开始等待 . 
Thread-0 : 我等待结束 , 耗时 : 1001
Thread-1 : 我给你解锁 . 
```



#### 使用模型

```java
public class TestDemo {

    public static void main(String[] args) throws InterruptedException {
        // 为何1把锁, 因为我们只需要一扇门就可以堵住线程执行
        CountDownLatch startSignal = new CountDownLatch(1);
        
        // 为何5把锁, 因为我们需要让5个Worker全部工作完,才能说全部工作完成
        CountDownLatch doneSignal = new CountDownLatch(5);

        // 依次创建并启动5个worker线程
        IntStream.range(0,5).forEach(e-> new Thread(new Worker(startSignal, doneSignal)).start());

        System.out.println("Driver is doing something...");
        System.out.println("Driver is Finished, start all workers ...");
        
        startSignal.countDown(); // Driver执行完毕，发出开始信号，使所有的worker线程开始执行
        
        doneSignal.await(); // 等待所有的worker线程执行结束
        
        System.out.println("Finished.");
    }


}

// 简单的接口
class Worker implements Runnable{
    private final CountDownLatch startSignal;
    private final CountDownLatch doneSignal;
    Worker(CountDownLatch startSignal, CountDownLatch doneSignal) {
        this.startSignal = startSignal;
        this.doneSignal = doneSignal;
    }
    public void run() {
        try {
            startSignal.await(); // 等待Driver线程执行完毕，获得开始信号
            System.out.println("Working now ..."+Thread.currentThread().getName());
            doneSignal.countDown(); // 当前worker执行完毕，释放一个完成信号
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
```

输出

```java
Driver is doing something...
Driver is Finished, start all workers ...
Working now ...Thread-1
Working now ...Thread-0
Working now ...Thread-2
Working now ...Thread-3
Working now ...Thread-4
Finished.
```



所以工具放在这 , 用不用由你, 别强用