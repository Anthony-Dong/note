# Netty - 系统学习

> ​	netty 最核心的部分就是掌握 ByteBuf 的用法 ,原理 , 业务逻辑主要是写ChannelHandler , 以及各种编解码器
>
> 其中快速入门可以看一下[官网](https://netty.io/wiki/user-guide-for-4.x.html):  https://netty.io/wiki/user-guide-for-4.x.html ,所以流程编写自己看文档吧

## 1. ChannelInboundHandlerAdapter & ChannelOutboundHandlerAdapter  - 核心实现类

### 1. API

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-18/94ac8bd7-5765-4c47-9b91-7b2abff8f4fd.jpg?x-oss-process=style/template01)

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-18/51866bd2-59f8-432c-9598-8822e7e0de53.jpg?x-oss-process=style/template01)

```java
// 判断是否和其他管道共享数据
isSharable
// 每一个客户端只会注册一次handler
handlerAdded
// 每一个客户端只会一次注册
channelRegistered
// 活跃
channelActive
// out指的是 outbound
out : read
//接收客户端信息
channelRead
PooledUnsafeDirectByteBuf(ridx: 0, widx: 5, cap: 1024)   
out : write
out : flush    
// 已经写出去了 ....
channelReadComplete

channelRead
PooledUnsafeDirectByteBuf(ridx: 0, widx: 5, cap: 1024)
channelReadComplete

channelRead
PooledUnsafeDirectByteBuf(ridx: 0, widx: 5, cap: 512)
channelReadComplete

// 结果 ....
channelInactive
channelUnregistered
handlerRemoved
```

### 1. exceptionCaught() 和  handlerRemoved()  关闭连接问题

**1 . exceptionCaught()** : 当服务器端异常关闭 ,举个例子: 当服务器端执行了 ctx.close() , 此时客户端这边的exceptionCaught() 是不会执行的 , 此时无法通过exceptionCaught () 方法去抓取关闭事件 .

 **2 . handlerRemoved() :  ** 当服务器端正常断开连接与客户端连接执行 ,  比如执行了ctx.close() 方法, 此时客户端服务器端都会在channel中将这个 handler移除  .

所以我们应该知道我们关闭连接属于哪种情况 . **一般在exceptionCaught()中执行 ctx.close() 然后再handlerRemoved() 方法中写业务逻辑 ,** 

### 2. ctx.writeAndFlush(msg)  与 ctx.channel().writeAndFlush(msg) 

```java
ChannelHandlerContext.writeAndFlush(msg);
ChannelHandlerContext.channel().writeAndFlush(msg);
1. 监听器执行的回调结果会在解码器执行完 write和flush方法调用完回调,所以issuccessful() 并不代表已经发送出去了,只是代表写到缓冲区了 ....

2. 这俩writeAndFlush()含义不同
2.1 第一个意思是从我当前位置向上找 , 如果我上面没有 输出流(编码器) 那么就不会走添加在当前位置以后的编码器
例如 : 当 ChannelHandlerContext.writeAndFlush(msg);
pipeline.addlast(in1,in2,out1,out2)
此时如果是in2的位置, 此时执行writeAndFlush()方法根本不会走 out1 out2 的  write() 和 flush() 方法的
直接出站了 ....

2.2 但是如果我们使用的是 : ChannelHandlerContext.channel().writeAndFlush(msg);
ChannelHandlerContext.channel().writeAndFlush(msg);
此时会执行 out1 和 out2 

3. 当一个客户端建立连接的时候 上面四个的顺序是 
in  -> out
读取数据写出顺序 : 
out#read -> in#channelRead -> out#write ->  out#flush -> in#channelReadComplete
```

## 2. MessageToByteEncoder<T> 与 ByteToMessageDecoder - 核心实现类

### 1. MessageToByteEncoder<T> 编码器

> ​	ChannelOutboundHandlerAdapter which encodes message in a stream-like fashion from one message to an ByteBuf.
>
> 将 msg 转成 buf 

```java
public class IntegerEncoder extends MessageToByteEncoder<Integer> {
    @Override
   public void encode(ChannelHandlerContext ctx, Integer msg, ByteBuf out)
           throws Exception {
       out.writeInt(msg);
   }
}
```

### 2. ByteToMessageDecoder  解码器

基本核心的文档内容在下面 : 

> ​	ChannelInboundHandlerAdapter which decodes bytes in a stream-like fashion from one ByteBuf to an other Message type.
>
> ​    **ChannelInboundHandlerAdapter 以字节流的方式 从ByteBuf 到  other Message  , 核心方法 `decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out)` 输入是byte ,输出是 out**

