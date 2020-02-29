# 带你走入Grpc(Java版本)

> ​	Grpc的优势这里我就不说了, 第一它是基于Http2.0协议的, 可以保持客户端与服务器端长连接, 基于二进制流,也就是字节流, 不是文本流. 
>
> ​	同时他提供了跨平台(语言)的开发 , 但是他也有成本就是需要定义他的接口规范, 遵循他的一套规范, 也就是说就算你是Java语言, 你不能随意定义一个接口, 我就能帮你实现RPC , 只是他可以帮助你生成一个符合他规范的接口.  
>
> ​	我们还是快速体验一下.   pom配置在文章最后

## 快速开始

#### 1.依赖 : 

```xml
<dependency>
    <groupId>io.grpc</groupId>
    <artifactId>grpc-all</artifactId>
    <version>1.12.0</version>
</dependency>
```

插件和全部依赖都在文章最后面 . 全局粘贴

#### 2.编写protobuf文件

idea 介意下载一个 protobuf的插件, 可以有代码提示. 这里直接去pluging里搜就行了. 

我是用的是proto3的版本. 

可以参考一下这个文章 https://www.cnblogs.com/tohxyblog/p/8974763.html 

注意这个文件是在 你的src/proto/XX.proto 中定义的, 文件目录必须写好.

```java
syntax = "proto3"; // 协议版本

// 选项配置
option java_package = "com.example.grpc.api";
option java_outer_classname = "RPCDateServiceApi";
option java_multiple_files = true;

// 定义包名
package com.example.grpc.api;

// 服务接口.定义请求参数和相应结果	
service RPCDateService {
    rpc getDate (RPCDateRequest) returns (RPCDateResponse) {
    }
}

// 定义请求体
message RPCDateRequest {
    string userName = 1;
}

// 定义相应内容
message RPCDateResponse {
    string serverDate = 1;
}
```

使用maven插件, 编译. 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/2a1fbe3e-64f2-4106-8f6b-52d81865cb83.png)



第一个命令执行完. 在 target目录里找就行了.  第二个命令也是找就行了. 然后将生成的Java文件拷贝到你的目录里.就可以了, 类似于Mybatis的mapper插件. 



#### 3. 编写接口实现类.

```java
package com.example.grpc.service;

// RPCDateServiceGrpc.RPCDateServiceImplBase 这个就是接口. 
// RPCDateServiceImpl 我们需要继承他的,实现方法回调
public class RPCDateServiceImpl extends RPCDateServiceGrpc.RPCDateServiceImplBase {
    @Override
    public void getDate(RPCDateRequest request, StreamObserver<RPCDateResponse> responseObserver) {
        // 请求结果,我们定义的
        RPCDateResponse rpcDateResponse = null;
        String userName = request.getUserName();
        String response = String.format("你好: %s, 今天是%s.", userName, LocalDate.now().format(DateTimeFormatter.ofPattern("yyyy-MM-dd")));
        try {
            // 定义响应,是一个builder构造器. 
            rpcDateResponse = RPCDateResponse
                    .newBuilder()
                    .setServerDate(response)
                    .build();
        } catch (Exception e) {
            responseObserver.onError(e);
        } finally {
            // 这种写法是observer,异步写法,老外喜欢用这个框架.
            responseObserver.onNext(rpcDateResponse);
        }
        responseObserver.onCompleted();
    }
}
```



#### 定义服务端

```java
public class GRPCServer {
    private static final int port = 9999;

    public static void main(String[] args) throws Exception {
        // 设置service接口.
        Server server = ServerBuilder.
                forPort(port)
                .addService(new RPCDateServiceImpl())
                .build().start();
        System.out.println(String.format("GRpc服务端启动成功, 端口号: %d.", port));
        server.awaitTermination();
    }
}
```



#### 定义客户端

