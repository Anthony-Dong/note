# ZipKin - 链路追踪

> ​	[Zipkin](https://zipkin.io/) is a distributed tracing system. It helps gather timing data needed to troubleshoot latency problems in service architectures. Features include both the collection and lookup of this data.
>
>  ZipKin是一个分布式的链路追踪系统 , 它有助于收集服务体系结构中的存在延迟问题所需的时间数据 , 他可以帮助`收集和查询`数据给我们. 



## 快速开始

他有一个服务端, 就是来收集数据的, 所以你可以去直接官网上下载一个  [下载链接](https://github.com/openzipkin/zipkin/releases)

 或者自己构建一个 , 

构建一个`Spring-Boot`项目 .  依赖中只用加入这些. 就可以了

```xml
<parent>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-parent</artifactId>
    <!--版本随意了-->
    <version>2.0.4.RELEASE</version>
    <relativePath/> 
</parent>
<dependencies>
    <dependency>
        <groupId>io.zipkin.java</groupId>
        <artifactId>zipkin-server</artifactId>
        <version>2.12.2</version>
    </dependency>

    <dependency>
        <groupId>io.zipkin.java</groupId>
        <artifactId>zipkin-autoconfigure-ui</artifactId>
        <version>2.12.2</version>
    </dependency>
</dependencies>
```

启动app , 可以参考`zipkin.server.ZipkinServer `实现. 

```java
@EnableZipkinServer
@SpringBootApplication
public class SpringTraceApplication {
    static {
        // Make sure java.util.logging goes to log4j2
        // https://docs.spring.io/spring-boot/docs/current/reference/html/howto-logging.html#howto-configure-log4j-for-logging
        System.setProperty("java.util.logging.manager", "org.apache.logging.log4j.jul.LogManager");
    }

    public static void main(String[] args) {
        new SpringApplicationBuilder(SpringTraceApplication.class)
                .listeners(new RegisterZipkinHealthIndicators())
                .properties("spring.config.name=zipkin-server").run(args);
    }
}
```

然后mvn打包一下就行了 `mvn clean install -Dmaven.test.skip=true `  就可以了 . 然后运行这个jar包就行 . 这个是最快的(可以翻墙就我没说, 官方不推荐自己写服务端).  默认端口是 `9441`  . 

然后直接 java -jar 启动就可以了

访问 http://localhost:9411/zipkin/ 就OK了

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-10-11/4156413d-7ccf-4812-a721-2e00f3f89757.png?x-oss-process=style/template01)

## 2. 使用

> ​	微服务 , 所以这个是给微服务使用的, 基本就是springcloud了 .  `Spring Cloud Sleuth` 其实就是. 

加入依赖 

```java
<dependency>
    <groupId>org.springframework.cloud</groupId>
    <artifactId>spring-cloud-starter-zipkin</artifactId>
</dependency>
```

然后加入配置 , 还有很多可以去官网找 , 链接 : https://www.springcloud.cc/spring-cloud-dalston.html#_spring_cloud_sleuth ,这里讲解了sleuth 的全部内容. 

```properties
spring.zipkin.base-url=http://localhost:9411
spring.zipkin.service.name=user-service
spring.zipkin.enabled=true
spring.zipkin.discovery-client-enabled=true
```

然后构建服务 

其实基本已经完成了 , 可以像写微服务一样直接使用, eureka注册中心哇 , 服务调用哇 . zuul网关, 都可以.  



如果你想详细了解其全部使用 就是看文档了, 

https://github.com/openzipkin/zipkin/wiki

https://github.com/openzipkin/brave

https://www.springcloud.cc/spring-cloud-dalston.html#_spring_cloud_sleuth

