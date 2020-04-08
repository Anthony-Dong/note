# 快速入门 Kafka (Java版本)

> ​	官网 :  [http://kafka.apachecn.org/documentation.html](http://kafka.apachecn.org/documentation.html)  开始学习吧. 



先启动zk , 然后启动kafka .就行了, 安装不做介绍了, 都是简单修改下配置就行了. 相信都会. 不过我担忧下载的网速(真的坑,先下载不懂,所以我找了个老版本).  前提环境就是有Java环境就可以了.

kafka版本号 前面部分是scala版本,后面部分是kafka版本, 我是用的是`0.11.0.3` 版本, 所以客户端最好跟着一致.

```java
[admin@hadoop1 kafka]$ ll
total 0
drwxr-xr-x 7 admin admin 94 Nov 10 06:18 kafka_2.11-0.11.0.3
```

启动命令 : 

```java
// 1. 测试环境推荐, 可以实时看日志报错.
./kafka-server-start.sh ../config/server.properties

// 2.线上推荐, 后台模式
./kafka-server-start.sh -daemon ../config/server.properties
```

## 快速开始 

> ​	最好加入日志框架, 让后放入配置文件, 看日志很重要. 

```xml
<dependencies>
    <dependency>
        <groupId>org.apache.kafka</groupId>
        <artifactId>kafka-clients</artifactId>
        <version>0.11.0.3</version>
    </dependency>
	// LOG4J 配置...
</dependencies>
```



服务器端, 

`org.apache.kafka.clients.CommonClientConfigs`    下面就是客户端/服务器的通用配置

`org.apache.kafka.clients.producer.ProducerConfig`  是生产者的配置信息.  

`org.apache.kafka.clients.consumer.ConsumerConfig`  是消费者配置.

`org.apache.kafka.common.config.TopicConfig` 是topic的通用配置信息. 

`org.apache.kafka.common.config.SslConfigs `  SSL配置

这些配置信息都是 _Config表示的配置的key , _Doc表示解释. 我表示不理解为啥呢. 哈哈哈不浪费内存吗

一般只是用 `ConsumerConfig`  和 `ProducerConfig` 足矣了. 

 

配置太多. 但是有个技巧. 我这里写了个程序反射获取他的成员变量. 然后可以输出他的配置信息. 

```java
Field field = ConsumerConfig.class.getDeclaredField("CONFIG");
field.setAccessible(true);
ConfigDef def = (ConfigDef) field.get(null);
String htmlTable = def.toHtmlTable();
FileOutputStream stream = new FileOutputStream("consumer.html");
stream.write(htmlTable.getBytes());
stream.close();
```

如果配置不懂的这个官方网站也可以看  [http://kafka.apachecn.org/documentation.html#producerconfigs](http://kafka.apachecn.org/documentation.html#producerconfigs)

基本就是文档了.,很详细./  那么开始吧. 

服务端代码: 

```java
public class Producer {

    public static void main(String[] args) {
         final String url = "hadoop1:9092";
		final String topic = "topic-1";
        // 配置.
        HashMap<String, Object> config = new HashMap<>();

        // 连接地址
        config.put(ProducerConfig.BOOTSTRAP_SERVERS_CONFIG, url);

        // ACK
        config.put(ProducerConfig.ACKS_CONFIG, "all");
        // 相应超时.
        config.put(ProducerConfig.TRANSACTION_TIMEOUT_CONFIG, 5000);

        // 缓冲区大小. (发送给服务器的)
        config.put(ProducerConfig.BUFFER_MEMORY_CONFIG, 1024 * 1024 * 10);
        // 每次最多发10K
        config.put(ProducerConfig.MAX_BLOCK_MS_CONFIG, 1024 * 10);
        // 不重试,有些非幂等性可以.
        config.put(ProducerConfig.RETRIES_CONFIG, 0);

        // snappy 压缩..
        config.put(ProducerConfig.COMPRESSION_TYPE_CONFIG, "snappy");

        // 序列化
        config.put(ProducerConfig.KEY_SERIALIZER_CLASS_CONFIG, StringSerializer.class);
        config.put(ProducerConfig.VALUE_SERIALIZER_CLASS_CONFIG, StringSerializer.class);

        // ok了.
        KafkaProducer<String, String> producer = new KafkaProducer<>(config);
        IntStream.range(0, 10).forEach(value -> {
            // 发送
            producer.send(new ProducerRecord<>(topic, "cur-time", String.format("id: %d, time : %d.", value, System.currentTimeMillis())), (metadata, exception) -> {
                
            });
        });
        // 最后记得刷新出去.
        producer.flush();
    }
}
```



消费端 : 

```java
public class Consumer {

    public static void main(String[] args) {

        final String topic = "topic-1";
        final String group = "consumer-1";
        final String url = "hadoop1:9092";

        HashMap<String, Object> config = new HashMap<>();
        config.put(ConsumerConfig.GROUP_ID_CONFIG, group);
        config.put(ConsumerConfig.BOOTSTRAP_SERVERS_CONFIG, url);
        config.put(ConsumerConfig.AUTO_OFFSET_RESET_CONFIG, "earliest");
        config.put(ConsumerConfig.ENABLE_AUTO_COMMIT_CONFIG, false);
        config.put(ConsumerConfig.KEY_DESERIALIZER_CLASS_CONFIG, StringDeserializer.class);
        config.put(ConsumerConfig.VALUE_DESERIALIZER_CLASS_CONFIG, StringDeserializer.class);

        KafkaConsumer<String, String> consumer = new KafkaConsumer<>(config);
        consumer.subscribe(Collections.singletonList(topic));
        while (true) {
            ConsumerRecords<String, String> poll = consumer.poll(500);
            poll.forEach(record ->
                    System.out.println(String.format("topic: %s, key: %s, value: %s, offset:%d.",
                            record.topic(), record.key(), record.value(), record.offset())));
            // 提交偏移量.
            consumer.commitSync();
        }
    }
}
```



结果 :  消费端输出: 

```java
topic: topic-1, key: cur-time, value: id: 0, time : 1582971209662., offset:0.
topic: topic-1, key: cur-time, value: id: 1, time : 1582971209859., offset:1.
topic: topic-1, key: cur-time, value: id: 2, time : 1582971209859., offset:2.
topic: topic-1, key: cur-time, value: id: 3, time : 1582971209859., offset:3.
topic: topic-1, key: cur-time, value: id: 4, time : 1582971209859., offset:4.
topic: topic-1, key: cur-time, value: id: 5, time : 1582971209859., offset:5.
topic: topic-1, key: cur-time, value: id: 6, time : 1582971209860., offset:6.
topic: topic-1, key: cur-time, value: id: 7, time : 1582971209860., offset:7.
topic: topic-1, key: cur-time, value: id: 8, time : 1582971209866., offset:8.
topic: topic-1, key: cur-time, value: id: 9, time : 1582971209867., offset:9.
```

我们可以发现kafka的偏移量是从0开始的.



我们发现服务端日志: 

```java
// 先去创建一个topic,去zk中.
[2020-03-01 02:10:26,840] INFO Topic creation {"version":1,"partitions":{"0":[0]}} (kafka.admin.AdminUtils$)
[2020-03-01 02:10:26,845] INFO [KafkaApi-0] Auto creation of topic topic-1 with 1 partitions and replication factor 1 is successful (kafka.server.KafkaApis)
[2020-03-01 02:10:26,891] INFO [ReplicaFetcherManager on broker 0] Removed fetcher for partitions topic-1-0 (kafka.server.ReplicaFetcherManager)
[2020-03-01 02:10:26,896] INFO Loading producer state from offset 0 for partition topic-1-0 with message format version 2 (kafka.log.Log)
// 创建日志文件.分区号是0
[2020-03-01 02:10:26,896] INFO Completed load of log topic-1-0 with 1 log segments, log start offset 0 and log end offset 0 in 1 ms (kafka.log.Log)
[2020-03-01 02:10:26,897] INFO Created log for partition [topic-1,0] in /home/admin/kafka/kafka_2.11-0.11.0.3/logs with properties {compression.type -> producer, message.format.version -> 0.11.0-IV2, file.delete.delay.ms -> 60000, max.message.bytes -> 1000012, min.compaction.lag.ms -> 0, message.timestamp.type -> CreateTime, min.insync.replicas -> 1, segment.jitter.ms -> 0, preallocate -> false, min.cleanable.dirty.ratio -> 0.5, index.interval.bytes -> 4096, unclean.leader.election.enable -> false, retention.bytes -> -1, delete.retention.ms -> 86400000, cleanup.policy -> [delete], flush.ms -> 9223372036854775807, segment.ms -> 604800000, segment.bytes -> 1073741824, retention.ms -> 604800000, message.timestamp.difference.max.ms -> 9223372036854775807, segment.index.bytes -> 10485760, flush.messages -> 9223372036854775807}. (kafka.log.LogManager)
    
    
// 消费者推出
[2020-03-01 02:12:03,532] INFO [GroupCoordinator 0]: Member consumer-1-f17da2c9-71ad-4035-b366-76bdebba5951 in group consumer-1 has failed, removing it from the group (kafka.coordinator.group.GroupCoordinator)    
```



## topic

topic的概念就是最小的主题单位, 不能比他在小了. 最起码也要有一个topic.  是消费的最小主题. 如果你学过RocketMQ那么他可能还有很多方式. 

我们要知道kafka是一个读写均衡的中间件, 所以他所做的  将top分区处理, 让

## 组/分区 究竟做什么

我们继续测试. 我们将服务器端不断发送. 

此时客户端同时有两个客户端再一个组内. 就是都是 `consumer-1` 

此时发现只有最先接入的那个组可以收到消息, 第二个不可以,  当第一个客户端退出的时候, 第二个客户端才去消费. 

我这里有一张图 : 

客户端二先去消费去了. 然后挂掉了. 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/8b645a15-a457-4603-97e8-9fc4238c43eb.png?x-oss-process=style/template01)

客户端一此时就终于可以收到消息了. 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/1463f5ff-0af8-459f-bf9e-130b42cf44fe.png?x-oss-process=style/template01)

