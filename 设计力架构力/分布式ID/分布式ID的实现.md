# 分布式ID的实现

## UUID/GUID

通用唯一识别码（`Universally Unique Identifier`，缩写：`UUID`）是用于计算机体系中以识别信息数目的一个128位标识符，也就是可以通过16个字节来表示。

`UUID`可以根据标准方法生成，不依赖中央机构的注册和分配，UUID具有唯一性，这与其他大多数编号方案不同。重复UUID码概率接近零，可以忽略不计。

`GUID`有时专指微软对UUID标准的实现(`Globally Unique Identifier`, 缩写:`GUID`)，通常表示成32个16进制数字（0－9，A－F）组成的字符串，如：`{21EC2020-3AEA-1069-A2DD-08002B30309D}`，实质上还是是一个128位长的二进制整数,在Windows生态圈中常用。

UUID 由开放软件基金会（OSF）标准化，作为分布式计算环境（DCE）的一部分。

UUID的标准型式包含32个16进位数字，以连字号分为五段，形式为`8-4-4-4-12`的32个字元。范例：`550e8400-e29b-41d4-a716-446655440000`

在其规范的文本表示中，UUID 的 16 个 8 位字节表示为 32 个十六进制（基数16）数字，显示在由连字符分隔 '-' 的五个组中，"8-4-4-4-12" 总共 36 个字符（32 个字母数字字符和 4 个连字符）。例如：

```
123e4567-e89b-12d3-a456-426655440000
xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx
```

四位数字 `M`表示 UUID 版本，数字 `N`的一至三个最高有效位表示 UUID 变体。在例子中，M 是 1 而且 N 是 a(10xx)，这意味着此 UUID 是 "变体1"、"版本1" UUID；即基于时间的 DCE/RFC 4122 UUID。

对于 "变体(variants)1" 和 "变体2"，标准中定义了五个"版本(versions)"，并且在特定用例中每个版本可能比其他版本更合适。

版本由 M 字符串中指示。

- "版本1" UUID 是根据时间和节点 ID（通常是MAC地址）生成;
- "版本2" UUID是根据标识符（通常是组或用户ID）、时间和节点ID生成;
- "版本3" 和 "版本5" 确定性UUID 通过散列 (hashing) 命名空间 (namespace) 标识符和名称生成;
- "版本4" UUID 使用随机性或伪随机性生成。

更详细的信息可以参考[wikipedia](https://en.wikipedia.org/wiki/Universally_unique_identifier)和[RFC文档](https://tools.ietf.org/html/rfc4122)。

- **优点**
  - 容易实现，产生快
  - ID唯一(几乎不会产生重复id)
  - 无需中心化的服务器
  - 不会泄漏商业机密
- **缺点**
  - 可读性差
  - 占用空间太多(16个字节)
  - 影响数据库的性能, Innodb的聚集索引依赖于一个键的顺序排列,比如[UUID or GUID as Primary Keys? Be Careful!](https://tomharrisonjr.com/uuid-or-guid-as-primary-keys-be-careful-7b2aa3dcb439)



## snowflake

> ​	雪花算法，Twitter发明的

### 结构

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-40-44/4ed4ef15-64e6-4dd3-bbbd-e362bcbb7a08.jpeg)

- `1位`不用。二进制中最高位为1的都是负数，但是我们生成的id一般都使用整数，所以这个最高位固定是0。
- `41位`，用来记录毫秒的时间戳。41位可以表示的数值范围是：0 至 2^{41}-1，减1是因为可表示的数值范围是从0开始算的，而不是1，转化为年则是`2^{41}-1) / (1000 * 60 * 60 * 24 * 365) = 69`年。
- `10位`，用来记录工作机器id。最多可以部署在2^{10} = 1024个节点，我们可以根据具体的业务来定制具体分配的机器数量和每台机器1毫秒产生的id序号number数。例如可以把10bit分5bit给IDC，分5bit给工作机器。这样就可以表示32个IDC，每个IDC下可以有32台机器，可以将内容配置在配置文件中，服务去获取。
- `12位`。用来表示单台机器每毫秒生成的id序号，12位bit可以表示的最大正整数为2^12 - 1 = 4096，若超过4096，则重新从0开始。即，每台机器1毫秒内最多产生4096个ID，足够用了。

![]( https://tyut.oss-accelerate.aliyuncs.com/image/2020-40-44/093a5ea5-3160-4b68-9b72-05b2ee86e6a4.png)

### 优点

- ID为数字且时间位在高位，整个ID都是趋势递增的。
- 不依赖任何第三方库，完全可以自己写，且性能非常高。
- 可根据业务定制分配bit位，非常灵活。得益于`10位`机器IDbit位。
- 不太容易破解

### 缺点

- 依赖机器的时间，如果机器时间不准或者回拨，可能导致重复





## Redis

Redis为单线程的，所以操作为原子操作，利用`incrby`命令可以生成唯一的递增ID。

### 单机架构

> ​	业务瓶颈，网络的最大吞吐量和最大连接数，

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-40-44/4bba2ec4-792b-4d2c-8088-2c65735cceca.png)

### 集群架构

> ​	 加入步长 ， 业务瓶颈依旧是上面的

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-40-44/cd1afbe9-d302-4826-b728-280553f9310b.png)



## MySQL - Flicker

基于数据库主键自增的方案，名为`Flicker`。主要是利用MySQL的自增主键来实现分布式ID。

以下为`Flicker`实现分布式ID的主流做法：

### 1、需要单独建立一个数据库实例：flicker

```
create database `flicker`;
```

### 2、创建一张表：sequence_id