```java
public class GRPCClient {
    private static final String host = "localhost";
    private static final int serverPort = 9999;

    public static void main(String[] args) throws Exception {
        // 1. 拿到一个通信的channel
        ManagedChannel managedChannel = ManagedChannelBuilder.forAddress(host, serverPort).usePlaintext().build();
        try {
            // 2.拿到道理对象
            RPCDateServiceGrpc.RPCDateServiceBlockingStub rpcDateService = RPCDateServiceGrpc.newBlockingStub(managedChannel);
            RPCDateRequest rpcDateRequest = RPCDateRequest
                    .newBuilder()
                    .setUserName("anthony")
                    .build();
            // 3. 请求
            RPCDateResponse rpcDateResponse = rpcDateService.getDate(rpcDateRequest);
            // 4. 输出结果
            System.out.println(rpcDateResponse.getServerDate());
        } finally {
            // 5.关闭channel, 释放资源.
            managedChannel.shutdown();
        }
    }
}
```



以上就是完整的流程.  启动服务器端, 启动客户端打印一下信息 : 

服务端日志 : 

```java
2020-02-29 11:04:50,991 4689   [LG-3-1] DEBUG .grpc.netty.NettyServerHandler  - [id: 0xed01e3fb, L:/127.0.0.1:9999 - R:/127.0.0.1:3994] INBOUND SETTINGS: ack=true
2020-02-29 11:04:51,075 4773   [LG-3-1] DEBUG .grpc.netty.NettyServerHandler  - [id: 0xed01e3fb, L:/127.0.0.1:9999 - R:/127.0.0.1:3994] INBOUND HEADERS: streamId=3 headers=GrpcHttp2RequestHeaders[:path: /com.example.grpc.api.RPCDateService/getDate, :authority: localhost:9999, :method: POST, :scheme: http, te: trailers, content-type: application/grpc, user-agent: grpc-java-netty/1.12.0, grpc-accept-encoding: gzip, grpc-trace-bin: ] streamDependency=0 weight=16 exclusive=false padding=0 endStream=false
2020-02-29 11:04:51,125 4823   [LG-3-1] DEBUG .grpc.netty.NettyServerHandler  - [id: 0xed01e3fb, L:/127.0.0.1:9999 - R:/127.0.0.1:3994] INBOUND DATA: streamId=3 padding=0 endStream=true length=14 bytes=00000000090a07616e74686f6e79
2020-02-29 11:04:51,172 4870   [LG-3-1] DEBUG .grpc.netty.NettyServerHandler  - [id: 0xed01e3fb, L:/127.0.0.1:9999 - R:/127.0.0.1:3994] OUTBOUND HEADERS: streamId=3 headers=GrpcHttp2OutboundHeaders[:status: 200, content-type: application/grpc, grpc-encoding: identity, grpc-accept-encoding: gzip] streamDependency=0 weight=16 exclusive=false padding=0 endStream=false
2020-02-29 11:04:51,189 4887   [LG-3-1] DEBUG .grpc.netty.NettyServerHandler  - [id: 0xed01e3fb, L:/127.0.0.1:9999 - R:/127.0.0.1:3994] OUTBOUND DATA: streamId=3 padding=0 endStream=false length=44 bytes=00000000270a25e4bda0e5a5bd3a20616e74686f6e792c20e4bb8ae5a4a9e698af323032302d30322d32392e
2020-02-29 11:04:51,190 4888   [LG-3-1] DEBUG .grpc.netty.NettyServerHandler  - [id: 0xed01e3fb, L:/127.0.0.1:9999 - R:/127.0.0.1:3994] OUTBOUND HEADERS: streamId=3 headers=GrpcHttp2OutboundHeaders[grpc-status: 0] streamDependency=0 weight=16 exclusive=false padding=0 endStream=true
2020-02-29 11:04:51,204 4902   [LG-3-1] DEBUG .grpc.netty.NettyServerHandler  - [id: 0xed01e3fb, L:/127.0.0.1:9999 - R:/127.0.0.1:3994] INBOUND GO_AWAY: lastStreamId=0 errorCode=0 length=0 bytes=
```



客户端日志 : 

