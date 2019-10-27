# HBASE

## 介绍

![](http://hbase.apache.org/images/hbase_logo_with_orca_large.png)

官网 : http://hbase.apache.org/



介绍:

> ​	[Apache](https://www.apache.org/) HBase™ is the [Hadoop](https://hadoop.apache.org/) database, a distributed, scalable, big data store.
>
> ​	Use Apache HBase™ when you need random, realtime read/write access to your Big Data. This project's goal is the hosting of very large tables -- billions of rows X millions of columns -- atop clusters of commodity hardware. Apache HBase is an open-source, distributed, versioned, non-relational database modeled after Google's [Bigtable: A Distributed Storage System for Structured Data](https://research.google.com/archive/bigtable.html) by Chang et al. Just as Bigtable leverages the distributed data storage provided by the Google File System, Apache HBase provides Bigtable-like capabilities on top of Hadoop and HDFS.



和普通行存储数据库相比列存储数据库的优点

> ​	一个用来查询数据
>
> ​	一个用来做数据分析



相关介绍学习 https://www.ibm.com/developerworks/cn/analytics/library/ba-cn-bigdata-hbase/index.html

future : 

> **海量存储** : Hbase适合存储PB级别的海量数据，在PB级别的数据以及采用廉价PC存储的情况下，能在几十到百毫秒内返回数据。
>
> **列式存储** : 这里的列式存储其实说的是**列族（ColumnFamily）存储**，Hbase是根据列族来存储数据的。列族下面可以有非常多的列，列族在创建表的时候就必须指定。
>
> **极易扩展** : Hbase的扩展性主要体现在两个方面，一个是基于上层处理能力（）的扩展，一个是基于存储的扩展（）。
>
> **高并发** : 这里说的高并发，主要是在并发的情况下，Hbase的单个IO延迟下降并不多。能获得高并发、低延迟的服务。
>
> **稀疏表**:  稀疏主要是针对Hbase列的灵活性，在列族中，你可以指定任意多的列，在列数据为空的情况下，是不会占用存储空间的。









## 快速开始

```shell

wget http://mirrors.tuna.tsinghua.edu.cn/apache/hbase/1.3.5/hbase-1.3.5-bin.tar.gz

sudo tar -zxvf hbase-1.3.5-bin.tar.gz -C /opt/software

chown -R admin:admin /opt/software/hbase-1.3.5

cd conf/

1. vim hbase-env.sh

# The java implementation to use.  Java 1.7+ required.
export JAVA_HOME=/usr/local/java/jdk1.8.0_211


# Tell HBase whether it should manage it's own instance of Zookeeper or not.
export HBASE_MANAGES_ZK=false

java8 取消下面的配置
# Configure PermSize. Only needed in JDK7. You can safely remove it for JDK8+
# export HBASE_MASTER_OPTS="$HBASE_MASTER_OPTS -XX:PermSize=128m -XX:MaxPermSize=128m -XX:ReservedCodeCacheSize=256m"
# export HBASE_REGIONSERVER_OPTS="$HBASE_REGIONSERVER_OPTS -XX:PermSize=128m -XX:MaxPermSize=128m -XX:ReservedCodeCacheSize=256m"



2. vim hbase-site.xml

<configuration>
<!-- hdfs nn地址 地址 -->
  <property>
    <name>hbase.rootdir</name>
    <value>hdfs://hadoop1:9000/hbase</value>
  </property>
 
  <!-- hbase.cluster.distributed 是否分布式 -->
   <property>
    <name>hbase.cluster.distributed</name>
    <value>true</value>
  </property>
  
    <!-- hbase.master.port 端口号 -->
   <property>
    <name>hbase.master.port</name>
    <value>16000</value>
  </property>
  
   <!-- zookeeper 集群地址,多个用,分割 -->
  <property>
    <name>hbase.zookeeper.quorum</name>
    <value>hadoop1:2181,hadoop2:2181,hadoop3:2181</value>
  </property>
  
     <!-- zookeeper 文件地址 -->
  <property>
    <name>hbase.zookeeper.property.dataDir</name>
    <value>/opt/software/zookeeper-3.4.10/zkData</value>
  </property>
  
</configuration>

3. vim regionservers
hadoop1
hadoop2
hadoop3

4. 创建软连接
ln -s /opt/software/hadoop-2.7.7/etc/hadoop/core-site.xml  /opt/software/hbase-1.3.5/conf/core-site.xml

ln -s /opt/software/hadoop-2.7.7/etc/hadoop/hdfs-site.xml  /opt/software/hbase-1.3.5/conf/hdfs-site.xml


5.发送给其他两个服务器


6. 启动 : 
start-hbase.sh
	关闭: 
stop-hbase.sh	




7.查看状态
[admin@localhost kafka_2.11-0.11.0.3]$ jps
2848 HRegionServer
3184 Jps
2309 DataNode
2407 QuorumPeerMain
2217 NameNode
2634 HMaster

8. 然后访问: 
http://hadoop1:16010

```



## 操作数据库

```shell
1. 查看 
list


2.创建表 
create 'tables' , 'info' ,'name'
创建表名 为 `tables` ,列族名称1 为`info`, 2为 `name`


3.查看表信息
hbase(main):012:0> describe 'tables'
COLUMN FAMILIES DESCRIPTION                                                               
{NAME => 'info', BLOOMFILTER => 'ROW', VERSIONS => '1', IN_MEMORY => 'false', KEEP_DELETED_CELLS => 'FALSE', DATA_BLOCK_ENCODING => 'NONE', TTL => 'FORE
VER', COMPRESSION => 'NONE', MIN_VERSIONS => '0', BLOCKCACHE => 'true', BLOCKSIZE => '65536', REPLICATION_SCOPE => '0'}                                 
{NAME => 'name', BLOOMFILTER => 'ROW', VERSIONS => '1', IN_MEMORY => 'false', KEEP_DELETED_CELLS => 'FALSE', DATA_BLOCK_ENCODING => 'NONE', TTL => 'FORE
VER', COMPRESSION => 'NONE', MIN_VERSIONS => '0', BLOCKCACHE => 'true', BLOCKSIZE => '65536', REPLICATION_SCOPE => '0'}                                 


4.插入数据(如果只有一个version,那么会覆盖,等同于更新 , 如果version有多个,就是增加)
put 'tables','10001','info:sex','male'
put 'tables','10002','info:name','name'
put 'tables','10001','info:name','name'
put 'tables','100011','info:name','name'
insert into tables (id , sex) value (10001,male)
表名 , 行键 , 列族:字段名称,数据


5.查看数据 (左闭右开)
hbase(main):021:0> scan 'tables' , {STARTROW=>'10001',STOPROW=>'10003'}
ROW                                     COLUMN+CELL                                                                                                     
 10001                                  column=info:name, timestamp=1571445611104, value=name                                                           
 10001                                  column=info:sex, timestamp=1571445489768, value=male                                                            
 100011                                 column=info:name, timestamp=1571445953460, value=name                                                           
 10002                                  column=info:name, timestamp=1571445605612, value=name                                                           
3 row(s) in 0.0300 seconds
100011 可以查到的原因是 它是从 1,0,0,0,1,1 开始一个个去比较 所以 到倒数第二位 就符合了


6.查看某行数据 get 'tables' ,'10001' 或者 get 'tables' ,'10001','info:name'
hbase(main):022:0>  get 'tables' ,'10001' 
COLUMN                                  CELL                                             
info:name                              timestamp=1571445611104, value=name               info:sex                               timestamp=1571445489768, value=male               1 row(s) in 0.0320 seconds


7. 查看数量
count 'tables'


8.删除数据
删除某一行
deleteall 'tables' ,'10001'
删除某一行对应的某一列
delete 'tables' ,'10001','info:name'


9.清空表数据 -> 表还在,只是数据么了
truncate 'tables'
hbase(main):031:0> truncate 'tables'
Truncating 'tables' table (it may take a while):
 - Disabling table...  禁用表
 - Truncating table...  清空表数据
0 row(s) in 3.6890 seconds


10. 删除表 - > 表,数据都没了
先警用表
disable 'tables'
再 drop 表  
drop 'tables'


11. 修改表信息
alter 'tables' ,{NAME=>'info',VERSION=>3}
将info列族数据存放三个版本


12. 创建namespace
create_namespace  'tyut'
创建表
create 'tyut:tables' , 'info' ,'desc'
```



## HBASE 的整体架构

![]( https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-20/0fb74722-9a93-4d11-91e3-af53b8b10f32.png?x-oss-process=style/template01)



- **HMaster**：监控RegionServer  ， 通过Zookeeper发布自己的位置给客户端 ，在空闲时间进行数据的负载均衡
- **HRegionServer** ： 负责存储hbase的实际数据，负责分配 ，刷新缓存到HDFS，维护Hlog，负责处理Region分片 , 负责和底层HDFS的交互，存储数据到HDFS ,  负责Region变大以后的拆分 ,
- **Region ： **Hbase表的分片，HBase表会根据RowKey值被切分成不同的region存储在RegionServer中，在一个RegionServer中可以有多个不同的region。
- **Store ： **HFile存储在Store中，一个Store对应HBase表中的一个列族(列簇，Column Family)。
- **HFile ： ** 这是在磁盘上保存原始数据的实际的物理文件，是实际的存储文件。StoreFile是以Hfile的形式存储在HDFS的。





## HBASE表的数据结构

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-19/679c09c1-cdf1-439d-962d-a8eaf3e56f17.jpg?x-oss-process=style/template01)

- RowKey 

  > 1. RowKey是用来检索记录的主键
  > 2. RowKey行键可以是**任意字符串**(最大长度是64KB，实际应用中长度一般为10-100bytes)，在HBASE内部，RowKey保存为字节数组。存储时，数据按照RowKey的字典序(byte order)排序存储。设计RowKey时，要充分排序存储这个特性，将经常一起读取的行存储放到一起。(位置相关性)
  >
  > 

- Column Family   :  

  > ​	HBASE表中的每个列，都归属于某个列族。列族是表的schema的一部 分(而列不是)，必须在使用表之前定义。列名都以列族作为前缀。例如courses:history，courses:math都属于courses
  > 这个列族。

- Cell

  > ​	由{rowkey,column Family:columu, version} 唯一确定的单元。cell中的数据是没有类型的，全部是字节码形式存贮。

- Time Stamp

  > ​	HBASE 中通过rowkey和columns确定的为一个存贮单元称为cell。每个 cell都保存 着同一份数据的多个版本。版本通过时间戳来索引。时间戳的类型是 64位整型。时间戳可以由HBASE(在数据写入时自动 )赋值，此时时间戳是精确到毫秒 的当前系统时间。

- namespace

  > 	1. 所有表都对应有命名空间，默认是default
  >  	2. 一个命名空间包含了 默认的 RegionServer Group
  >  	3. 命名空间可以控制访问权限，自定义ACL（Access Control List）
  >  	4. 可以限制regionserver的数量







## 读写过程

### 读数据流程

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-19/0f13ba31-fa98-43fa-b433-7b951270defc.jpg?x-oss-process=style/template01)



### 写数据流程

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-20/082adcf8-960a-4724-b5bd-4ddc730fcf41.png?x-oss-process=style/template01)



