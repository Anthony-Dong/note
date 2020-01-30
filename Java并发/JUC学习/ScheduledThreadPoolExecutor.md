# ScheduledExecutorService

##  ScheduledExecutorService

其中 `java.util.concurrent.ScheduledThreadPoolExecutor` 实现了 `java.util.concurrent.ScheduledExecutorService` 接口

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

`scheduleWithFixedDelay` 我们任务每次执行耗时3S, 任务之间延迟1S

```java
ScheduledExecutorService service = Executors.newScheduledThreadPool(5);
long start = System.currentTimeMillis();
service.scheduleWithFixedDelay(() -> {
    System.out.println(Thread.currentThread().getName() + " , time : " + (System.currentTimeMillis() - start));
    sleep(3000);
}, 0, 1, TimeUnit.SECONDS);
```

所以输出   , 每个任务间隔 4S  = 任务完成+ 任务延时

```java
pool-1-thread-1 , time : 62
pool-1-thread-1 , time : 4063
pool-1-thread-2 , time : 8071
```

我们再看看 `scheduleAtFixedRate`  ,任务周期为1S,  每次任务耗时3S 

```java
service.scheduleAtFixedRate(() -> {
    System.out.println(Thread.currentThread().getName() + " , time : " + (System.currentTimeMillis() - start));
    sleep(3000);
}, 0, 1, TimeUnit.SECONDS);
```

输出 , 所以每个任务间隔 3S = 任务完成时间 > 周期 ? 任务完成时间 : 周期

```java
pool-1-thread-1 , time : 63
pool-1-thread-1 , time : 3064
pool-1-thread-2 , time : 6064
pool-1-thread-1 , time : 9065
```

所以对于  `ScheduledExecutorService` 不推荐使用的原因 是任务如果周期长, 根本做不得到周期性, 第一个任务完成才能执行第二个任务 .  是同步执行的. 





对于 ScheduledExecutorService 他的任务执行是同步执行的 .  所以很不好, 需要使用任务调度框架 quartz  , 看我看起讲解  .................