```java
2020-02-29 11:00:43,480 1814   [LG-1-2] DEBUG .grpc.netty.NettyClientHandler  - [id: 0xcdbd6064, L:/127.0.0.1:3817 - R:localhost/127.0.0.1:9999] INBOUND HEADERS: streamId=3 headers=GrpcHttp2ResponseHeaders[grpc-status: 0] streamDependency=0 weight=16 exclusive=false padding=0 endStream=true
你好: anthony, 今天是20-02-29.
2020-02-29 11:00:43,487 1821   [LG-1-2] DEBUG .grpc.netty.NettyClientHandler  - [id: 0xcdbd6064, L:/127.0.0.1:3817 - R:localhost/127.0.0.1:9999] OUTBOUND GO_AWAY: lastStreamId=0 errorCode=0 length=0 bytes=
```

这就是一个Grpc的完整流程.



## 分析一下.

定义都是一个CS模型, GRPC他看到了未来一定是HTTP2的天下(这是必然的趋势), 所以他的没有制定自己的协议, 这里指的是应用层协议. 类似于我们的RestTemplate 一样都是基于http协议. 

所以他考虑到的是 JSON ,这种序列化比较慢的序列化方式. 采用了自己开发的一种Protobuf.

这里就说明了 GRPC其实只是解决了序列化的方式, 依靠这个Protobuf, 可以解决一系列问题, 比如接口信息的定义, 等等有很多, 

最简单的例子. 比如我Java中写了一个echo() 接口, golang如何调用 ? . 这就是难点, 

我们可以依靠统一的Protobuf文件生成对应的 Golang和Java的接口, 就可以了, 所以说本质上来说他是做了这点. 



我们看看Dubbo做了什么 , 他依靠TCP的可靠性, 自己基于TCP实现了一个应用层协议, dubbo协议. 同时也引用了一堆的序列化方式, 所以整个GRPC也是可行的. 

​	

感兴趣的同学可以做一次抓包, 抓取客户端请求,分析一个GRPC的请求格式. 

















## pom文件

```xml
<?xml version="1.0" encoding="UTF-8"?>
<project xmlns="http://maven.apache.org/POM/4.0.0"
         xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
         xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 http://maven.apache.org/xsd/maven-4.0.0.xsd">
    <modelVersion>4.0.0</modelVersion>

    <groupId>com.examole</groupId>
    <artifactId>grpc-study-demo-01</artifactId>
    <version>1.0-SNAPSHOT</version>


    <dependencies>
        <dependency>
            <groupId>junit</groupId>
            <artifactId>junit</artifactId>
            <version>4.13</version>
        </dependency>

        <dependency>
            <groupId>org.slf4j</groupId>
            <artifactId>slf4j-log4j12</artifactId>
            <version>1.7.21</version>
        </dependency>
        <dependency>
            <groupId>org.slf4j</groupId>
            <artifactId>slf4j-api</artifactId>
            <version>1.7.21</version>
        </dependency>

        <dependency>
            <groupId>io.grpc</groupId>
            <artifactId>grpc-all</artifactId>
            <version>1.12.0</version>
        </dependency>


    </dependencies>
    <build>
        <extensions>
            <extension>
                <groupId>kr.motd.maven</groupId>
                <artifactId>os-maven-plugin</artifactId>
                <version>1.4.1.Final</version>
            </extension>
        </extensions>
        <plugins>
            <plugin>
                <groupId>org.apache.maven.plugins</groupId>
                <artifactId>maven-compiler-plugin</artifactId>
                <version>3.8.0</version>
                <configuration>
                    <source>1.8</source>
                    <target>1.8</target>
                    <encoding>UTF-8</encoding>
                </configuration>
            </plugin>
            <plugin>
                <groupId>org.xolstice.maven.plugins</groupId>
                <artifactId>protobuf-maven-plugin</artifactId>
                <version>0.5.0</version>
                <configuration>
                    <pluginId>grpc-java</pluginId>
                    <protocArtifact>com.google.protobuf:protoc:3.0.2:exe:${os.detected.classifier}</protocArtifact>
                    <pluginArtifact>io.grpc:protoc-gen-grpc-java:1.2.0:exe:${os.detected.classifier}</pluginArtifact>
                </configuration>
                <executions>
                    <execution>
                        <goals>
                            <goal>compile</goal>
                            <goal>compile-custom</goal>
                        </goals>
                    </execution>
                </executions>
            </plugin>
        </plugins>
    </build>
</project>
```

