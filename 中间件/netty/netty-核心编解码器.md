# Netty - 核心编解码器

> ​	netty 最核心的部分就是掌握 ByteBuf 的用法 ,原理 , 业务逻辑主要是写ChannelHandler , 以及各种编解码器
>

## 1. ChannelInboundHandlerAdapter & ChannelOutboundHandlerAdapter  - 核心实现类

> ​	这俩实现类都是继承了 `io.netty.channel.ChannelHandlerAdapter` 类 , 基本实现都需要用户去实现, 所以他基本上对于用户来说是透明的, 我们开发可以控制我们全部的输入输出, 对对象进行释放等等

### 1. API 方法

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-18/94ac8bd7-5765-4c47-9b91-7b2abff8f4fd.jpg?x-oss-process=style/template01)

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-18/51866bd2-59f8-432c-9598-8822e7e0de53.jpg?x-oss-process=style/template01)

```java
// 判断是否和其他管道共享数据 ,每一个客户端连接会申请一个管道, 就和和别人共享
isSharable
// 每一个客户端只会注册一次handler
handlerAdded
// 每一个客户端只会一次注册
channelRegistered
// 活跃
channelActive

// 接收客户端信息
channelRead -> write/flush操作

// out输出流处理
out : write
out : flush   

// 已经写出去了 ....
channelReadComplete

// 断开 ....
channelInactive
channelUnregistered
handlerRemoved
```

### 1. exceptionCaught() 和  handlerRemoved()  事件何时触发

1 . **exceptionCaught()** : 异常关闭,比如关闭客户端或者服务器端 : 手动关闭

2 . **handlerRemoved()** :  正常关闭执行 , 比如执行 `ctx.close()`

所以一般场景是 :

1)  服务器端执行了`ctx.close()` , 此时客户端应当在 `handlerRemoved()`中执行被关闭的业务逻辑,同时服务器端也是 ,  当服务器异常关闭的时候 , 客户端的`exceptionCaught()`事件会触发

2)  当客户端执行了`ctx.close()`  , 服务器端的`handlerRemoved()`事件就会触发,  当客户端异常关闭,服务器端的`exceptionCaught()`事件会触发

所以异常关闭事件时 `exceptionCaught()` , 正常关闭事件时 `handlerRemoved()`

### 2. ctx.writeAndFlush(msg)  与 ctx.channel().writeAndFlush(msg) 区别

1)  `ChannelHandlerContext.writeAndFlush(msg);`

​	他的意思就是比如我的`pipeline.addlast(out1,in,out2)` ,此时比如in执行了`ChannelHandlerContext.writeAndFlush(msg);` ,此时解码器只会走out1 , 就是从开头到in, 也就是执行 `out1.write -> ou1.flush`

2) `ChannelHandlerContext.channel().writeAndFlush(msg);`

​	而这个呢, 还是上面种情况 , 此时我in输出改成了 `ChannelHandlerContext.channel().writeAndFlush(msg);` 此时会从开头到最后走一遍 , 也就是会从 `out1.write -> out2.write -> ou1.flush -> ou2.flush` 

### 3.  ctx.write() 和 ctx.flush() 和 ctx.writeAndFlush()

write就是写 ,flush就是推到缓冲区发出去

`ctx.write()` 会调用输出流的 `write`方法  , 同时 flush也是 , 然后 `writeAndFlush`是两者的结合体,俩都执行

## 2. MessageToByteEncoder<T> 与 ByteToMessageDecoder 编解码器

这俩类实现了上面我们刚刚提到的俩编解码器的实现, 所以他基本上封装了他主要的方法 , 所以我们主要关注`io.netty.handler.codec.ByteToMessageDecoder#channelRead` 这个方法

### 1. MessageToByteEncoder<T> 编码器

> ​	ChannelOutboundHandlerAdapter which encodes message in a stream-like fashion from one message to an ByteBuf.
>
> 看名字我们就知道他是将 发送出去的信息 转换成 `ByteBuf` 对象

由于我们执行写出操作 , 比如 ctx.write() 或者 writeAndFlush操作, 会调用输出流的write方法 

```java
@Override
    public void write(ChannelHandlerContext ctx, Object msg, ChannelPromise promise) throws Exception {
        ByteBuf buf = null;
        try {
            if (acceptOutboundMessage(msg)) {
                @SuppressWarnings("unchecked")
                // 我们发送的消息
                I cast = (I) msg;
                // 分配内存
                buf = allocateBuffer(ctx, cast, preferDirect);
                try {
                    // 解码-> 交给我们去写
                    encode(ctx, cast, buf);
                } finally {
                    ReferenceCountUtil.release(cast);
                }

                if (buf.isReadable()) {
                    ctx.write(buf, promise);
                } else {
                    buf.release();
                    ctx.write(Unpooled.EMPTY_BUFFER, promise);
                }
                buf = null;
            } else {
                //ChannelOutboundInvoker 处理链
                ctx.write(msg, promise);
            }
        } catch (EncoderException e) {
            throw e;
        } catch (Throwable e) {
            throw new EncoderException(e);
        } finally {
            // 他会帮助我们释放一次,所以不需要我们手动释放我们输出的对象
            if (buf != null) {
                buf.release();
            }
        }
    }

public class IntegerEncoder extends MessageToByteEncoder<Integer> {
    @Override
   public void encode(ChannelHandlerContext ctx, Integer msg, ByteBuf out)
           throws Exception {
       out.writeInt(msg);
   }
}
```

### 2. ByteToMessageDecoder  解码器 (重点)

他重写了 `ChannelInboundHandlerAdapter`  类 , 重写了父类好多方法 ,我们主要关注 `io.netty.handler.codec.ByteToMessageDecoder#channelRead` 这个方法

