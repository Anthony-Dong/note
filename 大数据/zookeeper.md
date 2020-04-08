# Zookeeper

## 介绍

![](http://zookeeper.apache.org/images/zookeeper_small.gif)



[官网地址](http://zookeeper.apache.org/) : http://zookeeper.apache.org/

什么是zookeeper

> ​	ZooKeeper is a centralized service for maintaining configuration information, naming, providing distributed synchronization, and providing group services. All of these kinds of services are used in some form or another by distributed applications. Each time they are implemented there is a lot of work that goes into fixing the bugs and race conditions that are inevitable. Because of the difficulty of implementing these kinds of services, applications initially usually skimp on them, which make them brittle in the presence of change and difficult to manage. Even when done correctly, different implementations of these services lead to management complexity when the applications are deployed.
>
> zookeeper是一个集中式的服务 , 用于维护配置信息、命名、提供分布式同步和提供组服务。
>
> ​		我对与 zookeeper 的理解 就是他是一个分布式服务的一个高可用的注册中心,他自带对节点检测,不用用户去做心跳检测.保证了服务的可用性.  同时他可以保存一些配置信息,去做分布式配置中心 .



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

也可以建立 软连接  
ln -s /opt/software/zk/zookeeper-3.4.10/bin/zkServer.sh  /usr/bin/zkServer
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



## ZK选举leader流程 

> 每一个集群节点的status :  looking  leader  follower  observer 
>
> Looking：系统刚启动时或者Leader崩溃后正处于选举状态
>
> follower  :  小弟
>
> leader   : 领导
>
> 刚开始启动都为 looking 状态 ,  每台机器启动会投自己一票 , 互相选举的时候选(myid和czid(版本最新的)大的)机器 , 1  2 3  4  5 顺序启动  当到3的时候已经半数以上了,所以 4 5 会跟随 1 2 3 投 3 ,所以 leader是3  
>
> （1）服务器1启动，发起一次选举。服务器1投自己一票。此时服务器1票数一票，不够半数以上（3票），选举无法完成，服务器1状态保持为LOOKING；
>
> （2）服务器2启动，再发起一次选举。服务器1和2分别投自己一票并交换选票信息：此时服务器1发现服务器2的ID比自己目前投票推举的（服务器1）大，更改选票为推举服务器2。此时服务器1票数0票，服务器2票数2票，没有半数以上结果，选举无法完成，服务器1，2状态保持LOOKING
>
> （3）服务器3启动，发起一次选举。此时服务器1和2都会更改选票为服务器3。此次投票结果：服务器1为0票，服务器2为0票，服务器3为3票。此时服务器3的票数已经超过半数，服务器3当选Leader。服务器1，2更改状态为FOLLOWING，服务器3更改状态为LEADING；
>
> （4）服务器4启动，发起一次选举。此时服务器1，2，3已经不是LOOKING状态，不会更改选票信息。交换选票信息结果：服务器3为3票，服务器4为1票。此时服务器4服从多数，更改选票信息为服务器3，并更改状态为FOLLOWING；
>
> （5）服务器5启动，同4一样当小弟。



## ZK的写数据流程

> 1. 当client发送了一条写请求, server接收到写请求,会将请求给leader,leader会同意,此时广播给其他的server,当有半数以上server同意时,将数据写入,然后发送给clinet通知,
> 2. 细节: server同意与不同意 是看数据的版本是否和写入数据的版本一致性,不一致则重启去leader拉去,



![]( https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-29/c77b6bb5-4c9a-4571-8eb7-f5d1bf600539.png?x-oss-process=style/template01)

## zab协议(zookeeper atomic broadcast)

[文章链接](https://blog.csdn.net/jin5203344/article/details/53142027)   https://blog.csdn.net/jin5203344/article/details/53142027

其实就是  zk选举流程 和 写数据流程 

## Zookeeper  API 使用

### 创建节点

记住 Ephemerals cannot have children 临时节点不准有子节点 , 但是持久节点可以有临时节点

```
public String create(final String path, byte data[], List<ACL> acl,        CreateMode createMode)

// 使用 就是如下 ,这个代表的含义是 完全开放的权限 ,且是一个永久节点
String path = zkCli.create("/idea", "HELLO WORLD".getBytes(), ZooDefs.Ids.OPEN_ACL_UNSAFE,CreateMode.PERSISTENT);
```

1. path  是 节点路径
2. data  是 节点数据
3. acl  : Access Control List 访问控制列表 ,  可以通过 `zkCli.setACL(path,list, 1);`设置我也不了解.
4. CreateMode  : 
   1. `PERSISTENT`   : The znode will not be automatically deleted upon client's disconnect.
   2. `PERSISTENT_SEQUENTIAL`   its name will be appended with a **monotonically increasing number**.(单调递增)
   3. `EPHEMERAL`  : The znode will be deleted upon the client's disconnect.
   4. `EPHEMERAL_SEQUENTIAL ` :  its name will be appended with a monotonically increasing number.



### 节点类型 

ephemeral 节点 ? 他的生命周期就是一个session  ,就是客户端和服务器端的一次会话 

persistent 节点 ? 他的生命周期是永久存储

![]( https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-29/31b60435-ad6c-4d50-9797-a914d4ddad4d.png?x-oss-process=style/template01)



最后给一些权限组合的测试结果：

要修改某个节点的ACL属性，必须具有read、admin二种权限

要删除某个节点下的子节点，必须具有对父节点的read权限，以及父节点的delete权限



### 节点状态

```shell
[zk: localhost:2181(CONNECTED) 175] get /test/test1
test
cZxid = 0x800000261
ctime = Wed Oct 30 04:32:59 CST 2019
mZxid = 0x800000261
mtime = Wed Oct 30 04:32:59 CST 2019
pZxid = 0x800000261
cversion = 0
dataVersion = 0
aclVersion = 0
ephemeralOwner = 0x36e1828d3230000
dataLength = 4
numChildren = 0
```

1) czxid  : 创建节点的事务zxid

每次修改ZooKeeper状态都会收到一个zxid形式的时间戳，也就是ZooKeeper事务ID。

事务ID是ZooKeeper中所有修改总的次序。每个修改都有唯一的zxid，如果zxid1小于zxid2，那么zxid1在zxid2之前发生。

2）ctime - znode被创建的毫秒数(从1970年开始)

3）mzxid - znode最后更新的事务zxid

4）mtime - znode最后修改的毫秒数(从1970年开始)

5）pZxid-znode最后更新的子节点zxid

6）cversion - znode子节点变化号，znode子节点修改次数

7）dataversion - znode数据变化号

8）aclVersion - znode访问控制列表的变化号

9）ephemeralOwner- 如果是临时节点，这个是znode拥有者的session id。如果不是临时节点则是0。

10）dataLength- znode的数据长度

11）numChildren - znode子节点数量

### 删除节点

```java
// 也是必须知道 version 版本 
zkCli.delete("/idea", stat.getVersion());
```



### 修改节点

```
// 后面有个stat , 因为我们不知道默认 null ,返回值为 zNode数据,
byte[] data = zkCli.getData("/server", true,null);

// 修改 节点 , 没一次修改会使得 version+1 ,所以需要首先获取version ,需要调用
// Stat stat = zkCli.exists("/server", false); 这个函数 , 来获取stat,
Stat Nstat = zkCli.setData("/server", "contents2".getBytes(), stat.getVersion());
```



### 查找节点

```java
// 1. 注册到客户端监听器上(true),同时返回一个list 集合, 放着 子节点
List<String> children = zkCli.getChildren("/idea", true);
// 2. 获取节点数据 , 需要穿一个 watcher , 也可以注册到客户端的监听器上(true),或者自己创建监听器
byte[] data = zkCli.getData("/idea",true, stat);
```



### 客户端代码流程

```java
public class DistributeClient {

    private static String connectString = "hadoop1:2181,hadoop2:2181,hadoop3:2181";
    private static int sessionTimeout = 2000;
    private ZooKeeper zk = null;
    private String parentNode = "/server";

    // 创建到zk的客户端连接
    public void getConnect() throws IOException {
        zk = new ZooKeeper(connectString, sessionTimeout, new Watcher() {
            @Override
            public void process(WatchedEvent event) {
                System.out.println("监听到");
                try {
                    getServerLists();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
    }

  
    public void getServerLists() throws Exception {
        List<String> children = zk.getChildren(parentNode,true);
        System.out.println(children);
    }


    public void business() throws Exception {
        Thread.sleep(Long.MAX_VALUE);
    }

    // 无脑获取节点信息
    public static void main(String[] args) throws Exception {

        // 1 获取zk连接
        DistributeClient client = new DistributeClient();
        client.getConnect();

        // 2 获取servers的子节点信息 , 此时 watcher 监听到 , 然后watcher又执行了个 getServerLists ,无穷无尽 哈哈哈哈
        client.getServerLists();

        // 3 业务进程启动
        client.business();
    }
}

```

### Spring-Boot 整合 zookeeper

zk 的基本信息

```java
public class ZookeeperInfo {

    static String CONNECT_STRING = "hadoop1:2181,hadoop2:2181,hadoop3:2181";

    static int SESSION_TIMEOUT = 2000;

    static String SERVER_PATH = "/test-server";
}

```

zkCli 创建

```java
@Slf4j
@Configuration
public class ZookeeperConfig {

    private ZooKeeper zooKeeper;

    @Bean
    public ZooKeeper zooKeeper() {
        try {
            zooKeeper = new ZooKeeper(ZookeeperInfo.CONNECT_STRING, ZookeeperInfo.SESSION_TIMEOUT, new Watcher() {
                @Override
                public void process(WatchedEvent event) {
                    // 循环监听 , 类似于递归
                    getChild();
                }
            });
        } catch (IOException e) {
            e.printStackTrace();
        }
        return zooKeeper;
    }


    private void getChild() {
        try {
            List<String> children = zooKeeper.getChildren(ZookeeperInfo.SERVER_PATH, true);
            System.out.println(children);
        } catch (KeeperException e) {
            //
        } catch (InterruptedException e) {
            //
        }
    }
}

```

spring 容器初始化完成时, 注册 父节点信息 

```java
@Slf4j
@Component
public class ZookeeperApplicationListener implements ApplicationListener<ContextRefreshedEvent> {

    @Autowired
    private ZooKeeper zooKeeper;
    
    @Override
    public void onApplicationEvent(ContextRefreshedEvent event) {

        Stat stat=null;
        try {
            stat = zooKeeper.exists(ZookeeperInfo.SERVER_PATH, false);
        } catch (KeeperException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        // 服务器启动删除节点
        if (null != stat) {
            try {
                zooKeeper.delete(ZookeeperInfo.SERVER_PATH, stat.getVersion());
            } catch (InterruptedException e) {
                e.printStackTrace();
            } catch (KeeperException e) {
                e.printStackTrace();
            }
        }

        // 创建节点
        try {
            zooKeeper.create(ZookeeperInfo.SERVER_PATH,"hello world".getBytes(), ZooDefs.Ids.OPEN_ACL_UNSAFE, CreateMode.PERSISTENT);
        } catch (KeeperException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        // 获取节点 children
        try {
            zooKeeper.getChildren(ZookeeperInfo.SERVER_PATH, true);
        } catch (KeeperException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
```



## ZK客户端的监听流程

​	我们上面代码那个简单客户端代码就是执行的这个流程 

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-29/e770ea97-e960-4787-b62e-95c998009f05.png?x-oss-process=style/template01)



## Zookeeper的选举机制

文章 : [https://www.cnblogs.com/shuaiandjun/p/9383655.html](https://www.cnblogs.com/shuaiandjun/p/9383655.html)

Zookeeper的Leader选举机制叫做 . FastLeaderElection

其中 上诉说的可能是ZAB协议. 是分布式一致性算法. 



