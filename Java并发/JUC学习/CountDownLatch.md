# JUC - CountDownLatch & CyclicBarrier  线程计数器

## `CountDownLatch `

> ​	CountDownLatch是一个同步工具类，它允许一个或多个线程一直等待，直到其他线程执行完后再执行。例如，应用程序的主线程希望在负责启动框架服务的线程已经启动所有框架服务之后执行。

其实这个玩意跟 `java.util.concurrent.Semaphore` 很相似 , 但是又不相似 , 他使用的是`AQS` , 一个多线程共享的状态量 .  如果你玩过 `Golang`的话, 和他的 `sync.waitgroup` 很相似近乎一模一样. 但是CountDownLatch少一个功能.

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



## 做一个线程状态管理器

现在我们对 X, Y 进行累加10万次, 但是我们不知道这俩线程啥时候结束. 

```java
private static int x = 0;
private static int y = 0;

public static void main(String[] args) throws InterruptedException {
    new Thread(() -> {
        while (x < 100000) {
            x++;
        }
    }).start();

    new Thread(() -> {
        while (y < 100000) {
            y++;
        }
    }).start();

    System.out.println(String.format("terminal  x : %d ,y : %d.", x, y));
}
```

我们就这么执行 , 会发现什么  输出可能每个人不一致,主线程是不会等待 , 那俩子线程执行完毕的, 所以需要. 

```java
terminal  x : 5190 ,y : 0.
```



但是我们加以修饰 . 

```java
private static int x = 0;
private static int y = 0;

public static void main(String[] args) throws InterruptedException {
    CountDownLatch BOOT = new CountDownLatch(2);

    new Thread(() -> {
        while (x < 100000) {
            x++;
        }
        BOOT.countDown();
    }).start();

    new Thread(() -> {
        while (y < 100000) {
            y++;
        }
        BOOT.countDown();
    }).start();

    BOOT.await();
    System.out.println(String.format("terminal  x : %d ,y : %d.", x, y));
}
```

输出呢 :  所以满足我们的预期. 

```java
terminal  x : 100000 ,y : 100000.
```



## `CyclicBarrier`

`Cyclic` 是循环的意思 , 意思就是循环呗.  `barrier` 是屏障的意思. 

构造器 : 

```java
CyclicBarrier barrier = new CyclicBarrier(2, new Runnable() {
            @Override
            public void run() {
               // todo
            }
        });
```

就俩参数, 第一个参数意思就是 `parties` 指的是成员数量.  第二个参数指的是成员数如果达到了2 , 就触发这个runnable . 

成员数 : 其实是指的 waiting中的线程,  所以他就是这个这意思. 当waiting中的线程到达了2 , 就触发runnable . 到了就触发 . 循环往复

```java
public class TestCyclicBarrier {

    public static void main(String[] args) {
        CyclicBarrier barrier = new CyclicBarrier(2, new Runnable() {
            @Override
            public void run() {
                System.out.println("你俩过去吧");
            }
        });

        IntStream.range(0, 4).forEach(e -> new Thread(() -> {
            try {
                ThreadLocalRandom random = ThreadLocalRandom.current();
                int x = random.nextInt(1000);
                TimeUnit.MILLISECONDS.sleep(random.nextInt(x));
                barrier.await();
                System.out.println(Thread.currentThread().getName() + " : 等待" + x + "ms");
            } catch (InterruptedException | BrokenBarrierException e1) {
                e1.printStackTrace();
            }
        }).start());
    }
}
```

输出 : 

```java
你俩过去吧
Thread-2 : 等待16ms
Thread-3 : 等待69ms
你俩过去吧
Thread-1 : 等待574ms
Thread-0 : 等待259ms
```



基本就这个用法,  `barrier.getNumberWaiting()` 返回当前等待中的线程数 , 那个reset最好别用/ 

