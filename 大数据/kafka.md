# kafka 

## 介绍

![](http://kafka.apache.org/images/logo.png)

官网 http://kafka.apache.org/

介绍 http://kafka.apache.org/intro





### 快速开始

路径: http://kafka.apache.org/downloads

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
kafka-server-start.sh -daemon config/server.properties

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



### CRUD 使用

```shell
1. 增加
kafka-topics.sh --zookeeper localhost:2181 --create --topic myfirsttopic --partitions 1 --replication-factor 1
意思就是: 执行zk地址 创建topic 指定topic名字  创建1个分区 创建一个副本

此时 zkCli查看
[zk: localhost:2181(CONNECTED) 6] ls /
[app, cluster, controller, controller_epoch, brokers, zookeeper, admin, isr_change_notification, consumers, latest_producer_id_block, config]
[zk: localhost:2181(CONNECTED) 7] ls /brokers
[ids, topics, seqid]
[zk: localhost:2181(CONNECTED) 8] ls /brokers/topics
[myfirsttopic] (这里多了一个topic节点)
[zk: localhost:2181(CONNECTED) 9] 

2.查看全部信息
kafka-topics.sh --zookeeper localhost:2181 --list

查看某个节点信息
kafka-topics.sh --zookeeper localhost:2181 --describe --topic myfirsttopic

[admin@localhost kafka_2.11-0.11.0.3]$ kafka-topics.sh --zookeeper localhost:2181 --describe --topic myfirsttopic
Topic:myfirsttopic	PartitionCount:1	ReplicationFactor:1	Configs:
	Topic: myfirsttopic	Partition(分区从0开始): 0	Leader(所位于的broker-id): 0	Replicas(副本所在的broker-id位置): 0	Isr(和leader保持同步的副本集合): 0

3. 删除
kafka-topics.sh --zookeeper localhost:2181 --delete --topic myfirsttopic

4.改(alter)
kafka-topics.sh --zookeeper localhost:2181 --alter --topic myfirsttopic --patication 2(修改后的分区只能增加不能减少)



5.启动生产者消费者
kafka-console-producer.sh --topic myfirsttopic --broker-list localhost:9092(可以指定多个)

第一种方式
kafka-console-consumer.sh --topic myfirsttopic --bootstrap-server localhost:9092

第二种方式
kafka-console-consumer.sh --topic myfirsttopic --bootstrap-server localhost:9092 --from-beginning(从头开始消费,涉及到offset)
```