此时我们发现客户端2成功拉去到了 397.

服务器日志是 :  

```java
[2020-03-01 02:13:41,787] INFO [GroupCoordinator 0]: Preparing to rebalance group consumer-1 with old generation 4 (__consumer_offsets-6) (kafka.coordinator.group.GroupCoordinator)
```

我们发现一个组内,是不可以同时消费的, 而且他限制组内只允许一个人访问. 



**kafka会帮助我们记住这个组的偏移量, 以组名字做区分, 所以一个新的组, 必须最开始开始读, 不能设置为"null"** 



### 偏移量

```java
long offset = producer.send(new ProducerRecord<>("topic-1", "cur-time", String.format("id: %d, time : %d.", value, System.currentTimeMillis())), (metadata, exception) -> {
}).get().offset();
logger.info("偏移量 : {}", offset);
```

我们可以通过以下方式来每次获取偏移量. 

消费端 偏移量,是通过这个参数设计的,  第一初始化组的时候不能使用"none" (因为Broker没有记录此消费者组的offset), 但是可以使用"earliest" .  如果其组内已经有人启动了, 那么此时就算你设置"earliest" , 也会根据组内人士偏移量决定的. 

```java
config.put(ConsumerConfig.AUTO_OFFSET_RESET_CONFIG, "earliest");
```



