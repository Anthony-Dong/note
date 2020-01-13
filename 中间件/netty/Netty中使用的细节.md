
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

很显然是错误的. 这个只是执行 `Flush` 成功罢了 , 也就是写到了我们字节缓冲区(Netty中是 `Bytebuf` ,底层其实是socket缓冲区) . 所以他并不是代表发送成功 , 以此判断成功是错误的 , 有可能服务器不处理也很无奈, 丢包也常见



## 4. Bytebuf

对于我们开发, netty帮我们维护一个`Bytebuf` 对象, 但是这个并不代表可以解决问题 , 每次客户端发送一个信息过来, netty 就会提供给我们一个 `Bytebuf` 来代表此次我所接受的字节缓冲区的包

此时可能发生 比如 我收到的是`一个半包` , `半个包` , `半个一个包` , 反正就是不完整的包 , 这里有人会问包是啥, 数据包对于编程者就是 从网络中接收到的字节数组, 但是他是无规则的哇.  可能客户端发送了 `ABCD` , 可能发送出去的时候是 `A` `BCD` , 服务器接收到的是 `AB` ,`CD` , 所以会出现个问题, 就是数据很可能不完整行  , 所以我们就需要将我们每次收到的字节数组维护起来, 第一次到n次, 以防出现这种情况 . 

这里就需要对数据包进行 协议的编写了 , 类似于我们最常见的`Http` 协议, 他就包含请求行,请求头,请求体, 之类的, 里面包含了特殊信息, 这就是格式 , 我们对于数据包接收过来, 如果符合这种格式我们就解析成我们的`request`对象 . 这里就需要维护一个缓冲区, 

下面是我写的一个编解码器, 我们数据包协议很简单 , `协议版本号` 占用2字节, `数据体长度` 占用4字节, `数据体` 未知长度 , 但是会写入到数据长度中

使用是 `MessageToByteEncoder` 和 `ByteToMessageDecoder` 他会给我们维护好一个缓冲区对象. 所以我们核心在于拿到缓冲区对象进行编解码 . 建议好好看看这个`ByteToMessageDecoder`类的源码

Java对象转换成字节流,反过来同理 .  可以用Java序列化工具, 也可以使用其他工具 . 

首先是最简单的解码器 , 因为我们发送绝对是发送一个完整的对象哇 , 如果你这个都半半分 , 无语了

```java
/**
 * 编码器  将 {@link NPack} 编码成为  ByteBuf 然后放入字节缓冲区
 * <p>
 * 主要就是写 一个 版本号, 数据包长度, 数据包 , 来做校验
 * <p>
 *
 * @date:2019/11/10 13:20
 * @author: <a href='mailto:fanhaodong516@qq.com'>Anthony</a>
 */
public final class PackageEncoder extends MessageToByteEncoder<NPack> {

    /**
     * 自定义协议头
     */
    private final short version;

    public PackageEncoder(short version) {
        super();
        this.version = version;
    }

    @Override
    protected void encode(ChannelHandlerContext ctx, NPack msg, ByteBuf out)
            throws Exception {

        int release = out.writerIndex();

        byte[] body = null;

        try {
            // 1. 将 NPack对象 转换成 字节数组形式
            body = MessagePackPool.getPack().write(msg);

            // 2. 获取 NPack 字节数组长度
            int length = body.length;

            // 3. 写入一个协议头 , 2个字节 16位 (-32768,32767)
            out.writeShort(version);


            // 4. 写入一个数据包长度 , 做校验  , 4个字节 32位 (-2147483648 , 2147483647 )
            out.writeInt(length);


            // 5. 写入数据体 - 真正的数据包
            out.writeBytes(body);

        } catch (Throwable error) {

            // 抓取任何异常 -> 出现异常 -> 重置
            out.writerIndex(release);
        } finally {

            // 清空 数组
            body = null;
            // 移除这个 MessagePack
            MessagePackPool.removePack();
        }
    }
}
```



编码器其实是难的 , 需要遍历迭代 , 需要对各种异常进行预知.

```java
/**
 * 解码器会很麻烦
 * <p>
 * 主要分为 4种情况
 * <p>
 * 1. 缓冲区只有一个数据包,此时只用做 版本校验 , 长度校验 , 然后读就可以了
 * 2. 缓冲区有多个数据包 , 可能是整数的倍数 , 就需要迭代读取
 * 3. 缓冲区可能有多个数据包 , 可能出现半个包的问题, 比如 2.5个 包, 此时就需要解码时注意
 * 4. 如果出现半个+整数个, 前面根本无法解码 , 此时就无法处理 , 可能出现丢包
 * <p>
 * 所以我们要求的是数据传输的完整性,最低要求将数据包完整的传输和接收
 *
 * @date:2019/11/10 13:40
 * @author: <a href='mailto:fanhaodong516@qq.com'>Anthony</a>
 */

public final class PackageDecoder extends ByteToMessageDecoder {

    /**
     * 默认值是 {@link com.chat.core.netty.Constants#PROTOCOL_VERSION}
     */
    private final short VERSION;

    /**
     * 构造方法
     */
    public PackageDecoder(short version) {
        super();
        this.VERSION = version;
    }

    /**
     * {@link ByteToMessageDecoder#channelRead(io.netty.channel.ChannelHandlerContext, java.lang.Object)}
     * <p>
     * 解码器
     */
    @Override
    protected void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) throws Exception {

        // 获取当前线程的 MessagePack
        MessagePack pack = MessagePackPool.getPack();

        try {
            handler(in, pack, out);
        } finally {
            // 移除
            MessagePackPool.removePack();
        }

    }

    /**
     * 处理器 - 主要处理逻辑
     */
    private void handler(ByteBuf in, MessagePack pack, List<Object> out) {

        // 如果可读
        while (in.isReadable()) {

            // 1. 记录最开始读取的位置 , 防止出错
            int release = in.readerIndex();

            // 2. 读取版本号 , 2个字节
            short version = in.readShort();

            // 3. 版本号一致 - > 继续执行
            if (version == VERSION) {

                // 4. 解码 -> 操作
                NPack read = null;
                byte[] bytes = null;

                try {

                    // 5. 读取长度 , 4个字节
                    int len = in.readInt();

                    // 6. 实例化数组
                    bytes = new byte[len];

                    // 7. 读取到数组中 , 此时可能会有异常 - > 我们抓住 indexOutOfBoundException
                    in.readBytes(bytes, 0, len);

                    // 8. 如果么问题, 就进行解码 -> 也可能出现异常 -> 抓取异常
                    read = pack.read(bytes, NPack.class);

                    // catch 抓取任何异常
                } catch (Throwable e) {
                    // 不做任何处理
                } finally {

                    // 清空数组引用 - 快速释放内存
                    bytes = null;
                }

                // 解码错误-> 重置读指针位置 -> 返回
                if (read == null) {
                    in.readerIndex(release);
                    return;
                } else {

                    // 一致就添加进去 - > 啥也不做
                    out.add(read);
                }
            } else {

                // 版本不一致 -> 重置读指针位置 -> 返回
                in.readerIndex(release);
                return;
            }
        }
    }

}
```



 