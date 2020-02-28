# Netty的参数细节

## Socket文件句柄数

linux中 需要修改 : `ulinit -n [xxx]`  来设置句柄数量. 防止出现异常.  (linux文件句柄数量) , 一次性. 



## `ChannelOption.SO_SNDBUF`  / `SO_RCVBUF`

这个值得是socket缓冲区的大小. 一般是 带宽*RTT值.  这里是一个动态调整的值. 



## `SO_KEEPALIVE`

> ​	这个keepalive是 tcp层的, 也就是传输层TCP的处理的,默认是30S, 但是我们可以加入我们自己的应用层实现. 
>
> 

## `ChannelOption.SO_REUSEADDR`

服务器开启 `.childOption(ChannelOption.SO_REUSEADDR, Boolean.TRUE)` 

客户端 就是channel了

解决多个网卡绑定相同的端口号的问题. ; 让关闭连接的端口释放的更快,可以更早的使用. 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/4e1a4151-6ea6-49f8-a467-0a9168bad8bf.png)



这个函数一就是 2msl ,   比如我们发送一个关闭请求, 因为有四次握手么,

最后一次要响应给服务器 . TCP这个玩意你要知道, 他就是个弱智, 你发送给服务器ACK包, 你并不知道服务器收到没有. 服务器收到后还会给你一次响应. 你只有接受到这个响应你才可以继续发送下一个包. 

这里就是这个意思. 等待发送成功. 如果我们没有这个等待时间. 我们直接关闭这个端口了. 此时有一个客户端连接来了, 收到了这个请求. 这个就给了这个客户端. 就发生了  上一个客户端的数据来到了这个客户端. 

TCP 包可能迷路, 会定义一个最大的转发次数, 超过了就会将这个包丢掉. 

## `ChannelOption.SO_LINGER`

关闭socket的延迟时间. 2msl知道不, 就是这个.  默认是不开启的. 

服务器开启可以使用 `.childOption(ChannelOption.SO_LINGER, Boolean.TRUE)` 



## ` ChannelOption.IP_TOS`

这个参数 如果有了解过TCP编程的学生, 会知道TCP有一个字段是设置优先级的. 所以这个就是修改优先级的.  默认是`0000`  , 但是可能被TCP忽略掉. .



## `ChannelOption.TCP_NODELAY`

这个一般在服务器端设置 :  `.childOption(ChannelOption.TCP_NODELAY, Boolean.TRUE)`  这样写. 

客户端就没有子channel了. 一般是 `.option(ChannelOption.TCP_NODELAY, Boolean.TRUE)` 



默认启用Nagle算法, 将小的碎片合并发送, 可以提高发送效率,  也就是比如你发送1个字节的东西, 他不会立马发送, 等待一下你有没有其他包了, 有就合并发送了 

意思就是这个. 所以一般还是开启为好. 



## `SO_BACKLOG`

这个是一个最大连接的等待数量.  也就是比如服务器启动, 来了1000个连接, 但是一下子连不了, 就需要有个等待队列, 这个就是设置等待队列的大小. 

默认值为 128 . 可以通过这个参数调整. 

服务端可以设置 : `.option(ChannelOption.SO_BACKLOG)`



## `ChannelOption.ALLOCATOR`

这个服务器一般设置为 `.childOption(ChannelOption.ALLOCATOR, PooledByteBufAllocator.DEFAULT)`

客户端设置为 : `.option(ChannelOption.ALLOCATOR, PooledByteBufAllocator.DEFAULT)`

这个属于一个池化的设计, 使用的是堆外内存, 意思就是我们的ByteBuf从哪来分配.  就是靠这个玩意生成的. 



## `ChannelOption.RCVBUF_ALLOCATOR`

这个服务端 `.childOption(ChannelOption.RCVBUF_ALLOCATOR, allocator)`

客户端是 `.option(ChannelOption.RCVBUF_ALLOCATOR, allocator)`

这个allocator 默认是 `AdaptiveRecvByteBufAllocator` ,  对于用户来说可以实现, `io.netty.channel.RecvByteBufAllocator` 接口. 进行控制. 

**他的意思是从selector中收到写请求, 读取到数据. 这个数据每次接收多少. 也就是channel.read() , 每次读多少Bytbuffer. 可以设置成固定值, 也可以设置成adaptive . 根据情况设置.** 



## 



## 其他参数

参考自 极客时间的一个课程中讲的参数. 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/3499ed2e-0f42-4079-9e4a-4505e7ae35e5.jpg)

不好理解的我解释一下 : 

`高低水位线`意思是,  channel写出去的缓冲区大小, channel连接时就会生成一个.  默认是32K-64K . 所以是一个很大的数字.  

`connect-timeout`  客户端连接服务器端的超时时间. 默认是30S超时 , 也很大 .

`SINGLE_EVENTEXECUTOR_PER_GROUP`  其实就是,如果有人注意过的话. 我们每个客户端其实保存在一个线程中, 也就是一个客户端处理就是单线程处理. 从始至终一直是开始分配的那个线程.   



