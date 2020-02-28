# Dubbo  - 入门

> ​	[Dubbo快速入门 : ](http://dubbo.apache.org/zh-cn/docs/user/quick-start.html)  http://dubbo.apache.org/zh-cn/docs/user/quick-start.html

> **分布式服务架构**
>
> ​	当垂直应用越来越多，应用之间交互不可避免，将核心业务抽取出来，作为独立的服务，逐渐形成稳定的服务中心，使前端应用能更快速的响应多变的市场需求。此时，用于提高业务复用及**整合的分布式服务框架(RPC)是关键。**  目前RPC主流的就是 Dubbo , gRPC , 之类的 , 传统的比如 RestRPC 请求比如目前比较火热的springcloud采用的就是http的方式 , 但是http我们都知道效率低, 所以走向rpc,采用dubbo很关键, 低延迟 



参考文章 : [http://jm.taobao.org/2018/06/13/%E5%BA%94%E7%94%A8/](http://jm.taobao.org/2018/06/13/应用/)



> ​	阿里巴巴内部使用的分布式RPC框架HSF , 哈哈哈吃惊不 

#### 注意事项

> ​	注意一下 **Dubbo 2.7以下**的版本(不包含2.7.0)是 `com.alibaba.dubbo` , 2.7.X 的版本都是 `org.apache.dubbo` , 所以注意一下, 看自己需求使用吧 , 可能Apache对于Dubbo的老项目不友好, dubbo提供了解决方案, 还有注意如果你使用的Zookeeper做的Dubbo的注册中心 ,当你使用了 Dubbo2.7.X以上的版本他的默认依赖是`curator-recipes`的jar包而不是`zookeeper`的客户端jar包, 此时需要**你的zookeeper服务器的版本高于3.5.X** , 如果不是请自行解决,  所以最好的方式就是使用 2.7一下的版本, 这个开发的时候很坑 , 
>
> ​	还要注意的是  Dubbo的QOS 服务 , 默认端口是22222 ,如果出现这个就是QOS端口绑定问题 ,你可以选择关闭 , 用 `-Ddubbo.application.qosEnable=false` ,更多信息可以去百度, 我一般直接关闭





## 1. 核心点

### 1. Java RMI 简介

Java RMI （Remote Method Invocation）- 远程方法调用，能够让**客户端**像使用本地调用一样调用**服务端** Java 虚拟机中的对象方法。RMI 是面向对象语言领域对 RPC （Remote Procedure Call）的完善，用户无需依靠 IDL 的帮助来完成分布式调用，而是通过依赖接口这种更简单自然的方式。

### 2. Dubbo 基本概念

现代的分布式服务框架的基本概念与 RMI 是类似的，同样是使用 Java 的 Interface 作为服务契约，通过注册中心来完成服务的注册和发现，远程通讯的细节也是通过代理类来屏蔽。具体来说，Dubbo 在工作时有以下四个角色参与：

1. 服务提供者 - 启动时在指定端口上暴露服务，并将服务地址和端口注册到注册中心上
2. 服务消费者 - 启动时向注册中心订阅自己感兴趣的服务，以便获得服务提供方的地址列表
3. 注册中心 - 负责服务的注册和发现，负责保存服务提供方上报的地址信息，并向服务消费方推送
4. 监控中心 - 负责收集服务提供方和消费方的运行状态，比如服务调用次数、延迟等，用于监控
5. 运行容器 - 负责服务提供方的初始化、加载以及运行的生命周期管理

![](https://cdn.yuque.com/lark/0/2018/png/13615/1527499479031-19b76c2f-29dc-4766-8dcb-9dd7c0c089f7.png)



**部署阶段**

- 服务提供者在指定端口暴露服务，并向注册中心注册服务信息。
- 服务消费者向注册中心发起服务地址列表的订阅。

**运行阶段**

- **注册中心向服务消费者推送地址列表信息**。
- 服务消费者收到地址列表后，从其中选取一个向目标服务发起调用。
- 调用过程服务消费者和服务提供者的运行状态上报给监控中心。

## 2. 快速开始

### 1. Java的 RMI  , 我不会整, 跑不通, 可以百度



### 2. Dubbo - Api 方式

#### 1. 对外提供的接口

```java
public interface DemoService {
    String sayName(String name);
}
```

#### 2. 服务器端对接口的实现

```java
public class DefaultService implements DemoService {
	// 接口实现
    public String sayName(String name) {
        RpcContext rpcContext = RpcContext.getContext();
        return String.format("Service [port : %d] %s(\"%s\") : Hello,%s",
                rpcContext.getLocalPort(),
                rpcContext.getMethodName(),
                name,
                name);
    }
}
```

#### 4. 服务器(提供者)代码

```java
package com.example;

import com.alibaba.dubbo.config.*;
import com.example.dubboprovider.service.DefaultService;
import com.example.dubboprovider.service.DemoService;
import org.junit.Test;

import javax.xml.ws.soap.MTOM;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;


/**
 * TODO
 *
 * @date:2019/12/11 21:25
 * @author: <a href='mailto:fanhaodong516@qq.com'>Anthony</a>
 */

public class Provider {


    @Test
    public void provider() throws IOException {
        // 1. 提供的服务对象
        DemoService providerService = new DefaultService();

        // 2. 当前服务配置
        ApplicationConfig application = new ApplicationConfig();
        application.setName("provider");


        // 3. 注册中心配置 , NO_AVAILABLE , 采用直连的方式
        RegistryConfig registry = new RegistryConfig();
        registry.setAddress("N/A");


        // 4. 服务提供者协议配置
        ProtocolConfig protocol = new ProtocolConfig();
        // 协议名称
        protocol.setName("dubbo");
        // 服务器地址
        protocol.setHost("0.0.0.0");
        // 端口
        protocol.setPort(8888);
        // 配置线程数
        protocol.setThreads(200);


        // 5. 暴露的方法 // 如果不写全部方法都暴露在外面
        List<MethodConfig> methods = new ArrayList<MethodConfig>();
        MethodConfig method = new MethodConfig();
        method.setName("sayName");
        // ms
        method.setTimeout(10000);
        // 重试次数
        method.setRetries(0);
        methods.add(method);


        // 注意：ServiceConfig为重对象，内部封装了与注册中心的连接，以及开启服务端口
        // 6. (核心对象) 服务提供者暴露服务配置
        ServiceConfig<DemoService> service = new ServiceConfig<DemoService>(); // 此实例很重，封装了与注册中心的连接，请自行缓存，否则可能造成内存和连接泄漏
        service.setApplication(application);
        service.setRegistry(registry); // 多个注册中心可以用setRegistries()
        service.setProtocol(protocol); // 多个协议可以用setProtocols()
        service.setInterface(DemoService.class); // 暴露接口
        service.setRef(providerService); // 设置提供的引用对象
        service.setMethods(methods);
        service.setVersion("1.0.0"); // 设置版本号


        // 暴露及注册服务
        service.export();


        System.in.read();
    }
}
```

#### 4. 引用方(消费者)实现

```java
package com.example;

import com.alibaba.dubbo.config.*;
import com.alibaba.dubbo.rpc.RpcContext;
import com.example.dubboprovider.service.DefaultService;
import com.example.dubboprovider.service.DemoService;
import org.junit.Test;


/**
 * TODO
 *
 * @date:2019/12/11 21:25
 * @author: <a href='mailto:fanhaodong516@qq.com'>Anthony</a>
 */

public class Consumer {


    @Test
    public void consumer() {

        // 1. 当前服务配置
        ApplicationConfig application = new ApplicationConfig();
        application.setName("consumer");


        //2. 配置注册中心 ,本地模式 , NO_AVAILABLE
        RegistryConfig registry = new RegistryConfig();
        registry.setAddress("N/A");


        // 注意：ReferenceConfig为重对象，内部封装了与注册中心的连接，以及与服务提供方的连接
        // 3. 实例化一个引用对象  引用远程服务
        ReferenceConfig<DemoService> reference = new ReferenceConfig<DemoService>(); // 此实例很重，封装了与注册中心的连接，请自行缓存，否则可能造成内存和连接泄漏
        reference.setApplication(application);
        reference.setRegistry(registry);
        // 直连方式
        reference.setUrl("dubbo://192.168.28.1:8888/com.example.dubboprovider.service.DemoService");
        reference.setInterface(DemoService.class);
        reference.setVersion("1.0.0");


        // 引用对象
        DemoService referenceService = reference.get();
        String dubbo = referenceService.sayName("Dubbo");

        System.out.println(dubbo);
    }

}

```

输出 : 

```java
Service [port : 8888] sayName("Dubbo") : Hello,Dubbo
```



## 3. 主要角色

![](https://cdn.yuque.com/lark/0/2018/png/13615/1527499479031-19b76c2f-29dc-4766-8dcb-9dd7c0c089f7.png)





其实我们关心的就是 provider 和 consumer , register  , (其他角色后期介绍)

provider  : 核心就是`ServiceConfig` , 他是Dubbo 服务器提供者唯一的API 对象, 其他对象都是为这么一个对象服务的 . 一般需要 `ApplicationConfig` , `RegistryConfig` , `ProtocolConfig` , `MethodConfig` 

consumer : 核心就是 `ReferenceConfig` , 他是Dubbo 引用方唯一的消费对象 , 其他对象对这个对象服务 ,一般需要  `ApplicationConfig` , `RegistryConfig` , 





## 4. Spring整合

1) provider.xml

```xml
<?xml version="1.0" encoding="UTF-8"?>
<beans xmlns="http://www.springframework.org/schema/beans"
       xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
       xmlns:dubbo="http://dubbo.apache.org/schema/dubbo"
       xmlns:context="http://www.springframework.org/schema/context"
       xsi:schemaLocation="http://www.springframework.org/schema/beans http://www.springframework.org/schema/beans/spring-beans.xsd
		http://www.springframework.org/schema/context http://www.springframework.org/schema/context/spring-context-4.3.xsd
		http://dubbo.apache.org/schema/dubbo http://dubbo.apache.org/schema/dubbo/dubbo.xsd
		http://code.alibabatech.com/schema/dubbo http://code.alibabatech.com/schema/dubbo/dubbo.xsd">
	
	// 协议+端口
    <dubbo:protocol name="dubbo" port="20880"></dubbo:protocol>

	// 注册地址
    <dubbo:registry address="zookeeper://47.93.251.248:2181"></dubbo:registry>
	
     // 应用名
    <dubbo:application name="provider-dubbo"></dubbo:application>
    
     // 暴露的服务接口 - > ref指向实例化对象   
 	<dubbo:service id="demoService" interface="com.example.dubboprovider.service.DemoService" ref="defaultService">
    </dubbo:service>

	// 实例化的对象
    <bean id="defaultService" class="com.example.dubboprovider.service.DefaultService"/>

</beans>
```

2) comsumer.xml

```xml
<?xml version="1.0" encoding="UTF-8"?>
<beans xmlns="http://www.springframework.org/schema/beans"
       xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
       xmlns:dubbo="http://dubbo.apache.org/schema/dubbo"
       xmlns:context="http://www.springframework.org/schema/context"
       xsi:schemaLocation="http://www.springframework.org/schema/beans http://www.springframework.org/schema/beans/spring-beans.xsd
		http://www.springframework.org/schema/context http://www.springframework.org/schema/context/spring-context-4.3.xsd
		http://dubbo.apache.org/schema/dubbo http://dubbo.apache.org/schema/dubbo/dubbo.xsd
		http://code.alibabatech.com/schema/dubbo http://code.alibabatech.com/schema/dubbo/dubbo.xsd">

	// 注册地址
    <dubbo:registry address="zookeeper://47.93.251.248:2181"></dubbo:registry>

    // 应用名
    <dubbo:application name="consumer-dubbo"></dubbo:application>

    // 引用服务
    <dubbo:reference id="demoService" check="false" interface="com.example.dubboprovider.service.DemoService">
        <dubbo:method name="sayName" timeout="1000">
        </dubbo:method>
    </dubbo:reference>

</beans>
```

3 ) Provider测试类

```java
public class Provider {

    public static void main(String[] args) throws IOException {
        ClassPathXmlApplicationContext context = new ClassPathXmlApplicationContext("classpath:/dubbo/provider.xml");

        context.start();
        
        System.out.println("--------------------- -  provider server  start");

        System.in.read();
    }
}
```

4) Consumer测试类

