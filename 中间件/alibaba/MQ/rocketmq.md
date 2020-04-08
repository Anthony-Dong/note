# rocketmq

## 为什么选择消息队列

- 原因只有一个 解耦写入和处理能力的差异,避免写入太多,处理不过来
- 异步处理给我带来什么好处
  - 写入操作和后续处理进行解耦,异步处理,此时用户只用点击一下请求,后续的记录日志,统计用户行为,以及写入用户信息等,操作都不需要用户进行等待,这时候用消息队列就很好
- 解耦
  - 当我们的业务进行拓展时,不用修改原来模块的代码,如果原来是 操作A模块,需要调用B,C模块 , 如果我又增加一个模块怎么办,所以使用mq,只需要保证消息的可靠性
- 我们不能盲目的使用mq,需要想到 幂等操作 和 消息的可靠性保证,有些业务就是强耦合,

## 1.配置环境

- 去官网下载 [下载地址](https://rocketmq.apache.org/release_notes/release-notes-4.5.2/)

- 环境搭建

- ```shell
  # 上传安装包
  # unzip rocketmq-all-4.4.0-bin-release.zip
  # mv rocketmq-all-4.4.0-bin-release rocketmq
  # 这时进去bin 目录跟着 demo 走一遍就行了,已经OK 了
  ```

- 快速开始(官网demo自己跟着敲)     [测试demo](https://rocketmq.apache.org/docs/quick-start/)

## 2. 搭建服务

- 修改 conf/2m-2s-sync/broker-a.properties 文件 (文章最后有配置文件详情)

- 修改 conf/2m-2s-sync/broker-b-s.properties 文件

- 注意 集群至少两台机器,否者无法启动 (人家已经写了 2m-2s了 )

- 新建目录 

- ```shell
  [root@iz2zegua78a74kqn8clokfz rocketmq]# mkdir store
  [root@iz2zegua78a74kqn8clokfz rocketmq]# cd store/
  [root@iz2zegua78a74kqn8clokfz store]# ll
  total 0
  [root@iz2zegua78a74kqn8clokfz store]# mkdir commitlog
  [root@iz2zegua78a74kqn8clokfz store]# mkdir consumequeue
  [root@iz2zegua78a74kqn8clokfz store]# mkdir index
  [root@iz2zegua78a74kqn8clokfz store]# mkdir checkpoint
  [root@iz2zegua78a74kqn8clokfz store]# mkdir abort
  ```

- 进入conf目录执行 `sed -i 's#${user.home}#/opt/rocketmq#g' *.xml` 修改logback的日志文件位置
- 进入 /bin目录下  修改 `runbroker.sh` 和 `runserver.sh` 文件的 jvm内存大小,根据电脑内存大小分配
- 建立mqshutdown 软链接 `ln -s /opt/rocketmq/bin/mqshutdown /usr/local/bin/mqshutdown`
- 单机启动时就直接这样子就行
  - `nohup sh mqnamesrv &`
  -   `nohup sh mqbroker -c ../conf/broker.conf -n 47.94.234.232:9876  &`
  - `jps` 查看进程 ,有 两个  就 成功了 一个是 **NamesrvStartup** ,另外一个是 **BrokerStartup** 
  - 然后 用 `netstat -nlp |grep LISTEN`  查看端口,记得开放全部这些端口,不然无法运行
- 关闭
  - mqshutdown namesrv
  - mqshutdown broker



## 2. rocketmq 中 相关概念

- 图

### 1.Name Server

​		它是一个几乎无状态节点(各个nameserver都一样上面的信息)，可集群部署，节点之间无任何信息同步。是专为 RocketMQ设计的轻量级名称服务，代码小于1000行，具有简单、可集群横吐扩展、无状态等特点。将要支持的主备自动切换功能会强依赖 Name Server。所以他放弃了选择Zookeeper这个管理工具.

​		Namesrv压力不会太大，平时主要开销是在**维持心跳**和提供**Topic-Broker**的关系数据。

​		但有一点需要注意，Broker向Namesrv发心跳时， 会带上当前自己所负责的所有Topic信息，如果Topic个数太多（万级别），会导致一次心跳中，就Topic的数据就几十M，网络情况差的话， 网络传输失败，心跳失败，导致Namesrv误认为Broker心跳失败。

### 2. Broker

​		Broker 部署相对复杂，Broker分为Master与Slave，一个Master可以对应多个Slave，但是一个Slave只能对应一个Master，Master与Slave的对应关系通过指定相同的BrokerName，不同的BrokerId来定义，BrokerId为0表示Master，非0表示Slave。

​	 单个Broker跟所有Namesrv保持心跳请求，心跳间隔为30秒，心跳请求中包括当前Broker所有的Topic信息。Namesrv会反查Broer的心跳信息， 如果某个Broker在2分钟之内都没有心跳，则认为该Broker下线.

​	同理 他们之间各种关系都是这种心跳检测机制,下面不描述了.

### 3.Producer

​		Producer 与Name Server集群中的其中一个节点（随机选择，但不同于上一次）建立**长连接**，每30秒从Namesrv获取Topic跟Broker的映射关系，更新到本地内存中。再跟Topic涉及的所有Broker建立长连接，每隔30秒发一次心跳。在Broker端也会每10秒扫描一次当前注册的Producer，如果发现某个Producer超过2分钟都没有发心跳，则断开连接。

​		Producer 发送时，会**自动轮询当前所有可发送的broker**，一条消息发送成功，下次换另外一个broker发送，以达到消息平均落到所有的broker上。(这个类似与**负载均衡**)

​		假如某个Broker宕机，意味生产者最长需要30秒才能感知到。在这期间会向宕机的Broker发送消息。当一条消息发送到某个Broker失败后，会往该broker自动再重发2次，假如还是发送失败，则抛出发送失败异常。业务捕获异常，重新发送即可。客户端里会自动轮询另外一个Broker重新发送，这个对于用户是透明的。

### 4.consumer

​		Consumer 与 Name Server 集群中的其中一个节点（随机选择）建立长连接，定期从 Name Server 取 Topic 路由信息，并向提供 Topic 服务的 Master、Slave 建立长连接，且定时向 Master、Slave 发送心跳。Consumer既可以从 Master 订阅消息，也可以从 Slave 订阅消息，订阅规则由 Broker 配置决定。

- 消费端 接收消息分为两种模式 :

  -  **广播模式**  (rocketmq会将**消息发送给group中的每一个消费者**，如果这种模式在公司的项目中，会造成消息重复消费的问题，理论上会有N-1次重复消费，那么rocketmq为什么还会保留这种消费模式呢？存在必有它的道理，比方说，**如果需要动态更细一些配置，我们需要在不重启服务的情况下，将新的配置推送给group中的每一个消费者，这时候广播消费就发挥它的独到之处了**)
  - **集群消费**(默认方式)  集群消费是用的最广泛的一种消费模式，在集群消费模式下，同一条消息，**只能被group中的任意一个消费者消费**，这个概念很重要，这是与广播消费的最明显区别。

- RocketMQ中Consumer有两种实现：PushConsumer和PullConsumer。

  - **PushConsumer**(由broker进行推送)

    推送模式（虽然RocketMQ使用的是长轮询）的消费者。消息的能及时被消费。使用非常简单，内部已处理如线程池消费、流控、负载均衡、异常处理等等的各种场景。

  - **PullConsumer**(自己拉去,需要手动处理)

  - 拉取模式的消费者。应用主动控制拉取的时机，怎么拉取，怎么消费等。主动权更高。但要自己处理各种场景。

### 5.Group

​	标识发送同一类消息的Producer，通常发送逻辑一致。发送普通消息的时候，仅标识使用，并无特别用处。

​	 **RocketMQ要求同一个Consumer Group的消费者必须要拥有相同的注册信息，即必须要听一样的topic(并且tag也一样)。**

### 6.Topic

​		每个主题可设置队列个数，自动创建主题时默认是4个多列. Broker上存Topic信息，Topic由多个队列组成，队列会平均分散在多个Broker上。

### 7. tag

​		RocketMQ支持给在发送的时候给topic打tag，同一个topic的消息虽然逻辑管理是一样的。但是消费topic1的时候，如果你订阅的时候指定的是tagA，那么tagB的消息将不会投递。

### 8. Message Queue

​		消息物理管理单位。一个Topic将有若干个Q(默认4条)。若Topic同时创建在不同的Broker，则不同的broker上都有若干Q，消息将物理地存储落在不同Broker结点上，具有水平扩展的能力。



### 9. offset

### 10. 总结

​		RocketMQ的高性能在于顺序写盘(CommitLog)、零拷贝和跳跃读(尽量命中PageCache)，高可靠性在于刷盘和Master/Slave，另外NameServer 全部挂掉不影响已经运行的Broker,Producer,Consumer。

​	最新版RocketMQ已经可以支持Protobuf协议，但本文只探究默认的基于JSON的协议。





## 3. rocketmq 的基本用法

-  [github项目地址,所有demo都在里面](https://github.com/Anthony-Dong/rocketmq-practice)

### 1. 导入maven坐标

```java
        <dependency>
            <groupId>org.apache.rocketmq</groupId>
            <artifactId>rocketmq-client</artifactId>
            <version>4.4.0</version>
        </dependency>
        
            最好和你的 rocketmq的版本一致
```



### 2. 提供方代码

```java
        // 1.创建生成者对象
        DefaultMQProducer producer = new DefaultMQProducer("demo-produce");
        producer.setVipChannelEnabled(false);
        producer.setNamesrvAddr("192.168.58.129:9876");

        //2. 开启produce
        producer.start();

        //3. 创建消息对象
        Message message = new Message("SyncProducer",//主题
                "tag", //标签
                "keys", //key 消息唯一值
                "hello".getBytes(RemotingHelper.DEFAULT_CHARSET)//发送内容
                );

        //4. 发送同步消息消息
        SendResult send = producer.send(message);

        //5. 关闭连接(最好整一个连接池)
        producer.shutdown();
```



### 3. 消费方代码

```java

        //1. 创建消费者对象
        DefaultMQPushConsumer consumer = new DefaultMQPushConsumer("demo-consumer");

        // 2. 指定 nameserver 地址(地址也可以写多个,用';'隔开)
        consumer.setNamesrvAddr("47.94.234.232:9876");

        // 3. 设置主题,指定topic, tag(可以选多个 , 用 ' || ' 隔开)
        consumer.subscribe("AsyncProducer", "*");

        //4. 开启消息监听,设置回调函数,处理消息 (这里不区分 异步,同步 ,单向发送)
        consumer.registerMessageListener(new MessageListenerConcurrently() {
            @Override
            public ConsumeConcurrentlyStatus consumeMessage(List<MessageExt> list, 		ConsumeConcurrentlyContext consumeConcurrentlyContext) {

                // MessageExt 返回的  body 是一个 byte[] 需要 做转换
                list.forEach((e)->{
                    String msg = new String(e.getBody());
                    System.out.println(msg);
                });
                // 指定消费成功后的反馈状态
                return ConsumeConcurrentlyStatus.CONSUME_SUCCESS;
            }
        });
        // 5.开启消费
        consumer.start();
```

> 
>
> 我们发现代码都是模块化的,所以后面我选择只截取片段代码,整个demo会放到github上

## 4. rocketmq的三种消息发送方式

### 1.发送同步消息

```
  		4. 发送同步消息消息
        SendResult send = producer.send(message);
        
        同步发送是指消息发送方发出数据后，会在收到接收方发回响应之后才发下一个数据包的通讯方式。
        此种方式应用场景非常广泛，例如重要通知邮件、报名短信通知、营销短信系统等。
  		   
```

### 2. 发送异步消息

```java
        //4. 发送异步消息
        // SendCallback 是一个回调函数
        producer.send(message, new SendCallback() {
            @Override
            public void onSuccess(SendResult sendResult) {
                System.out.println(sendResult);
            }
            
            @Override
            public void onException(Throwable throwable) {
                System.out.println("发送失败,出现异常");
            }
        });
	异步发送是指发送方发出数据后，不等接收方发回响应，接着发送下个数据包的通讯方式。 	
	 通常用于对发送消息响应时间要求更高/更快的场景：
```



### 3. 发送单向消息(只负责发送,不管是否收到,无返回值)

```java
producer.sendOneway(message);
```

## 5. rocketmq 的发送方式

### 1. 延迟发送

```java
  message.setDelayTimeLevel(2);

(messageDelayLevel = "1s 5s 10s 30s 1m 2m 3m 4m 5m 6m 7m 8m 9m 10m 20m 30m 1h 2h")
这个不可以自定义(custom) 只能从1开始选择对于的level
```

### 2. 发送批量消息(注意发送的大小,太大注意超时时间问题)

```java
   	public SendResult send(Collection<Message> msgs)
      
    public SendResult send(Collection<Message> msgs, long timeout)   
      
	public SendResult send(Collection<Message> msgs, MessageQueue messageQueue,long timeout)  顺序发送就需要指定这个 MessageQueue
    
     //	sendCallback Callback to execute on sending completed, either successful or unsuccessful.
     public void send(Message msg, SendCallback sendCallback)
      
      
      太多了我就不举例子了大概这些 
```



### 3.过滤消息

```java
1. 消费方
// 3. 设置消息的描述,
// 指定topic, tag(可以选多个) ,这里执行过滤方法
// 3.1  || 表示选择
// 3.2  "*"  全部的tag
// 3.3  MessageSelector.bySql()  特殊选择 sql语法,可以去 官网去看
// consumer.subscribe("FilterTopic", "tag || tag2");
// consumer.subscribe("FilterTopic", "*");
// MessageSelector.bySql("key>8")
consumer.subscribe("FilterTopic", "*");

2. 提供方 
// 设置特殊属性
message.putUserProperty("key", "10");
```



### 4. 顺序发送(队列是多条,消费是多线程并行接收,如何保证)

```java
 
1. 提供方: 主要是依靠了只发送到一个队列中
producer.send(message, new MessageQueueSelector() {
                /**
                 * 消息队列选择器 MessageQueueSelector
                 *
                 * @param list      消息队列集合(保证选择 一个消息队列)
                 * @param message   所要发送的消息
                 * @param o         key值(要保证你的顺序队列持有一个相同的key值,我这里选择的是订单对象的hash值)
                 * @return
                 */
                @Override
                public MessageQueue select(List<MessageQueue> list, Message message, Object o) {
                    int size = list.size();
                    Integer key = (Integer) o;
                    MessageQueue messageQueue = list.get(key % size);
                    return messageQueue;
                }
            }, list.hashCode());

2.消费方 单线程消费 

        consumer.registerMessageListener(new MessageListenerOrderly() {
            @Override
            public ConsumeOrderlyStatus consumeMessage(List<MessageExt> list, ConsumeOrderlyContext consumeOrderlyContext) {
                list.forEach((e)->{
                    System.out.println("线程名称: "+Thread.currentThread().getName());
                    System.out.println("消息"+new String(e.getBody()));
                });
                System.out.println("发送成功");
                return ConsumeOrderlyStatus.SUCCESS;
            }
        });

```



### 5. 事务控制

- ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/114f2cce-77f6-4c4b-9ca5-e1f228b20523.png?x-oss-process=style/template01)

```java
这时候就需要TransactionMQProducer
TransactionMQProducer producer = new TransactionMQProducer("demo-produce");

// 2. 设置 事务监听器(对事物的控制) 每当发送消息都会执行这一步
producer.setTransactionListener(new TransactionListener() {
    /**
     *  执行的 事务方法 ,如果 状态为 UNKNOW  会调用 checkLocalTransaction进行判断
     * @param message  消息对象( Half(prepare) message)
     * @param o        o 是 自定义的消息参数 ( Custom business parameter)
     * @return  COMMIT_MESSAGE 提交
     *          ROLLBACK_MESSAGE  回滚
     *          UNKNOW  未处理的状态 , 等待checkLocalTransaction调用
     */
    @Override
    public LocalTransactionState executeLocalTransaction(Message message, Object o) {
        if (StringUtils.equals(message.getTags(), "TAG1")) {
            System.out.println(message.getTags());
            return LocalTransactionState.COMMIT_MESSAGE;
        }
        if (StringUtils.equals(message.getTags(), "TAG2")) {
            System.out.println(message.getTags());
            return LocalTransactionState.ROLLBACK_MESSAGE;
        }
            System.out.println(message.getTags());
            return LocalTransactionState.UNKNOW;

    }

    /**
     *
     * 进行对UNKNOW状态的检测
     * @param messageExt
     * @return
     */
    @Override
    public LocalTransactionState checkLocalTransaction(MessageExt messageExt) {
        return LocalTransactionState.COMMIT_MESSAGE;
    }
});

```



### 6. springboot整合rocketmq

- https://github.com/apache/rocketmq-spring  下载地址跟着跑一边

- 如果大家不会整,我会把demo整个上传到github上

  ```shell
  mvn clean
  
  mvn package -Dmaven.test.skip=true
  
  mvn install:install-file -Dfile=D:\代码库\alibaba-package\springboot-rocketmq\rocketmq-spring\rocketmq-spring-boot\target\rocketmq-spring-boot-2.0.4-SNAPSHOT.jar  -DgroupId=org.apache.rocketmq -DartifactId=rocketmq-spring-boot-starter  -Dversion=2.0.4-SNAPSHOT -Dpackaging=jar
  
  ```

- [快速开始跟着](https://github.com/apache/rocketmq-spring)  跟着这个敲一边就行了 我的demo也是跟着这个走的

- 关于[RocketMQ ACL的使用指南](https://blog.csdn.net/prestigeding/article/details/94317946)  不懂的看一下其实就是一个权限控制(access control list),默认 密码是AK , AS











## QA

大家可以去参考  [ 这个链接  ](https://zhuanlan.zhihu.com/rocketmq)  ,里面讲的相当详细