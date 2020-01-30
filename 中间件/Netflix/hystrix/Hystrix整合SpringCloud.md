# Hystrix整合SpringCloud

### 引入依赖 . 

```xml
<dependencyManagement>
    <dependencies>
        <dependency>
            <groupId>org.springframework.cloud</groupId>
            <artifactId>spring-cloud-dependencies</artifactId>
            <version>Finchley.RELEASE</version>
            <type>pom</type>
            <scope>import</scope>
        </dependency>
    </dependencies>
</dependencyManagement>

<dependencies>

        ...
    <dependency>
        <groupId>org.springframework.cloud</groupId>
        <artifactId>spring-cloud-starter-netflix-hystrix</artifactId>
    </dependency>

</dependencies>
```

### 启动

使用 `HystrixCommandProperties` 和 `HystrixThreadPoolProperties` 配置 . 

```java
public class SpringCloudTemplateApplication {

    public static void main(String[] args) {
        SpringApplication.run(SpringCloudTemplateApplication.class, args);
    }

    @RestController
    public static class StoreIntegration {


        @GetMapping("/get/{name}")
        @HystrixCommand(fallbackMethod = "defaultStores",
                commandProperties = {
                        @HystrixProperty(name = "execution.isolation.thread.timeoutInMilliseconds", value = "500")
                }
        )
        public Object getStores(@PathVariable("name") String parameters) {
            sleep(1000);
            return "OK : " + parameters;
        }

        public Object defaultStores(String parameters) {
            System.out.println("fault : " + parameters);
            return "fault : " + parameters;
        }
    }
}
```

然后请求 

```shell
C:\Users\12986\Desktop>curl -X GET http://localhost:8080/get/aaa
fault : aaa
```

所以可以熔断 . 



###  配置类



`HystrixThreadPoolProperties` 配置线程

`HystrixCommandProperties`  配置基本配置



`com.netflix.hystrix.contrib.javanica.annotation.HystrixCommand` 注解 

配置 属性 , `commandProperties` -> `com.netflix.hystrix.HystrixCommandProperties`

`threadPoolProperties` -> `com.netflix.hystrix.HystrixThreadPoolProperties`

```java
String groupKey() default "";
String commandKey() default "";
String threadPoolKey() default "";
String fallbackMethod() default "";
HystrixProperty[] commandProperties() default {};
HystrixProperty[] threadPoolProperties() default {};
String defaultFallback() default "";
```