![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-20/a988229e-d197-4beb-aa84-589699fa5b66.png?x-oss-process=style/template01)



- **zk/habse : **保存了关于hbase的所有的相关信息，主要是一些元信息（ **hbase-meta** 表位置信息**regionserver**的位置）和其他信息

- **hbase-meta**  保存的是表空间以及表的的详细信息,以及**store**（HBASE中**column-family**是一个存储单元）位置所在

- **write ahead log**（**HLog**）  ： 保存了每次的写请求的数据 ，防止数据丢失，和数据写入

- **Menstore** ： 写缓存 ，将写入的数据缓存到里面 ，以备下次调用 ， 如果写入失败会回滚

- **BlockCache**  ：读缓存 ，就读取的数据写入到缓存，以备下次调用

- **Hfile**： 数据存储执行位置在位置在**storefile**中的**Hfile**,保存的数据实际却在**HDFS**上 

- **数据flush过程**：当MemStore数据达到阈值（默认是128M，老版本是64M），将数据刷到硬盘，将内存中的数据删除，同时删除HLog中的历史数据；并将数据存储到HDFS中；在HLog中做标记点。

- 读取流程 ： 

  > 先从MemStore找数据，如果没有，再到BlockCache里面读；
  >
  > BlockCache还没有，再到StoreFile上读(为了读取的效率)；
  >
  > 如果是从StoreFile里面读取的数据，不是直接返回给客户端，而是先写入BlockCache，再返回给客户端。









## 









