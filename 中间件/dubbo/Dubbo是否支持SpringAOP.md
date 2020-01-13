# Dubbo 是否支持SpringAOP

	>	Dubbo可以支持Spring的AOP  ,但是前提是必须注解驱动 ,需要实现 `@DubboComponentScan` 或者`@EnableDubbo`
	>
	>	[可以看一下这篇文章描述的](https://mercyblitz.github.io/2018/01/01/Dubbo-注解驱动/)



## 1. 事务

1) 依赖

```java
<dependency>
    <groupId>org.springframework</groupId>
    <artifactId>spring-tx</artifactId>
    <version>5.0.2.RELEASE</version>
</dependency>
```

2) 配置类

```java
@Configuration
@EnableTransactionManagement
public class ProviderConfiguration {
    /**
     * 自定义事务管理器 , 最高优先级
     */
    @Bean
    @Primary
    public PlatformTransactionManager transactionManager() {
     
        return new PlatformTransactionManager() {
            @Override
            public TransactionStatus getTransaction(TransactionDefinition definition) throws TransactionException {
                System.out.println("get transaction ...");
                return new SimpleTransactionStatus();
            }

            @Override
            public void commit(TransactionStatus status) throws TransactionException {
                System.out.println("commit transaction ...");
            }

            @Override
            public void rollback(TransactionStatus status) throws TransactionException {
                System.out.println("rollback transaction ...");
            }
        };
    }
}
```

3) 我们的需要做事务类

```java
@Transactional
@Service(version = "${demo.service.version}") // dubbo的注解
public class DefaultService implements DemoService {

    @Value("${demo.service.name}")
    private String serviceName;

    private static final Random RANDOM = new Random();

    @DubboLog
    public String sayName(String name) {
        try {
            TimeUnit.MILLISECONDS.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        RpcContext rpcContext = RpcContext.getContext();

        System.out.println("rpcContext.getUrl().toString() = " + rpcContext.getUrl().toString());

        return String.format("Service [name :%s , port : %d] %s(\"%s\") : Hello,%s",
                serviceName,
                rpcContext.getLocalPort(),
                rpcContext.getMethodName(),
                name,
                name);
    }

    @Override
    public void echo(String msg) {
        System.out.println(msg);
    }
}
```

测试类 : 

```java
@EnableDubbo(scanBasePackages = {"com.example.dubboprovider.service"})
@PropertySource(value = "classpath:/dubbo/provider-config.properties")
@ComponentScan(value = {"com.example.dubboprovider.aop"})
@EnableAspectJAutoProxy
public class DubboProviderApplication {

    public static void main(String[] args) throws IOException {
        System.setProperty("dubbo.application.qosEnable", "false");
        AnnotationConfigApplicationContext context = new AnnotationConfigApplicationContext();
        context.register(DubboProviderApplication.class);
        context.refresh();
        System.out.println("DemoService provider is starting...");
        System.in.read();
    }
}
```

输出 

```java
get transaction ...
rpcContext.getUrl().toString() = dubbo://192.168.28.1:20081/com.example.dubboprovider.service.DemoService?anyhost=true&application=dubbo-provider-demo&bean.name=ServiceBean:com.example.dubboprovider.service.DemoService:2.0.0&bind.ip=192.168.28.1&bind.port=20081&deprecated=false&dubbo=2.0.2&dynamic=true&generic=false&interface=com.example.dubboprovider.service.DemoService&methods=sayName,echo&pid=11680&qos.enable=false&register=true&release=2.7.3&revision=2.0.0&side=provider&timestamp=1576220426792&version=2.0.0
commit transaction ...
```

结论 发现可以使用 , 



## 2. AOP

1) pom

```xml
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-aop</artifactId>
</dependency>
```

2) AOP 注解类

```java
@Documented
@Retention(RUNTIME)
@Target(METHOD)
public @interface DubboLog {
}
```

3) AOP 类

```java
@Component
@Aspect
public class DubboAop {
    private static final Logger logger = LoggerFactory.getLogger(DubboAop.class);

    // 可以将 加上 @log 注解的类 环绕通知
    @Pointcut("@annotation(com.example.dubboprovider.aop.DubboLog)")
    public void logPointCut() {
    }


    //环绕通知
    @Around("logPointCut()")
    public Object around(ProceedingJoinPoint point) throws Throwable {
        Object result = null;
        try {
            logger.info("start 开始执行");
            result = point.proceed();
            logger.info("doBefore :  方法返回值 : " + result);
            return result;
        } catch (Exception e) {
            logger.error("doAfterThrowing :  {} ", e.getMessage());
            throw new RuntimeException("runtime  exception");
        } finally {
            logger.info("doAfter 执行完成");
        }
    }
}
```

4) 测试类

```java
@EnableDubbo(scanBasePackages = {"com.example.dubboprovider.service"})
@PropertySource(value = "classpath:/dubbo/provider-config.properties")
@ComponentScan(value = {"com.example.dubboprovider.aop"})
@EnableAspectJAutoProxy
public class DubboProviderApplication {


    public static void main(String[] args) throws IOException {
        System.setProperty("dubbo.application.qosEnable", "false");

        AnnotationConfigApplicationContext context = new AnnotationConfigApplicationContext();
        context.register(DubboProviderApplication.class);
        context.refresh();
        System.out.println("DemoService provider is starting...");
        System.in.read();
    }
}
```

5) 输出  : 

```java
15:00:48.068 [DubboServerHandler-192.168.28.1:20081-thread-11] INFO com.example.dubboprovider.aop.DubboAop - start 开始执行
rpcContext.getUrl().toString() = dubbo://192.168.28.1:20081/com.example.dubboprovider.service.DemoService?anyhost=true&application=dubbo-provider-demo&bean.name=ServiceBean:com.example.dubboprovider.service.DemoService:2.0.0&bind.ip=192.168.28.1&bind.port=20081&deprecated=false&dubbo=2.0.2&dynamic=true&generic=false&interface=com.example.dubboprovider.service.DemoService&methods=sayName,echo&pid=11680&qos.enable=false&register=true&release=2.7.3&revision=2.0.0&side=provider&timestamp=1576220426792&version=2.0.0
15:00:49.071 [DubboServerHandler-192.168.28.1:20081-thread-11] INFO com.example.dubboprovider.aop.DubboAop - doBefore :  方法返回值 : Service [name :demoService , port : 20081] sayName(" Dubbo") : Hello, Dubbo
15:00:49.071 [DubboServerHandler-192.168.28.1:20081-thread-11] INFO com.example.dubboprovider.aop.DubboAop - doAfter 执行完成
```

6)  结论

发现AOP 也是支持的

