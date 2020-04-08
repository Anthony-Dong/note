# Reactor-Netty框架的体验

这个库包含了, TCP , UDP 客户端 , 服务端, 常见的Http也实现了. 他对其进行了封装 .



我们以 服务端为例子. 

```java
public static void main(String[] args) {
    // 服务端
    HttpServer server = HttpServer.create(builder -> builder.port(8888));

    // 返回一个 hello-world
    // 必须设置content-length,因为HttpOperations的119行,自己看吧,根据这个判断的.
    BlockingNettyContext context = server.start((httpServerRequest, httpServerResponse) -> httpServerResponse.status(201).header("content-length", "11").sendString(s -> {
        s.onNext("hello-world");
        s.onComplete();
    }).then());

    // 阻塞
    context.getContext().onClose().block();
}
```



客户端例子:

```java
@Test
public void testClient() {
    // 客户端
    HttpClient client = HttpClient.create(builder -> {
        builder.option(ChannelOption.CONNECT_TIMEOUT_MILLIS, 10);
        builder.poolResources(PoolResources.elastic("elastic"));
    });
    // 申请.
    client.get("http://localhost:8888/hello/world").subscribe(System.out::println);
}
```

所以是不是很像Golang.  简直一模一样. 真心地.  所以我觉得他很有前途.

所以都采用的是Builder+Option组合 , 这样可以传参和创建对象 , 同时代码编辑也挺棒的.由于Java8支持的Lambda表达式更爽了.



强烈推荐这个包的源码, 大家可以看看体会一下哈. 很棒的. 









