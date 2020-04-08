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

maven 依赖

```xml
<dependency>
    <groupId>org.apache.dubbo</groupId>
    <artifactId>dubbo</artifactId>
    <version>2.7.3</version>
</dependency>
```

#### 1. 对外提供的接口

```java
public interface EchoService {
    String echo();
}
```

#### 2. 服务器端对接口的实现

```java
public class EchoServiceImpl implements EchoService {
    @Override
    public String echo() {
        RpcContext rpcContext = RpcContext.getContext();
        return String.format("Service [port : %d] %s() : Hello,%s",
                rpcContext.getLocalPort(),
                rpcContext.getMethodName(),
                "dubbo");
    }
}
```

#### 4. 服务器(提供者)代码

```java
public class DubboServer {

    public static void main(String[] args) throws IOException {
        ServiceConfig<EchoService> serviceConfig = new ServiceConfig<>();
        // 暴露的服务-接口实例对象
        serviceConfig.setRef(new EchoServiceImpl());
        // 上下文-配置
        serviceConfig.setApplication(new ApplicationConfig("dubbo-server"));
        // 注册中心
        serviceConfig.setRegistry(new RegistryConfig(RegistryConfig.NO_AVAILABLE));
        // 通信协议-dubbo
        serviceConfig.setProtocol(new ProtocolConfig("dubbo", 9999));
        // 暴露一下
        serviceConfig.export();
        System.in.read();
    }
}
```

查看日志 : 

主要是我们是直连 , 所以`dubbo://192.168.28.1:9999/com.example.java_api.inter.EchoService` 就是我们直连的url

```java
19:17:28.673 [main] INFO org.apache.dubbo.config.AbstractConfig -  [DUBBO] Export dubbo service com.example.java_api.inter.EchoService to url dubbo://192.168.28.1:9999/com.example.java_api.inter.EchoService?anyhost=true&application=dubbo-server&bind.ip=192.168.28.1&bind.port=9999&deprecated=false&dubbo=2.0.2&dynamic=true&generic=false&interface=com.example.java_api.inter.EchoService&methods=echo&pid=14500&register=true&release=2.7.3&side=provider&timestamp=1584703048520, dubbo version: 2.7.3, current host: 192.168.28.1
19:17:28.698 [main] DEBUG org.apache.dubbo.common.extension.AdaptiveClassCodeGenerator -  [DUBBO] package org.apache.dubbo.remoting;
```

暴露URL

#### 4. 引用方(消费者)实现

```java
public class DubboClient {

    public static void main(String[] args) {
        ReferenceConfig<EchoService> referenceConfig = new ReferenceConfig<>();
        // 引用的接口
        referenceConfig.setInterface(EchoService.class);
        // 注册中心
        referenceConfig.setRegistry(new RegistryConfig(RegistryConfig.NO_AVAILABLE));
        // 直连
        referenceConfig.setUrl("dubbo://192.168.28.1:9999/com.example.java_api.inter.EchoService");
        // 上下文
        referenceConfig.setApplication(new ApplicationConfig("default"));
        // 获取代理对象
        EchoService service = referenceConfig.get();
        // 调用
        System.out.println("service.echo() = " + service.echo());
    }
}

```

输出 : 

```java
19:22:36.773 [main] INFO org.apache.dubbo.config.AbstractConfig -  [DUBBO] Refer dubbo service com.example.java_api.inter.EchoService from url dubbo://192.168.28.1:9999/com.example.java_api.inter.EchoService?application=default&interface=com.example.java_api.inter.EchoService&lazy=false&pid=8304&register.ip=192.168.28.1&remote.application=&side=consumer&sticky=false, dubbo version: 2.7.3, current host: 192.168.28.1
service.echo() = Service [port : 9999] echo() : Hello,dubbo
```



## 3. 主要角色

![](https://cdn.yuque.com/lark/0/2018/png/13615/1527499479031-19b76c2f-29dc-4766-8dcb-9dd7c0c089f7.png)





其实我们关心的就是 provider 和 consumer , register  , (其他角色后期介绍)

provider  : 核心就是`ServiceConfig` , 他是Dubbo 服务器提供者唯一的API 对象, 其他对象都是为这么一个对象服务的 . 一般需要 `ApplicationConfig` , `RegistryConfig` , `ProtocolConfig` , `MethodConfig` 

consumer : 核心就是 `ReferenceConfig` , 他是Dubbo 引用方唯一的消费对象 , 其他对象对这个对象服务 ,一般需要  `ApplicationConfig` , `RegistryConfig` , 



## 4. 加入注册中心

我们使用zookeeper. 因为一般通用都是使用这个. 

http://dubbo.apache.org/zh-cn/docs/user/references/registry/zookeeper.html

目前 dubbo 2.7.X 默认是使用的 Netflix提供的[curator](https://github.com/apache/curator)  写的zk的客户端. 所以根据需求来.  所以必须整这个依赖. 

```java
<dependency>
    <groupId>org.apache.zookeeper</groupId>
    <artifactId>zookeeper</artifactId>
    // 根据你客户端版本走. 所以下面要排除客户端依赖.
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



加入注册中心. 

```java
serviceConfig.setRegistry(new RegistryConfig("zookeeper://192.168.58.131:2181?client=curator"));

referenceConfig.setRegistry(new RegistryConfig("zookeeper://192.168.58.131:2181?client=curator"));
```

此时就可以愉快的使用了. 





## 