>  
>
> ​	Generally frame detection should be handled earlier in the pipeline by adding a DelimiterBasedFrameDecoder, FixedLengthFrameDecoder, LengthFieldBasedFrameDecoder, or LineBasedFrameDecoder.
>
> 通常，通过添加**DelimiterBasedFrameDecoder**、**FixedLengthFrameDecoder**、**LengthFieldBasedFrameDecoder**或**LineBasedFrameDecoder**，可以在管道中更早地处理帧检测。
>
> Some methods such as ByteBuf.readBytes(int) will cause a memory leak if the returned buffer is not released or added to the out List. Use derived buffers like ByteBuf.readSlice(int) to avoid leaking memory.
>
> **ByteBuf.readBytes(int) 这个方法会造成内存泄漏,如果返回的对象没有被释放掉,或者添加到out List中 ,所以官方推荐使用ByteBuf.readSlice(int) 这些派生缓冲区,防止内存泄漏**

```java
public class SquareDecoder extends ByteToMessageDecoder {
    @Override
   public void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out)
           throws Exception {
       out.add(in.readBytes(in.readableBytes()));
   }
}
```

### 3. MessageToMessageEncoder  编码器

> ​	ChannelOutboundHandlerAdapter which encodes from one message to an other message For example here is an implementation which decodes an Integer to an String.
>
> message -> message -> (后续 -> bytebuf)

```java
public class IntegerToStringEncoder extends
       MessageToMessageEncoder<Integer> {

    @Override
   public void encode(ChannelHandlerContext ctx, Integer message, List<Object> out)
           throws Exception {
       out.add(message.toString());
   }
}
```



## 3. ObjectDecoder和ObjectEncoder - 对象

### 1. ObjectEncoder

> ​	An encoder which serializes a Java object into a {@link ByteBuf}.

将一个java对象序列化成一个 ByteBuf 对象

### 2. ObjectDecoder

> ​	A decoder which deserializes the received {@link ByteBuf}s into Java objects.
>
> 将一个 ByteBuf 反序列成一个java对象

### 3. 快速开始

第一步一个java对象 必须是实现Serializable接口

```java
public class OPack implements Serializable {

    private static final long serialVersionUID = -5734509523963527363L;
    String name;
    String msg;
    
    .... 其他省略 ,以及那个
}
```

客户端服务器都添加这两个 编码器和解码器

```java
ch.pipeline().addLast(new ObjectEncoder());


ch.pipeline().addLast(new ObjectDecoder(1004, new ClassResolver() {
    @Override
    public Class<?> resolve(String className) throws ClassNotFoundException {
        return OPack.class;
    }
}));
```

自己写一个 ChannelInboundHandlerAdapter 实现channelRead() 方法就可以了 , 很简单



### 4. 我们看看这俩编解码器原理

#### 1. ObjectEncoder 编码器

```java
public class ObjectEncoder extends MessageToByteEncoder<Serializable> {
    // 这是一个int 的 byte数组 , 因为int是四个字节
    private static final byte[] LENGTH_PLACEHOLDER = new byte[4];

    @Override
    protected void encode(ChannelHandlerContext ctx, Serializable msg, ByteBuf out) throws Exception {
        // 写指针
        int startIdx = out.writerIndex();

        // 
        ByteBufOutputStream bout = new ByteBufOutputStream(out);
        //
        ObjectOutputStream oout = null;
        try {
            // 首先写入一个4字节的数组
            bout.write(LENGTH_PLACEHOLDER);
			
            oout = new CompactObjectOutputStream(bout);
            
            // 其次再写入一个对象
            oout.writeObject(msg);
            oout.flush();
        } finally {
            if (oout != null) {
                oout.close();
            } else {
                // 关闭流
                bout.close();
            }
        }

        // 获取写指针
        int endIdx = out.writerIndex();

        // 设置长度值
        out.setInt(startIdx, endIdx - startIdx - 4);
    }
}

```

自己玩一玩 ........

