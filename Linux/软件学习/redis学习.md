# redis学习

## 1.nosql数据库有哪些

### 1.键值（Key-Value）对数据库(redis) Memcached
​	键值数据库就像在传统语言中使用的哈希表。你可以通过key来添加、查询或者删除数据，鉴于使用主键访问，所以会获得不错的性能及扩展性。
​	主要是redis 
​	适用的场景:储存用户信息，比如会话、配置文件、参数、购物车等等。这些信息一般都和ID（键）挂钩，这种情景下键值数据库是个很好的选择。

### 2.面向文档（Document-Oriented）数据库(MongoDB)
​		面向文档数据库会将数据以文档的形式储存。每个文档都是自包含的数据单元，是一系列数据项的集合。每个数据项都有一个名称与对应的值，值既可以是简单的数据类型，如字符串、数字和日期等；也可以是复杂的类型，如有序列表和关联对象。数据存储的最小单位是文档，同一个表中存储的文档属性可以是不同的，数据可以使用XML、JSON或者JSONB等多种形式存储。
​		适用的场景

  1. 日志。企业环境下，每个应用程序都有不同的日志信息。Document-Oriented数据库并没有固定的模式，所以我们可以使用它储存不同的信息。

  2. 分析。鉴于它的弱模式结构，不改变模式下就可以储存不同的度量方法及添加新的度量。

### 3.列存储（Wide Column Store/Column-Family）数据库 HBase

- 高压缩比，适用于统计类业务场景
- 适用的场景 :
  - 日志。因为我们可以将数据储存在不同的列中，每个应用程序可以将信息写入自己的列族中。
  - 博客平台。我们储存每个信息到不同的列族中。
  - 举个例子，标签可以储存在一个，类别可以在一个，而文章则在另一个。

### 4.图（Graph-Oriented）数据库 Neo4J

- 图数据库允许我们将数据以图的方式储存。
-  **使用场景** 在一些关系性强的数据中,推荐引擎。如果我们将数据以图的形式表现，那么将会非常有益于推荐的制定

