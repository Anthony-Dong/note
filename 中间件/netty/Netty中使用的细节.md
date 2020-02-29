# Netty 中的一些细节问题




> ​	有的时候我们看netty并没有研究那么细致, 因为我们没有基于他开发过东西,大多都是些一些小型demo , 那么此时我就把我开发时遇到的问题总结一下 
>
> ​	[项目地址](https://github.com/Anthony-Dong/netty-IM) : https://github.com/Anthony-Dong/netty-IM  自定义协议客户端服务器端基本实现文件上传 , 消息传递, Json传递, 文本传递, 其中Http协议的服务器也写了

## 1.  ChannelInitializer<T> 究竟是做啥了

```java
public static void main(String[] args) {

    ServerBootstrap bootstrap = new ServerBootstrap();

    ChannelFuture bind = bootstrap.group(new NioEventLoopGroup(1), new NioEventLoopGroup(4))
            .channel(NioServerSocketChannel.class)
            .handler(new ChannelInitializer<Channel>() {
                @Override
                protected void initChannel(Channel ch) throws Exception {
                    ChannelPipeline pipeline = ch.pipeline();
                    // DO            
                }
            })
            .bind(8888);
}
```

这个`io.netty.channel.ChannelInitializer#initChannel`方法会在一个新的连接建立时就会初始化一次 , 每次都会初始化 ,  所以会产生一个问题 , 如果我的`ChannelHandler` 每次都实例化会造成系统的大量浪费, 比如10W个连接就会实例化 10W*N个数量级倍数的处理器, 所以就会出现一个问题, 

这个问题就是共享与不共享 , 所以此时就会出现线程安全的问题, 比如处理A的handler和处理B的handler一样的话,会出现什么问题, 我们无法维护一个客户端的 `Bytebuf` , 后面我会讲解为什么要为每个连接都维护一个`Bytebuf` 对象.  

所以一般对于我们真正处理的 比如 `io.netty.channel.SimpleChannelInboundHandler` 或者 `io.netty.channel.ChannelInboundHandlerAdapter` 这些对象其实是可以共享的, 前提是我们每次拿到的 `Bytebuf` 对象或者已经成功解码的对象, 保证完整性其实是可以共享的. (这里涉及到线程安全的问题 , 看自己业务需求吧 )

如何共享

1) 重写`public boolean isSharable()`  这个接口方法

```java
@Override
public boolean isSharable() {
    return true;
}
```

2) 或者在我们的 `ChannelHandler` 实现类申明 `io.netty.channel.ChannelHandler.Sharable` 此注解 , 在默认的方法中会对此做校验的. 

```java
public boolean isSharable() {
    Class<?> clazz = getClass();
    Map<Class<?>, Boolean> cache = InternalThreadLocalMap.get().handlerSharableCache();
    Boolean sharable = cache.get(clazz);
    if (sharable == null) {
        sharable = clazz.isAnnotationPresent(Sharable.class);
        cache.put(clazz, sharable);
    }
    return sharable;
}
```



## 2. ChannelHandlerContext 是啥

> ​	Context 显而易见就是上下文的意思, 每一个客户端连接都会有一个他自己的ChannelHandlerContext   , 所以这个是属于每个客户端私有的.  所以拿到这个对象就等于拿到了与客户端的通信口 .  对于每一个客户端都会有一个唯一的ID . 

- 一个管道对象, 其实就是我们`io.netty.channel.ChannelInitializer#initChannel` 添加的Handler组成的channel, 他可以链式的处理我们的数据包 , 同时还封装了一堆信息

```java
context.channel() 
```

- 返回当前所在的 ChannelHandler 对象,

```java
context.handler()
```

- 返回是否与客户端连接着了

```java
context.channel().isOpen()
```

- 远程客户端地址

```java
context.channel().remoteAddress()
```

同时他还有很多核心方法 `context.close()` 可以关闭连接 , `context.writeAndFlush()` 可以写东西 , 所以他就是我们唯一的与客户端通信的对象了

## 3. future.isSuccess()

这个方法返回, 真的是已经客户端接收到了吗 ? 

```java
context.writeAndFlush("hello netty").addListener(new GenericFutureListener<Future<? super Void>>() {
    @Override
    public void operationComplete(Future<? super Void> future) throws Exception {
        if (future.isSuccess()) {
            System.out.println("发送成功");
        }
    }
});
```



其实对于Netty的回调来说, 其实是封装了一个Promise对象, 当write方法调用成功,或者未修改Promise对象. 

```java
public void write(ChannelHandlerContext ctx, Object msg, ChannelPromise promise) throws Exception {....}
```

这里可以控制回调. 那么我们是么时候实例化的这个对象呢 :  `io.netty.channel.AbstractChannelHandlerContext#writeAndFlush(java.lang.Object)` -> `io.netty.channel.AbstractChannelHandlerContext#newPromise` 这个会实例化一个. 

所以比如我们在write方法中,修改了promise. 其实默认就是方法的调用.

## 4. Bytebuf

对于我们开发, netty帮我们维护一个`Bytebuf` 对象, 但是这个并不代表可以解决问题 , 每次客户端发送一个信息过来, netty 就会提供给我们一个 `Bytebuf` 来代表此次我所接受的字节缓冲区的包