```java
        ByteBuf out = Unpooled.buffer(1024);

        // w指针位置
        int start = out.writerIndex();

        // 设置一个输出流
        ByteBufOutputStream bbos = new ByteBufOutputStream(out);
        
        // 首先写一个 4个字节长度的数组
        bbos.write(new byte[4]);
        
        // 对象流
        ObjectOutputStream objectOutputStream = new ObjectOutputStream(bbos);
        
        // 写一个obj
        objectOutputStream.writeObject(new OPack("hhh", "hhh"));

        // W 最终位置
        int end = out.writerIndex();

        // 在buf对象的 start 位置 设置一个 int值为 end-start-4 就是对象的长度 ,所以对象长度就算出来了
        out.setInt(start, end - start - 4);

        System.out.println("out = " + out);
        
        System.out.println("out.readInt() = " + out.readInt());

        // 看看计数引用
        System.out.println("out.refCnt() = " + out.refCnt());
```

输出

```java
out = UnpooledByteBufAllocator$InstrumentedUnpooledUnsafeHeapByteBuf(ridx: 0, widx: 117, cap: 1024)
out.readInt() = 113
out.refCnt() = 1
```



### 2. ObjectDecoder 解码器

```java
public class ObjectDecoder extends LengthFieldBasedFrameDecoder {

    private final ClassResolver classResolver;

    public ObjectDecoder(ClassResolver classResolver) {
        this(1048576, classResolver);
    }
	
    // 这个意思每帧 1048576 长度
    // 长度的偏移量为0 , 占用 4个字节 , 最后去掉前面的4个字节
    public ObjectDecoder(int maxObjectSize, ClassResolver classResolver) {
        super(maxObjectSize, 0, 4, 0, 4);
        this.classResolver = classResolver;
    }

    @Override
    protected Object decode(ChannelHandlerContext ctx, ByteBuf in) throws Exception {
        // 直接调用父类的decode() 方法 , 返回的是已经扣除了我们4个字节长度的 ByteBuf方法
        ByteBuf frame = (ByteBuf) super.decode(ctx, in);
        if (frame == null) {
            return null;
        }

        // 这个代码有两处 一处是释放ByteBuf , 一处是转成一个ObjectInputStream对象
        ObjectInputStream ois = new CompactObjectInputStream(new ByteBufInputStream(frame, true), classResolver);
        try {
            // 最后读取就行了
            return ois.readObject();
        } finally {
            ois.close();
        }
    }
}
```

我这里有一张图 , 可以清晰的看到前面变化 , 变化点就是我上面提到的 , 我会在下面讲解**LengthFieldBasedFrameDecoder** 源码的时候对其分析 , 还要注意一点就是你获取的计数引用此时是2 ,这个需要手动释放 , 上面注释我已经提到了 ....netty已经帮我们释放了 ,但是我debug 发现并没有释放,而是后面有一处进行了一次release . 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-17/920bee30-4966-4a0c-848c-e46ac27a914c.png?x-oss-process=style/template01)



## 4. FixedLengthFrameDecoder - 定长

```java
public class FixedLengthFrameDecoder extends ByteToMessageDecoder {

    // 定长
    private final int frameLength;

    public FixedLengthFrameDecoder(int frameLength) {
        checkPositive(frameLength, "frameLength");
        this.frameLength = frameLength;
    }

    // ByteToMessageDecoder 的实现方法
    @Override
    protected final void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) throws Exception {
        // 先去执行这个方法 -- >
        Object decoded = decode(ctx, in);   
        // null 直接不管了 , 直接丢入缓冲区了 
        if (decoded != null) {
            // 不为空就添加进去
            out.add(decoded);
        }
    }

    // -- > 到这里
    protected Object decode(
            @SuppressWarnings("UnusedParameters") ChannelHandlerContext ctx, ByteBuf in) throws Exception {
        
        // 小于直接返回null
        if (in.readableBytes() < frameLength) {
            return null;
        } else {
            // 等于或者等于就截取固定长度 
            return in.readRetainedSlice(frameLength);
        }
    }
}
```

```java
ByteBuf buf = Unpooled.buffer(8);

// 0000 0000 , 0000 0000 , 0010 0111 , 0001 0000
/ 0   0   39   16 
 buf.writeInt(10000);
//0000 0000 , 0000 0000 , 0000 0000 ,  0110 0100
// 0  0  0 100
buf.writeInt(100);

我们将 new FixedLengthFrameDecoder(6) 长度调整为6个字节
连续客户端发送两次给服务器
```

结果 : 

```java
第一次  :  0	0	39	16	0	0	
第二次  :  0	100	 0	 0	39	16	
```

我们发现第一次发送过去的数据 , 后两个字节在第二次发送的前面两个字节 , 此时 我们就肯定了 , 第一次发送过去没有收到的在缓冲区里面 , 这个缓冲区是每一个客户端与服务器连接都会有一个与这个客户端想对应的缓冲区 , 不是共同缓冲区 ...





