# Hystrix - 两种隔离模式

> ​	hystrix 使用 基本可以看看 https://github.com/Netflix/Hystrix/wiki 这个, 很不错  



> ​	This property indicates which isolation strategy `HystrixCommand.run()` executes with, one of the following two choices:
>
> - `THREAD` — it executes on a separate thread and concurrent requests are limited by the number of threads in the thread-pool
> - `SEMAPHORE` — it executes on the calling thread and concurrent requests are limited by the semaphore count

## 1. THREAD (默认)

> ​	The default, and the recommended setting, is to run `HystrixCommand`s using thread isolation (`THREAD`) and `HystrixObservableCommand`s using semaphore isolation (`SEMAPHORE`).

线程隔离模式下, 每一个任务都会被放入一个线程池中执行, 然后多余的任务放入到队列中执行  , 默认队列大小为0 , 也就是线程池不够了直接失败 . 

对于这个是默认配置, 所以一开始就是这种属性 ,我们看一下 . 

```java
public class CommandUsingThreadIsolation extends HystrixCommand<String> {

    private int x;

    protected CommandUsingThreadIsolation(int x) {
        super(Setter
                .withGroupKey(HystrixCommandGroupKey.Factory.asKey("ExampleGroup"))
                .andThreadPoolKey(HystrixThreadPoolKey.Factory.asKey("example"))
                .andThreadPoolPropertiesDefaults(HystrixThreadPoolProperties.Setter().withCoreSize(5))
        );
        this.x = x;
    }
    @Override
    protected String run() throws Exception {
        return "SUCCESS : " + x;
    }

    @Override
    protected String getFallback() {
        return "FALL-BACK";
    }

    public static void main(String[] args) {
        IntStream.range(0, 10).forEach(e -> {
            new Thread(() -> {
                CommandUsingThreadIsolation commandUsingSemaphoreIsolation = new CommandUsingThreadIsolation(e);
                System.out.println(commandUsingSemaphoreIsolation.execute());
            }).start();
        });
    }
}
```

输出 : 

```java
FALL-BACK
FALL-BACK
FALL-BACK
FALL-BACK
FALL-BACK
SUCCESS : 7
SUCCESS : 4
SUCCESS : 2
SUCCESS : 0
SUCCESS : 1
```



## 2. SEMAPHORE 

信号量模式 , 其实很简单就是个计数器, 超了失败, 也就是返回false, 没有超就返回true.

怎么配置 : 

```java
public class CommandUsingSemaphoreIsolation extends HystrixCommand<String> {

    private final int id;

    public CommandUsingSemaphoreIsolation(int id) {
        super(Setter.withGroupKey(HystrixCommandGroupKey.Factory.asKey("ExampleGroup"))
                .andCommandPropertiesDefaults(HystrixCommandProperties.Setter() .withExecutionIsolationStrategy(HystrixCommandProperties.ExecutionIsolationStrategy.SEMAPHORE));
              // 启动SEMAPHORE模式
        this.id = id;
    }

    @Override
    protected String run() {
        return "ValueFromHashMap_" + id;
    }
}
```

我们再看看信号量 ,他是`com.netflix.hystrix.AbstractCommand.TryableSemaphore` 的内部类实现. 

```java
static interface TryableSemaphore {

    public abstract boolean tryAcquire();

    public abstract void release();

    public abstract int getNumberOfPermitsUsed();
}
```

我们发现很简单其实, 就是一个 获取, 释放, 获取成功,执行完毕就释放, 

所以代码

```java
if (s.tryAcquire()) {
try {
// do work that is protected by 's'
} finally {
s.release();
}
}
```

子类实现是基于CAS实现的.  也就是atomicInteger计数器实现的. 

```java
static class TryableSemaphoreActual implements TryableSemaphore {
    protected final HystrixProperty<Integer> numberOfPermits;
    private final AtomicInteger count = new AtomicInteger(0);

    public TryableSemaphoreActual(HystrixProperty<Integer> numberOfPermits) {
        this.numberOfPermits = numberOfPermits;
    }

    @Override
    public boolean tryAcquire() {
        int currentCount = count.incrementAndGet();
        if (currentCount > numberOfPermits.get()) {
            count.decrementAndGet();
            return false;
        } else {
            return true;
        }
    }

    @Override
    public void release() {
        count.decrementAndGet();
    }

    @Override
    public int getNumberOfPermitsUsed() {
        return count.get();
    }
}
```



## 3. 异常问题 

### 1. 第一种

`com.netflix.hystrix.exception.HystrixRuntimeException: CommandUsingThreadIsolation fallback execution rejected.`

其实这个问题, 很简单, 有些人会说, 为啥我超过了信号量 , 或者线程数不是要快速熔断吗, 那么为啥不执行fallback呢, 是因为执行fallback也是需要信号量 , 由于默认这个fallback信号量是10,所以出现大量的fallback,以至于拿不到信号量就会出错, 所以唯一解决途径就是增大这个信号量 . 

```java
protected CommandUsingThreadIsolation(int x) {
    super(Setter
//..
.andCommandPropertiesDefaults(HystrixCommandProperties.Setter().withFallbackEnabled(true).withFallbackIsolationSemaphoreMaxConcurrentRequests(100))//设置fallback的信号量大小为100
    );
    this.x = x;
}
```

### 2. 第二种

`com.netflix.hystrix.exception.HystrixRuntimeException: CommandUsingSemaphoreIsolation could not acquire a semaphore for execution and no fallback available.`

这个是因为没有fallback . 回调方法

所以加上就行了 . 