![image](https://tyut.oss-cn-beijing.aliyuncs.com/edu/picture/20190821115057.png?x-oss-process=style/template01)

## 2.传统的关系型数据库与nosql数据库区别

### 1.ACID规则(关系型数据库遵循ACID规则)
	1、A (Atomicity) 原子性
	原子性很容易理解，也就是说事务里的所有操作要么全部做完，要么都不做，事务成功的条件是事务里的所有操作都成功，只要有一个操作失败，整个事务就失败，需要回滚。比如银行转账，从A账户转100元至B账户，分为两个步骤：1）从A账户取100元；2）存入100元至B账户。这两步要么一起完成，要么一起不完成，如果只完成第一步，第二步失败，钱会莫名其妙少了100元。
	2、C (Consistency) 一致性
	一致性也比较容易理解，也就是说数据库要一直处于一致的状态，事务的运行不会改变数据库原本的一致性约束。
	3、I (Isolation) 独立性
	所谓的独立性是指并发的事务之间不会互相影响，如果一个事务要访问的数据正在被另外一个事务修改，只要另外一个事务未提交，它所访问的数据就不受未提交事务的影响。比如现有有个交易是从A账户转100元至B账户，在这个交易还未完成的情况下，如果此时B查询自己的账户，是看不到新增加的100元的
	4、D (Durability) 持久性
	持久性是指一旦事务提交后，它所做的修改将会永久的保存在数据库上，即使出现宕机也不会丢失。
### 2.cap 
	分布式系统（distributed system）正变得越来越重要，大型网站几乎都是分布式的。分布式系统的最大难点，就是各个节点的状态如何同步。CAP 定理是这方面的基本定理，也是理解分布式系统的起点。1998年，加州大学的计算机科学家 Eric Brewer 提出，分布式系统有三个指标。Eric Brewer 说，这三个指标不可能同时做到。这个结论就叫做 CAP 定理。
![iamge](https://www.wangbase.com/blogimg/asset/201807/bg2018071607.jpg)
	C:Consistency（强一致性） 
	A:Availability（可用性）
	P:Partition tolerance（分区容错性）分布式必须具有这个(服务器之间可能通信断掉)

推荐个网站讲的很好(http://www.ruanyifeng.com/blog/2018/07/cap.html)

### 3. 三进二原则

CAP理论的核心是：一个分布式系统不可能同时很好的满足一致性，可用性和分区容错性这三个需求，最多只能同时较好的满足两个。因此，根据 CAP 原理将 NoSQL 数据库分成了满足 CA 原则、满足 CP 原则和满足 AP 原则三 大类：
CA - 单点集群，满足一致性，可用性的系统，通常在可扩展性上不太强大。
CP - 满足一致性，分区容忍必的系统，通常性能不是特别高。
AP - 满足可用性，分区容忍性的系统，通常可能对一致性要求低一些。


## 3.redis的安装和使用
### 1.下载启动

- 去官网下载二进制 gz包 或者  wget
- 然后进入目录直接  make && make install
- 安装目录默认在  `/usr/local/bin/redis-server`  可以通过 `which redis-server`
- 然后备份配置文件 `cp /redis/redis.conf  /opt/redis/redis1.conf`  一般都是推荐在 etc目录下,个人习惯吧
- 然后 直接 `redis-server redis1.conf`
- `redis-cli -h 192.168.58.129 -p 6379`   就启动了 ,然后 输入 `PING`  他回复 `pang` 就OK了
- 关闭 `shurdown`

### 2.远程访问
```shell
1. 关闭设备防火墙	
2. 修改配置文件redis.conf
2.1 daemonize yes  (yes是以守护进程的方式运行，就是关闭了远程连接窗口，redis依然运行)
2.2 protected-mode，将yes修改为no (yes就是禁止外网访问)
2.3 bind 127.0.0.1，注释掉(单机版本就这么做)
2.4 保存退出  esc   :wq

3.启动redis  redis-server ./redis.conf
4.启动客户端   redis-cli -h 192.168.58.129  -p 6379

5. redis-benchmark:性能测试工具，可以在自己本子运行，看看自己本子性能如何
6. redis-check-dump：修复有问题的dump.rdb文件
7. redis-check-aof：修复有问题的AOF文件，rdb和aof后面讲
8. redis-sentinel：redis集群使用
```

## 4.redis介绍
- 它是个单进程软件;基于对epoll(多路复用io)函数进行包装做到的.redis的执行效率完全依赖于主进程的效
- redis默认是16个库;可以通过 select index 操作
- 由于 Redis 是一个内存数据库，所谓内存数据库，就是将数据库中的内容保存在内存中，这与传统的MySQL，Oracle等关系型数据库直接将内容保存到硬盘中相比，内存数据库的读写效率比传统数据库要快的多（内存的读写效率远远大于硬盘的读写效率）。但是保存在内存中也随之带来了一个缺点，一旦断电或者宕机，那么内存数据库中的数据将会全部丢失。为了解决这个缺点，Redis提供了将内存数据持久化到硬盘，以及用持久化文件来恢复数据库数据的功能。Redis 支持两种形式的持久化，一种是RDB快照（snapshotting），另外一种是AOF（append-only-file）

## 5.redis数据类型
### 1.string（字符串） 

​	string 是Redis的最基本的数据类型，可以理解为与 Memcached 一模一样的类型，一个key 对应一个 value。string 类型是二进制安全的，意思是 Redis 的 string 可以包含任何数据，比如图片或者序列化的对象，一个 redis 中字符串 value 最多可以是 512M。

>自增自减操作，这在实际工作中还是特别有用的（分布式环境中统计系统的在线人数，利用Redis的高性能读写，在Redis中完成秒杀，而不是直接操作数据库。）。

```shell
set key value ex 5 	设置 key value  5s过期
set key value px 5 	设置 key value  5ms过期
get key  			获取key
ttl key  			查看还有多有过去 -1表示永不过期 -2表示 已过期  -2已经这个元素么了
EXPIRE k1 2         设置k1过期值

incr key  			使得value 每次+1
decr key  			使得value 每次 -1 
decrby k1 3   		每次减少 3

set k2 v2 ex 5 nx   只有在k2 不存在的时候才能重新设置k2的值,所以这个就是一个锁

```

### 2.hash 
	　hash 是一个键值对集合，是一个 string 类型的 key和 value 的映射表，key 还是key，但是value是一个键值对（key-value）。类比于 Java里面的 Map<String,Map<String,Object>> 集合。
### 3.List（列表）
- list 列表，它是简单的字符串列表，按照插入顺序排序，你可以添加一个元素到列表的头部（左边）或者尾部（右边），它的底层实际上是个链表。
-  Redis 列表是简单的字符串列表，按照插入顺序排序。你可以添加一个元素导列表的头部（左边）或者尾部（右边）。它的底层实际是个链表

### 4.Set（集合）

- Redis 的 set 是 string 类型的无序集合。它是通过HashTable实现实现的，

### 5. zset(sorted set：有序集合) 

​	　zset（sorted set 有序集合），和上面的set 数据类型一样，也是 string 类型元素的集合，但是它是有序的,且不允许重复的成员。不同的是每个元素都会关联一个double类型的分数。redis正是通过分数来为集合中的成员进行从小到大的排序。zset的成员是唯一的,但分数(score)却可以重复。

## 6.RDB-redis持久化方式
### 1.自动触发
	直接修改redis.conf文件 然后修改里面的save属性
### 2.手动触发

- 需要 输入指令 **save** 
  该命令会**阻塞当前Redis服务器**，执行save命令期间，Redis不能处理其他命令，直到RDB过程完成为止。显然该命令对于内存比较大的实例会造成长时间阻塞，这是致命的缺陷，为了解决此问题，Redis提供了第二种方式。

- 指令  **bgsave**
  　执行该命令时，**Redis会在后台异步进行快照操作**，快照同时还可以响应客户端请求。具体操作是Redis进程执行fork操作创建子进程，**RDB持久化过程由子进程负责**，完成后自动结束。阻塞只发生在fork阶段，一般时间很短。

- 区别

  ​		**SAVE  保存是阻塞主进程**，客户端无法连接redis，等SAVE完成后，主进程才开始工作，客户端可以连接
  **BGSAVE  是fork一个save的子进程，在执行save过程中，不影响主进程**，客户端可以正常链接redis，等子进程fork执行save完成后，通知主进程，子进程关闭。

### 3.结论
	基本上 Redis 内部所有的RDB操作都是采用 bgsave 命令。
	ps:执行执行 flushall 命令，也会产生dump.rdb文件，但里面是空的，无意义
### 4.如何关闭redis持久化功能
​		有些情况下，我们只想利用Redis的缓存功能，并不像使用 Redis 的持久化功能，那么这时候我们最好停掉 RDB 持久化。可以通过上面讲的在配置文件 **redis.conf** 中，可以注释掉所有的 save 行来停用保存功能或者直接一个空字符串来实现停用：**save ""**; 或者 输入指令启动的时候  `redis-cli config set save ''`

### 5.rdb的优劣势
	优势
	1.RDB是一个非常紧凑(compact)的文件，它保存了redis 在某个时间点上的数据集。这种文件非常适合用于进行备份和灾难恢复。
	2.生成RDB文件的时候，redis主进程会fork()一个子进程来处理所有保存工作，主进程不需要进行任何磁盘IO操作。
	3.RDB 在恢复大数据集时的速度比 AOF 的恢复速度要快。
	②、劣势
	1、RDB方式数据没办法做到实时持久化/秒级持久化。因为bgsave每次运行都要执行fork操作创建子进程，属于重量级操作(内存中的数据被克隆了一份，大致2倍的膨胀性需要考虑)，频繁执行成本过高(影响性能)
	2、RDB文件使用特定二进制格式保存，Redis版本演进过程中有多个格式的RDB版本，存在老版本Redis服务无法兼容新版RDB格式的问题(版本不兼容)
	3、在一定间隔时间做一次备份，所以如果redis意外down掉的话，就会丢失最后一次快照后的所有修改(数据有丢失)
## 8.aof持久化方式
	　　Redis的持久化方式之一RDB是通过保存数据库中的键值对来记录数据库的状态。而另一种持久化方式 AOF 则是通过保存Redis服务器所执行的写命令来记录数据库状态。　RDB 持久化方式就是将 str1,str2,str3 这三个键值对保存到 RDB文件中，而 AOF 持久化则是将执行的 set,sadd,lpush 三个命令保存到 AOF 文件中。
	例如 set str1 '123' ;sadd str2 '1' '2' '3' ;lpush str3 '1' '2' '3' ;如果是 rdb 则是将三个 str1,str2,str3 三个kv保存在文件中;而 aof则是 将 添加的三条指令存入文件中
### 1.开启
	在 redis.conf 配置文件的 APPEND ONLY MODE 下： appendonly：默认值为no，也就是说redis 默认使用的是rdb方式持久化，如果想要开启 AOF 持久化方式，需要将 appendonly 修改为 yes。 appendfsync 是写入的策略(三种)
### 2.AOF 重写
	　由于AOF持久化是Redis不断将写命令记录到 AOF 文件中，随着Redis不断的进行，AOF 的文件会越来越大，文件越大，占用服务器内存越大以及 AOF 恢复要求时间越长。为了解决这个问题，Redis新增了重写机制，当AOF文件的大小超过所设定的阈值时，Redis就会启动AOF文件的内容压缩，只保留可以恢复数据的最小指令集。可以使用命令 bgrewriteaof 来重新。　　如果不进行 AOF 文件重写，那么 AOF 文件将保存四条 SADD 命令，如果使用AOF 重写，那么AOF 文件中将只会保留下面一条整合后的命令;
### 3. aof的优缺点
	优点：
	　　①、AOF 持久化的方法提供了多种的同步频率，即使使用默认的同步频率每秒同步一次，Redis 最多也就丢失 1 秒的数据而已。
	　　②、AOF 文件使用 Redis 命令追加的形式来构造，因此，即使 Redis 只能向 AOF 文件写入命令的片断，使用 redis-check-aof 工具也很容易修正 AOF 文件。
	　　③、AOF 文件的格式可读性较强，这也为使用者提供了更灵活的处理方式。例如，如果我们不小心错用了 FLUSHALL 命令，在重写还没进行时，我们可以手工将最后的 FLUSHALL 命令去掉，然后再使用 AOF 来恢复数据。
	缺点：
	　　①、对于具有相同数据的的 Redis，AOF 文件通常会比 RDF 文件体积更大。
	　　②、虽然 AOF 提供了多种同步的频率，默认情况下，每秒同步一次的频率也具有较高的性能。但在 Redis 的负载较高时，RDB 比 AOF 具好更好的性能保证。
	　　③、RDB 使用快照的形式来持久化整个 Redis 数据，而 AOF 只是将每次执行的命令追加到 AOF 文件中，因此从理论上说，RDB 比 AOF 方式更健壮。官方文档也指出，AOF 的确也存在一些 BUG，这些 BUG 在 RDB 没有存在。
	 　　那么对于 AOF 和 RDB 两种持久化方式，我们应该如何选择呢？
	　　如果可以忍受一小段时间内数据的丢失，毫无疑问使用 RDB 是最好的，定时生成 RDB 快照（snapshot）非常便于进行数据库备份， 并且 RDB 恢复数据集的速度也要比 AOF 恢复的速度要快，而且使用 RDB 还可以避免 AOF 一些隐藏的 bug；否则就使用 AOF 重写。但是一般情况下建议不要单独使用某一种持久化机制，而是应该两种一起用，在这种情况下,当redis重启的时候会优先载入AOF文件来恢复原始的数据，因为在通常情况下AOF文件保存的数据集要比RDB文件保存的数据集要完整。Redis后期官方可能都有将两种持久化方式整合为一种持久化模型。


## 9.redis 常用操作指令

```shell
1. dbsize 看当前库 有多少个keys
2. keys* 查看所以keys
3. flushdb  清空当前库
4. select index 切换库
5. move key index 把 key 切换到 另外一个库
6. expire key second  设置过期时间
8. ttl key  查看还有多有过去 -1表示永不过期 -2表示 已过期  -2已经这个元素么了
9. type k2 类型
10. set key value 重复添加会覆盖
11. object encoding key  查看该元素的 value属性值
12.  watch key 监控 key
13.  multi 开启事务
14.  exec 执行事务
```
![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/91deb155-4074-4f34-ac3d-b3315668f99e.png?x-oss-process=style/template01)
![image](https://images2018.cnblogs.com/blog/1120165/201805/1120165-20180525234334105-751375191.png)

## 10. redis集群配置-主从配置
### 1.创建三个配置文件 redis_6380.conf
	配置PID文件路径 pidfile 
	配置端口 port 
	配置log 文件名字 
	配置rdb文件名 
	配置rdb文件路径名称 
	redis-server redis6380.conf
	redis-cli -h 192.168.58.129 -p 6380
### 2.启动进程
	启动redis-cli
	info replication  可以查看配置信息
	slaveof 192.168.58.129 6380  (salve 奴隶) 开启slave身份
### 3.特点
	slave 	特点 	关闭连接  则取消身份
	master 	特点  关闭连接,再次开启依旧会有salve;他依旧是master身份
### 4.取消奴隶身份
	slave no one

## 11. redis集群配置-哨兵模式(Sentinal)
![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/7e951cc0-7bc7-4778-bfe7-6f22f2835874.png?x-oss-process=style/template01)
### 1.配置环境
	在配置文件目录下新建 sentinel.conf 文件，名字绝不能错，然后配置相应内容
	添加一下内容:  sentinel monitor 被监控机器的名字(自己起名字) ip地址 端口号 得票数
	分别配置被监控的名字，ip地址，端口号，以及得票数。上面的得票数为1表示表示主机挂掉后salve投票看让谁接替成为主机，得票数大于1便成为主机
### 2.启动哨兵模式
	 1. 启动三个客户端 
	 2. 然后启动 三个client
	 3. slaveof   master
	 4. 启动 redis-sentinel
		其实我感觉吧 应该是 监控一台master 然后所有连到master上的slave 自动被添加进来
	 5. 还有就是 master走了以后;新选出来的master会顶替原来的 ;原来的回来就变成slaver
		redis-sentinel /etc/redis/sentinel.conf	
### 3.哨兵模式的利弊
	　　PS：哨兵模式也存在单点故障问题，如果哨兵机器挂了，那么就无法进行监控了，解决办法是哨兵也建立集群，Redis哨兵模式是支持集群的。
## 12. redis集群-Cluster
### 1.前期准备 配置环境
	1.yum install ruby
	2.yum install rubygems
	这时候 你下载肯定不能下载 因为没有ruby的库
	$ gem sources --add https://gems.ruby-china.com/ --remove https://rubygems.org/
	$ gem sources -l
	https://gems.ruby-china.com
	# 确保只有 gems.ruby-china.com
	而且 此时 你应该做的是 去整个网站下载对应版本的安装包 去 gem 安装  (https://rubygems.org/gems/redis/versions/3.3.0) 我这里安装的是3.3.0 可以使用
	gem install redis-1.7.1.gem --local   这样就行了 别直接 gem install redis  坑了我一个小时整版本 
	3.gem install redis
	
	redis-trib.rb
	在 redis/src目录下 有这个 ;可以把它移动到 usr目录下

### 2.主要redis.conf修改配置文件的 那些部分呢 
	1. port  端口
	2. bind  主机ip
	3. pidfile 不知道改不改这个 其实 一台机器的话有必要改
	3. dbfile 配置rdb的位置下面的 dir /opt/redis/redis-cluster/node_6380
	4. cluster-enabled yes
	5. cluster-config-file nodes-6380.conf
	6.  cluster-node-timeout 15000
	7.  appendonly yes  开启 aof 
	
	[root@admin redis-cluster]# ll
	总用量 0
	drwxr-xr-x. 2 root root 101 8月  10 20:10 node_6380
	drwxr-xr-x. 2 root root 101 8月  10 20:10 node_6381
	drwxr-xr-x. 2 root root 101 8月  10 20:10 node_6382
	drwxr-xr-x. 2 root root 101 8月  10 20:10 node_6383
	drwxr-xr-x. 2 root root  81 8月  10 20:10 node_6384
	drwxr-xr-x. 2 root root 101 8月  10 20:10 node_6385
	drwxr-xr-x. 2 root root  23 8月   9 23:38 node_6386
	-rw-r--r--. 1 root root   0 8月  10 18:52 redis_log.log
	
	
	这里 我建议 大家 每一个node节点配置一个文件夹 这样的好处是 redis.conf文件修改的地方会很少;而且 一个node一个文件夹很棒
	
	8.然后 开始redis-server redis.conf
	redis-server /opt/redis/redis-cluster/node_6380/redis.conf
	9.在 redis/src目录下 有这个 ;可以把它移动到 usr/local/redis目录下
	/home/user/redis/redis-4.0.9/src/redis-trib.rb create --replicas 1 192.168.58.129:6380 192.168.58.129:6381 192.168.58.129:6382 192.168.58.129:6383 192.168.58.129:6384 192.168.58.129:6385
	10.愉快的玩耍 了
	直接 redis-cli 
- 切记 保存好的节点信息千万别乱整;乱整要出事;每次启动就只能节点启动


## 13. java整合redis
	集群很简单 直接
	jedis 或者 spring-data-redis

## 14.redis开发中存在的问题
### 1.缓存穿透的问题(认为恶意使用)

​	由于后端代码设计的问题;一般是当访问redis数据key不存在的时候;会直接去访问db;然而可能db也没有;当用户连续请求这种么有的数据时;就会给db造成压力;严重使得宕机;所以我们可以在代码中给这些空的请求数据;设置短暂的生命周期;存入一个空值(其他值也可能)给redis;这样下次访问就不会直接去访问db;缓减压力;

### 2.缓存雪崩问题
	由于某些原因使得很多的key集体失效(可能是设置缓存的存在的时间一致,造成key集体失效),造成更大的压力给db,使得雪崩,
	解决问题 : 就是设置不均匀的存活时间
### 3.缓存击穿问题
	由于某些特殊原因,此时热点key失效,是得请求全部给了db,造成击穿问题,
### 4 、缓存预热
​	缓存预热这个应该是一个比较常见的概念，相信很多小伙伴都应该可以很容易的理解，缓存预热就是系统上线后，将相关的缓存数据直接加载到缓存系统。这样就可以避免在用户请求的时候，先查询数据库，然后再将数据缓存的问题！用户直接查询事先被预热的缓存数据！
​	解决思路：
​	1、数据量不大，可以在项目启动的时候自动进行加载；
​	2、定时刷新缓存；

### 5.缓存更新
	除了缓存服务器自带的缓存失效策略之外（Redis默认的有6中策略可供选择），我们还可以根据具体的业务需求进行自定义的缓存淘汰，常见的策略有两种：
	（1）定时去清理过期的缓存；
	（2）当有用户请求过来时，再判断这个请求所用到的缓存是否过期，过期的话就去底层系统得到新数据并更新缓存。
	两者各有优劣，第一种的缺点是维护大量缓存的key是比较麻烦的，第二种的缺点就是每次用户请求过来都要判断缓存失效，逻辑相对比较复杂！具体用哪种方案，大家可以根据自己的应用场景来权衡。
### 6、缓存降级

​	当访问量剧增、服务出现问题（如响应时间慢或不响应）或非核心服务影响到核心流程的性能时，仍然需要保证服务还是可用的，即使是有损服务。系统可以根据一些关键数据进行自动降级，也可以配置开关实现人工降级。
​	降级的最终目的是保证核心服务可用，即使是有损的。而且有些服务是无法降级的（如加入购物车、结算）。
​	在进行降级之前要对系统进行梳理，看看系统是不是可以丢卒保帅；从而梳理出哪些必须誓死保护，哪些可降级；比如可以参考日志级别设置预案：

>其实这里 我觉得访问 db都需要锁
>
>因为 db操作可能很长时间 ; 此时大量请求访问数据库 会造成数据库 压力 ;此时 只能加锁 ;所以 我感觉 基于 好并发情况下直接操作数据库 都应该加锁

### 7. 如何解决?

#### 1. 分布式锁:使用redis自带的分布式锁, set  ex  nx
	1. SET key value EX second NX  效果等同于 SETEX key second value NX 
	2. 
	NX ：只在键不存在时，才对键进行设置操作。 SET key value NX 效果等同于 SETNX key value 
	XX ：只在键已经存在时，才对键进行设置操作。
	指令: set key value ex second px millisecond nx|xx
	其中 在 second内不能重新赋值 会取到nil;如果 拿到锁的人 可以执行删除锁的操作使得后面的人可以继续set
	如果设置了 NX 或者 XX ，但因为条件没达到而造成设置操作未执行，那么命令返回空批量回复（NULL Bulk Reply）。
	
	其中怎么设计这个问题呢;拿到锁的人;可以继续执行访问数据库的代码;拿不到锁的人 ;则自旋(可以让线程睡眠三秒钟;然后继续return 递归操作)  这里切记不能不加return

#### 2. redisson 框架 

​	实现的分布式锁: redisson是一个带有juc的lock功能的框架;同时带有和jedis一样的功能

### 8. 解决案例

### 1. redis分布式锁 例子

- 主要利用了  

  `set k2 v2 ex 5 nx `  只有在k2 不存在的时候才能重新设置k2的值,所以这个就是一个锁

```java
public class Demo {
    private static Jedis jedis = new Jedis("xx.xx.234.232", 6379);
	//  private JedisCluster jedisCluster = new JedisCluster();


    // 数据库访问次数
    private static int dbcunt = 0;

    // 模拟数据库中的信息
    private static List<Person> peoples = new ArrayList<Person>();

    static {
        peoples.add(new Person("tom", 0));
        peoples.add(new Person("tom1", 1));
        peoples.add(new Person("tom2", 2));
        peoples.add(new Person("tom3", 3));
        peoples.add(new Person("tom4", 4));
        peoples.add(new Person("tom5", 5));
    }


    public static void main(String[] args) {

        Demo demo = new Demo();

        List<Integer> requests = new ArrayList<Integer>();

        // 模拟制造请求列表
        for (int i = 0; i < 100; i++) {
            requests.add(new Random().nextInt(11));
        }

        for (Integer request : requests) {
            System.out.println("username = " + demo.testPress(request));
        }

//        String set = jedis.set("user_lock", "15", "nx", "ex", 20);
//        System.out.println(set);

    }


    public String testPress(Integer id) {
        //1.用户请求,首先要去 redis里面取数据
        String name = jedis.get(id.toString());
        //2.redis中不为空 ,则直接返回值
        if (null != name) {
            System.out.println("[从redis里面取]");
            return name;
        }
        // 3.设置锁(设置过期时间长一点 20S)
        String uuid = UUID.randomUUID().toString();
        // 设置成功返回 OK
        String set = jedis.set("user_lock", uuid, "nx", "ex", 20);
        if (null != set && set.equals("OK")) {
            //4.如果为空则请求数据库
            dbcunt++;
            System.out.println("[从数据库中请求次数] :" + dbcunt);
            Person info = getInfo(id);
            //5.如果数据库中的数据不为空,则返回 数据数据库中的数据 并且把数据存入到redis中
            if (null != info) {
                jedis.setex(id.toString(), 60, info.getName());
                if (uuid.equals(jedis.get("user_lock"))) {
                    jedis.del("user_lock");
                }
                return jedis.get(id.toString());
            } else {
                //5.如果数据库中的数据为空,则应该给 存入给redis一个空值
                jedis.setex(id.toString(), 30, "用户不存在");
                if (uuid.equals(jedis.get("user_lock"))) {
                    jedis.del("user_lock");
                }
                return jedis.get(id.toString());
            }
        }
        try {
            //这里 就是 设置 一个自旋锁
            Thread.sleep(1000);
            return testPress(id);
        } catch (InterruptedException e) {
            return null;
        }
    }

    //模拟 数据库 操作
    public Person getInfo(Integer id) {
        try {
            Person person = peoples.get(id);
            return person;
        } catch (Exception e) {
            return null;
        }
    }
}

class Person {
    public Person(String name, int id) {
        this.name = name;
        this.id = id;
    }

    private String name;
    private int id;

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }
}
```



- 利用redisson的分布式锁

  ```java
  RLock disLock = redissonClient.getLock("DISLOCK");
  //加锁
  disLock.lock();
  
  //执行 代码
  
  //释放锁
  disLock.unlock();
  ```

  

###  2. redis完成秒杀操作

- 如何完成秒杀的?
- 利用**redis的事务**,进行完成秒杀操作,这个比较好,因为比较公平
- ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/70a04c22-5313-442d-9388-65bb077bb5d8.png?x-oss-process=style/template01)

```java
    // 2.获取k1
    String k1 = jedis.get("k1");

    if (Integer.parseInt(k1) > 0) {
        // 1. 开启监控
        jedis.watch("k1");
        //3. 开启事务
        Transaction multi = jedis.multi();
        //4. 执行减票操作
        multi.incrBy("k1", -1);
        //5. 执行事务
        List<Object> exec = multi.exec();
        //6. 失败/成功
        if (!CollectionUtils.isEmpty(exec)) {
            System.out.println("抢购完成----------->" + k1);
        } else {
            System.out.println("抢购失败");
        }
        // 7. 取消监控
        jedis.unwatch();
    }
```

- 利用**redisson** 进行操作(先到先得的那种)

  ```java
      Jedis jedis = new Jedis("xx.xx.234.232", 6379);
      jedis.set("k1", "10");
      Config config = new Config();
      config.useSingleServer().setAddress("redis://xx.xx.234.232:6379");
      RedissonClient redissonClient = Redisson.create(config);
  
      for (int i = 0; i <100; i++) {
          RSemaphore k1 = redissonClient.getSemaphore("k1");
          boolean b = k1.tryAcquire();
          if (b) {
              String k11 = jedis.get("k1");
              System.out.println("抢票成功,剩余票数"+k11);
          }
          System.out.println("抢票失败");
      }
  ```

  

## 16.redis常用配置  redis.conf
	参数说明:
	
	1. Redis默认不是以守护进程的方式运行，可以通过该配置项修改，使用yes启用守护进程
	  daemonize no
	2. 当Redis以守护进程方式运行时，Redis默认会把pid写入/var/run/redis.pid文件，可以通过pidfile指定
	  pidfile /var/run/redis.pid
	3. 指定Redis监听端口，默认端口为6379，作者在自己的一篇博文中解释了为什么选用6379作为默认端口，因为6379在手机按键上MERZ对应的号码，而MERZ取自意大利歌女Alessia Merz的名字
	  port 6379
	4. 绑定的主机地址
	  bind 127.0.0.1
	5.当 客户端闲置多长时间后关闭连接，如果指定为0，表示关闭该功能
	  timeout 300
	6. 指定日志记录级别，Redis总共支持四个级别：debug、verbose、notice、warning，默认为verbose
	  loglevel verbose
	7. 日志记录方式，默认为标准输出，如果配置Redis为守护进程方式运行，而这里又配置为日志记录方式为标准输出，则日志将会发送给/dev/null
	  logfile stdout
	8. 设置数据库的数量，默认数据库为0，可以使用SELECT <dbid>命令在连接上指定数据库id
	  databases 16
	9. 指定在多长时间内，有多少次更新操作，就将数据同步到数据文件，可以多个条件配合
	  save <seconds> <changes>
	  Redis默认配置文件中提供了三个条件：
	  save 900 1
	  save 300 10
	  save 60 10000
	  分别表示900秒（15分钟）内有1个更改，300秒（5分钟）内有10个更改以及60秒内有10000个更改。
	 
	10. 指定存储至本地数据库时是否压缩数据，默认为yes，Redis采用LZF压缩，如果为了节省CPU时间，可以关闭该选项，但会导致数据库文件变的巨大
	  rdbcompression yes
	11. 指定本地数据库文件名，默认值为dump.rdb
	  dbfilename dump.rdb
	12. 指定本地数据库存放目录
	  dir ./
	13. 设置当本机为slav服务时，设置master服务的IP地址及端口，在Redis启动时，它会自动从master进行数据同步
	  slaveof <masterip> <masterport>
	14. 当master服务设置了密码保护时，slav服务连接master的密码
	  masterauth <master-password>
	15. 设置Redis连接密码，如果配置了连接密码，客户端在连接Redis时需要通过AUTH <password>命令提供密码，默认关闭
	  requirepass foobared
	16. 设置同一时间最大客户端连接数，默认无限制，Redis可以同时打开的客户端连接数为Redis进程可以打开的最大文件描述符数，如果设置 maxclients 0，表示不作限制。当客户端连接数到达限制时，Redis会关闭新的连接并向客户端返回max number of clients reached错误信息
	  maxclients 128
	17. 指定Redis最大内存限制，Redis在启动时会把数据加载到内存中，达到最大内存后，Redis会先尝试清除已到期或即将到期的Key，当此方法处理 后，仍然到达最大内存设置，将无法再进行写入操作，但仍然可以进行读取操作。Redis新的vm机制，会把Key存放内存，Value会存放在swap区
	  maxmemory <bytes>
	18. 指定是否在每次更新操作后进行日志记录，Redis在默认情况下是异步的把数据写入磁盘，如果不开启，可能会在断电时导致一段时间内的数据丢失。因为 redis本身同步数据文件是按上面save条件来同步的，所以有的数据会在一段时间内只存在于内存中。默认为no
	  appendonly no
	19. 指定更新日志文件名，默认为appendonly.aof
	   appendfilename appendonly.aof
	20. 指定更新日志条件，共有3个可选值： 
	  no：表示等操作系统进行数据缓存同步到磁盘（快） 
	  always：表示每次更新操作后手动调用fsync()将数据写到磁盘（慢，安全） 
	  everysec：表示每秒同步一次（折衷，默认值）
	  appendfsync everysec
	 
	21. 指定是否启用虚拟内存机制，默认值为no，简单的介绍一下，VM机制将数据分页存放，由Redis将访问量较少的页即冷数据swap到磁盘上，访问多的页面由磁盘自动换出到内存中（在后面的文章我会仔细分析Redis的VM机制）
	   vm-enabled no
	22. 虚拟内存文件路径，默认值为/tmp/redis.swap，不可多个Redis实例共享
	   vm-swap-file /tmp/redis.swap
	23. 将所有大于vm-max-memory的数据存入虚拟内存,无论vm-max-memory设置多小,所有索引数据都是内存存储的(Redis的索引数据 就是keys),也就是说,当vm-max-memory设置为0的时候,其实是所有value都存在于磁盘。默认值为0
	   vm-max-memory 0
	24. Redis swap文件分成了很多的page，一个对象可以保存在多个page上面，但一个page上不能被多个对象共享，vm-page-size是要根据存储的 数据大小来设定的，作者建议如果存储很多小对象，page大小最好设置为32或者64bytes；如果存储很大大对象，则可以使用更大的page，如果不 确定，就使用默认值
	   vm-page-size 32
	25. 设置swap文件中的page数量，由于页表（一种表示页面空闲或使用的bitmap）是在放在内存中的，，在磁盘上每8个pages将消耗1byte的内存。
	   vm-pages 134217728
	26. 设置访问swap文件的线程数,最好不要超过机器的核数,如果设置为0,那么所有对swap文件的操作都是串行的，可能会造成比较长时间的延迟。默认值为4
	   vm-max-threads 4
	27. 设置在向客户端应答时，是否把较小的包合并为一个包发送，默认为开启
	  glueoutputbuf yes
	28. 指定在超过一定的数量或者最大的元素超过某一临界值时，采用一种特殊的哈希算法
	  hash-max-zipmap-entries 64
	  hash-max-zipmap-value 512
	29. 指定是否激活重置哈希，默认为开启（后面在介绍Redis的哈希算法时具体介绍）
	  activerehashing yes
	30. 指定包含其它的配置文件，可以在同一主机上多个Redis实例之间使用同一份配置文件，而同时各个实例又拥有自己的特定配置文件
	  include /path/to/local.conf
	31.redis3.2版本后新增protected-mode配置，默认是yes，即开启。设置外部网络连接redis服务，设置方式如下：
		1、关闭protected-mode模式，此时外部网络可以直接访问
		2、开启protected-mode保护模式，需配置bind ip或者设置访问密码
