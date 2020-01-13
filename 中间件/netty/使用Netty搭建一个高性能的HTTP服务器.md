# Netty - 做一个Http服务器

> ​	我们知道Spring的webflux默认实现的netty做的服务器, 因为我也很想自己做一个玩玩 , netty 提供了http编解码器这个不需要用户去写, 只要写后续逻辑, 所以还是很方便的 , 等我后期做一个实现了springnvc的无缝实现



> ​	里面有两个核心的解码器`io.netty.handler.codec.http.HttpServerCodec` 和 `io.netty.handler.codec.http.HttpObjectAggregator`, 实现原理可以看看源码, 有兴趣的可以看看
>
> 其中第一个是Http对象的编解码器, 第二个是限制请求体的解码器.

### 1. 依赖

```xml
<dependency>
    <groupId>io.netty</groupId>
    <artifactId>netty-all</artifactId>
    <version>4.1.36.Final</version>
</dependency>

<dependency>
    <groupId>org.slf4j</groupId>
    <artifactId>slf4j-api</artifactId>
    <version>1.7.21</version>
</dependency>

<dependency>
    <groupId>org.slf4j</groupId>
    <artifactId>slf4j-log4j12</artifactId>
    <version>1.7.21</version>
</dependency>

// 同时记得加入 log4j.properties文件
```

### 2. 服务器代码

抽象接口

```java
public abstract class Server {

    public abstract void start() throws Exception;

    abstract void destroy() throws Exception;
}
```

核心类: 

```java
public class HttpServer extends Server {

    private static final Logger logger = LoggerFactory.getLogger(HttpServer.class);

    private EventLoopGroup bossGroup;
    private EventLoopGroup workerGroup;

    private final String host;
    private final int port;
    
    
    // BIND IP
    public HttpServer(String host, int port) {
        this.host = host;
        this.port = port;
    }

    /**
     * 启动
     * @throws Exception
     */
    @Override
    public void start() throws Exception {

        // 事件循环组
        bossGroup = new NioEventLoopGroup();
        workerGroup = new NioEventLoopGroup(Runtime.getRuntime().availableProcessors()
                , new NamedThreadFactory("http-server"));

        // 启动器
        ServerBootstrap bootstrap = new ServerBootstrap();
		
        // 自己的处理器
        final HttpProcessHandler handler = new HttpProcessHandler();

        bootstrap.group(bossGroup, workerGroup)
                // NioServerSocketChannel 处理器
                .channel(NioServerSocketChannel.class)
                // 子处理器
                .childHandler(new ChannelInitializer<Channel>() {
                    @Override
                    protected void initChannel(Channel channel) {
                        ChannelPipeline pipeline = channel.pipeline();
                        
                        // 解码器 一 : HTTP解码器,和编码器
                        pipeline.addLast(new HttpServerCodec());
                        
                        // 解码器 二 : 设置请求体多大
                        pipeline.addLast(new HttpObjectAggregator(1024));
                        
                        // 解码器 三  : 自定义的
                        pipeline.addLast(handler);
                    }
                })
                // 子选项 , 数据连接成更大的报文来最小化所发送的报文的数量 , 心跳检测
                .childOption(ChannelOption.TCP_NODELAY, true)
                .childOption(ChannelOption.SO_KEEPALIVE, true);
        try {
            
            // bind端口
            ChannelFuture future = bootstrap.bind(host, port).sync();

            // 打印日志
            logger.info("服务器启动成功 , host is {} , port is {}.", host, port);
            
            // 关闭管道
            future.channel().closeFuture().sync();
        } finally {
            logger.info("服务器关闭, host is {} , 8087 is {}.", host, port);
            // 销毁
            destroy();
        }
    }

    @Override
    protected void destroy() throws Exception{
        if (workerGroup != null) {
            workerGroup.shutdownGracefully();
        }
        if (bossGroup != null) {
            bossGroup.shutdownGracefully();
        }
    }
}
```



### 3. 自定义处理器代码

```java
public class HttpProcessHandler extends SimpleChannelInboundHandler<FullHttpRequest> {

    /**
     * 或者HttpProcessHandler 类加入 @ChannelHandler.Sharable注解
     * @return
     */
    @Override
    public boolean isSharable() {
        return true;
    }

    @Override
    protected void channelRead0(ChannelHandlerContext channelHandlerContext, FullHttpRequest fullHttpRequest) throws Exception {
        // 请求远程地址
        SocketAddress address = channelHandlerContext.channel().remoteAddress();
        
        // 请求方法
        fullHttpRequest.method();

        // 请求URI
        fullHttpRequest.uri();


        // 请求头
        fullHttpRequest.headers();

        // 请求体
        fullHttpRequest.content();


        String json = "[\"a\",\"b\",\"c\"]";
        
        // 响应
        DefaultHttpResponse response = new DefaultFullHttpResponse(HTTP_1_1, HttpResponseStatus.OK, Unpooled.copiedBuffer(json, CharsetUtil.UTF_8));
        response.headers().add(HttpHeaderNames.CONTENT_LENGTH, json.getBytes().length);
        response.headers().add(HttpHeaderNames.CONTENT_TYPE, "application/json");


        // 不需要解码器直接写出去
        channelHandlerContext.writeAndFlush(response);
    }
	
    // 直接写出去 , 会往上找
    @Override
    public void channelReadComplete(ChannelHandlerContext ctx) {
        ctx.flush();
    }
}
```



### 4. 启动测试

```java
public class Boot {
    public static void main(String[] args) {
        Server httpServer = new HttpServer("0.0.0.0", 8888);
        try {
            httpServer.start();
        } catch (Exception e) {
            //
        }
    }
}
```

```shell
[D:\~]$ curl -X GET "http://localhost:8888/aaa"
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100    13  100    13    0     0     13      0  0:00:01 --:--:--  0:00:01   276
["a","b","c"]
```