```
create table sequence_id(
    id bigint(20) unsigned NOT NULL auto_increment, 
    stub char(10) NOT NULL default '',
    PRIMARY KEY (id),
    UNIQUE KEY stub (stub)
) ENGINE=MyISAM;
```

为什么用`MyISAM`？不用`InnoDB`？个人推测原因是：`flicker`算法出来的时候，MySQL的默认引擎还依旧是`MyISAM`而不是`InnoDB`，作者只是想用默认引擎而已，并无其他原因。

- stub: 票根，对应需要生成 Id 的业务方编码，可以是项目名、表名甚至是服务器 IP 地址。
- stub 要设置为唯一索引

### 3、使用以下SQL来获取ID

```
REPLACE INTO ticket_center (stub) VALUES ('test');  
SELECT LAST_INSERT_ID();
```

`Replace into` 先尝试插入数据到表中，如果发现表中已经有此行数据（根据主键或者唯一索引判断）则先删除此行数据，然后插入新的数据， 否则直接插入新数据。
一般`stub`为特殊的相同的值。

这样，一个分布式ID系统算是可以搭建运行了。但是，有人要问：“这是一个单实例、单点的系统，万一挂了，岂不是影响所有关联的业务方？”

### 改进升华

是的。确实如此，因此又有人说：“可以利用MySQL主从模式，主库挂了，使用从库。”这只能算是一种比较low的策略，因为如果主库挂了，从库没来得及同步，就会生成重复的ID。有没有更好的方法呢？我们可以使用“双主模式“，也就是有两个MySQL实例，这两个都能生成ID。
如图所示，我们原来的模式：

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-40-44/13f86eb4-f0b2-47f2-9a3a-e1f2856f1e4e.png)

双主模式是该怎么样呢？如何保持唯一性？
我们可以让一台实例生成奇数ID，另一台生成偶数ID。

奇数那一台：

```
set @@auto_increment_offset = 1;     -- 起始值
set @@auto_increment_increment = 2;  -- 步长
```

偶数那一台：

```
set @@auto_increment_offset = 2;     -- 起始值
set @@auto_increment_increment = 2;  -- 步长
```

当两台都OK的时候，随机取其中的一台生成ID；若其中一台挂了，则取另外一台生成ID。
如图所示
![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-40-44/fd3210b9-343e-438c-ae0b-b8c460d71d3b.png)

细心会发现，N个节点，只要起始值为1，2，…N，然后步长为N，就会生成各不相同的ID。

最终形态

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-40-44/68a5c319-1979-4e39-b849-05b7473ccb27.png)

### 总结

优点：

- 简单。充分利用了数据库自增 ID 机制，生成的 ID 有序递增。
- ID递增

缺点：

- 系统水平扩展比较困难，比如定义好了步长和机器台数之后，如果要添加机器该怎么做？假设现在只有一台机器发号是1,2,3,4,5（步长是1），这个时候需要扩容机器一台。可以这样做：把第二台机器的初始值设置得比第一台超过很多，比如14（假设在扩容时间之内第一台不可能发到14），同时设置步长为2，那么这台机器下发的号码都是14以后的偶数。然后摘掉第一台，把ID值保留为奇数，比如7，然后修改第一台的步长为2。让它符合我们定义的号段标准，对于这个例子来说就是让第一台以后只能产生奇数。扩容方案看起来复杂吗？貌似还好，现在想象一下如果我们线上有100台机器，这个时候要扩容该怎么做？简直是噩梦。所以系统水平扩展方案复杂难以实现。
- ID没有了单调递增的特性，只能趋势递增，这个缺点对于一般业务需求不是很重要，可以容忍。
- 数据库压力还是很大，每次获取ID都得读写一次数据库，只能靠堆机器来提高性能。







## Mongodb - ObjectId

> ​	官方文档链接 ： [https://docs.mongodb.com/manual/reference/method/ObjectId/](https://docs.mongodb.com/manual/reference/method/ObjectId/)

`ObjectId`（*<十六进制>* ）

返回一个新的[ObjectId](https://docs.mongodb.com/manual/reference/bson-types/#objectid)值。12字节的[ObjectId](https://docs.mongodb.com/manual/reference/bson-types/#objectid) 值包括：

- 一个4字节的*时间戳记值*，代表自Unix时代以来以秒为单位的ObjectId的创建
- 5字节*随机值*
- 3字节*递增计数器*，初始化为随机值

由24位十六进制表示，好处就是可以直接根据ID调用[`ObjectId.getTimestamp()`](https://docs.mongodb.com/manual/reference/method/ObjectId.getTimestamp/#ObjectId.getTimestamp)来获取时间搓。



## 利用zookeeper生成唯一ID

zookeeper主要通过其znode数据版本来生成序列号，可以生成32位和64位的数据版本号，客户端可以使用这个版本号来作为唯一的序列号。

很少会使用zookeeper来生成唯一ID。主要是由于需要依赖zookeeper，并且是多步调用API，如果在竞争较大的情况下，需要考虑使用分布式锁。因此，性能在高并发的分布式环境下，也不甚理想。







## 引用

[https://i6448038.github.io/2019/09/28/snowflake/](https://i6448038.github.io/2019/09/28/snowflake/)

[https://tech.meituan.com/2017/04/21/mt-leaf.html](https://tech.meituan.com/2017/04/21/mt-leaf.html)



IDC ： [互联网数据中心](https://baike.baidu.com/item/互联网数据中心/8471124)（Internet Data Center）简称IDC，就是电信部门利用已有的互联网通信线路、带宽资源，建立标准化的电信专业级机房环境，为企业、政府提供[服务器](https://baike.baidu.com/item/服务器)托管、租用以及相关增值等方面的全方位服务。

