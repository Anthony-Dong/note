# netty 学习

## netty 服务端实现流程 

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-25/6fa70a06-5229-4881-903f-65cc85d10775.jpg?x-oss-process=style/template01)

> ​	实现代码 :
>

```java
		// 1、创建事件循环组 (线程模型,初始化线程的)
        NioEventLoopGroup bossGroup = new NioEventLoopGroup();
        NioEventLoopGroup workerGroup = new NioEventLoopGroup();
        try {

		// 2、创建一个服务器启动器
            ServerBootstrap bootstrap = new ServerBootstrap();

		// 	3、主要逻辑
            bootstrap.group(bossGroup, workerGroup).
                // 服务器
                    channel(NioServerSocketChannel.class).
                // 服务器端配置
               		handler(new LoggingHandler(LogLevel.INFO)).
                // 连接配置
                    childHandler(new TestInitializer());
            //绑定一个端口并且同步，生成一个ChannelFuture对象  future 对象是对线程执行结果的封装     			 // sync  Waits for this future until it is done,
            ChannelFuture channelFuture = bootstrap.bind(8899).sync();
            // 也可以添加一个listener 
            //关闭连接的channel.(与future关联deio操作的channel) 也可以添加一个listener
            channelFuture.channel().closeFuture().sync();
        } finally {
            // 关闭线程组
            bossGroup.shutdownGracefully();
            workerGroup.shutdownGracefully();
        }

```



![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-25/ab0f4efe-df85-4e75-9b36-6e0043865fc6.jpg?x-oss-process=style/template01)

```java
public class NettyClient {


    public static void main(String[] args) {
        // 创建一个事件循环组
        NioEventLoopGroup worker = new NioEventLoopGroup();
        
        // 创建一个启动器
        Bootstrap bootstrap = new Bootstrap();
        // 放入 配置 NioSocketChannel  MyChannaleInitializer(配置pipeline 其实就是 处理流 分为 inbound 的 outbound 
        bootstrap.group(worker).channel(NioSocketChannel.class).handler(new MyChannaleInitializer());

        try {
            // 事件回调 
            ChannelFuture channelFuture = bootstrap.connect("localhost", 8899).addListener(new GenericFutureListener<Future<? super Void>>() {
                @Override
                public void operationComplete(Future<? super Void> future) throws Exception {
                    if (future.isSuccess()) {
                        System.out.println("客户端启动成功");
                    }
                }
            });

        } catch (Exception e) {
            e.printStackTrace();
        }finally {
           // worker.shutdownGracefully();
        }
    }
  private static class MyChannaleInitializer extends ChannelInitializer<SocketChannel>{

        @Override
        protected void initChannel(SocketChannel ch) throws Exception {
            // 配置 
            ChannelPipeline pipeline = ch.pipeline();
            pipeline.addLast(new MyByteToMessageDecoder());
            pipeline.addLast(new MyMessageToByteEncoder());
            pipeline.addLast(new  MySimpleChannelInboundHandler());
        }
    }

    private static class MySimpleChannelInboundHandler extends SimpleChannelInboundHandler<Long> {
      @Override
        public void channelRegistered(ChannelHandlerContext ctx) throws Exception {
            System.out.println("发送成功");

        }
        @Override
        protected void channelRead0(ChannelHandlerContext ctx, Long msg)  {
            System.out.println("接收到服务器端 : "+msg );

        }
        @Override
        public void channelActive(ChannelHandlerContext ctx) throws Exception {
            ctx.writeAndFlush(123l);
        }
    }
}

```







##  Future - ChannelFuture

