# Flink 入门 理解思想和架构

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

