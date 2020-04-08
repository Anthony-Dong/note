# Spring Async 上下文的问题

异步 就涉及到一个上下文切换的问题, 像Spring的Request对象其实存在一个 上下文对象中 , 我们可以去拿取, 前提是你的所有执行逻辑是一个线程.   像 `ServletRequestAttributes requestAttributes = (ServletRequestAttributes) RequestContextHolder.getRequestAttributes();`  就是获取上下文. 他会在我么逻辑中拦截 , 并且注入. 

但是有的时候涉及到异步上下文. 

```java
@Slf4j
@Service
public class UserService {

    @Async
    public void find() {
        ServletRequestAttributes requestAttributes = (ServletRequestAttributes) RequestContextHolder.getRequestAttributes();

        if (requestAttributes == null) {
            log.info("当前线程为 {} 位置为{} require对象为空.", Thread.currentThread().getName(),  "service");
        } else {
            HttpServletRequest request = requestAttributes.getRequest();
            log.info("当前线程为 {}，请求方法为 {}，请求路径为：{} 位置为:{}", Thread.currentThread().getName(), request.getMethod(), request.getRequestURL(), "service");
        }
    }
}
```



开启异步, 主程序. 

```java
@Slf4j
@EnableAsync
@RestController
@SpringBootApplication
public class SpringAsyncApplication {
    private final UserService service;

    @Autowired
    public SpringAsyncApplication(UserService service) {
        this.service = service;
    }

    public static void main(String[] args) {
        SpringApplication.run(SpringAsyncApplication.class, args);
    }

    @GetMapping("/find/{id}")
    public String echo(@PathVariable(value = "id") String id) {
        service.find();
        ServletRequestAttributes requestAttributes = (ServletRequestAttributes) RequestContextHolder.getRequestAttributes();
        HttpServletRequest request = requestAttributes.getRequest();
        log.info("当前线程为 {} 请求方法为 {} 请求路径为{} 位置为{}", Thread.currentThread().getName(), request.getMethod(), request.getRequestURL(),"controller");
        return id;
    }
}
```



我们发现日志

```java
2020-03-30 17:09:14.475  INFO 16720 --- [nio-8080-exec-1] c.e.springasync.SpringAsyncApplication   : 当前线程为 http-nio-8080-exec-1 请求方法为 GET 请求路径为http://localhost:8080/find/111 位置为controller
2020-03-30 17:09:14.480  INFO 16720 --- [cTaskExecutor-1] c.e.springasync.service.UserService      : 当前线程为 SimpleAsyncTaskExecutor-1 位置为service require对象为空.
```



所以此时上下文不存在 . 所以需要我们手动去配置一下  ,spring提供了 很好地操作对象. 

```java
@Configuration
public class Config {
    private static final String ASYNC_EXECUTOR_NAME = "asyncExecutor";

    @Bean(name = ASYNC_EXECUTOR_NAME)
    public Executor executor() {
        ThreadPoolTaskExecutor executor = new ThreadPoolTaskExecutor();
        executor.setThreadNamePrefix(ASYNC_EXECUTOR_NAME);
        executor.setCorePoolSize(5);
        executor.setMaxPoolSize(5);
        executor.setQueueCapacity(-1);
        // 这里在每次异步调用的时候, 会包装一下.
        executor.setTaskDecorator(runnable -> {
            // 这个时候还是同步状态
            RequestAttributes requestAttributes = RequestContextHolder.currentRequestAttributes();
            // 返回的这个 runnable对象 才是去调用线程池.
            return () -> {
                try {
                    // 我们set 进去 ,其实是一个ThreadLocal维护的.
                    RequestContextHolder.setRequestAttributes(requestAttributes);
                    runnable.run();
                } finally {
                    // 最后记得释放内存
                    RequestContextHolder.resetRequestAttributes();
                }
            };
        });
        return executor;
    }
}
```



所以此时 

```java
2020-03-30 17:10:48.377  INFO 18060 --- [nio-8080-exec-2] c.e.springasync.SpringAsyncApplication   : 当前线程为 http-nio-8080-exec-2 请求方法为 GET 请求路径为http://localhost:8080/find/111 位置为controller
2020-03-30 17:10:48.382  INFO 18060 --- [ asyncExecutor1] c.e.springasync.service.UserService      : 当前线程为 asyncExecutor1，请求方法为 GET，请求路径为：http://localhost:8080/find/111 位置为:service
```



这个就是一个解决方案.  