### 分区 和 组的关系

我们单机上再拷贝一个 kafka , 改成 partition为2 ,  只需要改以下三个参数 ,根据情况改,  (懒得开台虚拟机,外网测试根本连不上,不知道为啥哈哈哈.)

```properties
# 第一台我的是0 , 第二台是1
broker.id=1 
listeners=PLAINTEXT://:9093
log.dirs=/home/admin/kafka_2/kafka_2.11-0.11.0.3/logs
num.partitions=2
```



代码: 

```java
final String url = "hadoop1:9092,hadoop1:9093";
final String topic = "topic-2";
// 同时我们打印partition的位置
// 服务端
RecordMetadata future = producer.send(new ProducerRecord<>("topic-2", "cur-time" + value, String.format("id: %d, time : %d.", value, System.currentTimeMillis())), (metadata, exception) -> {
}).get();
logger.info("偏移量 : {} , 分区 : {}.", future.offset(), future.partition());

// 客户端
ConsumerRecords<String, String> poll = consumer.poll(500);
poll.forEach(record ->
        System.out.println(String.format("topic: %s, key: %s, value: %s, offset:%d , partition:%d.",
                record.topic(), record.key(), record.value(), record.offset(), record.partition())));
consumer.commitAsync();
```

启动. 一台生产者  , 三个消费者在同一个组内. 

