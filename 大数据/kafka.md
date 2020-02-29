# kafka 

## 1. 介绍 

![](http://kafka.apache.org/images/logo.png)

官网 http://kafka.apache.org/

介绍 http://kafka.apachecn.org/intro.html



## 2. 快速开始

### 1. 安装

路径: http://kafka.apache.org/downloads

> ​	kafka集群搭建特别简单, 只需要broker.id不同就可以了
>
> ​	然后就是zookeeper的地址.

```shell
wget https://archive.apache.org/dist/kafka/0.11.0.3/kafka_2.11-0.11.0.3.tgz

sudo tar -zxvf kafka_2.11-0.11.0.3.tgz -C /opt/software/

cd /opt/software/kafka_2.11-0.11.0.3/config/

sudo chown -C admin:admin /opt/software/kafka_2.11-0.11.0.3/

vim server.properties  修改配置信息
# The id of the broker. This must be set to a unique integer for each broker.
broker.id=1
delete.topic.enable=true
log.dirs=/opt/software/kafka_2.11-0.11.0.3/logs

zookeeper.connect=localhost:2181
多个节点配置最好是 hadoop1 , hadoop2 , hadoop3 ,kafka依赖于zookeeper的,集群配置如下
zookeeper.connect=hadoop1:2181,hadoop2:2181,hadoop3:2181


创建日志文件
前提是你zk启动好了

kafka启动 就下面 (最好配置下环境变量) ,记住给其他几台机器穿文件时注意 broker.id=1记得修改.
kafka-server-start.sh -daemon /opt/software/kafka_2.11-0.11.0.3/config/server.properties

启动成功就是
[admin@localhost kafka_2.11-0.11.0.3]$ jps
2880 Jps
2804 Kafka
2185 QuorumPeerMain


修改 bin 下的 kafka-server-stop.sh 文件
PIDS=$(ps ax | grep -i 'kafka\.Kafka' | grep java | grep -v grep | awk '{print $1}')
修改后:
PIDS=$(ps ax | grep -i 'Kafka' | grep java | grep -v grep | awk '{print $1}')

就可以使用了 kafka-server-stop.sh关闭


群启动脚本
```

### 2. 使用跟着官方文档

[官方文档链接](http://kafka.apachecn.org/quickstart.html) 





## 3. Kafka整体架构和角色

### 1. 整体架构模型

最开始的模型 P - T - C 模型 , 消费者将自己发布的主题发给Broker,然后存入Broker中,等待消费者使用 , 此时会发现啥问题 ,单点问题, 一个Topic ,那么写入效率会很低, 而且容易宕机

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-09/856f3ef4-610a-43bf-b497-77704015b49c.jpg?x-oss-process=style/template01)

继续 , 进步,完善上面写入较慢的问题 :  , 我们发现,此时消费者这边读就麻烦了  , 我要三个都读一遍 , 无语 ,继续改进 ....

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-09/3ef5cf9c-4064-47f3-be16-0f66ba3d6576.jpg?x-oss-process=style/template01)

此时到了我们第三种架构 , 你有几个 partition ,我去消费的时候, 就派三个人去消费 ,此时就引入了下面的架构 : 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-09/384a08a4-bedd-433a-b470-551fea2a5b88.jpg?x-oss-process=style/template01)

但是此时又有一个问题 , 玩意我哪个Topic-Partition 挂掉了咋办呢  ? ,此时就需要建立 副本(duplicate) ,因此引入下面架构 : ,每一个分片都会有相同的副本 ,所以不怕挂掉一个 ,有副本可以接管 . 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-09/20134966-5bf1-4a9e-aeca-0aaf8cb0b44d.jpg?x-oss-process=style/template01)

所以以上架构模型 , 使得 Kafka的 读写 吞吐量 都是特别高的  , 具体细节实现就不讲解了 .么那本事 .



### 2. 角色分工

**1）Producer** **：** 消息生产者，就是向kafka broker发消息的客户端；

**2）Consumer** **：**消息消费者，向kafka broker取消息的客户端；

**3）Consumer Group** **：**消费者组，由多个consumer组成。**消费者组内每个消费者负责消费不同分区的数据，一个分区只能由一个消费者消费；消费者组之间互不影响。**所有的消费者都属于某个消费者组，即**消费者组是逻辑上的一个订阅者**。

**4）Broker** **：**一台kafka服务器就是一个broker。一个集群由多个broker组成。一个broker可以容纳多个topic。

**5）Topic** **：**可以理解为一个队列，**生产者和消费者面向的都是一个topic**；

**6）Partition**：可以理解过分片 , 为了实现扩展性，一个非常大的topic可以分布到多个broker（即服务器）上，一个topic可以分为多个partition，每个partition是一个有序的队列；

**7）Replicate**： 副本，为保证集群中的某个节点发生故障时，该节点上的partition数据不丢失，且kafka仍然能够继续工作，kafka提供了副本机制，一个topic的每个分区都有若干个副本，一个**leader**和若干个**follower**。

**8）leader：**每个分区多个副本的“主”，生产者发送数据的对象，以及消费者消费数据的对象都是leader。

**9）follower：**每个分区多个副本中的“从”，实时从leader中同步数据，保持和leader数据的同步。leader发生故障时，某个follower会成为新的follower。

## 4. 工作流程 & 存储机制

### 1. 整个工作流程 , 讲解侧重于读 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-09/50a3ff89-e543-4503-89d2-a83d4f82fbe4.jpg?x-oss-process=style/template01)

我们主要关注一个点就是`偏移量(offset)` ,读写全部取决于offset 

那么问题来了 ,offset是啥 ,在哪 , 此时就引出了落盘操作 ,在一个log日志文件里, 他的每一条插入记录都有一个offset , 消费者消费数据,只用拿着 offset(前提是他得自个拿个本本记着,我这次看到哪了)去查找就行了 .

但是我们知道如果一个Topic的分片`partition`全部让一个日志文件去写的话,会造成,这个日志文件很大 ,查起来也不方便 , 难受 ,此时就引入了 再分片`segment` , 和 索引 `index` . 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-09/bbac0b3f-193f-429a-96ec-8d2eb372f399.jpg?x-oss-process=style/template01)

 

根据上面那张图 , 我们差不多可以看清一个topic的架构了 . 

先说文件是怎么存储的把 , 因为任何一个抽象的概念都对应这一个物理概念 .,不然找个就空的 . 

首先一个Topic名为 `TName`的 , 此时他的分区名字叫做 `TName-0`,`TName-1`,`TName-2` 的三个文件夹 , 举个栗子 `TName-0` 文件夹里 ,会对应着3个`segment` ,每一个`segment`会是一个 `偏移量.index`和`偏移量.log`的文件 ,所以一共6个文件 , 三个log ,三个index . 

再看看 `index` 和 `log `究竟是啥吧 : 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-09/c9e2cddc-5128-4993-97f3-ff02a6a4bfde.jpg?x-oss-process=style/template01)

其中 , 我们比如说拿着一个`offset =3` 的去找 ,会先找到 `segment-1 `,然后去找` 0.index `,去里面找偏移量为3的 ,然后找到一个物理偏移值 ,拿着这个值 我们又去了 `0.log` 文件, 终于找到了我们想要的数据 .

### 2. 分区策略

我们将一个信息 发出去 ,这个信息主要包含 