## 5. LengthFieldBasedFrameDecoder - 自定义长度

### 1. 构造方法

```java
new LengthFieldBasedFrameDecoder(ByteOrder byteOrder, int maxFrameLength, int lengthFieldOffset, int lengthFieldLength,int lengthAdjustment, int initialBytesToStrip, boolean failFast);

byteOrder  :  一个枚举类 , 默认是 ByteOrder.BIG_ENDIAN ,我们可以通过 ByteOrder.nativeOrder() ,获取本机的 ByteOrder ,来设置对应的属性 . 熟称大端,小端 ,又兴趣的可以百度一下 . win10默认是小端

maxFrameLength : 帧 指的意思就是每次一客户端像服务器发送的数据包接收的缓冲区最大值 , 

lengthFieldOffset : 数据包长度的偏移量

lengthFieldLength : 数据包长度的长度 ,比如short就是2个字节 , int 就是4个字节

lengthAdjustment : 这个意思是 比如 我的数据包长度(12字节)=长度(2字节)+数据包实际长度(10字节) , 此时就需要调整一下 , 然后此时获取到的就是调整后的 , 但是此时你最终获取到的数据长度就是数据包长度12字节,而不是实际数据包长度10个字节 , 

initialBytesToStrip : 他的意思是 decode() 返回的ByteBuf的时候跳过的字节数, in.skipBytes(initialBytesToStrip);
```

### 2. 简单使用  

#### 1. 第一种简单情况(10, 0, 2,0,0)

```java
1. 服务器先添加....到管道中去
ch.pipeline().addLast(new LengthFieldBasedFrameDecoder(8, 0, 2,0,0));


2. 客户端发送数据
// 客户端发送给服务器的数据
ByteBuf buf = Unpooled.buffer(10);
// 长度为8 
buf.writeShort(8);
buf.writeInt(10000);
buf.writeInt(100);

3. 服务器端代码
@Override
public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
    if (msg instanceof ByteBuf) {
        ByteBuf messages = (ByteBuf) msg;
        System.out.println("数据长度 = " + messages.readShort());
        System.out.println("数据1 = " + messages.readInt());
        System.out.println("数据2 = " + messages.readInt());
    } else {
        super.channelRead(ctx, msg);
    }
}
```

输出

```java
数据长度 = 8
数据1 = 10000
数据2 = 100
```

确实使我们发送的 ...

#### 2. 第二种情况(10, 0, 2,-2,0)

```java
ByteBuf buf = Unpooled.buffer(10);
// 长度为10 ,包含长度字段 ,所以调整为 -2
buf.writeShort(10);
buf.writeInt(10000);
buf.writeInt(100);

服务器端代码不变
```

输出 , 输出数据一致 

```java
数据长度 = 10
数据1 = 10000
数据2 = 100
```

#### 3. 第三种情况 (10, 0, 2,-2,2)

```java
ByteBuf buf = Unpooled.buffer(10);
// 长度为10 ,包含长度字段 ,所以调整为 -2
buf.writeShort(10);
buf.writeInt(10000);
buf.writeInt(100);

服务器端代码只是去掉长度
@Override
public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
    if (msg instanceof ByteBuf) {
        ByteBuf messages = (ByteBuf) msg;
        System.out.println("数据1 = " + messages.readInt());
        System.out.println("数据2 = " + messages.readInt());
    } else {
        super.channelRead(ctx, msg);
    }
}
```

输出 , 结果一致 ,很正确 ,是不是会玩了 .........

```java
数据1 = 10000
数据2 = 100
```

### 3. 源码分析 : 

