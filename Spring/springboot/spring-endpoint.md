# Spring - Endpoint

> ​	Identifies a type as being an actuator endpoint that provides information about the running application. Endpoints can be exposed over a variety of technologies including JMX and HTTP.
>
> ​	Endpoint可以给提供一些信息关于running application, Endpoint可以通过JMX 和HTTP技术来暴露 , 
>
> ​	重点就是一个是JMX 和HTPP技术, 还有他可以提供一些信息关于JAVA进程, 所以可以称作为运维工具



## 1. Spring 自定义 Endpoint

申明一个Endpoint需要Bean上注入`org.springframework.boot.actuate.endpoint.annotation.Endpoint` 这个注解, 其实还有一个 `JmxEndpoint`

> ​	Most @Endpoint classes will declare one or more @ReadOperation, @WriteOperation, @DeleteOperation annotated methods which will be automatically adapted to the exposing technology (JMX, Spring MVC, Spring WebFlux, Jersey etc.).
>
> ​	@Endpoint represents the lowest common denominator for endpoints and intentionally limits the sorts of operation methods that may be defined in order to support the broadest possible range of exposure technologies. If you need deeper support for a specific technology you can either write an endpoint that is filtered to a certain technology, or provide extension for the broader endpoint.

首先需要加入依赖

```xml
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-actuator</artifactId>
</dependency>
```

其次配置加入

```properties
management.endpoints.web.exposure.include=*
management.endpoint.health.show-details=always
```

Endpoint 类 : 

```java
@Endpoint(id = "nacos-config")
public class NacosConfigEndpoint {

    private static final Map<String, Object> MAP = new HashMap<>();

	// 对应着 GET 方法 , 注意方法名别重复,会抛出异常
    @ReadOperation
    public Map<String, Object> get() {
        MAP.clear();
        MAP.put("GET", "hello world");
        return MAP;

    }

    // 对应着 POST 方法
    // @Selector 相当于 @PathVariable
    @WriteOperation
    public Map<String, Object> post(@Selector String name) {
        MAP.clear();
        MAP.put("POST",name);
        return MAP;
    }

    // 对应着 DELETE 方法
    @DeleteOperation
    public Map<String, Object> delete(@Selector String name) {
        MAP.clear();
        MAP.put("DELETE",name);
        return MAP;
    }
}
```

配置类 : 

```java
@Configuration
public class NacosConfigEndpointAutoConfiguration {

    @Bean
    @ConditionalOnMissingBean
    @ConditionalOnEnabledEndpoint
    public NacosConfigEndpoint nacosEndpoint() {
        return new NacosConfigEndpoint();
    }
}
```

启动 日志会发现注册进去了, 

```java
2019-12-18 17:19:25.497  INFO 5572 --- [           main] s.b.a.e.w.s.WebMvcEndpointHandlerMapping : Mapped "{[/actuator/nacos-config],methods=[GET],produces=[application/vnd.spring-boot.actuator.v2+json || application/json]}" onto public java.lang.Object org.springframework.boot.actuate.endpoint.web.servlet.AbstractWebMvcEndpointHandlerMapping$OperationHandler.handle(javax.servlet.http.HttpServletRequest,java.util.Map<java.lang.String, java.lang.String>)
2019-12-18 17:19:25.497  INFO 5572 --- [           main] s.b.a.e.w.s.WebMvcEndpointHandlerMapping : Mapped "{[/actuator/nacos-config/{name}],methods=[DELETE],produces=[application/vnd.spring-boot.actuator.v2+json || application/json]}" onto public java.lang.Object org.springframework.boot.actuate.endpoint.web.servlet.AbstractWebMvcEndpointHandlerMapping$OperationHandler.handle(javax.servlet.http.HttpServletRequest,java.util.Map<java.lang.String, java.lang.String>)
2019-12-18 17:19:25.498  INFO 5572 --- [           main] s.b.a.e.w.s.WebMvcEndpointHandlerMapping : Mapped "{[/actuator/nacos-config/{name}],methods=[POST],produces=[application/vnd.spring-boot.actuator.v2+json || application/json]}" onto public java.lang.Object org.springframework.boot.actuate.endpoint.web.servlet.AbstractWebMvcEndpointHandlerMapping$OperationHandler.handle(javax.servlet.http.HttpServletRequest,java.util.Map<java.lang.String, java.lang.String>)

```