我们发现 : 

生产者 : 

```java
2020-02-29 20:40:25,590 390057 [  main] INFO   com.example.producer.Producer  - 偏移量 : 610 , 分区 : 1.
2020-02-29 20:40:26,112 390579 [  main] INFO   com.example.producer.Producer  - 偏移量 : 611 , 分区 : 1.
2020-02-29 20:40:26,629 391096 [  main] INFO   com.example.producer.Producer  - 偏移量 : 508 , 分区 : 0.
2020-02-29 20:40:27,153 391620 [  main] INFO   com.example.producer.Producer  - 偏移量 : 612 , 分区 : 1.
2020-02-29 20:40:27,657 392124 [  main] INFO   com.example.producer.Producer  - 偏移量 : 613 , 分区 : 1.
2020-02-29 20:40:28,164 392631 [  main] INFO   com.example.producer.Producer  - 偏移量 : 509 , 分区 : 0.
```

消费者1: 

```java
topic: topic-2, key: cur-time756, value: id: 756, time : 1582980020546., offset:502 , partition:0.
topic: topic-2, key: cur-time757, value: id: 757, time : 1582980021048., offset:503 , partition:0.
topic: topic-2, key: cur-time758, value: id: 758, time : 1582980021552., offset:504 , partition:0.
topic: topic-2, key: cur-time759, value: id: 759, time : 1582980022056., offset:505 , partition:0.
topic: topic-2, key: cur-time760, value: id: 760, time : 1582980022561., offset:506 , partition:0.
```

消费者2: 

```jaa
topic: topic-2, key: cur-time804, value: id: 804, time : 1582980044811., offset:633 , partition:1.
topic: topic-2, key: cur-time810, value: id: 810, time : 1582980047837., offset:634 , partition:1.
topic: topic-2, key: cur-time812, value: id: 812, time : 1582980048844., offset:635 , partition:1.
topic: topic-2, key: cur-time813, value: id: 813, time : 1582980049347., offset:636 , partition:1.
topic: topic-2, key: cur-time815, value: id: 815, time : 1582980050352., offset:637 , partition:1.
topic: topic-2, key: cur-time820, value: id: 820, time : 1582980052870., offset:638 , partition:1.
```

消费者3: 显然是死的, 卡着不动, 此时当我们断掉消费者2 , 此时会打印如下日志. 成功均衡. 

```java
020-02-29 20:41:49,409 454117 [  main] INFO  .internals.AbstractCoordinator  - (Re-)joining group consumer-1
2020-02-29 20:41:51,850 456558 [  main] INFO  .internals.AbstractCoordinator  - Successfully joined group consumer-1 with generation 41
2020-02-29 20:41:51,851 456559 [  main] INFO  .internals.ConsumerCoordinator  - Setting newly assigned partitions [topic-2-1] for group consumer-1
topic: topic-2, key: cur-time912, value: id: 912, time : 1582980099342., offset:690 , partition:1.    
```



### 模拟down机,保证可靠性

当我们把一台机器关闭 , 生产者消费者都会抛出一下异常. 

```java
2020-02-29 20:56:11,733 202726 [ucer-1] WARN  he.kafka.clients.NetworkClient  - Connection to node 1 could not be established. Broker may not be available.
2020-02-29 20:56:13,785 204778 [ucer-1] WARN  he.kafka.clients.NetworkClient  - Connection to node 1 could not be established. Broker may not be available.
// 抛出一次. 就是无法和topic-2-0 也就是第0个分区联系.会等待30S超时,这个属性我们可以自己设置.
Caused by: org.apache.kafka.common.errors.TimeoutException: Expiring 1 record(s) for topic-2-0: 30084 ms has passed since batch creation plus linger time
2020-02-29 20:56:40,313 231306 [  main] INFO   com.example.producer.Producer  - 偏移量 : 836 , 分区 : 1.
```



这个会不断的重试. 失败就放弃,继续重试, 其实这个partition策略我们可以自己写. 