```java
public class LengthFieldBasedFrameDecoder extends ByteToMessageDecoder {

    this.byteOrder = byteOrder;
    this.maxFrameLength = maxFrameLength;
    this.lengthFieldOffset = lengthFieldOffset;
    this.lengthFieldLength = lengthFieldLength;
    this.lengthAdjustment = lengthAdjustment;
    lengthFieldEndOffset = lengthFieldOffset + lengthFieldLength;
    this.initialBytesToStrip = initialBytesToStrip;
    this.failFast = failFast;
}

@Override
protected final void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) throws Exception {
    // 先执行 decode 方法 ,自己实现的
    Object decoded = decode(ctx, in);
    if (decoded != null) {
        out.add(decoded);
    }
}

// 注意 in 此时只直接内存 
protected Object decode(ChannelHandlerContext ctx, ByteBuf in) throws Exception {
    // 太长丢弃 , 一开始是false
   if (discardingTooLongFrame) {
        discardingTooLongFrame(in);
    }
    
    //  lengthFieldEndOffset = lengthFieldOffset + lengthFieldLength;
    // 可读长度小于长度末端的偏移量 ,直接返回空
    if (in.readableBytes() < lengthFieldEndOffset) {
        return null;
    }

    // 长度的真实偏移量 = r指针+长度偏移量
    int actualLengthFieldOffset = in.readerIndex() + lengthFieldOffset;

    // 每一帧的长度 ,其实这个方法就是switch语句1 2 4 8 那种的根据你长度占的字节数进行读取数据长度
    long frameLength = getUnadjustedFrameLength(in, actualLengthFieldOffset, lengthFieldLength, byteOrder);

    // 长度小于0 
    if (frameLength < 0) {
        failOnNegativeLengthField(in, frameLength, lengthFieldEndOffset);
    }

    // 比如我的长度包含了长度的字节, 此时就需要调整去处长度所占的字节
    // 数据包真实长度 = frameLength + 调整的长度 + 长度末端偏移量
    frameLength += lengthAdjustment + lengthFieldEndOffset;

    // 长度小于
    if (frameLength < lengthFieldEndOffset) {
        failOnFrameLengthLessThanLengthFieldEndOffset(in, frameLength, lengthFieldEndOffset);
    }

    // 数据包长度如果大于maxFrameLength 直接抛出异常了其实
    if (frameLength > maxFrameLength) {
        exceededFrameLength(in, frameLength);
        return null;
    }
   
    int frameLengthInt = (int) frameLength;
    if (in.readableBytes() < frameLengthInt) {
        return null;
    }

    if (initialBytesToStrip > frameLengthInt) {
        failOnFrameLengthLessThanInitialBytesToStrip(in, frameLength, initialBytesToStrip);
    }
    // 跳过初始化的initialBytesToStrip
    in.skipBytes(initialBytesToStrip);

    // r 指针
    int readerIndex = in.readerIndex();
    // 真实长度  = 长度-initialBytesToStrip
    int actualFrameLength = frameLengthInt - initialBytesToStrip;
    
    // 这里也是切片 , 会计数+1 ,后面会讲到
    ByteBuf frame = extractFrame(ctx, in, readerIndex, actualFrameLength);
    
    // 修改reader指针 位置
    in.readerIndex(readerIndex + actualFrameLength);
    
    return frame;
}
```



## 6. DelimiterBasedFrameDecoder - 自定义Delimiter

> ​	自定义分隔符 解码器

### 1. 构造方法

```java
new DelimiterBasedFrameDecoder(50, false, true, Unpooled.copiedBuffer("a", CharsetUtil.UTF_8))
arg1 : maxFrameLength 每一次的数据包最大多少
arg2 : stripDelimiter  是够去除分隔符
arg3 : failFast  如果异常快速抛出
arg4 : delimiter  分隔符
```

### 2. 简单使用

```java
1. 服务器端
// 分隔符解码器
ch.pipeline().addLast(new DelimiterBasedFrameDecoder(50, false, true, Unpooled.copiedBuffer("a", CharsetUtil.UTF_8)));
// 字符串解码器
ch.pipeline().addLast(new StringDecoder());
// 自己的解码器
 ch.pipeline().addLast(new MyDecoder());

我们在客户端注册的是否发送了 : 
ctx.writeAndFlush(Unpooled.copiedBuffer("HELLOa我是a服务器a", CharsetUtil.UTF_8));

channelRead() 方法 : 
if (msg instanceof String) {
	System.out.println(msg);
}

2. 客户端代码
三个解码器一致
我们启动后发送两个 : 
run.channel().writeAndFlush(Unpooled.copiedBuffer("你好a你真的很棒", CharsetUtil.UTF_8));
run.channel().writeAndFlush(Unpooled.copiedBuffer("啊a", CharsetUtil.UTF_8));
```

输出结果 : 

```java
服务器端输出 : 
你好a
你真的很棒啊a

客户端输出 : 
HELLOa
我是a
服务器a
```

我们发现 分隔符是在句尾分割 , 如果没有分隔符另外一部分就放入缓冲区了,等待下一次读取后一起放入进去.



## 7. LineBasedFrameDecoder - 按行分

> 换行符解码器

### 1. 构造方法

```java
maxLength : 最大帧
failFast : 快速抛出异常
stripDelimiter : 去除分隔符
```

