# Netty 提供并发数

## 1. Netty的线程

我们知道 Netty服务端的写法 .

```java
NioEventLoopGroup boss = new NioEventLoopGroup();
NioEventLoopGroup work = new NioEventLoopGroup(Runtime.getRuntime().availableProcessors()*2);
```

**根据线程一般都是设置为CPU核数的两倍. **

实例化俩线程,然后交给ServerBootstrap . 那么为啥叫work线程了. 其实他是NIO模型. **boot线程就是一直在轮询事件,当连接来了, 交给work处理.  work线程是真正来用来处理当前连接的客户端的所以操作.**  

那么如果 10W个连接. 每个连接又有好多个请求. 那么这16个线程能处理吗, 万一遇见那种也别久的请求呢 ,难以处理呢. 



首先我们先来试试这些操作行不行.   我相信下面这段代码大家并不陌生.  这个是我们的服务器端 . 

```java
public class DemoServer {

    public static void main(String[] args) {

        NioEventLoopGroup boss = new NioEventLoopGroup(1,new NamedThreadFactory("boss"));
        NioEventLoopGroup work = new NioEventLoopGroup(Runtime.getRuntime().availableProcessors() * 2, new NamedThreadFactory("worker"));

        ServerBootstrap bootstrap = new ServerBootstrap();
        bootstrap.group(boss, work).channel(NioServerSocketChannel.class)
                .childHandler(new ChannelInitializer<NioSocketChannel>() {
                    @Override
                    protected void initChannel(NioSocketChannel ch) throws Exception {
                        ChannelPipeline pipeline = ch.pipeline();
                        // 字符串编码器.
                        pipeline.addLast(new StringEncoder());
                        // 每行行解码器.
                        pipeline.addLast(new LineBasedFrameDecoder(Integer.MAX_VALUE));
                        // 字符串解码器.
                        pipeline.addLast(new StringDecoder());
                        // 自己的Handler
                        pipeline.addLast(MyChannelDuplexHandler.INSTANCE);
                    }
                });
        ChannelFuture future = null;
        try {
            future = bootstrap.bind(10086).sync();
            System.out.println("bind success : localhost:10086");
            future.channel().closeFuture().sync();
        } catch (Exception e) {
            System.out.println(e.getMessage());
        } finally {
            boss.shutdownGracefully();
            work.shutdownGracefully();
        }
    }

    @ChannelHandler.Sharable
    private static class MyChannelDuplexHandler extends ChannelDuplexHandler {
        // 写日志信息的. 
        final CharSink sink;
        MyChannelDuplexHandler() {
            sink = Files.asCharSink(new File("D:\\MyDesktop\\template\\log.txt"), Charset.forName("utf-8"), FileWriteMode.APPEND);
        }
        static ChannelDuplexHandler INSTANCE = new MyChannelDuplexHandler();

        @Override

        public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
               func(ctx, msg);
        }

        // 主要处理逻辑. 
        private void func(ChannelHandlerContext ctx, Object msg) throws IOException {
            String message = (String) msg;
            long start = Long.parseLong(message.trim());
            System.out.printf("thread : %s ,addr : %s , spend : %dms\n", Thread.currentThread().getName(), ctx.channel().remoteAddress().toString(), System.currentTimeMillis() - start);
            String format = LocalDateTime.now().format(DateTimeFormatter.ofPattern("YYYY-MM-dd hh:mm:ss SSS\n"));
            ctx.writeAndFlush(format);
            sink.write((System.currentTimeMillis() - start) + "\n");
        }


        @Override
        public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) throws Exception {
            ctx.close();
        }

        @Override
        public void handlerRemoved(ChannelHandlerContext ctx) throws Exception {
            //no ting
        }
    }
}
```



压测代码.  Golang写的

```go
// @date : 2020/2/2 21:16
// @author : <a href='mailto:fanhaodong516@qq.com'>Anthony</a>

/**
	todo
 */
package main

import (
	"fmt"
	"log"
	"net"
	"sync"
	"time"
)

func main() {
	var wg sync.WaitGroup

	var start = time.Now().UnixNano() / 1e6

	for x := 0; x < 50; x++ {
		wg.Add(1)
		go func() {
			conn(&wg)
		}()
	}
	wg.Wait()
	var end = time.Now().UnixNano() / 1e6
	fmt.Printf("耗时 : %d\n", end-start)
}

func conn(wg *sync.WaitGroup) {
	conn, e := net.Dial("tcp", "localhost:10086")
	if e != nil {
		log.Fatal(e)
	}
	var start = time.Now().UnixNano() / 1e6
	defer func() {
		wg.Done()
		conn.Close()
		var end = time.Now().UnixNano() / 1e6
		fmt.Printf("耗时 : %d\n", end-start)
	}()


	for x := 0; x < 500; x++ {
		fmt.Fprintf(conn, "%s \n", "hello world")
	}
}
```



