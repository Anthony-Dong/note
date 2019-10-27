# Executor

## 1. 为什么要选择线程池

​	有时候，系统需要处理非常多的执行时间很短的请求，如果每一个请求都开启一个新线程的话，系统就要不断的进行线程的创建和销毁，有时花在创建和销毁线程上的时间会比线程真正执行的时间还长。而且当线程数量太多时，系统不一定能受得了。

​	使用线程池主要为了解决一下几个问题：(和连接池差不多)

- 通过重用线程池中的线程，来减少每个线程创建和销毁的性能开销。
- 对线程进行一些维护和管理，比如定时开始，周期执行，并发数控制等等。



## 2. Executor 是什么

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-19/8032be83-2c10-45d5-bc47-cb3e4262eab7.png?x-oss-process=style/template01)

Executor接口 ，只有一个execute方法。

ExecutorService是Executor的子接口，增加了一些常用的对线程的控制方法，之后使用线程池主要也是使用这些方法。

AbstractExecutorService是一个抽象类。

ThreadPoolExecutor就是实现了这个类。



## 3. ThreadPoolExecutor

### 1. 构造方法 

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-19/736d9e02-6b6d-417f-bda9-bd1aa0e65f4b.jpg?x-oss-process=style/template01)

​	我们看最多的那个 实现:

```java
    /**
     * Creates a new {@code ThreadPoolExecutor} with the given initial
     * parameters.
     *
     * @param corePoolSize the number of threads to keep in the pool, even
     *        if they are idle(闲置), unless {@code allowCoreThreadTimeOut} is set
     
     * @param maximumPoolSize the maximum number of threads to allow in the
     *        pool
     
     *  其实就是 maximumPoolSize 的最大空闲时间,超过这个时间就会被回收。
     * @param keepAliveTime when the number of threads is greater(great 比较级) than
     *        the core, this is the maximum(最长的) time that excess(超过) idle threads
     *        will wait for new tasks before terminating.
     
     * @param unit the time unit for the {@code keepAliveTime} argument
     
     * @param workQueue the queue to use for holding tasks before they are
     *        executed.  This queue will hold only the {@code Runnable}
     *        tasks submitted by the {@code execute} method.
     
     * @param threadFactory the factory to use when the executor
     *        creates a new thread
     //  默认的handler是  AbortPolicy();直接抛出异常
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

-  **RejectedExecutionHandler** 主要有四种策略

  - **DiscardOldestPolicy** 丢弃队列中最老的任务 ,尝试再次提交当前任务
  - **DiscardPolicy**  直接丢弃掉无法处理的线程
  - **AbortPolicy** 是 直接抛出异常 ,阻止系统正常运行
  - **CallerRunsPolicy**  不会丢弃, 在他被调用的线程中执行此任务 ,如果你是main方法调用就是 main线程执行(in the calling thread of the {@code execute} method)

  

- **BlockingQueue**

  [五种常见的blockingqueue](https://anthony-dong.github.io/post/ni-zhen-zheng-liao-jie-java-ji-he-ma/#4%E7%AE%80%E8%A6%81%E6%A6%82%E8%BF%B0blockingqueue%E5%B8%B8%E7%94%A8%E7%9A%84%E4%BA%94%E4%B8%AA%E5%AE%9E%E7%8E%B0%E7%B1%BB)



* **执行流程如下**

​	![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-19/e6bb8bab-5009-4187-a219-fdcdb165897b.png?x-oss-process=style/template01)

所以就是  corePoolSize 满了就存放到队列中, 队列中满了,就新建线程, 线程满了就抛出异常

threadSize  - (corePoolSize  + workQueue )  + (maximumPoolSize-corePoolSize) > 0 

```java
new ThreadPoolExecutor(2, 3, 10L, TimeUnit.SECONDS, new ArrayBlockingQueue<>(2), new ThreadPoolExecutor.CallerRunsPolicy());

执行 6个  executor.execute(()->{
            try {
                TimeUnit.SECONDS.sleep(1);
                System.out.println(Thread.currentThread().getName()+"-- 1");
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
         });

1. 首先 1 2个放入核心线程
2. 然后将 3 4 放入队列
3. 然后此时队列满了 5 6 无法放入, 此时新建线程, 此时 发现 空闲的线程 (maximumPoolSize-corePoolSize) 就 1个了 ,5就去执行那个新建的线程, 6就放入到了 main线程
所以 结果很显然 

pool-1-thread-2-- 2
pool-1-thread-1-- 1
main-- 6
pool-1-thread-3-- 5
pool-1-thread-2-- 3
pool-1-thread-1-- 4

```





### 2. 关闭方式

- **shutdown()**