### 2. 快速使用 

```java
// 1.换行符解码器
ch.pipeline().addLast(new LineBasedFrameDecoder(1000, true, true));
// 2. 字符串解码器
ch.pipeline().addLast(new StringDecoder());
// 3. 自己的处理器
ch.pipeline().addLast(new MyHandler());

服务器端发送 : 
ctx.writeAndFlush(Unpooled.copiedBuffer("HELLO,我是服务器\n", CharsetUtil.UTF_8));

客户端发送 : 
run.channel().writeAndFlush(Unpooled.copiedBuffer("你好,你真的很棒", CharsetUtil.UTF_8))
run.channel().writeAndFlush(Unpooled.copiedBuffer("啊,\n", CharsetUtil.UTF_8))
```

输出结果 : 

```java
服务器端 : 
你好,你真的很棒啊,

客户端: 
HELLO,我是服务器
```



### 3. 分析

```java
ByteBuf buf = Unpooled.buffer(10);

buf.writeCharSequence("hell\n", CharsetUtil.UTF_8);

byte x = '\n';

//ByteProcessor.FIND_LF
int i = buf.forEachByte(0, 10, new ByteProcessor() {
    @Override
    public boolean process(byte value) throws Exception {

        //不想循环 返回 false
        if (value == x) {
            return false;
        }
        // 想继续循环返回 true
        return true;
    }
});

System.out.println("index : "+i);
```

输出 : 

```java
index : 4
```

所以很简单就能发现换行符在哪里,然后切割就行了 ....... 





## 8. IdleStateHandler - 心跳检测

> ​	Triggers an IdleStateEvent when a Channel has not performed read, write, or both operation for a while.
>
> 触发一个 IdleStateEvent  事件 ,当一个channel在一段时间内没有执行 read write 或者两者 , 所以他属于一个事件流

官方源码中的推荐用法 , 其实一般使用也是如此 , 

第一个参数是 : (READER_IDLE)  No data was received for a while.

第一个参数是 : (WRITER_IDLE)  No data was sent for a while.

第一个参数是 : (ALL_IDLE)  No data was either received or sent for a while.

```java
public class MyChannelInitializer extends ChannelInitializer<Channel> {
    @Override
   public void initChannel(Channel channel) {
       channel.pipeline().addLast("idleStateHandler", new IdleStateHandler(60, 30, 0));
       channel.pipeline().addLast("myHandler", new MyHandler());
   }
}

// Handler should handle the IdleStateEvent triggered by IdleStateHandler.
public class MyHandler extends ChannelDuplexHandler {
    @Override
   public void userEventTriggered(ChannelHandlerContext ctx, Object evt) throws Exception {
       if (evt instanceof IdleStateEvent) {
           IdleStateEvent e = (IdleStateEvent) evt;
           if (e.state() == IdleState.READER_IDLE) {
               ctx.close();
           } else if (e.state() == IdleState.WRITER_IDLE) {
               ctx.writeAndFlush(new PingMessage());
           }
       }
   }
}
```

### 1. 如何去设计心跳检测 

​	文章也找不到了, 我是参考一篇阿里Dubbo的文章的 , 他的思想就是 主在于客户端, 从在于服务器端 ,因为我们的思想一般是让服务器端发送心跳包,定时去检测,如果累计达到几次没有的话,那么直接out掉, 这样给我们带来的不好处就是逻辑复杂 ,考虑情况太多 , 所以改成客主服从,特别适用于客户端开发的 .

​	思路大致就是客户端和服务器端 , 客户端当60S没有收到服务器端发来的消息,会主动给服务器端发送一个心跳包, 此时,当服务器端没有收到和发送给客户端的时长超过120S我们就断开与客户端的连接 , 所以服务器端很简单,客户端也很简单

####  1. 服务器端代码

```java
@Override
protected void initChannel(SocketChannel socketChannel) throws Exception {

	ChannelPipeline pipeline = socketChannel.pipeline();
	// 心跳检测 , 读写超时时间是120S , 没有收到也没有发送消息
	pipeline.addLast("idleStateHandler", new IdleStateHandler(0, 0, 120));

	// 心跳检测处理器
	pipeline.addLast("serverHeartBeatHandler", new ServerHeartBeatHandler(listener));

}

// 处理器逻辑
public class ServerHeartBeatHandler extends ChannelDuplexHandler {
    @Override
    public void userEventTriggered(ChannelHandlerContext ctx, Object evt) throws Exception {
		// 判断是否是IdleStateEvent
        if (evt instanceof IdleStateEvent) {
            // TODO: 2019/11/16   心跳检测服务器端  超时直接关闭
             // 交给 handlerremove处理
            ctx.close();
        } else {
            //否则不做处理
            super.userEventTriggered(ctx, evt);
        }
    }
}
```