scala 计算均值 . 

```scala
object Count {
	def main(args: Array[String]): Unit = {

		val count: Int =Source.fromFile("D:\\MyDesktop\\template\\log.txt").getLines().count((_: String) => true)

		val sum: Int = Source.fromFile("D:\\MyDesktop\\template\\log.txt").getLines().map(_.toInt).sum

		printf("总请求数 : %d  , 总耗时 :  %dms , 平均响应时间 : %dms\n", count, sum, sum / count)
	}
}
```



测试  50个客户端, 没人发送500次请求.  也就是处理 2.5w个请求响应和 50个客户端



输出  `15875` , 说明处理很严重, 存在严重的处理不了的现象.  说明 1W请求存在 无法处理的问题. 

```java
总请求数 : 15875  , 总耗时 :  9610317ms , 平均响应时间 : 605ms
```



为什么呢. 我们先模拟一下. 5个客户端发送请求的问题. 看输出

```java
thread : worker-thread-1 ,addr : /127.0.0.1:6235 , spend : 2ms
thread : worker-thread-2 ,addr : /127.0.0.1:6236 , spend : 3ms
thread : worker-thread-8 ,addr : /127.0.0.1:6233 , spend : 3ms
thread : worker-thread-1 ,addr : /127.0.0.1:6235 , spend : 4ms
thread : worker-thread-2 ,addr : /127.0.0.1:6236 , spend : 4ms
thread : worker-thread-3 ,addr : /127.0.0.1:6237 , spend : 4ms
thread : worker-thread-3 ,addr : /127.0.0.1:6237 , spend : 5ms
thread : worker-thread-7 ,addr : /127.0.0.1:6234 , spend : 6ms
thread : worker-thread-8 ,addr : /127.0.0.1:6233 , spend : 6ms
thread : worker-thread-7 ,addr : /127.0.0.1:6234 , spend : 6ms
```

我们发现所有的客户端与线程都是一一对应的关系 , 也就是说一个客户端会分配死了, 就是那个线程. 那就是worker线程.  那这个就是为啥慢的原因了. 他的每一个 客户端只绑定一个线程 ,这个线程还是死的.   

## 解决方式 一

根据 , Netty 分配请求的原则. 我们可以知道. 

```java
ChannelPipeline addLast(EventExecutorGroup group, ChannelHandler... handlers);
```

我们可以在 pipeline中添加. 一个 `EventExecutorGroup` . 来处理改Handler的请求.  所以我们就创建一个全局的`NioEventLoopGroup`  , 来放入进去. 我们继续测试.

```java
public static void main(String[] args) {

    NioEventLoopGroup boss = new NioEventLoopGroup(1,new NamedThreadFactory("boss"));
    NioEventLoopGroup work = new NioEventLoopGroup(Runtime.getRuntime().availableProcessors() * 2, new NamedThreadFactory("worker"));

    // 专门用来处理Handler处理器的. 
    NioEventLoopGroup handler = new NioEventLoopGroup(50, new NamedThreadFactory("handler"));
    ServerBootstrap bootstrap = new ServerBootstrap();
    bootstrap.group(boss, work).channel(NioServerSocketChannel.class)
            .childHandler(new ChannelInitializer<NioSocketChannel>() {
                @Override
                protected void initChannel(NioSocketChannel ch) throws Exception {
                    ChannelPipeline pipeline = ch.pipeline();
                    // 字符串编码器.
                    pipeline.addLast(new StringEncoder());
                    // 每行行解码器.
                    pipeline.addLast(new LineBasedFrameDecoder(Integer.MAX_VALUE));
                    // 字符串解码器.
                    pipeline.addLast(new StringDecoder());
                    // 自己的Handler
                    pipeline.addLast(handler,MyChannelDuplexHandler.INSTANCE);
                }
            });
    ChannelFuture future = null;
    try {
        future = bootstrap.bind(10086).sync();
	    System.out.println("bind success : localhost:10086");
        future.channel().closeFuture().sync();
    } catch (Exception e) {
        System.out.println(e.getMessage());
    } finally {
        boss.shutdownGracefully();
        work.shutdownGracefully();
    }
}
```

我们先模拟一下.   50客户端 2.5请求.  我们的handler 有 50个线程处理. 

再次请求 .  总请求数为 `2.5w` , 接收响应并且都处理了, 但是平均响应时间也很长. 处理很不恰当. 

我们统计三组数据. 

```java
总请求数 : 25000  , 总耗时 :  45855712ms , 平均响应时间 : 1834ms
```

```java
总请求数 : 25000  , 总耗时 :  48299098ms , 平均响应时间 : 1931ms
```

```java
总请求数 : 25000  , 总耗时 :  45203872ms , 平均响应时间 : 1808ms
```

