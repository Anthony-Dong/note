## zookeeper

### 介绍 官网有

### 安装 
#### 1.安装 java(centos7 带有测试版的jdk7需要 删了)
#### 2.安装 maven
	mkdir /usr/maven
	cd /usr/maven
	wget http://mirrors.hust.edu.cn/apache/maven/maven-3/3.5.4/binaries/apache-maven-3.5.4-bin.tar.gz 
	tar -zxvf  apache-maven-3.5.4-bin.tar.gz 
	mv apache-maven-3.5.4-bin.tar.gz  maven-3.5.4
	修改 文件 vim /etc/profile
	追加：
	export MAVEN_HOME=/usr/maven/apache-maven-3.5.4
	PATH=$PATH:$MAVEN_HOME/bin
	最后 source /etc/profile

#### 3.安装zookeeper
	cd /opt
	wget http://mirror.bit.edu.cn/apache/zookeeper/stable/apache-zookeeper-3.5.5-bin.tar.gz
	tar -zxvf apache-zookeeper-3.5.5-bin.tar.gz
	重命名
	cd zookeeper/conf
	cp zoo_sample.cfg zoo.cfg
	修改 zoo.cfg  
	zoo.cfg 内容是:
	# headBeat心跳时间
	tickTime=2000
	# 数据存储路劲
	dataDir=/opt/apache-zookeeper-3.5.5/data
	# 监听端口
	clientPort=2181
	运行 /opt/zookeeper/bin/zkServer.sh start

	bin/zkServer.sh start conf/zoo-1.cfg
	提示	ZooKeeper JMX enabled by default
		Using config: /opt/zookeeper/bin/../conf/zoo.cfg
		Starting zookeeper ... STARTED
	查看进程(默认是2181端口号) netstat -apn |grep 2181   原来是java 运行对应的jar包
		tcp6       0      0 :::2181                 :::*                    LISTEN      35660/java    

	启动  bin/zkCli.sh -server 192.168.58.129:2181		
	1. 启动ZK服务:       sh bin/zkServer.sh start
	2. 查看ZK服务状态: sh bin/zkServer.sh status
	3. 停止ZK服务:       sh bin/zkServer.sh stop
	4. 重启ZK服务:       sh bin/zkServer.sh restart 
#### 4.安装可视化工具
	可以下载文件
	然后找到 main工程 直接 cmd 然偶 maven clean package	

#### 如果zookeeper宕机了  dubbo的 consumer可以连上 provider
	现象：zookeeper注册中心宕机，还可以消费dubbo暴露的服务。
	原因：
		监控中心宕掉不影响使用，只是丢失部分采样数据
		数据库宕掉后，注册中心仍能通过缓存提供服务列表查询，但不能注册新服务
		注册中心对等集群，任意一台宕掉后，将自动切换到另一台
		注册中心全部宕掉后，服务提供者和服务消费者仍能通过本地缓存通讯
		服务提供者无状态，任意一台宕掉后，不影响使用
		服务提供者全部宕掉后，服务消费者应用将无法使用，并无限次重连等待服务提供者恢复
	高可用：通过设计，减少系统不能提供服务的时间
	