此时可能发生 比如 我收到的是`一个半包` , `半个包` , `半个一个包` , 反正就是不完整的包 , 这里有人会问包是啥, 数据包对于编程者就是 从网络中接收到的字节数组, 但是他是无规则的哇.  可能客户端发送了 `ABCD` , 可能发送出去的时候是 `A` `BCD` , 服务器接收到的是 `AB` ,`CD` , 所以会出现个问题, 就是数据很可能不完整行  , 所以我们就需要将我们每次收到的字节数组维护起来, 第一次到n次, 以防出现这种情况 . 

这里就需要对数据包进行 协议的编写了 , 类似于我们最常见的`Http` 协议, 他就包含请求行,请求头,请求体, 之类的, 里面包含了特殊信息, 这就是格式 , 我们对于数据包接收过来, 如果符合这种格式我们就解析成我们的`request`对象 . 这里就需要维护一个缓冲区, 

下面是我写的一个编解码器, 我们数据包协议很简单 , `协议版本号` 占用2字节, `数据体长度` 占用4字节, `数据体` 未知长度 , 但是会写入到数据长度中

使用是 `MessageToByteEncoder` 和 `ByteToMessageDecoder` 他会给我们维护好一个缓冲区对象. 所以我们核心在于拿到缓冲区对象进行编解码 . 建议好好看看这个`ByteToMessageDecoder`类的源码.





## 5. Netty事件的流程. 

![img](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/dd6eee32-218d-488c-931f-464ad5791884.png)

 其实大致流程就是这个, 上诉是一个客户端流程, 从 inithandler入口 , 就开始了回调. 

如何区分是否要传递, , `fire` 就是传递的意思. 对于我们开发来说 , 其实我们可以使用. `ChannelDuplexHandler` 继承类.  还有就是看父类是否传递了, 也就是默认实现传递, 基本就传递. 没有的系统会帮助我们调用. 



我们可以通过调用 : 

`ctx.writeAndFlush` -> 会调用上层的outbound接口 , 会调用 write()和flush() 方法.

  `ctx.channel().writeAndFlush` -> 会从尾部开始调用, 也就是tail调用, 然后调用尾部, 尾部向上传播. 就这个. 



## channel 属性传递

// 定义一个 key , 类似于map中的key.

```java
static final AttributeKey<String> key = AttributeKey.valueOf("ip");
```

// 注册的时候我们将它设置进去. 

```java
ctx.channel().attr(key).set(ctx.channel().remoteAddress().toString());
```

// 其他时候就可以随意读取以及修改, 默认实现了 CAS . 所以线程安全操作.  

```java
String ip = ctx.channel().attr(attributeKey).get();
```



## epool / nio 循环组

NIO 是Java提供了, 在linux系统中也是`Epoll` , 但是对于Netty 来说也封装了一个`Epoll`处理. 

所以我们对于 linux内核 

```java
NioEventLoopGroup → EpollEventLoopGroup

NioEventLoop → EpollEventLoop

NioServerSocketChannel → EpollServerSocketChannel

NioSocketChannel → EpollSocketChannel
```

区别就是 : 

> Netty的 epoll transport使用 epoll edge-triggered 而 java的 nio 使用 level-triggered.
>
> 另外netty epoll transport 暴露了更多的nio没有的配置参数， 如 TCP_CORK, SO_REUSEADDR等等



其实对于Java的epoll来说,  如果select的轮询结果为空，也没有wakeup操作或者新的消息需要处理，则说明是个空轮询，可能会**触发JDK的epoll-bug** ,它会导致Selector的空轮询，是IO线程处于100%状态。



## boss / worker 

我们看一张图. 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/7ca48721-a564-4861-9ccf-01c60af9885d.jpg)

这就是 Netty 的 boss/worker模型. 

boss线程主要是监听 selector.accept事件., 也就是注册事件  

然后其他事件会交给 worker去处理事件. 

具体实现在 `io.netty.channel.nio.NioEventLoop#run` 这个方法里面. 不管是worker . 还是 boss, 都是死循环. 他们只负责他们对应的事件. 

注意一下.  事件 : 

Netty 只关注与一下事件. 第一个 `OP_CONNECT` 是客户端的事件 / 

第二个是 `OP_WRITE` 写事件  ,  第三个是 `OP_READ`/`OP_ACCEPT` 事件.  /服务器客户端

所以根据事件进行传播, 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/4fa43ee9-2064-4433-a205-904a4c3c258d.png?x-oss-process=style/template01)





我们再看 :  下面这个是监听到连接事件.  然后他传递的是一个socketchannel. 

你还记得我们注册的时候会 `.channel(NioServerSocketChannel.class)` 就是这个. 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/c63a591f-d1a0-4f70-bf30-1b6f3d2751ff.png?x-oss-process=style/template01)





这个会分发给 . 线程去处理.  我们看看 `io.netty.bootstrap.ServerBootstrap.ServerBootstrapAcceptor#channelRead`

这里就是处理的 socketcchannel.

```java
public void channelRead(ChannelHandlerContext ctx, Object msg) {
    final Channel child = (Channel) msg;

    child.pipeline().addLast(childHandler);

    setChannelOptions(child, childOptions, logger);

    for (Entry<AttributeKey<?>, Object> e: childAttrs) {
        child.attr((AttributeKey<Object>) e.getKey()).set(e.getValue());
    }

    try {
        childGroup.register(child).addListener(new ChannelFutureListener() {
            @Override
            public void operationComplete(ChannelFuture future) throws Exception {
                if (!future.isSuccess()) {
                    forceClose(child, future.cause());
                }
            }
        });
    } catch (Throwable t) {
        forceClose(child, t);
    }
}
```



这么一看其实客户端服务端公用的一套代码 . 只是具体的选择不同罢了. 