```java
public class Consumer {
    public static void main(String[] args) throws Exception {
        ClassPathXmlApplicationContext context = new ClassPathXmlApplicationContext(new String[]{"classpath:/dubbo/consumer.xml"});
        context.start();
        // 所以Dubbo直接将自己的Bean直接注入到了Spring中 , 很方便的
        DemoService  demoService = (DemoService ) context.getBean("demoService");
        // Executing remote methods
        String s = demoService.sayName("1");
        System.out.println(s);
    }
}
```



## 5. Spring-Boot 整合

> ​	Dubbo 对外提供的注解 `@Service` 已经帮我们实例化了 , 不需要再次使用`@Component`了, 但是当你服务内部引用的时候注意`@Autowired` 需要引用他的接口类或者实现类 , 比如 `@Autowired  # DemoService` , 但是如果你又多个接口的实现类的时候,就需要导入实现类了`@Autowired  # DefaultService`

### 1. 快速开始

> ​	注意如果你是使用了Apache的 `dubbo-spring-boot-starter` , 里面是没有zookeeper的依赖的,还有需要zk客户端3.5以上

> ​	我下面提供的这个是 com.alibaba.boot 的依赖 , 直接导入一个就可以直接使用了 

主要依赖 , 各个模块都需要

```xml
<dependency>
    <groupId>com.alibaba.boot</groupId>
    <artifactId>dubbo-spring-boot-starter</artifactId>
    <version>0.2.0</version>
</dependency>

自带了 zookeeper 和 dubbo的包这个玩意
```