然后测试接口

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-18/240db8b8-e606-49cd-96d2-b7ee3e6a47cf.jpg?x-oss-process=style/template01)

测试结果 很显然是匹配的



最后一个问题 ,什么是JMX哇 , 我们上面用的是HTTP请求哇, 下面这个就是 ,可以通过jconsole工具打开可以看到.

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-18/fef92706-1dc8-42a6-b277-c0770eb6cee5.jpg?x-oss-process=style/template01)

## 2. ControllerEndpoint 更加方便

`org.springframework.boot.actuate.endpoint.web.annotation.ControllerEndpoint` 我们上述的都是使用我们不认识的注解, 开发不友好, 这个提供了 , 但是这个不会提供JMX , 所以不能使用jconsole 管理

> ​	Identifies a type as being an endpoint that is only exposed over Spring MVC or Spring WebFlux. Mapped methods must be annotated with @GetMapping, @PostMapping, @DeleteMapping, etc annotations rather than @ReadOperation, @WriteOperation, @DeleteOperation.

测试类

```java
@ControllerEndpoint(id = "web-mvc", enableByDefault = true)
public class WebMVCControllerEndpoint {

    @ResponseBody
    @GetMapping("/get/{id}")
    public List<String> get(@PathVariable("id") String id) {
        return Arrays.asList("hello ", id, " !");
    }
}
```

加入配置

```java
@Configuration
public class NacosConfigEndpointAutoConfiguration {

    @Bean
    @ConditionalOnMissingBean
    @ConditionalOnEnabledEndpoint
    public NacosConfigEndpoint nacosEndpoint() {
        return new NacosConfigEndpoint();
    }

    @Bean
    @ConditionalOnMissingBean
    @ConditionalOnEnabledEndpoint
    public WebMVCControllerEndpoint webMVCControllerEndpoint() {
        return new WebMVCControllerEndpoint();
    }

}
```

再次启动



## 2. Java 自定义 JMX

> ​	JMX（Java Management Extensions，即Java管理扩展）是一个为应用程序、设备、系统等[植入](https://baike.baidu.com/item/植入/7958584)管理功能的框架。JMX可以跨越一系列异构操作系统平台、[系统体系结构](https://baike.baidu.com/item/系统体系结构/6842760)和[网络传输协议](https://baike.baidu.com/item/网络传输协议/332131)，灵活的开发无缝集成的系统、网络和服务管理应用。



我简单copy了一份demo

```java
public interface HelloMBean {

    void echo(String msg);

    String getName();

    void setName(String name);
}
```

```java
/*
 * 该类名称必须与实现的接口的前缀保持一致（即MBean前面的名称
 */
public class Hello implements HelloMBean {

    private String name;

    @Override
    public void echo(String msg) {
        System.out.println("name : " + name + " , echo : " + msg);
    }

    @Override
    public String getName() {
        return this.name;
    }

    @Override
    public void setName(String name) {
        this.name = name;
    }
}
```

启动类

```java
public class HelloAgent {
    public static void main(String[] args) throws JMException, Exception {

        // 1. 通过工厂类获取Mbean Server，用来做Mbean的容器
        MBeanServer server = ManagementFactory.getPlatformMBeanServer();

        // 2. 域名:name=Mbean名称
        ObjectName helloName = new ObjectName("jmxBean:name=hello");

        //2. 最后将Hello这个类注册到MbeanServer中，注入需要创建一个ObjectName类
        server.registerMBean(new Hello(), helloName);

        System.in.read();
    }
}
```



最后用CMD执行一下 , jconsole , 然后由于我们上面提供了 getName和setName方法所以, 此时是可以输入值的,你接口中没有提供,是不可以展示值的.或修改值的

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-18/b228821d-4bce-4a75-9dfc-600457b4d7c6.jpg?x-oss-process=style/template01)

我们调用 echo方法, 可以往方法中输入参数

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-18/c156edb6-2f97-41fd-b9d1-1abb57d01c8a.jpg?x-oss-process=style/template01)

此时我们看我们的输出结果 : 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-18/6fc4c382-b736-40e7-a255-16ecccae33fa.jpg?x-oss-process=style/template01)



是不是成功调用成功了 , 这就是 JMX ,其他的详细使用就百度吧, 基本流程就是这个

