# Flink 入门 理解思想和架构

什么是大数据，Big Data，处理数据每个人都会，比如拿到数据做一些聚合，最简单的单机体系进行一些数据计算拿到想要的结果，换句话说可能CRUD也是数据。

那么大数据的大体现在哪，其实对于单机体系是有业务瓶颈的（我们没有天河2号超级计算机），也就是商用计算机的贫瘠，那就是分布式的数据计算，所以大数据解决了，分布式计算、分布式存储的问题。

那么分布式计算需要我们去care吗，显然不需要，理由：现有的框架都去做了。

所以大数据的框架，可以实现让我们用单机的计算逻辑，依靠框架实现分布式的计算。那么这个分布式更多的体现在并行处理的能力，所以可以面对海量数据。



大数据的运算框架，热门的就是MapReduce ， Spark ， Flink

MapReduce 是因为其复杂的代码逻辑，基本落下了舞台，用于离线计算

Spark利用优秀的 函数式编程，RDD抽象、弹性数据流，实现了简单的编程，用户离线计算

Flink发现了离线计算的贫瘠，加入了时实计算，其实需要care的是，实时计算的维度问题，全局的实时计算、时间段内的实时计算、流式计算（单个个体为最小处理单元），所以Flink都提供了。





首先我们要做的是这个：

![](https://zhisheng-blog.oss-cn-hangzhou.aliyuncs.com/images/NyzY5t.jpg)



## Flink如何解决的这个问题

下面是flink 的介绍，大致就是是一个 流，在这个流上可以加入有状态的计算。

可以看到它支持的处理事件很多

![](https://zhisheng-blog.oss-cn-hangzhou.aliyuncs.com/images/WI7lRD.jpg)



一下是它的核心特点。

![](https://zhisheng-blog.oss-cn-hangzhou.aliyuncs.com/images/sMDTgD.jpg)



以下是flink的几种玩法， sql 、类sql、有界/无界流，状态计算。

![](https://zhisheng-blog.oss-cn-hangzhou.aliyuncs.com/images/nJou61.jpg)



![](https://zhisheng-blog.oss-cn-hangzhou.aliyuncs.com/images/ozmU46.jpg)





代码如下。大致就是 输入(source)  ->  处理  -> 输出(sink) ，这里的输入可以是多个流。

![](https://user-gold-cdn.xitu.io/2020/3/13/170d37e9809265da?imageslim)



// 



![](https://zhisheng-blog.oss-cn-hangzhou.aliyuncs.com/images/u3RagR.jpg)





//  