由于我们模拟的单线程操作, 也就是会阻塞. 所以很正常. 正常开发都是多线程. 但是这个超时是逃避不了的. 比如一个web 请求, 你这里超时30S, 这个绝对不行. 

所以我们调整参数,   改成3S.

```java
 config.put(ProducerConfig.REQUEST_TIMEOUT_MS_CONFIG, 3000);
```

此时报错就是 , 这个超时时间自己根据业务把握, 不一定越小越好. 

```java
Caused by: org.apache.kafka.common.errors.TimeoutException: Expiring 1 record(s) for topic-2-0: 3058 ms has passed since batch creation plus linger time
```



所以kafka的可靠性是极高的, 不会因为一个broker挂掉了,业务就无法进行了 . 此时连不上会将数据全部写到另外一个分区中, 当重新启动又会恢复平衡, 所以可靠性极高. 





### 分区策略

简单实现一个 `org.apache.kafka.clients.producer.Partitioner` 接口吧. 

```java
public class OrderPartitioner implements Partitioner {

    // topic计数器. 每个topic都维护一个计数器. 这里可以考虑把map设置为安全的, 因为会出现并发问题.
    private HashMap<String, AtomicInteger> map = new HashMap<>();

    private static final Function<String, AtomicInteger> provider = s -> new AtomicInteger(0);

    // 这里业务逻辑其实不对,如果写入失败,那么永远也是
    @Override
    public int partition(String topic, Object key, byte[] keyBytes, Object value, byte[] valueBytes, Cluster cluster) {
        List<PartitionInfo> list = cluster.availablePartitionsForTopic(topic);
        AtomicInteger integer = map.computeIfAbsent(topic, provider);
        return integer.incrementAndGet() % list.size();
    }

    @Override
    public void close() {
        // 清空释放内存
        map.clear();
    }

    @Override
    public void configure(Map<String, ?> configs) {
    }
}
```

简单的加入到 生成者的配置中去, 

```java
config.put(ProducerConfig.PARTITIONER_CLASS_CONFIG, OrderPartitioner.class);
```

结果就是 :  发现很均匀.

```java
2020-02-29 21:38:53,309 1358   [  main] INFO   com.example.producer.Producer  - 偏移量 : 1224 , 分区 : 1.
2020-02-29 21:38:53,813 1862   [  main] INFO   com.example.producer.Producer  - 偏移量 : 1006 , 分区 : 0.
2020-02-29 21:38:54,318 2367   [  main] INFO   com.example.producer.Producer  - 偏移量 : 1225 , 分区 : 1.
2020-02-29 21:38:54,820 2869   [  main] INFO   com.example.producer.Producer  - 偏移量 : 1007 , 分区 : 0.
2020-02-29 21:38:55,323 3372   [  main] INFO   com.example.producer.Producer  - 偏移量 : 1226 , 分区 : 1.
```



### 拦截器功能

```java
// 拦截器, 必须传入一个集合,
config.put(ProducerConfig.INTERCEPTOR_CLASSES_CONFIG, Collections.singletonList(MyProducerInterceptor.class));
```

简单写一个吧, 

```java
public class MyProducerInterceptor implements ProducerInterceptor {
    // 调用send方法会回调到这里.
    @Override
    public ProducerRecord onSend(ProducerRecord record) {
        System.out.println("MyProducerInterceptor-onSend ");
        return record;
    }

    // 当服务器返回数据会调用这里.
    @Override
    public void onAcknowledgement(RecordMetadata metadata, Exception exception) {
        System.out.println("MyProducerInterceptor-onAcknowledgement");
    }

    @Override
    public void close() {
    }

    @Override
    public void configure(Map<String, ?> configs) {
    }
}
```



我们再看看打印日志 : 

```java
MyProducerInterceptor-onSend
OrderPartitioner
MyProducerInterceptor-onAcknowledgement
2020-02-29 21:52:29,262 3371   [  main] INFO   com.example.producer.Producer  - 偏移量 : 1238 , 分区 : 1.
```

就是先拦截.  后给分区去处理, 如果你把send 方法返回一个null. 我的程序反正直接退出了, 哈哈哈哈.  因为`org.apache.kafka.clients.producer.KafkaProducer#send` 这里往下走, 有个sendon方法里需要处理这个结果. 所以基本上这个是用作包装或者统计, 或者实现回调 , 让主线程可以不使用get,使其阻塞. 就这个. 



