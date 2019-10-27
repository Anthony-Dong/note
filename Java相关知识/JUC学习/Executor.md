# JUC -- Executor

## 1. Executor

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-19/4089681c-020d-45c7-ba5d-3c9a16e04caf.png?x-oss-process=style/template01)

- Executor接口很简单，只有一个execute方法。  @since 1.5  java5开始有的
- ExecutorService是Executor的子接口，增加了一些常用的对线程的控制方法，之后使用线程池主要也是使用这些方法。
- AbstractExecutorService是一个抽象类。
- ThreadPoolExecutor就是实现了这个类。



**其中主要分为两大部分** 

- **Executors的几个实现方法**

```java
1. 创建固定大小的线程,可以延时或定时的执行任务;
ExecutorService executorService = newCachedThreadPool();

2. 创建单个线程池, 线程池中只有一个线程;
ExecutorService executorService1 = newSingleThreadExecutor();

3. 缓存线程池,线程池中线程的数量不固定,可以根据需求自动更改数量;
ExecutorService executorService2 = Executors.newFixedThreadPool(10);

4. 创建固定大小的线程,可以延时或定时的执行任务;
ScheduledExecutorService scheduledExecutorService = Executors.newScheduledThreadPool(10);


5. jdk 1.8 新加的 ,它是新的线程池类ForkJoinPool的扩展，但是都是在统一的一个Executors类中实现，由于能够合理的使用CPU进行对任务操作（并行操作），所以适合使用在很耗时的任务中;
ExecutorService newWorkStealingPool = Executors.newWorkStealingPool();   
    
```



- ThreadPoolExecutor

主要讲解的对象



## 2. ThreadPoolExecutor

### 1. 构造方法

```
/**
     * Creates a new {@code ThreadPoolExecutor} with the given initial
     * parameters.
     
    // 核心线程数
     * @param corePoolSize the number of threads to keep in the pool, even
     *        if they are idle(空闲), unless {@code allowCoreThreadTimeOut} is set
     
     // 总线程数(最大的)
     * @param maximumPoolSize the maximum number of threads to allow in the
     *        pool
     
     // 非corePool thread 的存活时间,到了 就回收了 
     * @param keepAliveTime when the number of threads is greater than
     *        the core, this is the maximum time that excess idle threads
     *        will wait for new tasks before terminating.
     
     * @param unit the time unit for the {@code keepAliveTime} argument
     
     // 工作队列,会保存任务,在他们被调用之前, 一般是 corePoolSize 满了以后就放入队列中了
     * @param workQueue the queue to use for holding tasks before they are
     *        executed.  This queue will hold only the {@code Runnable}
     *        tasks submitted by the {@code execute} method.
     
     // 创建线程的工厂
     * @param threadFactory the factory to use when the executor
     *        creates a new thread
     
     // 默认是  AbortPolicy() 模式 ,直接 throws a RejectedExecutionException 异常
     * @param handler the handler to use when execution is blocked
     *        because the thread bounds and queue capacities are reached
     * @throws IllegalArgumentException if one of the following holds:<br>
     *         {@code corePoolSize < 0}<br>
     *         {@code keepAliveTime < 0}<br>
     *         {@code maximumPoolSize <= 0}<br>
     *         {@code maximumPoolSize < corePoolSize}
     * @throws NullPointerException if {@code workQueue}
     *         or {@code threadFactory} or {@code handler} is null
     */   

	public ThreadPoolExecutor(int corePoolSize,
                              int maximumPoolSize,
                              long keepAliveTime,
                              TimeUnit unit,
                              BlockingQueue<Runnable> workQueue,
                              ThreadFactory threadFactory,
                              RejectedExecutionHandler handler) {}
```

这七个参数 需要根据具体的业务流量以及业务需求来选择

### 2. **RejectedExecutionHandler** 的四种实现 

1. **AbortPolicy**  默认的实现方式 ,当 超过最大线程数时 会抛出异常
2. **DiscardPolicy**   (Discard 丢弃的意思)  就是直接丢弃 超过的线程任务
3. **DiscardOldestPolicy**  当满了时,会丢弃队列中最老的那个线程任务
4. **CallerRunsPolicy**    当满了时,会调用其(in the calling thread of the {@code execute} method,) 调用execute() 方法的线程 来执行 超出的线程任务 ,

这四种根据具体业务选择



### 3. 判断流程

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-19/4ef6d91e-2c80-4a9e-9ebe-fd317184bb64.png?x-oss-process=style/template01)