接口实现有所改变 ,注意这个`Service` 标签的含义是我们上面所说的 Dubbo对象唯一的引用实例 , 与SpringBoot的Service注解不一样的.

```java
@Service(version = "${demo.service.version}")
public class DefaultService implements DemoService {

    @Value("${demo.service.name}")
    private String serviceName;

    public String sayName(String name) {
        // 全局对象
        RpcContext rpcContext = RpcContext.getContext();
        return String.format("Service [name :%s , port : %d] %s(\"%s\") : Hello,%s",
                serviceName,
                rpcContext.getLocalPort(),
                rpcContext.getMethodName(),
                name,
                name);
    }
}
```

服务方的配置  `dubbo/provider-config.properties`

```properties
## application
dubbo.application.name = dubbo-provider-demo

## Nacos registry address
dubbo.registry.address = zookeeper://47.93.251.248:2181

## Dubbo Protocol
dubbo.protocol.name = dubbo
dubbo.protocol.port = -1

# Provider @Service version
demo.service.version=1.0.0
demo.service.name = demoService
```

服务启动类

```java
// spring 自动注入模式 ,其实类似于引入一个config类
@EnableDubbo(scanBasePackages = {"com.example.dubboprovider.service"})
@PropertySource(value = "classpath:/dubbo/provider-config.properties")
public class DubboProviderApplication {

    public static void main(String[] args) throws IOException {
        AnnotationConfigApplicationContext context = new AnnotationConfigApplicationContext();
        // 注册 
        context.register(DubboProviderApplication.class);
        // 刷新 . 必须刷新
        context.refresh();
        
        System.out.println("DemoService provider is starting...");
        System.in.read();
    }
}
```