基本核心的文档内容在下面 : 

> ​	ChannelInboundHandlerAdapter which decodes bytes in a stream-like fashion from one ByteBuf to an other Message type.
>
>   就是将输入进来的`ByteBuff`转换成我们想要的数据类型对象 , 添加到他的集合中



>  ​	Generally frame detection should be handled earlier in the pipeline by adding a DelimiterBasedFrameDecoder, FixedLengthFrameDecoder, LengthFieldBasedFrameDecoder, or LineBasedFrameDecoder.
>
> 通常，通过添加**DelimiterBasedFrameDecoder**、**FixedLengthFrameDecoder**、**LengthFieldBasedFrameDecoder**或**LineBasedFrameDecoder**，可以在管道中更早地处理帧检测。
>

`io.netty.handler.codec.ByteToMessageDecoder#channelRead`

```java
@Override
public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
    // 判断 , 他是不是ByteBuf 类型
    if (msg instanceof ByteBuf) {
        CodecOutputList out = CodecOutputList.newInstance();
        try {
            // 转换一下
            ByteBuf data = (ByteBuf) msg;
            first = cumulation == null;
            if (first) {
                // 第一次就是我们的输入对象 , cumulation是一个ByteBuf对象
                cumulation = data;
            } else {
                // 以后就是在原来基础上的ByteBuf,处理未消费的可能
                cumulation = cumulator.cumulate(ctx.alloc(), cumulation, data);
            }
            // 调用解码方法
            callDecode(ctx, cumulation, out);
        } catch (DecoderException e) {
            throw e;
        } catch (Exception e) {
            throw new DecoderException(e);
        } finally {
            // 读完就要释放 ,所以一般不需要我们手动释放
            if (cumulation != null && !cumulation.isReadable()) {
                numReads = 0;
                cumulation.release();
                cumulation = null;
            } else if (++ numReads >= discardAfterReads) {
               ...
            }
            ...
        }
    } else {
        // 传递给下一个
        ctx.fireChannelRead(msg);
    }
}
```

`io.netty.handler.codec.ByteToMessageDecoder#callDecode`  方法 , 其中`....`代表忽略,太简单了

```java
protected void callDecode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) {
    try {
        // 当可读一直执行 , 就是读指针<写指针
        while (in.isReadable()) {
            // 一直观察out数组是否有数据
            int outSize = out.size();
            if (outSize > 0) {
                // 有就传递给下一个处理器
                fireChannelRead(ctx, out, outSize);
                // 清空out数组
                out.clear();
               	... 
                //顺便设置一下
                outSize = 0;
            }

            int oldInputLength = in.readableBytes();
            
            // 调用方法
            decodeRemovalReentryProtection(ctx, in, out);
          
           	...
        }
    } catch (Exception cause) {
        ....
    }
}
```

`io.netty.handler.codec.ByteToMessageDecoder#decodeRemovalReentryProtection` 方法

```java
final void decodeRemovalReentryProtection(ChannelHandlerContext ctx, ByteBuf in, List<Object> out){
        decodeState = STATE_CALLING_CHILD_DECODE;
        try {
            // 执行解码过程 , 就是我们重写的方法
            decode(ctx, in, out);
        } finally {
          ....
        }
    }
```

`io.netty.handler.codec.ByteToMessageDecoder#decode` 抽象方法需要我们去实现

```java
protected abstract void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) throws Exception;
```



## 3. MessageToMessageDecoder<T> 和 MessageToMessageEncoder<T> 编解码器

### 1. MessageToMessageEncoder<T>  编码器

> ​	ChannelOutboundHandlerAdapter which encodes from one message to an other message For example here is an implementation which decodes an Integer to an String.
>
> ​	他继承了 `ChannelOutboundHandlerAdapter` 类 , 所以是一个输出流  , 编码器 , 
>
> ​	这里的泛型指的是输入端  , out里添加输出 , 这个不管你是啥, 一般情况都是ByteBuf对象

```java
public class IntegerToStringEncoder extends
       MessageToMessageEncoder<Integer> {

    // 差不多,很简单
   @Override
   public void encode(ChannelHandlerContext ctx, Integer message, List<Object> out)
           throws Exception {
       out.add(message.toString());
   }
}
```

### 2. MessageToMessageDecoder<T>  解码器

> ​	这里的泛型指的是输入端 , 输出中添加你想添加的对象

重写下面的方法就行 ,下面这个例子表示的是输入是String, 输出是Int 类型

```java
public class StringToIntegerDecoder extends
       MessageToMessageDecoder<String> {

    @Override
   public void decode(ChannelHandlerContext ctx, String message,
                      List<Object> out) throws Exception {
       out.add(message.length());
   }
}
```



## 4 . SimpleChannelInboundHandler<T>

T是我们已经解码后的类型 , 他会将解码后的类型传递给我们 , 

同时他并不需要我们手动释放, 他实现了`ChannelInboundHandlerAdapter` 类 , 

基本上他是不需要我们做类型转换的 ,专注于实现我们的方法`channelRead0`

```java
@Override0
public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
	// 每次都会release初始化为true
    boolean release = true;
    try {
        // 判断类型,因为SimpleChannelInboundHandler有一个泛型T, 就是判断和他记录的类型是否相同
        if (acceptInboundMessage(msg)) {
            I imsg = (I) msg;
            // 这个就是我们写的业务逻辑
            channelRead0(ctx, imsg);
        } else {
            release = false;
            // 不是类型就传递给下一个
            ctx.fireChannelRead(msg);
        }
    } finally {
    	// 最后会手动给我们释放 ,所以我们不需要人工去释放,会造成不必要的浪费
        if (autoRelease && release) {
            ReferenceCountUtil.release(msg);
        }
    }
}
```
