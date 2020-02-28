# 深入学习 ByteToMessageDecoder

> ​		对于Netty来说, 他的网络模型属于一个NIO. 就selector的多路复用.  入口在 `io.netty.channel.nio.NioEventLoop`  这个玩意实现了了nio的`Executor` , 基本上可以认为是一个线程池,封装的. 

核心在 `io.netty.channel.nio.NioEventLoop#run` 这个run方法里面. 

首先看 `selectStrategy`  他就 三个状态 `SELECT` , `CONTINUE` , `BUSY_WAIT` , 明白了三个状态

```java
protected void run() {
    for (;;) {
       /// 处理selectkey .->read()方法=>   拿到 pipeline - >pipeline.fireChannelRead(byteBuf) ,基本流程就是这个 . 
    }
}
```

那么 pipeline . 就是我们自己加入的 责任链模式进行处理. 



我们看看今天的主角 . `ByteToMessageDecoder`

我们知道网络传输中会有大量的粘包, 半包等现象, 服务端如何去处理和解决呢. 这就是一个难题.  我们需要一个缓冲量, 也就是将没有读完的缓冲起来. 



我们看看 `ByteToMessageDecoder` 如何做的.  我们知道 read方法的入口函数, 

```java
public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
    // 类型转换不说了
    if (msg instanceof ByteBuf) {
        // 这个是一个输出类型 , 一个装有输出对象的集合.
        CodecOutputList out = CodecOutputList.newInstance();
        try {
            // 类型转换
            ByteBuf data = (ByteBuf) msg;
            // 如果缓冲量为空? , 我们就直接使用拿来的数据
            first = cumulation == null;
            if (first) {
                cumulation = data;
            } else {
                // 否则添加缓冲量 (代码段一)
                cumulation = cumulator.cumulate(ctx.alloc(), cumulation, data);
            }
            // 调用我们写的解码器 (代码段三)
            callDecode(ctx, cumulation, out);
        } catch (DecoderException e) {
            throw e;
        } catch (Exception e) {
            throw new DecoderException(e);
        } finally {
            // 如果读完了. 直接清空引用
            if (cumulation != null && !cumulation.isReadable()) {
                numReads = 0;
                cumulation.release();
                cumulation = null;

            // 他这里是一个防止OOM的问题. 不断的appen.会造成Bytebuf长度不断的增加.执行16次以后会执行丢弃掉已读的部分.也就是重新实例化一个Bytebuf.
            } else if (++ numReads >= discardAfterReads) {
                numReads = 0;
                discardSomeReadBytes();
            }

            // 我们处理的.
            int size = out.size();
            decodeWasNull = !out.insertSinceRecycled();
            // 发送给下一个处理器.
            fireChannelRead(ctx, out, size);
            // 回收这个list.
            out.recycle();
        }
    } else {
        ctx.fireChannelRead(msg);
    }
}
```



代码段一 : 

```java
public static final Cumulator MERGE_CUMULATOR = new Cumulator() {
    @Override
    // 第一个是一个buf生成器. 第二个是缓冲量. 第三个是我们这次收到的数据包
    public ByteBuf cumulate(ByteBufAllocator alloc, ByteBuf cumulation, ByteBuf in) {
        final ByteBuf buffer;
        // 如果 c.w+in.len>c.capacity ? 这时候代表不足了. 就需要扩容.
        if (cumulation.writerIndex() > cumulation.maxCapacity() - in.readableBytes()
            // 或者计数器大于一了.防止内存泄漏. 或者是只读对象.
                || cumulation.refCnt() > 1 || cumulation.isReadOnly()) {
            // 此时就需要申请一块内存 (代码块二)
            buffer = expandCumulation(alloc, cumulation, in.readableBytes());
        } else {
            buffer = cumulation;
        }
        // 写入此次收到的数据包
        buffer.writeBytes(in);
        // 释放收到的数据包对象
        in.release();
        // 返回
        return buffer;
    }
};
```

代码块二 :  

```java
static ByteBuf expandCumulation(ByteBufAllocator alloc, ByteBuf cumulation, int readable) {
    // 第一旧的缓冲区先拿到
    ByteBuf oldCumulation = cumulation;
    // 创建一个新的缓冲区. 新的缓冲区是原来可读部分加上(w-r) 这次接收的数据包可读部分
    cumulation = alloc.buffer(oldCumulation.readableBytes() + readable);
    // 写入原来的缓冲区
    cumulation.writeBytes(oldCumulation);
    // 释放原来的缓冲区(释放内存)
    oldCumulation.release();
    // 返回新缓冲区
    return cumulation;
}
```



代码块三 

```java
protected void callDecode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) {
    try {
        while (in.isReadable()) {
            // 记录
            int outSize = out.size();
            if (outSize > 0) {
                // 告诉下一个人
                fireChannelRead(ctx, out, outSize);
                out.clear();
                if (ctx.isRemoved()) {
                    break;
                }
                outSize = 0;
            }

            // 记录可读长度
            int oldInputLength = in.readableBytes();
            // 我们自己的解码器
            decodeRemovalReentryProtection(ctx, in, out);

            // 防止读者读者.这个ctx已经被移除
            if (ctx.isRemoved()) {
                break;
            }
            if (outSize == out.size()) {
                // 完全没动.直接放弃
                if (oldInputLength == in.readableBytes()) {
                    break;
                } else {
                    continue;
                }
            }

            // 
            if (isSingleDecode()) {
                break;
            }
        }
    } catch (DecoderException e) {
        throw e;
    } catch (Exception cause) {
        throw new DecoderException(cause);
    }
}
```



```java
final void decodeRemovalReentryProtection(ChannelHandlerContext ctx, ByteBuf in, List<Object> out)
        throws Exception {
    decodeState = STATE_CALLING_CHILD_DECODE;
    try {
        // 我们的解码器.需要做的.
        decode(ctx, in, out);
    } finally {
        boolean removePending = decodeState == STATE_HANDLER_REMOVED_PENDING;
        decodeState = STATE_INIT;
        if (removePending) {
            handlerRemoved(ctx);
        }
    }
}

```