消费者配置

```properties
## Dubbo Application info
dubbo.application.name = dubbo-consumer

## Nacos registry address
dubbo.registry.address = zookeeper://47.93.251.248:2181


# @Reference version
demo.service.version= 1.0.0
```



消费者启动类

```java
@EnableDubbo(scanBasePackages = {"com.example.dubboprovider.service"})
@PropertySource(value = "classpath:/dubbo/consumer-config.properties")
public class DubboConsumerApplication {

    @Reference(version = "${demo.service.version}")
    private DemoService demoService;

    @PostConstruct
    public void init() {
        for (int i = 0; i < 10; i++) {
            System.out.println(demoService.sayName(" Dubbo"));
        }
    }

    public static void main(String[] args) throws IOException {
        AnnotationConfigApplicationContext context = new AnnotationConfigApplicationContext();
        context.register(DubboConsumerApplication.class);
        context.refresh();
        context.close();
    }
}
```

输出 

```java
20:15:01.480 [DubboClientHandler-192.168.28.1:20880-thread-1] DEBUG com.alibaba.dubbo.remoting.transport.DecodeHandler -  [DUBBO] Decode decodeable message com.alibaba.dubbo.rpc.protocol.dubbo.DecodeableRpcResult, dubbo version: 2.6.2, current host: 192.168.28.1
Service [name :demoService , port : 20880] sayName(" Dubbo") : Hello, Dubbo

20:15:01.484 [DubboClientHandler-192.168.28.1:20880-thread-1] DEBUG com.alibaba.dubbo.remoting.transport.DecodeHandler -  [DUBBO] Decode decodeable message com.alibaba.dubbo.rpc.protocol.dubbo.DecodeableRpcResult, dubbo version: 2.6.2, current host: 192.168.28.1
Service [name :demoService , port : 20880] sayName(" Dubbo") : Hello, Dubbo
```