大约响应处理时间大约在 2s左右. 所以很不理想. 

通常来说对于客户端连接多 . 请求少的情况下 . 可以选择这种. 但是当客户端少. 连接多这就容易出问题. 

所以我们采用线程池. 

## 解决方式2

我们在 `MyChannelDuplexHandler` 中添加 一个线程池来处理 . 

```java
@ChannelHandler.Sharable
private static class MyChannelDuplexHandler extends ChannelDuplexHandler {
    final CharSink sink;
    MyChannelDuplexHandler() {
        sink = Files.asCharSink(new File("D:\\MyDesktop\\template\\log.txt"), Charset.forName("utf-8"), FileWriteMode.APPEND);
    }
    // 添加一个线程池 . 来处理全部的处理逻辑.  
    static final ExecutorService service = Executors.newFixedThreadPool(50);

    static ChannelDuplexHandler INSTANCE = new MyChannelDuplexHandler();

    @Override

    public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
        service.execute(() -> {
            try {
                this.func(ctx, msg);
            } catch (IOException e) {
                e.printStackTrace();
            }
        });
    }

    private void func(ChannelHandlerContext ctx, Object msg) throws IOException {
        String message = (String) msg;
        long start = Long.parseLong(message.trim());
        // System.out.printf("thread : %s ,addr : %s , spend : %dms\n", Thread.currentThread().getName(), ctx.channel().remoteAddress().toString(), System.currentTimeMillis() - start);
        String format = LocalDateTime.now().format(DateTimeFormatter.ofPattern("YYYY-MM-dd hh:mm:ss SSS\n"));
        ctx.writeAndFlush(format);
        sink.write((System.currentTimeMillis() - start) + "\n");
    }


    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) throws Exception {
        ctx.close();
    }

    @Override
    public void handlerRemoved(ChannelHandlerContext ctx) throws Exception {
        //no ting
    }
}
```



我们再次压测. 

```java
总请求数 : 25000  , 总耗时 :  40851493ms , 平均响应时间 : 1634ms
```



再次修改 , 修改线程数. 为 100

```go
总请求数 : 25000  , 总耗时 :  41833614ms , 平均响应时间 : 1673ms
```





其实不管你多少个线程数. 首先就是线程需要初始化. 大量的线程切换. 会消耗很长的时间, 所以看如何做处理. 



## Dubbo 的做法



他的 ThreadPoll 是一个SPI 接口

`org.apache.dubbo.remoting.transport.dispatcher.WrappedChannelHandler#WrappedChannelHandler`

```java
executor = (ExecutorService) ExtensionLoader.getExtensionLoader(ThreadPool.class).getAdaptiveExtension().getExecutor(url);
```



一共有这几种  , 默认使用的是 `fixed` 也就是`org.apache.dubbo.common.threadpool.support.fixed.FixedThreadPool`

```go
fixed=org.apache.dubbo.common.threadpool.support.fixed.FixedThreadPool
cached=org.apache.dubbo.common.threadpool.support.cached.CachedThreadPool
limited=org.apache.dubbo.common.threadpool.support.limited.LimitedThreadPool
eager=org.apache.dubbo.common.threadpool.support.eager.EagerThreadPool
```

然后看看fix的实现

```java
@Override
public Executor getExecutor(URL url) {
    String name = url.getParameter(THREAD_NAME_KEY, DEFAULT_THREAD_NAME);
    int threads = url.getParameter(THREADS_KEY, DEFAULT_THREADS);
    int queues = url.getParameter(QUEUES_KEY, DEFAULT_QUEUES);
    return new ThreadPoolExecutor(threads, threads, 0, TimeUnit.MILLISECONDS,
            queues == 0 ? new SynchronousQueue<Runnable>() :
                    (queues < 0 ? new LinkedBlockingQueue<Runnable>()
                            : new LinkedBlockingQueue<Runnable>(queues)),
            new NamedInternalThreadFactory(name, true), new AbortPolicyWithReport(name, url));
}
```



如果队列大小为 0 , 那么就是一个 `SynchronousQueue`  , 否则为`LinkedBlockingQueue`  , 对于超出的策略, 他使用的是自定义策略. 自定义策略需要继承 `ThreadPoolExecutor.AbortPolicy` 类. 



说到底 都是 `ThreadPoolExecutor`  , 只是他的细节做得好罢了.  

默认是 thread 大小为 10个. 他会在你启动 `ServiceConfig` 的时候创建好那个url, 默认是 10个. 如果threads为空的话就是 200个.  queues为 0 ,  需要配置可以加 queues=1000 , 如果queues=-1 ,则是无界的. 



反正基本就是这么一个情况 , 一般是在业务方法中添加线程池 , 来处理的, 因为这个线程池可以很好的处理 . 不要使用Netty的. 



