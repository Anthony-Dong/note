# JUC - ScheduledThreadPoolExecutor

> ​	这个相比于Timer的功能是他多了个线程池, Timer是单个线程执行. 而他呢就是可以运行多个实例 . 懂了吧, 一个周期任务你用timer就行了 , 但是我可以运行多个周期任务  , 其实实例化多个timer就可以.  但是他们都有一个共同点就是同步, 第一个任务必须等待第二个任务执行完毕, 不管你的周期性. 

##  ScheduledExecutorService

其中 `java.util.concurrent.ScheduledThreadPoolExecutor` 实现了 `java.util.concurrent.ScheduledExecutorService` 接口 , 这是Java唯一实现的 , 其他还有很多. 

基本上就是三方法  :  一个是 `schedule` (有返回值, 无返回值)一个是 `scheduleAtFixedRate` ,一个是 `scheduleWithFixedDelay`就是这俩方法, 第一个延迟任务, 第二个周期任务 , 俩都支持回调. 

```java
ScheduledExecutorService service = Executors.newScheduledThreadPool(5);

ScheduledFuture<String> schedule = service.schedule(new Callable<String>() {
    @Override
    public String call() throws Exception {
        return "Hello ScheduledExecutorService";
    }
}, 10, TimeUnit.SECONDS);

// 阻塞方法 , 不推荐使用 , 所以不推荐有返回值的. 
String o = schedule.get();
```

我们看周期任务 :  scheduleAtFixedRate 和  scheduleWithFixedDelay的区别

#### `scheduleAtFixedRate`

`scheduleAtFixedRate ` 每隔500ms 执行一次. 

那么我们的时间轴就是   0  500 1000  1500 , 但是我们任务每个执行时间是1000ms , 所以.  1000  1500 2000 2500 . 会输出. 但是会吗? 

```java
public static void main(String[] args) {
    ScheduledExecutorService executor = new ScheduledThreadPoolExecutor(4);
    long start = System.currentTimeMillis();
    executor.scheduleAtFixedRate(() -> {
        try {
            TimeUnit.MILLISECONDS.sleep(1000);
        } catch (InterruptedException e) {
            //
        }
        System.out.println(String.format("thread : %s run task-1 spend : %dms.", Thread.currentThread().getName(), System.currentTimeMillis() - start));
    }, 0, 500, TimeUnit.MILLISECONDS);
}
```

我们查看输出结果是 : 1000 2000 3000 4000 . 是为什么呢, 因为他是同步关系 . 这个么办法. 改变不了. **第二个任务执行必须等待第一个任务执行完毕才可以.** 

```java
thread : pool-1-thread-1 run task-1 spend : 1059ms.
thread : pool-1-thread-1 run task-1 spend : 2079ms.
thread : pool-1-thread-2 run task-1 spend : 3079ms.
thread : pool-1-thread-1 run task-1 spend : 4080ms.
```

#### `scheduleWithFixedDelay`

他的意思很简单, 就是上一个任务完成, 我加一个延迟时间, 就可以了.  也是同步关系 必须等待上一个任务完成 . 

我们再看看 `scheduleAtFixedRate`  ,任务周期为1S,  每次任务耗时3S 

```java
public static void main(String[] args) {
    ScheduledExecutorService executor = new ScheduledThreadPoolExecutor(4);
    long start = System.currentTimeMillis();
    executor.scheduleWithFixedDelay(() -> {
        try {
            TimeUnit.MILLISECONDS.sleep(1000);
        } catch (InterruptedException e) {
            //
        }
        System.out.println(String.format("thread : %s run task-2 spend : %dms.", Thread.currentThread().getName(), System.currentTimeMillis() - start));
    }, 0, 500, TimeUnit.MILLISECONDS);
}
```

输出 , 

```java
thread : pool-1-thread-1 run task-2 spend : 1064ms.
thread : pool-1-thread-1 run task-2 spend : 2611ms.
thread : pool-1-thread-2 run task-2 spend : 4112ms.
thread : pool-1-thread-1 run task-2 spend : 5614ms.
```

我们发现, 第一个任务执行完毕是在 1000ms,然后延迟了500ms , 消费了1000ms执行, 所以第二次执行完事2500ms.

一次类推. 





所以对于  `ScheduledExecutorService` 不推荐使用的原因是 : `他是强同步的, 但是有些任务不需要同步, 他就无法使用. `    所以Quartz 呢, 他考虑到同步和非同步. 