# Deferred Result

## Purpose

详细介绍请看[Spring DeferredResult](https://docs.spring.io/spring/docs/current/javadoc-api/org/springframework/web/context/request/async/DeferredResult.html)  如何实现异步化，从而大大增加长连接数量. 

> ​	`DeferredResult` provides an alternative to using a [`Callable`](https://docs.oracle.com/javase/8/docs/api/java/util/concurrent/Callable.html?is-external=true) for asynchronous request processing. While a `Callable` is executed concurrently on `behalf of`(代表) the application, with a `DeferredResult` the application can produce the result from a thread of its choice.
>
> `DeferredResult `提供了一种使用`Callable`接口的方式来实现异步请求  . 虽然`Callable` 是代表应用程序并发执行的，但是使用`DeferredResult application `可以得到结果从它执行的线程中。



## Prepare to using the DeferredResult application

### // 我目前捕捉不到 他的 `onError() `事件 

```java
@Slf4j
@CrossOrigin
@RestController
@SpringBootApplication
public class DeferredresultSpringmvcApplication {

    public static void main(String[] args) {
        SpringApplication.run(DeferredresultSpringmvcApplication.class, args);
    }

    private static ExecutorService executors = Executors.newFixedThreadPool(10);


    @RequestMapping("/get0")
    public DeferredResult<HashMap<String, Object>> getData() {
        log.info("controller  调用的 thread : {}",Thread.currentThread().getName());

        log.info("controller 开始执行 ");


        // 设置超时时间 ,其实只有异步有效 我测试了 ,同步根本不会执行
        DeferredResult<HashMap<String, Object>> result = new DeferredResult<>(1000L);


        result.setResult(getMap());

        //如果不是线程执行 , 则返回true
        executors.execute(new Runnable() {
            @Override
            public void run() {

                // 结果是处理是否成功  有返回结果直接返回 success , 如果 异步请求已过期(必须是异步处理 才会生效) 返回false
                boolean isok = false;

                isok = result.setResult(getMap());

                log.info("DeferredResult  处理是否成功 : {}", isok);


                // 成功以后的处理结果 (必须 放到 调用的 Runnable接口里)
                result.setResultHandler(new DeferredResult.DeferredResultHandler() {
                    @Override
                    public void handleResult(Object result) {
                        log.info("DeferredResult  处理结果 : {} ",result.toString());
                    }
                });

            }
        });



        // 我这里有问题 // 我调用线程异常全部都是在 超时里面
        // 处理异常 , 我捕捉不到这个 异常 ,不知道 哪个地方的 , 其实可以自定义异常 ,然后 根据异常类型进行判断
        result.onError((e) -> {
            log.info("onError");
        });


        // 完成执行 , 处理结果不管是啥 都会执行
        result.onCompletion(new Runnable() {
            @Override
            public void run() {
                log.info("处理  onCompletion");
            }
        });


        // 处理超时的处理 , 只有异步结果才会处理 
        result.onTimeout(new Runnable() {
            @Override
            public void run() {
                log.info("处理超时  onTimeout");
            }
        });

        log.info("controller 执行完毕 ");
        // 记住 只能返回 result , 不能返回null
        return result;
    }


    /**
     * 不用线程池执行
     * @return
     */
    @RequestMapping("/get1")
    public DeferredResult<HashMap<String, Object>> getByNoExecutor(){
        log.info("controller  调用的 thread : {}",Thread.currentThread().getName());
        log.info("controller 开始执行 ");

        DeferredResult<HashMap<String, Object>> result = new DeferredResult<>(3000L);

        result.setResult(getMap());

        log.info("controller 执行完毕 ");

        return result;
    }


    /**
     * 用线程池执行
     * @return
     */
    @RequestMapping("/get2")
    public DeferredResult<HashMap<String, Object>> getByExecutor(){
        log.info("controller  调用的 thread : {}",Thread.currentThread().getName());
        log.info("controller 开始执行 ");

        DeferredResult<HashMap<String, Object>> result = new DeferredResult<>(3000L);

        executors.execute(new Runnable() {
            @Override
            public void run() {
                result.setResult(getMap());
            }
        });

        log.info("controller 执行完毕 ");

        return result;
    }


 

    /**
     * 模拟service 服务
     * @return
     */
    public HashMap<String, Object> getMap(){
        log.info("service 开始执行 ");
        // 模拟 service 延迟服务
        HashMap<String, Object> map = new HashMap<>();
        try {
            Thread.sleep(2000);

//            int i = 1/0;

            map.put("key",Thread.currentThread().getName());
            log.info("service 服务调用的线程 : {} ",Thread.currentThread().getName());
        } catch (Exception e) {
            throw new RuntimeException();
        }

        log.info("service 执行完毕 ");
        return map;
    }
}

```

我们看打印结果 发现

```java
1. get1 执行结果

2019-10-12 12:44:17.549  INFO 2020 --- [nio-8080-exec-2] c.s.DeferredresultSpringmvcApplication   : controller  调用的 thread : http-nio-8080-exec-2
2019-10-12 12:44:17.549  INFO 2020 --- [nio-8080-exec-2] c.s.DeferredresultSpringmvcApplication   : controller 开始执行 
2019-10-12 12:44:17.549  INFO 2020 --- [nio-8080-exec-2] c.s.DeferredresultSpringmvcApplication   : service 开始执行 
2019-10-12 12:44:19.549  INFO 2020 --- [nio-8080-exec-2] c.s.DeferredresultSpringmvcApplication   : service 服务调用的线程 : http-nio-8080-exec-2 
2019-10-12 12:44:19.549  INFO 2020 --- [nio-8080-exec-2] c.s.DeferredresultSpringmvcApplication   : service 执行完毕 
2019-10-12 12:44:19.549  INFO 2020 --- [nio-8080-exec-2] c.s.DeferredresultSpringmvcApplication   : controller 执行完毕 

2. get2执行结果
2019-10-12 12:44:47.993  INFO 2020 --- [nio-8080-exec-4] c.s.DeferredresultSpringmvcApplication   : controller  调用的 thread : http-nio-8080-exec-4
2019-10-12 12:44:47.993  INFO 2020 --- [nio-8080-exec-4] c.s.DeferredresultSpringmvcApplication   : controller 开始执行 
2019-10-12 12:44:47.994  INFO 2020 --- [nio-8080-exec-4] c.s.DeferredresultSpringmvcApplication   : controller 执行完毕 
2019-10-12 12:44:47.997  INFO 2020 --- [pool-1-thread-1] c.s.DeferredresultSpringmvcApplication   : service 开始执行 
2019-10-12 12:44:49.999  INFO 2020 --- [pool-1-thread-1] c.s.DeferredresultSpringmvcApplication   : service 服务调用的线程 : pool-1-thread-1 
2019-10-12 12:44:49.999  INFO 2020 --- [pool-1-thread-1] c.s.DeferredresultSpringmvcApplication   : service 执行完毕 
```



### 那我们如何自己实现一个呢 ?

```java
  /**
     * 自定义简单使用
     * @return
     */
    @RequestMapping("/get3")
    public MyCallable<HashMap<String, Object>> get(){
        log.info("controller  调用的 thread : {}",Thread.currentThread().getName());
        log.info("controller 开始执行 ");

        MyCallable<HashMap<String, Object>> objectMyCallable = new MyCallable<>();

        log.info("controller 执行完毕 ");

        return objectMyCallable;
    }



    /**
     * 简单的执行逻辑
     * @param <T>
     */
    private class MyCallable<T> implements Callable<T> {
        @Override
        public T call() throws Exception {
            return (T)getMap();
        }
    }

    /**
     * 模拟service 服务
     * @return
     */
    public HashMap<String, Object> getMap(){
        log.info("service 开始执行 ");
        // 模拟 service 延迟服务
        HashMap<String, Object> map = new HashMap<>();
        try {
            Thread.sleep(2000);

//            int i = 1/0;

            map.put("key",Thread.currentThread().getName());
            log.info("service 服务调用的线程 : {} ",Thread.currentThread().getName());
        } catch (Exception e) {
            throw new RuntimeException();
        }

        log.info("service 执行完毕 ");
        return map;
    }
```

我们看一下我们实现的 执行结果

```java
2019-10-12 12:42:23.869  INFO 2020 --- [nio-8080-exec-5] c.s.DeferredresultSpringmvcApplication   : controller  调用的 thread : http-nio-8080-exec-5
2019-10-12 12:42:23.870  INFO 2020 --- [nio-8080-exec-5] c.s.DeferredresultSpringmvcApplication   : controller 开始执行 
2019-10-12 12:42:23.872  INFO 2020 --- [nio-8080-exec-5] c.s.DeferredresultSpringmvcApplication   : controller 执行完毕 
2019-10-12 12:42:23.875  INFO 2020 --- [         task-1] c.s.DeferredresultSpringmvcApplication   : service 开始执行 
2019-10-12 12:42:25.875  INFO 2020 --- [         task-1] c.s.DeferredresultSpringmvcApplication   : service 服务调用的线程 : task-1 
2019-10-12 12:42:25.875  INFO 2020 --- [         task-1] c.s.DeferredresultSpringmvcApplication   : service 执行完毕 
```



这里的task 线程是谁呢 ? 

**ThreadPoolTaskExecutor**  是 springboot自带的默认的 异步线程池 , 我们可以自己设置 

```java
@Configuration
public class AsyncPoolConfig {

    @Bean
    public ThreadPoolTaskExecutor asyncThreadPoolTaskExecutor(){
        ThreadPoolTaskExecutor executor = new ThreadPoolTaskExecutor();
        executor.setCorePoolSize(20);
        executor.setMaxPoolSize(200);
        executor.setQueueCapacity(25);
        executor.setKeepAliveSeconds(200);
        executor.setThreadNamePrefix("asyncThread");
        executor.setWaitForTasksToCompleteOnShutdown(true);
        executor.setAwaitTerminationSeconds(60);

        executor.setRejectedExecutionHandler(new ThreadPoolExecutor.CallerRunsPolicy());

        executor.initialize();
        return executor;
    }
}
```





### // 复杂的呢  

```java
    /**
     * 自定义灵活性 增加代码的可拓展性,加入线程池
     * @return
     */
    @RequestMapping("/get4")
    public MyCallable2<HashMap<String, Object>> get2(){

        log.info("controller  调用的 thread : {}",Thread.currentThread().getName());


        log.info("controller 开始执行 ");

        MyCallable2<HashMap<String, Object>> objectMyCallable = new MyCallable2<>(new Callable<HashMap<String, Object>>() {
            @Override
            public HashMap<String, Object> call() throws Exception {
                return getMap();
            }
        });

        log.info("controller 执行完毕 ");

        return objectMyCallable;
    }



    /**
     * 这里 其实 就是 一个 deferred result
     * @param <T>
     */
    private class MyCallable2<T> implements Callable<T> {

        private  ExecutorService executors = Executors.newFixedThreadPool(10);

        private Callable<T> callable;

        public MyCallable2(Callable<T> callable) {
            this.callable = callable;
        }

        // 让一个新的线程执行 call方法
        @Override
        public T call() throws Exception {
            Future<T> submit = executors.submit(callable);
            return submit.get();
        }
    }

    /**
     * 模拟service 服务
     * @return
     */
    public HashMap<String, Object> getMap(){
        log.info("service 开始执行 ");
        // 模拟 service 延迟服务
        HashMap<String, Object> map = new HashMap<>();
        try {
            Thread.sleep(2000);

//            int i = 1/0;

            map.put("key",Thread.currentThread().getName());
            log.info("service 服务调用的线程 : {} ",Thread.currentThread().getName());
        } catch (Exception e) {
            throw new RuntimeException();
        }

        log.info("service 执行完毕 ");
        return map;
    }
```

执行结果

```java
2019-10-12 12:42:44.879  INFO 2020 --- [nio-8080-exec-8] c.s.DeferredresultSpringmvcApplication   : controller  调用的 thread : http-nio-8080-exec-8
2019-10-12 12:42:44.880  INFO 2020 --- [nio-8080-exec-8] c.s.DeferredresultSpringmvcApplication   : controller 开始执行 
2019-10-12 12:42:44.882  INFO 2020 --- [nio-8080-exec-8] c.s.DeferredresultSpringmvcApplication   : controller 执行完毕 
2019-10-12 12:42:44.883  INFO 2020 --- [pool-2-thread-1] c.s.DeferredresultSpringmvcApplication   : service 开始执行 
2019-10-12 12:42:46.884  INFO 2020 --- [pool-2-thread-1] c.s.DeferredresultSpringmvcApplication   : service 服务调用的线程 : pool-2-thread-1 
2019-10-12 12:42:46.884  INFO 2020 --- [pool-2-thread-1] c.s.DeferredresultSpringmvcApplication   : service 执行完毕 
```







> ​	感觉我写的很详细 , 其他的就不多说了 , 其实他的应用场景就是 当一个处理请求消耗很长的时间时 , 需要去使用这个 , 主要就是采用发挥一个异步处理的线程事件, 不是返回一个结果 , 这点要明白     