#### 2. 客户端代码

```java
@Override
protected void initChannel(SocketChannel socketChannel) throws Exception {
    ChannelPipeline pipeline = socketChannel.pipeline();

    // 心跳检测 , 如果60S我们没有收到服务器信息,我们就发送一个心跳包
    pipeline.addLast("nettyHeartBeatHandler", new IdleStateHandler(60, 0 , 0)

    pipeline.addLast("heartBeatHandler", new ClientHeartBeatHandler(listener));
}

//处理器
public class ClientHeartBeatHandler extends ChannelDuplexHandler {
    private ChatBootListener listener;
    public ClientHeartBeatHandler(ChatBootListener listener) {
        this.listener = listener;
    }
    @Override
    public void userEventTriggered(ChannelHandlerContext ctx, Object evt) throws Exception {
        if (evt instanceof IdleStateEvent) {
            // 发送一个心跳包
            ctx.channel().writeAndFlush(Constants.HEART_BEAT_NPACK).addListener(new GenericFutureListener<Future<? super Void>>() {
                @Override
                public void operationComplete(Future<? super Void> future) throws Exception {
                    if (future.isSuccess()) {
                    // 成功一般是发送到了缓冲区 , 但是并不是代表服务器端收到了请求
                    } else {
                        // TODO: 2019/11/16  失败 ........... 看需求
                    }
                }
            });
        } else {
            // 交给父类处理
            super.userEventTriggered(ctx, evt);
        }
    }
}
```

### 2. 基本原理

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-19/f4cb4318-c7a6-4d9e-97f9-0ae2f95218ed.jpg?x-oss-process=style/template01)

就是 ChannelHandlerContext 的 EventExecutor 定期去执行一个任务 ,需要传入一个runnable对象和一个延迟时间 ,人后一直定期执行 ..........



## 9. 深拷贝与浅拷贝

### 1. 视图-浅拷贝

> You can create a view of an existing buffer by calling one of the following methods:
>
> - duplicate()
> - slice()
> - slice(int, int)
> - readSlice(int)
> - retainedDuplicate()
> - retainedSlice()
> - retainedSlice(int, int)
> - readRetainedSlice(int)

### 2. 视图 - 浅拷贝

#### 1. slice() = duplicate()  只展示一种

```java
// Unpooled 是非池化的意思 ,官方推荐自己定义使用非池化 , 可以看看文档去
ByteBuf buf = Unpooled.buffer(10);
buf.writeShort(100);
buf.writeInt(1000);
buf.writeInt(10000);
// 读取一个 r -> 2
System.out.println("buf.readShort() = " + buf.readShort());

System.out.println("buf.slice() = " + buf.slice());

// 再读取一个 r- > 4
System.out.println("buf.readInt() = " + buf.readInt());
```

输出结果 :  我们发现就是截取了从 r -> capacity 的长度

```java
buf.readShort() = 100
buf.slice() = UnpooledSlicedByteBuf(ridx: 0, widx: 8, cap: 8/8, unwrapped: UnpooledByteBufAllocator$InstrumentedUnpooledUnsafeHeapByteBuf(ridx: 2, widx: 10, cap: 10))
buf.readInt() = 1000
```

#### 2. slice(int index, int len)  与 readSlice(int length)

```java
ByteBuf buf = Unpooled.buffer(10);
buf.writeShort(100);
buf.writeInt(1000);
buf.writeInt(10000);
// 读取一个 r -> 2
System.out.println("buf.readShort() = " + buf.readShort());

// 2开始 截取 4个长度
ByteBuf slice = buf.slice(2, 4);

// 这个buf上 从0开始设置一个 数值为66666占四个字节的数字
slice.setInt(0, 66666);

// 在原来的上面从 r指针开始 4个字节长度 , 他相当于 slice() 方法第一个参数是 buf.readerIndex() , 
ByteBuf buf1 = buf.readSlice(4);

//输出读到的
System.out.println("slice.readInt() = " + buf1.readInt());
```

输出结果 :  我们发现副本值是相互影响的 ,所以不会产生额外对象

```java
buf.readShort() = 100
slice.readInt() = 66666
```

#### 3.  加了retained 前缀的方法