### 总结一下. 

一 . 一个分区只能被同一个组内某一个人消费.

其实就是 一个消费者组  和分区是一一对应的, 也就是上面那个问题. 

两个分区, 但是一个消费组确有三个人/或者更多人消费. 此时只会有两个人,俩人各连一个分区. 

二. 分区数, 比如我第一个topic-1, 一开始是一个分区, 就算我服务器配置改成了两个分区, 此时需要修改元信息. 

三 . 一个分区默认就是有序的. 不用考虑顺序性.  对于顺序性比较强的业务可以考虑将其设置为一个分区, 获取通过接口编写你所需要的需求. 分区可以解决down机等问题, 所以并不推荐

四. 高可靠性, 可以保证一台服务器down机,其他仍然可以处理(主机从机一样,他会自行选举,我两台机器都没啥问题). 

五. 分区策略, 灵活性. 相信我这些基本满足你开发, .



以上虽然有大量的日志, 是让大家方便理解. 谢谢.  其实对于大多数概念来说, 比如自己练习练习 , 概念毕竟是概念, 





## 如何修改分区/副本数量

分区 / 副本是两个不同的概念.   分区属于leader , 副本属于follower , 所以这里就是一个防止leader  Down机的问题, follower只会做一件事就是同步leader.   但是follower不对外提供读写服务的 , 这里是为了防止数据不一致问题, 因为从机跟随会有延时的.  但是有些场景我感觉是满足的, 因为kafka这种一个分区对应一个组的一个消费者很好地捆绑并不会发生不一致问题 , 最多也就是一个读慢了, 个人觉得. 



在线修改配置. 这个简单 , 其实有些小伙伴经常查看zk的话, 发现他就是将信息保存在zk中 ,, 其实修改zk就可以了 , 

 比如查看 topic-2的主题信息 

```java
[admin@hadoop1 bin]$ ./kafka-topics.sh --zookeeper localhost:2181 --describe  --topic topic-2
Topic:topic-2	PartitionCount:2	ReplicationFactor:1	Configs:
	Topic: topic-2	Partition: 0	Leader: 1	Replicas: 1	Isr: 1
	Topic: topic-2	Partition: 1	Leader: 0	Replicas: 0	Isr: 0
```

Leader: 分区0的leader是主机1 上,分区1的leader是主机0.

Replicas : 分区0的副本在主机1上, 分区1的副本在主机0上

ISR : 表示副本跟随的进度. 如果和副本主机号一致 ,说明跟随一致 . 

如果我们想修改分区数量 : 

```java
./kafka-topics.sh --zookeeper localhost:2181 -alter --partitions 2 --topic topic-1
```

修改会有一个警告信息 , 就是分区可能影响你原来的业务逻辑.  不过提示成功了. 

```java
[admin@hadoop1 bin]$ ./kafka-topics.sh --zookeeper localhost:2181 -alter --partitions 2 --topic topic-1
WARNING: If partitions are increased for a topic that has a key, the partition logic or ordering of the messages will be affected
Adding partitions succeeded!
```

修改完赶紧测试一下.. 发现 , 其实不同分区的偏移量都是独立计算的. 不过也无所谓, 

```java
2020-02-29 22:32:59,876 1456   [  main] INFO   com.example.producer.Producer  - 偏移量 : 0 , 分区 : 1.
2020-02-29 22:33:00,408 1988   [  main] INFO   com.example.producer.Producer  - 偏移量 : 638 , 分区 : 0.
2020-02-29 22:33:00,915 2495   [  main] INFO   com.example.producer.Producer  - 偏移量 : 1 , 分区 : 1.
```



从机选举成主机的过程, 两台机器无法实现, 必须是 至少一个leader 和 两个follower才可以选举成功. 实在懒得测试请求理解.  哈哈哈哈. 其实这些都是运维做的. 可以感兴趣再测试一下. 很简单的. 







## 下一期 深入理解kafka



这一节只是了解了如何使用, 这个根本不够我们需要学习kafka是如何做的 . 实现高读写, 高可靠性, 高拓展性 , 这是一个分布式设计必备的. 




