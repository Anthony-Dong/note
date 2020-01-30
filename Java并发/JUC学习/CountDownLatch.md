# JUC - CountDownLatch  启动器

> ​	CountDownLatch是一个同步工具类，它允许一个或多个线程一直等待，直到其他线程执行完后再执行。例如，应用程序的主线程希望在负责启动框架服务的线程已经启动所有框架服务之后执行。

其实这个玩意跟 `java.util.concurrent.Semaphore` 很相似 , 但是又不相似 , 他使用的是`AQS` , 一个多线程共享的状态量 . 

#### 构造方法 

`java.util.concurrent.CountDownLatch#CountDownLatch` 方法

参数count 就是一个计数器 .  后续操作都是基于这个计数器的 . 

```java
public CountDownLatch(int count) {
        if (count < 0) throw new IllegalArgumentException("count < 0");
        this.sync = new Sync(count);
}
```

#### 核心方法

`CountDownLatch`就俩方法指的我们注意 , 一个是`await`, 一个是`countDown`  ,

`await `是等到计数器为0时候才能向下执行 (就是当前线程阻塞了) , `countDown`是每次减一

- 等待计数器到0 才能停止等待

```java
public void await() throws InterruptedException{...}
```

- 等待超时, 如果还没有计数器到0 , 就自动放弃等待. 

```java
public boolean await(long timeout, TimeUnit unit){...}
```

- 计数器减一操作

```java
public void countDown() {...}
```



## 为啥叫他为启动器 ? 

### 例子一

> ​	简单来个例子 , 我们现在有一个任务, 需要等待初始化完成 , 才能继续启动 .  这时候就是需要启动器

```java
public class TestCountdownLatch {
    public static void main(String[] args) throws InterruptedException {
        CountDownLatch Launcher = new CountDownLatch(1);
        long start = System.currentTimeMillis();
        new Thread(() -> {
            try {
                Launcher.await();
            } catch (InterruptedException e) {
                //
            }
            System.out.println("启动成功 : " + (System.currentTimeMillis() - start));
        }).start();

        TimeUnit.SECONDS.sleep(2);
        Launcher.countDown();
    }
}
```

输出

```java
启动成功 : 2064
```

### 例子二

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