```java
ByteBuf buf = Unpooled.buffer(10);
buf.writeShort(100);
buf.writeInt(1000);
buf.writeInt(10000);

System.out.println("buf.refCnt() = " + buf.refCnt());

buf.retainedSlice(2, 4);

System.out.println("buf.refCnt() = " + buf.refCnt());

buf.slice(2, 4);

System.out.println("buf.refCnt() = " + buf.refCnt());
```

输出结果 :  retain 使得计数引用加一 

```java
buf.refCnt() = 1
buf.refCnt() = 2
buf.refCnt() = 2
```

#### 4. retain() 与 release() 方法

```java
buf.retain();  会使得 计数引用加一

buf.release(); 会使得 计数引用减少一

当等于零时 ,这个数据会自动被彻底清空 再调用会抛出IllegalReferenceCountException异常
```



### 3. 完全拷贝 - copy

```java
ByteBuf buf = Unpooled.buffer(10);

buf.writeShort(100);
buf.writeInt(1000);
buf.writeInt(10000);

// 复制
ByteBuf copy = buf.copy();
copy.setShort(0, 1000);
System.out.println("buf.readShort() = " + buf.readShort() + " : " + buf.readerIndex()+" : "+buf.refCnt());

//重置r指针 , 不能使用buf.clear();会使得 w指针也重置为0
buf.readerIndex(0);
// 视图
ByteBuf duplicate = buf.duplicate();
duplicate.setShort(0, 1000);
System.out.println("buf.readShort() = " + buf.readShort() + " : " + buf.readerIndex()+" : "+buf.refCnt());
```

输出 : 

```java
buf.readShort() = 100 : 2 : 1
buf.readShort() = 1000 : 2 : 1
```

所以copy()方法是在原来的基础上直接拷贝了一份 , 所以修改对原来不影响 ,但此时容易造成内存泄漏,而 视图确实不一样 他修改会改变原来的, 看情况选择两者的使用



### 4. 完全拷贝 - buf.readBytes(int len);

```java
ByteBuf buf = Unpooled.buffer(2);

//设置一个值
buf.writeShort(100);

// 读取两个字节返回一个 buf1
ByteBuf buf1 = buf.readBytes(2);

System.out.println("buf1 = " + buf1);

// buf1 中设置值为2000
buf1.setShort(0, 2000);

//输出设置的值
System.out.println("buf1.readShort() = " + buf1.readShort());

// 调整reader指针位置
buf.readerIndex(0);

//输出buf的值
System.out.println("buf.readShort() = " + buf.readShort());
```

结果 : 所以此时获取到的是  : 

```java
buf1 = UnpooledByteBufAllocator$InstrumentedUnpooledUnsafeNoCleanerDirectByteBuf(ridx: 0, widx: 2, cap: 2)
buf1.readShort() = 2000
buf.readShort() = 100
```





## 10. 大端小端 

[参考文章](https://blog.csdn.net/jason_cuijiahui/article/details/79010257) : https://blog.csdn.net/jason_cuijiahui/article/details/79010257 对于概念讲的比较细

> 1. 大端模式是指**数据的低位保存在内存的高地址中，而数据的高位保存在内存的低地址中**.
> 2. 小端模式是指**数据的低位保存在内存的低地址中，而数据的高位保存在内存的高地址中**。

```java
System.out.println(ByteOrder.nativeOrder());


ByteBuf buf = Unpooled.buffer(10);

// 这个必须用返回的视图操作 ,netty中文档说了 , 
ByteBuf order = buf.order(ByteOrder.LITTLE_ENDIAN);

order.writeShort(100);
order.writeInt(1000);
order.writeInt(10000);


System.out.println("buf.readerIndex() = " + buf.readerIndex());


System.out.println("buf.readerIndex(2) = " + buf.readerIndex(2));

// 小端读取 的意思 LITTLE_ENDIAN的缩写
System.out.println("buf.readIntLE() = " + buf.readIntLE());
// 大端读取 
System.out.println("buf.readInt() = " + buf.readInt());
```

输出 : X86目前都是小端模式 ,netty默认是大端模式

```java
LITTLE_ENDIAN
buf.readerIndex() = 0
buf.readerIndex(2) = UnpooledByteBufAllocator$InstrumentedUnpooledUnsafeHeapByteBuf(ridx: 2, widx: 10, cap: 10)
buf.readIntLE() = 1000
buf.readInt() = 270991360
```





## 11. Http

### HttpRequestDecoder

### ChunkedWriteHandler

### HttpResponseEncoder