### 2. 问题

比如说我将 Reference 实现一个@Method , 此时会出现失效的事情, sayName方法并没有执行里面的约束

```java
@Reference(version = "${demo.service.version}",methods = {
        @Method(name="sayName",loadbalance = "consistenthash",timeout = 500)
})
private DemoService demoService;
```

如何解决呢 ?  只能采用这种方式了 .............. 因为这个parameter会在URL中添加参数

```java
@Reference(version = "${demo.service.version}", parameters = {
        "sayName.timeout", "1000", "sayName.loadbalance", "consistenthash"
})
private DemoService demoService;
```







## 6. Apache - Dubbo依赖

如果你的zookeeper版本低于3.5.X 需要将它排除在外

```java
  <dependencies>
        <dependency>
            <groupId>org.springframework.boot</groupId>
            <artifactId>spring-boot-starter-web</artifactId>
        </dependency>
        <dependency>
            <groupId>org.springframework.boot</groupId>
            <artifactId>spring-boot-starter</artifactId>
        </dependency>

        <dependency>
            <groupId>org.apache.dubbo</groupId>
            <artifactId>dubbo-spring-boot-starter</artifactId>
            <version>2.7.3</version>
        </dependency>

        <dependency>
            <groupId>org.apache.zookeeper</groupId>
            <artifactId>zookeeper</artifactId>
            <version>3.4.10</version>
        </dependency>

        <dependency>
            <groupId>org.apache.curator</groupId>
            <artifactId>curator-recipes</artifactId>
            <version>4.0.1</version>
            <exclusions>
                <exclusion>
                    <groupId>org.apache.zookeeper</groupId>
                    <artifactId>zookeeper</artifactId>
                </exclusion>
            </exclusions>
        </dependency>

    </dependencies>
```



还要注意的是  Dubbo的QOS 服务

```java

```

