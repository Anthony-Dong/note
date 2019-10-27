



# hadoop

## 介绍

​		The Apache Hadoop software library is a framework that allows for the distributed processing of large data sets across clusters of computers using simple programming models. It is designed to scale up from single servers to thousands of machines, each offering local computation and storage. Rather than rely on hardware to deliver high-availability, the library itself is designed to detect and handle failures at the application layer, so delivering a highly-available service on top of a cluster of computers, each of which may be prone to failures.

![](https://hadoop.apache.org/hadoop-logo.jpg)

我感觉人家说的很好 ,不用我说了 -------------  我写的只是为了我以后记忆 --------- 目的不是为了详细

## 快速入门

### 安装(linux环境)

-  准备java环境

  > ​	下载jdk的linux版本     ---- oracle官网 推荐安装在  /usr/java目录下
  >
  > ```shell
  > vim /etc/profile
  > 
  > export JAVA_HOME=/opt/java/jdk1.8/jdk1.8.0_171
  > export PATH=$PATH:$JAVA_HOME/bin
  > 
  > source /etc/profile
  > 
  > java -version
  > ```

- 准备 hadoop 环境

  > ​	下载  hadoop   [下载链接](https://mirrors.cnnic.cn/apache/hadoop/common/)     地址 : https://mirrors.cnnic.cn/apache/hadoop/common/
  >
  > ​	直接 解压 不用说
  >
  > ```shell
  > vim /etc/profile
  > 
  > export HADOOP_HOME=/opt/bigdata/hadoop-2.7.7
  > export PATH=$PATH:$HADOOP_HOME/bin
  > 
  > source /etc/profile
  > 
  > hadoop version
  > 
  > vim /etc/hostname
  > hadoop1 
  > 
  > 
  > vim etc/hosts
  > 192.168.58.131 hadoop1
  > 
  > 可以重启
  > ```

  

- 单机测试

  > 可以参照官网   [地址](https://hadoop.apache.org/docs/stable/hadoop-project-dist/hadoop-common/SingleCluster.html)
  >
  >  
  >
  > 进入 hadoop的安装目录 ,然后  新建一个  文件 test.txt  添加  几个字段
  >
  > ```shell
  > touch test.txt
  > 
  > vim test.txt
  > test 
  > test2 test3
  > test3 test2 test4
  > test4 test4 test3 test4
  > 环境配置
  > ```
  >
  > 环境配置
  >
  > ```shell
  > vim etc/hadoop/hadoop-env.sh
  > 修改这个地方 改成你的 java_home 路径
  > # The java implementation to use.
  > export JAVA_HOME=/usr/java/jdk1.8.0_21
  > ```
  
> 测试代码 
  >
  > ```shell
  > bin/hadoop jar share/hadoop/mapreduce/hadoop-mapreduce-examples-2.7.7.jar wordcount input/test.txt wcoutput
  > ```
  >
  > 然后看测试`cat wcoutput/* `   就可以了 

- 单节点启动 

  > 可以参照[官网 文档](https://hadoop.apache.org/docs/stable/hadoop-project-dist/hadoop-common/SingleCluster.html).跟我写的一模一样
  >
  > ```xml
  > 记住 不要使用root 用户 ，因为实际开发中root用户根本不会让你使用。
  > 
  > 
  > 1. 配置  首先我的虚拟机在 阿里云上,所以跟用vm跑的虚拟机好多区别,所以根据自己需求取舍
  > vim etc/hadoop/hadoop-env.sh   只要是env后缀的都改一下
  > 修改这个地方 改成你的 java_home 路径
  > # The java implementation to use.
  > export JAVA_HOME=/usr/local/java/jdk1.8.0_211
  > 
  > 设置pid 路径 防止无权限访问
  > vim sbin/hadoop-daemon.sh
  > 打开注释的 HADOOP_PID_DIR 这一行
  > HADOOP_PID_DIR=/opt/software/hadoop-2.7.7/pid
  > 
  > 设置pid 路径 防止无权限访问   或者你可以将tmp 目录权限给 admin 用户
  > vim sbin/yarn-daemon.sh
  > HADOOP_PID_DIR=/opt/software/hadoop-2.7.7/pid
  > 
  > 创建文件夹记得 。。。。。。。。
  > 
  > vim etc/hadoop/core-site.xml
  > 
  > <configuration>	
  > <property>
  > 	<name>fs.defaultFS</name>
  >  <value>hdfs://hadoop1:9000</value>
  > </property>
  > <property>
  >   <name>hadoop.tmp.dir</name>
  >   <value>/opt/software/hadoop-2.7.7/data/tmp</value>
  > </property>
  > </configuration>
  > 
  > 
  > vim etc/hadoop/hdfs-site.xml
  > // 这里单机没必要做副本,小于等于1就行,默认是三个
  > <configuration>
  > <property>
  >   <name>dfs.replication</name>
  >   <value>1</value>
  > </property>
  > </configuration>
  > 
  > 2. 上面两部执行完 就配置OK了 , 然后进入 hadoop的根目录 执行 格式化fdfs(只用格式化一次就行,没必要重复启动)
  > bin/hdfs namenode -format
  > 
  > 3. 启动执行
  > sbin/hadoop-daemon.sh start namenode
  > sbin/hadoop-daemon.sh start datanode
  > // 单机没必要启动 Secondary NameNode
  > 
  > // 关闭进程
  > hadoop-daemon.sh stop namenode
  > hadoop-daemon.sh stop datanode
  > 
  > 
  > 
  > 4. 查看进程
  > jps
  > 
  > 5. 访问 ip:50070  网址就行  前面那个ip 9000 是一个rpc端口
  > 
  > 6. fds
  > [root@localhost hadoop-2.7.7]# cat data/tmp/dfs/name/current/VERSION 
  > #Tue Oct 08 18:45:33 CST 2019
  > namespaceID=1772554801
  > clusterID=CID-4b135e28-4243-4454-962c-246c313c7669
  > cTime=0
  > storageType=NAME_NODE
  > blockpoolID=BP-250502361-172.24.22.95-1570531533311
  > layoutVersion=-63
  > 
  > 7. 启动yarn服务
  > 
  > 7.1 修改启动脚本配置信息
  > vim etc/hadoop/yarn-env.sh
  > # some Java parameters
  > export JAVA_HOME=/usr/local/java/jdk1.8.0_211
  > 
  > 
  > 7.2 vim etc/hadoop/yarn-site.xml
  > 
  > <configuration>
  > <!-- Site specific YARN configuration properties -->
  > <property>
  >   <name>yarn.nodemanager.aux-services</name>
  >   <value>mapreduce_shuffle</value>
  > </property>
  > <property>
  > <name>yarn.resourcemanager.hostname</name>
  > <value>hadoop1</value>	
  > </property>
  > 
  > // hadoop  需要去修改 /etc/hosts文件 增加一行 内网ip  hadoop
  > // 这个真的很坑,你设置外网ip 以及 127.0.0.1都不可以被访问的.必须是内网ip
  > <property>
  >   <name>yarn.resourcemanager.webapp.address</name>
  >   <value>hadoop:8088</value>
  > </property>
  > 
  > </configuration>
  > 
  > 7.3 vim  etc/hadoop/mapred-env.sh
  > 配置java_home环境变量
  > 
  > 7.4 vim etc/hadoop/mapred-site.xml
  > 	// 默认的mapreduce运行在 yarn 上
  > <configuration>
  > <property>
  >   <name>mapreduce.framework.name</name>
  >   <value>yarn</value>
  > </property>
  > </configuration>
  > 7.5 启动
  > 去到 sbin 目录下
  > yarn-daemon.sh start resourcemanager
  > yarn-daemon.sh start nodemanager
  > 
  > yarn-daemon.sh stop resourcemanager
  > yarn-daemon.sh stop nodemanager
  > 
  > 7.6 查看
  > [root@lokfz sbin]# jps
  > 22113 ResourceManager
  > 22451 Jps
  > 21415 NameNode
  > 21511 DataNode
  > 22351 NodeManager
  > 21416 SecondaryNameNode
  > 
  > 7.7 然后访问  http://外网ip:8088   如果出现 hadoop页面就成功了
  > 
  > 
  > 8. 使用文件上传系统 这里我写反了 output和input
  > 8.1 将wcoutput文件夹想要上传到 fs服务器中   切记 / 前面必须有个空格,
  > hadoop fs -put wcoutput /
  > 
  > 8.2 然后执行 测试我们的文件系统(FS)是 hdfs
  > ```
> hadoop jar share/hadoop/mapreduce/hadoop-mapreduce-examples-2.7.7.jar wordcount /wcoutput /wcinput
>
> 8.3 查看  hadoop fs -cat /wcinput/*
> test1	1
> test2	2
> test3	3
> test4	4
>
> 8.4 也可以直接去网站下载

  

  ```shell
  hadoop-daemon.sh start namenode
  hadoop-daemon.sh start datanode
  
  yarn-daemon.sh start resourcemanager
  yarn-daemon.sh start nodemanager
  ```

  

  

  

  ## 集群搭建 



> ​	 至少需要6台服务器,跑hadoop的机器配置(内存 4G , 硬盘 40G起步) , 电脑内存大的可以开6台虚拟机, 有钱的买6台云主机 , 所以起步怎么也得24个G的内存的电脑,  然后算啥我们用户留下的留个 12G ,32G玩玩搭建6台机器集群.

>
>​		穷人3台搭建  4*3=12G 弄个16G内存的电脑勉强玩玩,所以我是穷人,三台搭建 
>
>​    	远程传输文件最好使用   **rsync**  进行传输 ,直接 
>
>​		全部必须内网访问 , 所以还是建议内网开发 . 因为好多只能内网访问

  ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-09/7cc9ddf6-6248-4385-9219-ef71822a1669.png?x-oss-process=style/template01)

  1. 配置java_home  (3台集群配置)

  ```shell
  /opt/bigdata/hadoop-2.7.7/etc/hadoop/hadoop-env.sh
  /opt/bigdata/hadoop-2.7.7/etc/hadoop/mapred-env.sh
  /opt/bigdata/hadoop-2.7.7/etc/hadoop/yarn-env.sh
  
  这三个 文件的 java_home 都需要配置
  
  同时配置 hosts文件
  内网ip hadoop1
  内网ip hadoop2
  内网ip hadoop3
  ```

  

  2. 修改 下面配置文件

  ```xml
  /opt/bigdata/hadoop-2.7.7/etc/hadoop/core-site.xml
  /opt/bigdata/hadoop-2.7.7/etc/hadoop/mapred-site.xml
  /opt/bigdata/hadoop-2.7.7/etc/hadoop/yarn-site.xml
  /opt/bigdata/hadoop-2.7.7/etc/hadoop/hdfs-site.xml
  

你 bin/hdfs namenode -format
记住 将 data/tmp/dfs/data 数据删了 
然后  就可以正常启动了
  
  1. core-site.xml 中需要修改  namenode配置 
  
  <configuration>
      <!--NN 地址  hadoop1 -->
	<property>
		<name>fs.defaultFS</name>
    	<value>hdfs://hadoop1:9000</value>
	</property>
   <property>
       <name>hadoop.tmp.dir</name>
       <value>/opt/bigdata/hadoop-2.7.7/data/tmp</value>
   </property>
  </configuration>
  
  2. hdfs-site.xml     secondarynamenode配置
  <configuration>
    <property>
       <name>dfs.replication</name>
       // 副本有3个
       <value>3</value>
   </property>
       <!--2NN 地址  hadoop2 -->
     <property>
       <name>dfs.namenode.secondary.http-address</name>
       <value>hadoop2:50090</value>
   </property>
  </configuration>
  
  3. mapred-site.xml     
     // 默认的资源调度器改成 yarn
   <property>
       <name>mapreduce.framework.name</name>
       <value>yarn</value>
   </property>
   
   4. yarn-site.xml      resourcemanager 配置
   
  <configuration>
  <!-- Site specific YARN configuration properties -->
   <property>
       <name>yarn.nodemanager.aux-services</name>
       <value>mapreduce_shuffle</value>
   </property>
       <!--resourcemanager 地址  hadoop3 -->
   <property>
     <name>yarn.resourcemanager.hostname</name>
     <value>hadoop3</value>	
   </property>
  </configuration>    
  
  5. 从机  nodemanager  和 datanode 不需要配置
  然后将这个 etc下的配置文件同步发送给 hadoop2 和 hadoop3 机器
  
  6. 然后根据表格启动就好了
  ```

  

  3. 自动化启动配置:

  ```shell
1. 在 hadoop1  机器上

2. 生成 key ,记得用admin用户
ssh-keygen -t rsa 
然后三台机器都执行一遍  . 把他输入到 auth文件里

3.
修改sudo vim /etc/ssh/ssh_config文件的配置，以后则不会再出现此问题
StrictHostKeyChecking no (原来是ask)


4.执行下面
ssh-copy-id hadoop2
ssh-copy-id hadoop3


那怎么实现无缝登录了 :
rsync -av /home/admin/.ssh/  hadoop2:/home/admin/.ssh/
rsync -av /home/admin/.ssh/  hadoop3:/home/admin/.ssh/
 
 
 
 etc/hadoop/slaves  这个文件配置了hadoop所有的从机节点
  
  比如上面3台配置 需要将salves 修改为 (前提是实现免密登录)
  hadoop1
  hadoop2
  hadoop3
  
  然后执行启动脚本需要在namenode的机器上执行  start-dfs.sh   这样子hdfs集群就配置成功了,一键式启动
  启动yarn 脚本需要在resourcemanager的机器上执行 start-yarn.sh  一键式启动
  
  关闭   stop-dfs.sh
  	     start-yarn.sh
  ```



  4. 配置 历史服务器

  ```xml
  1. 修改  mapred-site.xml 文件
  
   <property>
       <name>mapreduce.framework.name</name>
       <value>yarn</value>
   </property>
   <!-- 历史服务器地址 -->
    <property>
       <name>mapreduce.jobhistory.address</name>
         <!-- 随便找一台就行了 -->
       <value>hadoop3:10020</value>
    </property>
    <!-- 历史服务器web地值 -->
    <property>
       <name>mapreduce.jobhistory.webapp.address</name>
       <value>hadoop3:19888</value>
    </property>
    
  2. 修改 yarn-site.xml 文件  
  
  <configuration>
  <!-- Site specific YARN configuration properties -->
       <property>
           <name>yarn.nodemanager.aux-services</name>
           <value>mapreduce_shuffle</value>
       </property>
      <!-- resourcemanager 地址 -->
       <property>
         <name>yarn.resourcemanager.hostname</name>
         <value>hadoop3</value>	
       </property>
      <!-- 日志聚合功能 -->
        <property>  
      	<name>yarn.log-aggregation-enable</name>  
         <value>true</value>  
  	   </property>  
      <!-- 日志保留时间 7天 -->
        <property>
         <name>yarn.log-aggregation.retain-seconds</name>
         <value>604800</value>	
       </property>   
  </configuration>  
  
  3. 去服务器启动 历史服务器
  启动 mr-jobhistory-daemon.sh start historyserver
  关闭 
  
  
  然后运行脚本 
  hadoop jar share/hadoop/mapreduce/hadoop-mapreduce-examples-2.7.7.jar wordcount /wcoutput /wcinputss
  
  
  <property>
      <name>mapreduce.jobhistory.done-dir</name>
      <value>${yarn.app.mapreduce.am.staging-dir}/history/done</value>
  </property>
  
  <property>
      <name>mapreduce.jobhistory.intermediate-done-dir</name>
      <value>${yarn.app.mapreduce.am.staging-dir}/history/done_intermediate</value>
  </property>
  
  <property>
      <name>yarn.app.mapreduce.am.staging-dir</name>
      <value>/tmp/hadoop-yarn/staging</value>
  </property>
  ```

  

5. 拓展机器

   可以直接开一台新机器 ,将 已有机器的 hadoop 和 java 发给 新机器 ,然后 ` hadoop-daemon.sh start datanode`   然后就自动去注册 namenode







  ## HDFS  学习  

  ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-09/46cc3b39-15b7-4cdc-8e5f-0a3d53399b75.png?x-oss-process=style/template01)

  

整体的架构图  : (上图所示)

> ​	**NameNode** :  管理fdfs的名称空间 ,  配置副本策略 , 管理数据块(block,默认是128M ,文件的寻址时间约为10ms,寻址和输出比例1:100 ,所以1S传输数据量大约为 100M ,)的映射信息 , 处理客户端请求(处理读写请求)
>
> ​	**DateNode**(slave) :  存储实际的数据块 , 执行读写操作
>
> ​	**client : ** 文件切片 , 与 NN交互,获取文件具体位置信息  ,  与DN交互进行读写操作  , 提供命令进行管理HDFS(CRUD操作) ,
>
> ​	**SecondaryNameNode : ** 不是完全的 NN , 只能部分顶替 , 辅助NN部分操作 ,



> 上传(本地上传到hdfs)
>
> - hadoop fs -put /local /fdfs
> - moveFromLocal  本地剪切过去
> - copyFromLocal   本地复制过去
> - copyFromLocal   本地复制过去
> - appendToFile  本地文件 添加到 fdfs另一个文件
>
> 本地 (hdfs内部操作) 
>
> - hadoop fs -cp  /text1  /text2
> - hadoop fs -help rm
> - -rm -r
> - -du  -h 每个文件大小
> - -df  -h 内存大小
> - -mkdir 
> - -cat
> - -tail -F 实时查看
> - -setrep 10  设置10个副本(replication)
>
> 下载(hdfs下载操作)
>
> - -hadoop fs -get /fdfs  /local
> - -copyToLocal   
> - -getmerge   合并操作





### 上传流程 

这里的距离是根据 拓扑网络距离选的, 选离得最近的那个

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-09/7c02702f-8a70-4ec2-8efd-8328363f2b02.jpg?x-oss-process=style/template01)

### 下载流程

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-09/07202ea4-d298-4a7b-b69e-f6d99b5d689b.jpg?x-oss-process=style/template01)





## NN 与 2NN 保证数据可靠性

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-09/192443bb-5501-4c80-95ee-f1812fcc5298.jpg?x-oss-process=style/template01)

**此过程主要分为两个阶段 :** 

> ​	一阶段 :  启动NN 服务会加载fsimage 和 edits_inprogress_001 ,此时系统会读取fsimage上的数据, 当有新的client请求时,会记录在日志上 , 
>
> ​	二阶段:  每隔一段时间 , 2NN 会向 NN发送一次是否需要进行生成镜像操作 , 此时NN会根据一定的阈值进行判断 , 响应给 2NN 是否进行生成image操作 , 当需要的时候 , 此时 NN 会新成成一个  edits_inprogress_002 ,将以后的日志加载到这里 ,并且同时将 edits_inprogress_001 修改为 edits_001,同时将 fsimage 和  edits_001 发送给 2NN , 2NN 会将 fsimage 加载到内存中去 , 然后将edits_001操作也加载进去 , 最后生成一个新的 fsimage.chkpoint , 此时再将这个新生成的给了 NN , NN 再将 fsimage.chkpoint 改个名字 fsimage  写入到内存中 , 
>
> 

**我们看一下启动进程**

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-09/4c5dabd3-05dc-4b19-afe2-32ed40c05ae9.png?x-oss-process=style/template01)



我们去具体目录查看一下 (NN)元数据 : 

`/opt/bigdata/hadoop-2.7.7/data/tmp/dfs/name/current`  

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-09/8437aec0-1626-43bf-b148-0a73072a88c5.jpg?x-oss-process=style/template01)



inprogress 是 正在使用的 ,除了他 最多保留两个最新的 , 

log 不会删除



想要查看他 可以使用 hadoop自带的

`hdfs oiv -p XML -i 源文件 -o 目的文件`     查看 image

`hdfs oev -p XML -i 源文件 -o 目的文件`    查看日志文件





## DN  作用

路径 `/opt/bigdata/hadoop-2.7.7/data/tmp/dfs/data/current/BP-250502361-172.24.22.95-1570531533311/current/finalized/subdir0/subdir0`

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-09/b2c94c3e-1d41-42d1-9262-68ccd80e4bcf.jpg?x-oss-process=style/template01)



#### **元数据  和 数据体**

校验和   其中**数据采用的是 crc算法  元数据采用的是 md5 算法**

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-09/4113e3c9-72ef-4318-814b-6bd5be52f759.jpg?x-oss-process=style/template01)



****



#### **超时问题**

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-09/f62cffc1-0980-4a8c-9850-a15fd6f3b645.jpg?x-oss-process=style/template01)

这里的 last contact 就是最近一次联系时间

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-09/082cf1bf-bc2f-4fe8-8a46-2d1adf60e82a.jpg)

**超时断连时计算**  :  第一个变量默认值是 5 minutes , 第二个参数是心跳时间,默认值为3 seconds 



## MAP-REDUCE  系统学习

### 输入输出类型

| **Java类型**      | **Hadoop Writable类型** |
| ----------------- | ----------------------- |
| Boolean           | BooleanWritable         |
| Byte              | ByteWritable            |
| Int               | IntWritable             |
| Float             | FloatWritable           |
| Long              | LongWritable            |
| Double            | DoubleWritable          |
| String            | Text                    |
| Map               | MapWritable             |
| Array             | ArrayWritable           |
| (Serialize)String | readUTF()               |



### map-reduce阶段

![]( https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-23/b6f3512d-d084-49e8-b526-53f02554f1b5.png?x-oss-process=style/template01)

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-23/bea6bcfe-bca7-4a8d-b751-dcc45e6214ba.png?x-oss-process=style/template01)





### suffle阶段



![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-23/2208e560-ce33-44aa-a1cb-76a30beb9f69.png?x-oss-process=style/template01)



### 自我总结



![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-23/cdd559d7-d030-47c7-a547-37d35a7551d3.png?x-oss-process=style/template01)





### InputFormat   切片与MapTask并行度决定机制

![]( https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-24/cadb4d96-3712-4bd3-ad69-83463bd03ea3.jpg)

默认切片规则都是 `FileInputFormat `

#### FileInputFormat

> ​	`FileInputFormat` 是一个抽象类 , 默认的实现规则有 `NLineInputFormat` ,`TextInputFormat` ,`CombineFileInputFormat` ,`FixedLengthInputFormat` ,`KeyValueTextInputFormat` ,`SequenceFileInputFormat` 的实现类  ,

> ​	**TextInputFormat** :  它是按行读取每一行的记录,k是存储改行在文件中的起始字节偏移量,为LongWritable类型 , v为改行数据的内用,不包含终止符(\n 或者 \r) 为 Text 类型
>
> ​	**KeyValueTextInputFormat** : 每一行均为一条记录,被分隔符分割为 k,v . 可以通过`configuration.set(KeyValueLineRecordReader.KEY_VALUE_SEPERATOR, "\t");` 进行设置 分隔符(seperator) .默认的分隔符为 `\t` (tab) . 
>
> **NLineInputFormat :**  指定行进入划分,根据指定行计算切片数 , 不整除 int(总行/n) + 1 ,使用 ` NLineInputFormat.setNumLinesPerSplit(job, 3); job.setInputFormatClass(NLineInputFormat.class); `
>
> **SequenceFileInputFormat :**  map 和 reduce 的中间流
>
> 

#### CombineTextInputFormat

- CombineTextInputFormat用于小文件过多的场景，它可以将多个小文件从逻辑上规划到一个切片中，这样，多个小文件就可以交给一个MapTask处理。

- 如何使用

  ```java
  // 如果不设置InputFormat，它默认用的是TextInputFormat.class
  job.setInputFormatClass(CombineTextInputFormat.class);
  
  //虚拟存储切片最大值设置4m
  CombineTextInputFormat.setMaxInputSplitSize(job, 4194304);
  ```

- 切面规则 :  

- ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-24/2d897afa-3e71-435d-b347-a6e1ba3ab6e9.jpg?x-oss-process=style/template01)



### CacheFile 缓存文件(类似于配置文件)

> ​		在Map端缓存多张表，提前处理业务逻辑，这样增加Map端业务，减少Reduce端数据的压力，尽可能的减少数据倾斜。

```java
// 
job.addCacheFile(URI.create("file:///D:/test/input/pd.txt"));

// 获取cache 
URI[] cacheFiles = context.getCacheFiles();
URI cacheFile = cacheFiles[0];
BufferedReader bufferedReader = new BufferedReader(new FileReader(cacheFile.getPath()));
```



### Counter  计数器

```java
   			if (word.length()==1) {
                context.getCounter("length", "1").increment(1);
            }

            if (word.length()==2) {
                context.getCounter("length", "2").increment(1);
            }
            if (word.length()==3) {
                context.getCounter("length", "3").increment(1);
            }

日志  输出 :    
	length
		1=13
		2=9
		3=9
```



### 压缩与解压缩

| 压缩格式 | hadoop自带？ | 算法    | 文件扩展名 | 是否可切分 | 换成压缩格式后，原来的程序是否需要修改 |
| -------- | ------------ | ------- | ---------- | ---------- | -------------------------------------- |
| DEFLATE  | 是，直接使用 | DEFLATE | .deflate   | 否         | 和文本处理一样，不需要修改             |
| Gzip     | 是，直接使用 | DEFLATE | .gz        | 否         | 和文本处理一样，不需要修改             |
| bzip2    | 是，直接使用 | bzip2   | .bz2       | 是         | 和文本处理一样，不需要修改             |
| LZO      | 否，需要安装 | LZO     | .lzo       | 是         | 需要建索引，还需要指定输入格式         |
| Snappy   | 否，需要安装 | Snappy  | .snappy    | 否         | 和文本处理一样，不需要修改             |

默认不压缩 就是 DEFLATE     

其中自带的  就是 Gzip     bzip2    ,这俩比较慢 ,使用如下

```java
1. suffle 阶段落盘

configuration.setBoolean("mapreduce.map.output.compress", true);
configuration.setClass("mapreduce.map.output.compress.codec", BZip2Codec.class, CompressionCodec.class);

2.输出阶段
    
FileOutputFormat.setCompressOutput(job, true);
FileOutputFormat.setOutputCompressorClass(job, BZip2Codec.class);
```

那么么带的 绝对比较猛哇  : 

LZO    :  使用麻烦,需要自己建立索引,但是能切割文件 

Snappy    : 压缩快,不麻烦 ,就是不能切割文件  

| 压缩算法 | 原始文件大小 | 压缩文件大小 | 压缩速度 | 解压速度 |
| -------- | ------------ | ------------ | -------- | -------- |
| gzip     | 8.3GB        | 1.8GB        | 17.5MB/s | 58MB/s   |
| bzip2    | 8.3GB        | 1.1GB        | 2.4MB/s  | 9.5MB/s  |
| LZO      | 8.3GB        | 2.9GB        | 49.3MB/s | 74.6MB/s |
| Snappy   | 8.3GB        | 2.9GB        | 250M/S   | 500M/S   |





### 详细讲解 和 代码流程展示

>    大致举例一下使用到的东西  ---- 这个虽然 hive 不需要学习这些,但是确实思想
>
> - 框架默认的TextInputFormat切片机制是对任务按文件规划切片，不管文件多小，都会是一个单独的切片，都会交给一个MapTask，这样如果有大量小文件，就会产生大量的MapTask，处理效率极其低下。
> - 下面我讲解的就是  `TextInputFormat`  使用的基本流程 
>
> 1. InputFormat   可以控制输入流  (默认使用的text ) 就是按行读取,按文件进行分配maptask
>
>    - 需要继承对应的 `FileInputFormat` 类,重写 `createRecordReader` 这个方法 
>    - `RecordReader` 对象 需要自己重写 `RecordReader` 的几个方法
>    - 如何获取文件系统  : `FileSystem fileSystem = FileSystem.get(job.getConfiguration());` 
>    -  `InputSplit` 对象 : 解释是 `A section of an input file.` 此对象转换为`FileSplit` 可以获取`Path` ,这个路径是相对于`FDFS`的   ,所以呢 他本质上实际是一个 `File` 对象
>    - 如何获取  `FSDataInputStream` 呢 :   `inputStream = fileSystem.open(fileSplit.getPath());`   所以本质上是一个 `InputStream`,他需要拿到一个 `File` 对象 .
>
>    - 输出(RecordReader) 读记录  给---> Mapper 阶段 
>
> 2. Mapper
>
>    - 输入一般是 一个long值的偏移量,一个text的每行数据
>
>    - 其实读取的是一个 RecordReader 对象
>
>    - 每一个task进来都会执行run方法并且执行下面三个阶段 (可以重写run方法)
>
>      - setup   很简单就是开始task
>      - map      真正的读取数据   ---> map 执行过程中 进行分区操作`Partitioner`的`getPartition()` 方法 
>      - cleanup   真正的结束task
>
>      ​      以上三个阶段共享一个context , 可以获取  `InputSplit`  (`FileSplit`) 就可以获取hdfs的文件路径 , context会对数据进行输出
>
>    - 然后输出  ---> 到Combiner阶段 
>
> 3. Combiner (每一个分区执行一次 )
>
>    - 每一个 maptask 结束都会执行一个Combiner ,`Combiner `实现需要继承`Reducer` 其实就是在进行reduce,可以减少 reduce阶段的数据
>
> 4. Reducer  (每一个分区执行一次 )
>
>    - 获取到的数据是一个 按照 key进行有序排列的数据 , 
>
>    - value 是一个迭代器 ,key对于与value , 每次迭代都会执行  RawComparator 进行比较是reduce否为一组key ,可以根据需求进行控制
>
>    - 也是分为三个阶段
>
>      - setup   很简单就是开始task
>      - map      真正的读取数据,处理数据
>      - cleanup   真正的结束task
>
>      结束完输出一个 Record  对象
>
> 5. RawComparator 
>
>    - 被 Reducer 所执行
>
> 6. OutputFormat   可以控制输入  
>
>    - FileOutputFormat  需要实现一个 一个 ` RecordWriter`  ,其中 这个对象  ` RecordWriter` 就是我们reduce阶段输出的对象 Record 
>    - 如何获取文件系统  : `FileSystem fileSystem = FileSystem.get(job.getConfiguration());` 
>    - FileSystem可以说就是一个 FS ,他可以操作任何文件系统里的对象
>    - 如何 获取 `Configuration`对象呢 ?    Context 里面可以 有
>    - 那么如何获取一个`FSDataOutputStream` 输出流呢 ?    fileSystem.create(new Path("相对于fdfs路径"));
>
> 7. ` FileInputFormat.setInputPaths(path)` 和  `FileOutputFormat.setOutputPath()`  两个方法只是给 `job.getConfiguration().set(FileOutputFormat.OUTDIR, outputDir.toString());` 其实就是个 `Configuration `对象注入了输入输出信息   ,拿到就是 需要通过 `Configuration `对象 `FileOutputFormat.OUTDIR` 和  `FileInputFormat.INPUT_DIR` 两个key值 拿到
>
> 8. 分区的概念  :    就是read 的数据 进行 map操作完 ,进行 分区操作
>
>    - 需要继承`Partitioner` ,重写`getPartition()`  方法   ,
>    - 分区大小 对应着getPartition 返回的数量 , 0-n 不能随意修改 
>    - job.setNumReduceTasks(5);
>
> 



```java
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Partitioner;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

import java.io.IOException;
import java.util.Iterator;

/**
 * Job 流程
 *
 * @date:2019/10/11 20:33
 * @author: <a href='mailto:fanhaodong516@qq.com'>Anthony</a>
 */

public class MyJob {


    public static void main(String[] args) throws Exception {
        // 设置配置
        Configuration configuration = new Configuration();

        //创建 job
        Job job = Job.getInstance(configuration);

        // 设置 加载类
        job.setJarByClass(MyJob.class);
       
        // 设置 map 和 reduce
        job.setMapperClass(MyMapper.class);
        job.setReducerClass(MyReduce.class);

        // 设置分组比较方式
        job.setGroupingComparatorClass(TextGroupingComparator.class);

        //设置分区大小 , 以及分区方式
        job.setNumReduceTasks(3);
        job.setPartitionerClass(MyPartitioner.class);

        // 设置 Combiner 可以减少map阶段的数据 (看需求是否选择)
        job.setCombinerClass(MyCombiner.class);

        //设置 map阶段的输出类型
        job.setMapOutputKeyClass(Text.class);
        job.setMapOutputValueClass(IntWritable.class);

        //设置 reduce阶段输出类型
        job.setOutputKeyClass(Text.class);
        job.setOutputValueClass(IntWritable.class);

        // 5.输入输出流  FileInputFormat 
        FileInputFormat.setInputPaths(job, new Path("D:\\test\\input"));
        FileOutputFormat.setOutputPath(job, new Path("D:\\test\\output"));

        //是否等待完成
        boolean b = job.waitForCompletion(true);

        //成功,失败状态码
        System.exit(b ? 0 : 1);

    }

    //mapper阶段 第一个类型 是偏移量 , 第二个是每行数据
    static class MyMapper extends Mapper<LongWritable, Text, Text, IntWritable> {

        private Text k = new Text();
        private IntWritable v = new IntWritable(1);

        @Override
        protected void map(LongWritable key, Text value, Context context) throws IOException, InterruptedException {
            // 每行数据
            String line = value.toString();

            // 按 空格 分开
            String[] words = line.split(" ");

            for (String word : words) {
                k.set(word);
                //写出去
                context.write(k, v);
            }
        }
    }

    // reduce 阶段
    static class MyReduce extends Reducer<Text, IntWritable, Text, IntWritable> {

        @Override
        protected void reduce(Text key, Iterable<IntWritable> values, Context context) throws IOException, InterruptedException {
            int sum = 0;
            Iterator<IntWritable> iterator = values.iterator();
            // 每次迭代进行获取 数据 (按组分的)
            while (iterator.hasNext()) {
                sum += iterator.next().get();
            }
            //写出去
            context.write(key, new IntWritable(sum));
        }
    }

    static class MyCombiner extends Reducer<Text, IntWritable, Text, IntWritable>{

        IntWritable v = new IntWritable();

        @Override
        protected void reduce(Text key, Iterable<IntWritable> values, Context context) throws IOException, InterruptedException {

            // 1 汇总
            int sum = 0;

            // 2. 累加
            for(IntWritable value :values){
                sum += value.get();
            }
            v.set(sum);

            // 3 写出
            context.write(key, v);
        }
    }

    // reduce阶段执行区分分组 也就是 哪一组key进行 分组
    static class TextGroupingComparator extends WritableComparator {

        // 必须写这个构造方法
        protected TextGroupingComparator() {
            super(Text.class, true);
        }
        /**
         * 返回  0  相同
         * 返回 -1 不同
         */
        @Override
        public int compare(WritableComparable a, WritableComparable b) {
            Text pres = (Text) a;
            Text curs = (Text) b;
            // 根据前面对象和后一个对象比较
            return pres.compareTo(curs);
        }

    }
    
    // 分区方法 返回值最大值 = 分区大小-1
    static class MyPartitioner extends Partitioner<Text, IntWritable> {
        @Override
        public int getPartition(Text text, IntWritable writable, int numPartitions) {
            return text.toString().length()-1;
        }
    }

}
```



input 内的数据

```txt
1 2 3 4
1 22 33 4
2 333 444 3
33 22 444 2
11 22 11 33 333
555 444 222 333 111 22
1 2 3 4
```

output 对比

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-23/eab3d5ed-3067-421b-a621-210ef70e28d2.png?x-oss-process=style/template01)







## YARN

### yarn 的 组成部分



![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-24/fecfd726-f2cb-4766-adf2-00eeaa470056.jpg?x-oss-process=style/template01)

### yarn  执行流程

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-24/dbeb9a67-7244-4d37-9d66-f35b147aa1e5.jpg?x-oss-process=style/template01)



**作业提交全过程详解**

（1）作业提交

第1步：Client调用job.waitForCompletion方法，向整个集群提交MapReduce作业。

第2步：Client向RM申请一个作业id。

第3步：RM给Client返回该job资源的提交路径和作业id。

第4步：Client提交jar包、切片信息和配置文件到指定的资源提交路径。

第5步：Client提交完资源后，向RM申请运行MrAppMaster。

（2）作业初始化

第6步：当RM收到Client的请求后，将该job添加到容量调度器中。

第7步：某一个空闲的NM领取到该Job。

第8步：该NM创建Container，并产生MRAppmaster。

第9步：下载Client提交的资源到本地。

（3）任务分配

第10步：MrAppMaster向RM申请运行多个MapTask任务资源。

第11步：RM将运行MapTask任务分配给另外两个NodeManager，另两个NodeManager分别领取任务并创建容器。

（4）任务运行

第12步：MR向两个接收到任务的NodeManager发送程序启动脚本，这两个NodeManager分别启动MapTask，MapTask对数据分区排序。

第13步：MrAppMaster等待所有MapTask运行完毕后，向RM申请容器，运行ReduceTask。

第14步：ReduceTask向MapTask获取相应分区的数据。

第15步：程序运行完毕后，MR会向RM申请注销自己。

（5）进度和状态更新

YARN中的任务将其进度和状态(包括counter)返回给应用管理器, 客户端每秒(通过mapreduce.client.progressmonitor.pollinterval设置)向应用管理器请求进度更新, 展示给用户。

（6）作业完成

除了向应用管理器请求作业进度外, 客户端每5秒都会通过调用waitForCompletion()来检查作业是否完成。时间间隔可以通过mapreduce.client.completion.pollinterval来设置。作业完成之后, 应用管理器和Container会清理工作状态。作业的信息会被作业历史服务器存储以备之后用户核查。