> ​	The result of an asynchronous operation.  异步操作的返回结果. 就是一个线程操作的回调结果 [future链接](https://anthony-dong.github.io/post/executor-xian-cheng-chi/)
>
> ​	public interface Future<V> extends java.util.concurrent.Future<V> {}
>
> - 我们发现他是继承 juc中的future 的 , juc的future 可以看我的这个 文章 [连接](https://anthony-dong.github.io/post/executor-xian-cheng-chi/)
> - 同时 future 也拓展了一些方法 (这里我就大致的去描述,因为源码文档真的很详细)
>   - boolean isSuccess();   返回trur,只有当io操作成功时
>   - boolean isCancellable();    取消
>   - Future<V> addListener(GenericFutureListener<? extends Future<? super V>> listener);   添加监听器,这个很重要的,当这个io操作完成时,会立刻通知,
>   - Future<V> sync() throws InterruptedException;    Waits for this future until it is done, 等待这个结果知道这个结果已经完成
>   - Future<V> await()  等待future完成 .线程阻塞,可能发生死锁 ,不建议使用



```java
serverBootstrap.bind(BEGIN_PORT).addListener(new GenericFutureListener<Future<? super Void>>() {
            @Override
            public void operationComplete(Future<? super Void> future) throws Exception {
                // 判断 future的状态 :
                //  1.
                if (future.isSuccess()) {
                    System.out.println("端口[" + port + "]绑定成功!");
                } else {
                    System.err.println("端口[" + port + "]绑定失败!");
                    //  切换绑定的地址 ---------
                    bind(serverBootstrap, port + 1);
                }
            }
 });
```



我们看看 他的接口 **ChannelFuture**

下面这个文档描述的是 future已经完成状态返回结果 和 未完成的结果,很详细

```java
 *                                      +---------------------------+
 *                                      | Completed successfully    |
 *                                      +---------------------------+
 *                                 +---->      isDone() = true      |
 * +--------------------------+    |    |   isSuccess() = true      |
 * |        Uncompleted       |    |    +===========================+
 * +--------------------------+    |    | Completed with failure    |
 * |      isDone() = false    |    |    +---------------------------+
 * |   isSuccess() = false    |----+---->      isDone() = true      |
 * | isCancelled() = false    |    |    |       cause() = non-null  |
 * |       cause() = null     |    |    +===========================+
 * +--------------------------+    |    | Completed by cancellation |
 *                                 |    +---------------------------+
 *                                 +---->      isDone() = true      |
 *                                      | isCancelled() = true      |
 *                                      +---------------------------+
     
```





## bytebuf  核心的数据传输对象

> ​	bytebuf提供了两个指针变量 去支持有序的读写操作 (readerIndex) 和 (writerIndex)  .
>
> ​	其实现的**ReferenceCounted** 是一个引用计数接口,实现垃圾回收的,他不是由GC控制的,所以使用时注意不然会出现内存溢出的现象
>
> ​	**当一个对象初始化完成后，它的reference count 是1, 调用retain()加1，调用release()减1并且如果reference count是0的话就释放。使用被释放掉的对象将会报错。** 
>
> 数据结构 :   R+W指针 分为 三块区域 { discardable bytes,  readable bytes,writable bytes}

```java
 * {@link ByteBuf} provides two pointer variables to support sequential
 * read and write operations - {@link #readerIndex() readerIndex} for a read
 * operation and {@link #writerIndex() writerIndex} for a write operation
 * respectively.  The following diagram shows how a buffer is segmented into
 * three areas by the two pointers:

* When a new {@link ReferenceCounted} is instantiated, it starts with the reference count of {@code 1}. -- 所以需要 release释放

public abstract class ByteBuf implements ReferenceCounted, Comparable<ByteBuf> 

 *      +-------------------+------------------+------------------+
 *      | discardable bytes |  readable bytes  |  writable bytes  |
 *      |  可删除的字节       |     (CONTENT)    |    可写的字节      |
 *      +-------------------+------------------+------------------+
 *      |                   |                  |                  |
 *      0      <=      readerIndex   <=   writerIndex    <=    capacity
 readerIndex 和 writerIndex 初始位置在 0 ,随着读写改变
 
 
 *  BEFORE discardReadBytes()
 *
 *      +-------------------+------------------+------------------+
 *      | discardable bytes |  readable bytes  |  writable bytes  |
 *      +-------------------+------------------+------------------+
 *      |                   |                  |                  |
 *      0      <=      readerIndex   <=   writerIndex    <=    capacity
 *
 *	
     其实就是r指针 指向 0 ,read指针像左移
 *  AFTER discardReadBytes()
 *
 *      +------------------+--------------------------------------+
 *      |  readable bytes  |    writable bytes (got more space)   |
 *      +------------------+--------------------------------------+
 *      |                  |                                      |
 * readerIndex (0) <= writerIndex (decreased)        <=        capacity
 
```

- 使用 **bytebuf**  的技巧

```java
   		// 可以这样子
		ByteBuf buffer = Unpooled.copiedBuffer("hello world".getBytes());
		// 不设置 长度是 256
        ByteBuf buffer = Unpooled.buffer(10);
        for (int i = 0; i < 10; i++) {
            buffer.writeByte(i);

        }
        // R 和 w 都恢复到 0
        // buffer.clear();
        // 浅拷贝 修改会影响原数组
        ByteBuf duplicate = buffer.duplicate();
        duplicate.setByte(1, 10);

        // 深拷贝
        ByteBuf copy = buffer.copy();

        //转换成 nio byte buffer
        ByteBuffer byteBuffer = buffer.nioBuffer();
        

        // 根据字符集转换成字符串
        String s = buffer.toString(CharsetUtil.UTF_8);
      
Zuul
        // w>r
        while (buffer.isReadable()) {
            //  (ridx: 1, widx: 10, cap: 10) 由三个量组成
            System.out.println("buffer = " + buffer);

            // writerIndex - readerIndex;
            System.out.println("buffer.readableBytes() = " + buffer.readableBytes());

            // 整个相当于删除执行左移(其实就是复制) 就是使得 readindex始终处于0,w移动 ,但是这个会修改原来的bytebuf
            // System.out.println("buffer.discardReadBytes() = " + buffer.discardReadBytes());

            //capacity() - writerIndex;
            System.out.println("buffer.writableBytes() = " + buffer.writableBytes());

            //readerIndex = readerIndex + 1;
            System.out.println("buffer.readByte() = "+buffer.readByte());
            
            // 不会改变 指针
            System.out.println("buffer.getByte(1) = " + buffer.getByte(1));

        }

		
		  // 释放bytebuf对象
        buffer.release();
```





## pipeline -- **ChannelHandler**   

 **ChannelInitializer**

```java
  A special {@link ChannelInboundHandler} which offers an easy way to initialize a {@link Channel} once it was registered to its {@link EventLoop}.
      
     当一个channel注册到eventloop中时,ChannelInitializer提供一种很简单的方式去处理 , 用什么去处理 就是 ChannelHandler . 
      详细请看 ChannelHandler 源码文档 很详细 因为对象是变化的所以需要各种handler 
```

> ​	两个大的实现类
>
>  -	ChannelInboundHandler
>        -	StringDecoder  解码器  new StringDecoder(CharsetUtil.UTF_8)
>    
> -	ChannelOutboundHandler
>    - StringEncoder  编码器



## 高效的编码方式 Google Protocol

Java 中也可以实现 Serializable 接口来实现序列化，但由于它性能等原因在一些 RPC 调用中用的很少。

而 `Google Protocol` 则是一个高效的序列化框架

我们只需要在 pipeline 中添加这些就行了 

```java
.addLast(new ProtobufDecoder(BaseRequestProto.RequestProtocol.getDefaultInstance()))
.addLast(new ProtobufEncoder())
.addLast(new ProtobufVarint32FrameDecoder())
.addLast(new ProtobufVarint32LengthFieldPrepender())
    
ProtobufVarint32FrameDecoder 是什么 ? 
其实他就是 由 数据头(数据长度)+数据体组成  可以有效的解决拆包粘包问题
​```
 * BEFORE DECODE (302 bytes)       AFTER DECODE (300 bytes)
 * +--------+---------------+      +---------------+
 * | Length | Protobuf Data |----->| Protobuf Data |
 * | 0xAC02 |  (300 bytes)  |      |  (300 bytes)  |
 * +--------+---------------+      +---------------+
​```    
Varint 是一种紧凑的表示数字的方法。它用一个或多个字节来表示一个数字，值越小的数字使用越少的字节数。这能减少用来表示数字的字节数。 Varint 中的每个 byte 的最高位 bit 有特殊的含义，如果该位为 1，表示后续的 byte 也是该数字的一部分，如果该位为 0，则结束。其他的 7 个 bit 都用来表示数字。因此小于 128 的数字都可以用一个 byte 表示。大于 128 的数字，会用两个字节。
```

文章 [链接](https://www.cnblogs.com/tankaixiong/p/6366043.html)



## netty 常规option配置总结	

```java
1. 官方配置 :  一个是请求连接数 一个是心跳检测
.option(ChannelOption.SO_BACKLOG, 128)   // (5)
.childOption(ChannelOption.SO_KEEPALIVE, true) // (6)

ServerBootstrap serverBootstrap = new ServerBootstrap();
1.配置一 
serverBootstrap.option(ChannelOption.SO_BACKLOG, 1024);

2. 配置二
 serverBootstrap.option(ChannelOption.SO_REUSEADDR, true);

3. 配置三
 serverBootstrap.option(ChannelOption.TCP_NODELAY, true);

4. 配置四
  serverBootstrap.option(ChannelOption.SO_KEEPALIVE, false);

5. 配置五
  serverBootstrap.option(ChannelOption.SO_RCVBUF,1024*8);
  serverBootstrap.option(ChannelOption.SO_SNDBUF,1024*8);
```



- 配置一(**ChannelOption.SO_BACKLOG**) 就是三次握手

  > ​	服务器端TCP内核模块维护有2个队列，我们称之为A，B吧，客户端向服务端connect的时候，发送带有SYN标志的包（**第一次握手**），服务端收到客户端发来的SYN时，向客户端发送SYN ACK 确认(**第二次握手**)，此时TCP内核模块把客户端连接加入到A队列中，然后服务器收到客户端发来的ACK时（**第三次握手**）TCP把客户端连接从A队列移到B队列，连接完成，**应用程序的accept会返回**，也就是说**accept从B队列中取出完成三次握手的连接，A队列和B队列的长度之和是backlog**,当A，B队列的长之和大于backlog时，新连接将会被TCP内核拒绝，所以，如果backlog过小，可能会出现accept速度跟不上，A.B 队列满了，导致新客户端无法连接，要注意的是，backlog对程序支持的连接数并无影响，backlog影响的只是还没有被accept 取出的连接

- 配置二(**ChannelOption.SO_REUSEADDR**)  reused address  重复使用地址 

  > ​	**参数表示允许重复使用本地地址和端口， 比如，某个服务器进程占用了TCP的80端口进行监听，此时再次监听该端口就会返回错误，**
  > ​	使用该参数就可以解决问题，该参数允许共用该端口，这个在服务器程序中比较常使用，比如某个进程非正常退出，该程序占用的端口可能要被占用一段时间才能允许其他进程使用，而且程序死掉以后，内核一需要一定的时间才能够释放此端口，不设置SO_REUSEADDR 就无法正常使用该端口

- 配置三(**ChannelOption.TCP_NODELAY**) 

  > 解释：**是否启用Nagle算法，该算法将小的碎片数据连接成更大的报文来最小化所发送的报文的数量。** 
  >
  > 　　使用建议：如果需要发送一些较小的报文，则需要禁用该算法，从而最小化报文传输延时。只有在网络通信非常大时（通常指已经到100k+/秒了），设置为false会有些许优势，因此建议大部分情况下均应设置为true。

- 配置四(**SO_KEEPALIVE**)  

  > 　**解释：**是否使用TCP的心跳机制； 
  > **使用建议：** 心跳机制由应用层自己实现；

- 配置五 (**SO_SNDBUF**) 同 (**SO_RCVBUF**)

  > ​		**解释**： Socket参数，TCP数据发送缓冲区大小，即TCP发送滑动窗口，linux操作系统可使用命令：cat /proc/sys/net/ipv4/tcp_smem查询其大小。缓冲区的大小决定了网络通信的吞吐量（网络吞吐量=缓冲区大小/网络时延）。 
  >
  >   **使用建议**： 缓冲区大小设为网络吞吐量达到带宽上限时的值，即缓冲区大小=网络带宽*网络时延。以千兆网卡为例进行计算，假设网络时延为1ms，缓冲区大小=1000Mb/s * 1ms = 128KB。

- 





