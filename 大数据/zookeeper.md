# Zookeeper

## 介绍

![](http://zookeeper.apache.org/images/zookeeper_small.gif)



官网地址 : http://zookeeper.apache.org/

什么是zookeeper

> ​	ZooKeeper is a centralized service for maintaining configuration information, naming, providing distributed synchronization, and providing group services. All of these kinds of services are used in some form or another by distributed applications. Each time they are implemented there is a lot of work that goes into fixing the bugs and race conditions that are inevitable. Because of the difficulty of implementing these kinds of services, applications initially usually skimp on them, which make them brittle in the presence of change and difficult to manage. Even when done correctly, different implementations of these services lead to management complexity when the applications are deployed.



## 快速开始

- 单机搭建

```shell
网址  :  http://archive.apache.org/dist/zookeeper/ 自己下载或者用我这个
wget http://archive.apache.org/dist/zookeeper/zookeeper-3.4.10/zookeeper-3.4.10.tar.gz

tar -zxvf zookeeper-3.4.10.tar.gz -C /opt/software

记得修改权限
chown -R admin:admin zookeeper-3.4.10

然后修改配置文件,conf目录下
cp zoo_sample.cfg zoo.cfg
vim zoo.cfg  修改下面这段 
# the directory where the snapshot is stored.
# do not use /tmp for storage, /tmp here is just 
# example sakes.
dataDir=/opt/software/zookeeper-3.4.10/zkData

然后进入bin目录 直接执行 
./zkServer.sh  start 
查看状态
./zkServer.sh  status
关闭
./zkServer.sh  stop

也可以建立 软连接  ls -s /opt/software/zookeeper-3.4.10/bin/zkServer.sh  /usr/bin/zkServer
```

- 集群搭建

```shell
修改配置文件,在配置文件最下面添加 
server.id  =   ip  : 通信端口号: 选举端口
server.1=hadoop1:2888:3888
server.2=hadoop2:2888:3888
server.3=hadoop3:2888:3888

然后进入你的 /opt/software/zookeeper-3.4.10/zkData 
执行下面这行(意思就是告诉zk , 你的id 是 2) 记得跟集群对应起来
echo 2 > myid

修改zk的bin 目录下的 zkENV.sh , 将ZOO_LOG_DIR的日志目录改成自己新建的 ,只有远程启动才需要配置JAVA_HOME
if [ "x${ZOO_LOG_DIR}" = "x" ]
then
    ZOO_LOG_DIR="/opt/software/zookeeper-3.4.10/logs"
fi


然后你执行 zkServer.sh  start  就行了, 三台机器启动就行.


客户端连接 就是 zkCli.sh
```



- 客户端命令 

```shell
ls 查看文件(zk里文件就是节点)

ls / watch 监控根节点下的变化state (create操作)

create /app content
	-s 创建一个带序号的节点 会给你节点名字添加	一个序号
	-e 创建一个临时节点,执行退出(quit)就删除了,生命周期一个session
所以一共四种节点

get /app   获取节点信息
get /app watch  可以监控节点信息变化(set操作)
set /app contents  修改
stat /app  查看状态  stat结构体
history  查看操作	的记录
delete  删除文件,无法删除有子节点的
rmr 递归删除
```



### zob协议 (zookeeper atomic broadcase)  zk原子广播 (原子性和数据一致性)

- 选举机制
- 写数据流程



### zk的选举leader 的流程

> ​	status :  looking leader follower observer
>
> 刚开始启动都为 looking 状态 ,  每台机器启动会投自己一票 , 互相选举的时候选(myid和czid(版本最新的)大的)机器 , 1  2 3  4  5 顺序启动  当到3的时候已经半数以上了,所以 4 5 会跟随 1 2 3 投 3 ,所以 leader是3  



### zk的写数据流程

> 1. 当client发送了一条写请求, server接收到写请求,会将请求给leader,leader会同意,此时广播给其他的server,当有半数以上server同意时,将数据写入,然后发送给clinet通知,
> 2. 细节: server同意与不同意 是看数据的版本是否和写入数据的版本一致性,不一致则重启去leader拉去,