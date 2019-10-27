# JUC - Exchange

> ​	 A synchronization point at which threads can pair and swap elements within pairs.

​	JUC中的Exchanger允许**成对的**线程在指定的同步点上通过`exchange`方法来交换数据。如果第一个线程先执行`exchange`方法，它会一直等待第二个线程也 执行`exchange`方法，当两个线程都到达同步点时，这两个线程就可以交换数据，将当前线程生产 出来的数据传递给对方。

## 使用 Exchange  

### 当两个线程执行的时候

##### **实现代码 :** 

```java
 public class TestExchange {

    public static void main(String[] args) {
        final Exchanger<String> exchanger = new Exchanger<>();

        new Thread(() -> {
            System.out.println("thread1开始");
            try {
                String exchange = exchanger.exchange("来自thread1的数据");
                System.out.println("thread1接收到的数据：" + exchange);
            } catch (InterruptedException e) {
                System.out.println("thread1 等不及了 , 不想交换数据");
            }
            System.out.println("thread1结束");
        }, "thread1").start();

        new Thread(() -> {
            System.out.println("thread2开始");
            try {
                String exchange = exchanger.exchange("来自thread2的数据");
                System.out.println("thread2接收到的数据：" + exchange);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            System.out.println("thread2结束");
        }, "thread2").start();
        
    }
}

```

##### **程序的输出结果**

```java
thread1开始
thread2开始
thread2接收到的数据：来自thread1的数据
thread2结束
thread1接收到的数据：来自thread2的数据
thread1结束
```

很显然我们发现 线程一和线程二成功的交换了数据,那么想到一个问题,如果插入第三个线程会怎么办,那么我们继续

### 当第三个线程加入会怎么办

###### **实现代码**

```java
public class TestExchange {
    
    public static void main(String[] args) {
        final Exchanger<String> exchanger = new Exchanger<>();
        new Thread(() -> {
            System.out.println("thread1开始");
            try {
                String exchange = exchanger.exchange("来自thread1的数据");
                System.out.println("thread1接收到的数据：" + exchange);
            } catch (InterruptedException e) {
                System.out.println("thread1 等不及了 , 不想交换数据");
            }
            System.out.println("thread1结束");
        }, "thread1").start();

        new Thread(() -> {
            System.out.println("thread2开始");
            try {
                String exchange = exchanger.exchange("来自thread2的数据");
                System.out.println("thread2接收到的数据：" + exchange);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            System.out.println("thread2结束");
        }, "thread2").start();

        new Thread(() -> {
            System.out.println("thread3开始");
            try {
                String exchange = exchanger.exchange("来自thread3的数据");
                System.out.println("thread3接收到的数据：" + exchange);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            System.out.println("thread3结束");
        }, "thread3").start();
    }
}

```

##### **程序输出结果**

```java
thread1开始
thread2开始
thread3开始
thread3接收到的数据：来自thread1的数据
thread3结束
thread1接收到的数据：来自thread3的数据
thread1结束

```

​		我们发现 线程2没有结束,而是处于一种阻塞的状态,那么结果表明: 如果一个线程决定交换数据,只有等到另外一个线程接收才能结束 ,但是这么做有个不好处就线程一直处于等待状态, 而且交换数据时随机匹配的,可能是一种竞争关系,谁最先抢到就能交换了

#### 引入  timeout 

```java
public class TestExchange {

    public static void main(String[] args) {
        final Exchanger<String> exchanger = new Exchanger<>();
        new Thread(() -> {
            System.out.println("thread1开始");
            try {
                String exchange = exchanger.exchange("来自thread1的数据",5, TimeUnit.SECONDS);
                System.out.println("thread1接收到的数据：" + exchange);
            } catch (InterruptedException | TimeoutException e) {
                System.out.println("thread1 等不及了 , 不想交换数据");
                e.printStackTrace();
            }
            System.out.println("thread1结束");
        }, "thread1").start();

        new Thread(() -> {
            System.out.println("thread2开始");
            try {
                TimeUnit.SECONDS.sleep(10);
                String exchange = exchanger.exchange("来自thread2的数据");
                System.out.println("thread2接收到的数据：" + exchange);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            System.out.println("thread2结束");
        }, "thread2").start();

        new Thread(() -> {
            System.out.println("thread3开始");
            try {
                TimeUnit.SECONDS.sleep(10);
                String exchange = exchanger.exchange("来自thread3的数据");
                System.out.println("thread3接收到的数据：" + exchange);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            System.out.println("thread3结束");
        }, "thread3").start();
    }
}

```

##### **结果** 

```java
thread1开始
thread2开始
thread3开始
thread1 等不及了 , 不想交换数据
java.util.concurrent.TimeoutException
	at java.util.concurrent.Exchanger.exchange(Exchanger.java:626)
	at com.exchange.TestExchange.lambda$main$0(TestExchange.java:21)
	at java.lang.Thread.run(Thread.java:748)
thread1结束
thread2接收到的数据：来自thread3的数据
thread2结束
thread3接收到的数据：来自thread2的数据
thread3结束
```

​		我们发现`thread1`到了超时时间他就自动抛出 `TimeoutException `异常 ,此线程就结束了 ,