**举个栗子**

```
// 实例化一个
ThreadPoolExecutor executor = new ThreadPoolExecutor(2, 3, 10L, TimeUnit.SECONDS, new ArrayBlockingQueue<>(2), new ThreadPoolExecutor.CallerRunsPolicy());

调用6次
        executor.execute(()->{
            try {
                TimeUnit.SECONDS.sleep(1);
                System.out.println(Thread.currentThread().getName()+"-- num");
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        });

1. 当执行时, 会把 1 2 任务放入 核心线程池
2. 此时 核心线程池 已经满了 >2 ,此时 把 3 4 放入到了队列中 
3. 由于队列也满了,于是创建新的线程(最大线程-核心线程),但是此时只能创建一个,因此 5 放入到了这个创建的线程中
4. 6任务 于是就只能调用 handler ,此时会执行CallerRunsPolicy()策略,由于是main线程调用的此方法 ,此时只能main线程执行 6任务

5. 结果就是
pool-1-thread-1-- 1
pool-1-thread-2-- 2
pool-1-thread-3-- 5
main-- 6
pool-1-thread-2-- 4
pool-1-thread-1-- 3
    
所以我们的分析是对的    
```

### 4. BlockingQueue 的五种实现队列

[可以参考我的这篇文章]([https://anthony-dong.github.io/post/ni-zhen-zheng-liao-jie-java-ji-he-ma/#4%E7%AE%80%E8%A6%81%E6%A6%82%E8%BF%B0blockingqueue%E5%B8%B8%E7%94%A8%E7%9A%84%E4%BA%94%E4%B8%AA%E5%AE%9E%E7%8E%B0%E7%B1%BB](https://anthony-dong.github.io/post/ni-zhen-zheng-liao-jie-java-ji-he-ma/#4简要概述blockingqueue常用的五个实现类))



## 3. ThreadPoolExecutor关闭的三种方式

[实现方式大家可以参考我的推荐链接](https://www.cnblogs.com/trust-freedom/p/6693601.html)

### 1. shutdown()方法

流程 : **更新线程池状态为shutdown**、**中断所有空闲线程**、**tryTerminated()尝试终止线程池**

将线程池状态置为`SHUTDOWN`,并不会立即停止：

- 停止接收外部submit的任务
- **内部正在跑的任务和队列里等待的任务，会执行完成**
- 等到第二步完成后，才真正停止
- **无返回值**



### 2. shutdownNow() 方法

流程 :  将线程池更新为stop状态 , 调用 interruptWorkers() 中断所有线程(包括正在运行的线程)，将workQueue中待处理的任务移到一个List中，并在方法最后返回

将线程池状态置为`STOP`。**企图**立即停止，事实上不一定：

- 跟shutdown()一样，先停止接收外部提交的任务
- **忽略队列里等待的任务**
- 尝试将正在跑的任务`interrupt`中断
- **返回未执行的任务列表**



### 3.  awaitTermination(long timeout, TimeUnit unit) 方法

**当前线程阻塞**，

然后返回**true（shutdown请求后所有任务执行完毕）**或**false（已超时）**



### 4. shutdown()和awaitTermination()的区别

- shutdown()后，不能再提交新的任务进去；但是awaitTermination()后，可以继续提交。
- awaitTermination()是阻塞的，返回结果是线程池是否已停止（true/false）；shutdown()不阻塞。
- awaitTermination()与shutdown()方法结合使用时，尤其要注意的是shutdown()方法必须要在awaitTermination()方法之前调用，该方法才会生效。否则会造成死锁。

```
		es.shutdown();  
		if(!es.awaitTermination(20,TimeUnit.SECONDS)){//20S
		   System.out.println(" 到达指定时间，还有线程没执行完，不再等待，关闭线程池!");
		   es.shutdownNow();  
		}

	// 这里为啥我不死锁.... 第三点我试着么啥事,大家可以试试,这个
```



### 4. 对比

- 优雅的关闭，用shutdown()
- 想立马关闭，并得到未执行任务列表，用shutdownNow()
- 优雅的关闭，并允许关闭声明后新任务能提交，用awaitTermination()



举个栗子

```
 private static ThreadPoolExecutor executor = new ThreadPoolExecutor(2, 4, 10L,  TimeUnit.SECONDS, new ArrayBlockingQueue<>(2), new ThreadPoolExecutor.CallerRunsPolicy());  

 public static void main(String[] args) throws InterruptedException {

        executor.execute(() -> new Task(1).run());
        executor.execute(() -> new Task(2).run());
        executor.execute(() -> new Task(3).run());
        executor.execute(() -> new Task(4).run());
        executor.execute(() -> new Task(5).run());
        executor.execute(() -> new Task(6).run());

		// 查看队列中的任务
        executor.getQueue().forEach(System.out::println);
		
     	// 设置2000L 其中不需要加 L 因为他是 long不是 Long类型
     	// 设置等待时间, 等待完毕还可以 继续执行其他任务
        executor.awaitTermination(2000L, TimeUnit.MILLISECONDS);


        executor.execute(() -> new Task(7).run());

    }


    private static class Task implements Runnable{
        private int num;

        public Task(int num) {
            this.num = num;
        }

        @Override
        public void run() {
            try {
                TimeUnit.SECONDS.sleep(1);
                System.out.println(Thread.currentThread().getName() + " ---- " + num);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
```



## 4. 提交方式 -  submit & execute

使用方式 :  

```java
public class TestThreadPoolExecutor {
    public static void main(String[] args) throws ExecutionException, InterruptedException {
        ThreadPoolExecutor executor = new ThreadPoolExecutor(10, 20, 240, TimeUnit.SECONDS, new ArrayBlockingQueue<>(1024),
                new ThreadPoolExecutor.CallerRunsPolicy());

        // 我一种 方式
        executor.execute(()->{
            System.out.println("new  runnable");

        });


        // 第二种 方式
        Future<String> over = executor.submit(new Runnable() {
            @Override
            public void run() {
                try {
                    TimeUnit.SECONDS.sleep(1);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }, "over");
        // 这个方法会阻塞




        // submit 方法
        Future<String> future = executor.submit(new Callable<String>() {
            @Override
            public String call() throws Exception {
                TimeUnit.SECONDS.sleep(2);
                return "HELLO WORLD , Future";
            }
        });


        FutureTask<String> task = new FutureTask<String>(new Callable<String>() {
            @Override
            public String call() throws Exception {
                TimeUnit.SECONDS.sleep(3);
                return  "HELLO WORLD , FutureTask";
            }
        });

        // execute 方法
        executor.execute(task);

        // get 方法 获取结果 
        System.out.println("task.get() = " + task.get());

        System.out.println("future.get() = " + future.get());

        System.out.println("over.get() = " + over.get());

        executor.shutdown();

    }
}
```

结果

```java
new  runnable
task.get() = HELLO WORLD , FutureTask
future.get() = HELLO WORLD , Future
over.get() = over
```



> ​	上面我们发现  submit 提交了 一个`Callable` 对象 ,  那么 `Callable` 和 `Runnable` 有啥区别 , 我们发现 一个携带了返回值 , 一个不携带 , 
>
> ​	那我们知道创建一个线程唯一的方法就是执行 `Runnable`  的实现类,然后执行 , , 所以 submit 实际上 做的就是 execute 我们看看源码就知道了, 最终执行的还是 去实现一个 `FutureTask`  ,然后执行 `execute()` 方法
>

```java
   public <T> Future<T> submit(Callable<T> task) {
        if (task == null) throw new NullPointerException();
       // FutureTask<V> 是 RunnableFuture的实现类 , 
        RunnableFuture<T> ftask = newTaskFor(task);
       // 执行 execute 
        execute(ftask);
        return ftask;
    }
```



> `RunnableFuture` 是什么 

```java
public interface RunnableFuture<V> extends Runnable, Future<V> {
    /**
     * Sets this Future to the result of its computation
     * unless it has been cancelled.
     */
    将这个Future 作为计算结果,直到他被取消,  
    所以他只是一个执行结果, 还是需要实现Runnable,开启一个线程
    void run();
}
```

> ​	我们看看 Runnable 
>
> `The <code>Runnable</code> interface should be implemented by any class whose instances are intended to be executed by a thread.`
>
> 说的大概就是  这个接口可以被任意一个类去实现,这个类被用来让一个线程去执行的 , 

> ​	我们再去看看 `Thread` 类
>
>  `A <i>thread</i> is a thread of execution in a program. The Java Virtual Machine allows an application to have multiple threads of  execution running concurrently.`
>
> 在一个程序中一个`thread`是一个执行线程,java虚拟机允许一个应用去拥有多个执行线程并发运行



> ​	所以创建一个线程 终究还是需要实现一个 `thread`  然后 start 一个 runnable任务

```java
start 方法 解释说 
Any new functionality added to this method in the future may have to also be added to the VM.\
```