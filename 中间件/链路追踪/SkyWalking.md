官方地址就是在这里 : https://github.com/apache/skywalking

官方给的设计图在这里 :   这个就是整体的设计架构图. ![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-40-44/624299cd-c18e-4153-9f2d-64e9c53be449.jpg)

## 1. 安装

由于我小白的原因, 其中skywalking 配置文件先介绍一下吧. 

```java
[admin@hadoop1 apache-skywalking-apm-bin]$ ll
total 96
drwxrwxr-x 8 admin admin   136 Nov 12 21:36 agent
drwxrwxr-x 2 admin admin  4096 Apr  4 03:20 bin
drwxrwxr-x 2 admin admin  4096 Apr  4 03:21 config
-rwxrwxr-x 1 admin admin 28903 Nov 12 21:26 LICENSE
drwxrwxr-x 3 admin admin  4096 Apr  4 03:21 licenses
drwxrwxr-x 2 admin admin    94 Apr  4 03:23 logs
drwxrwxr-x 2 admin admin    75 Apr  4 03:24 mesh-buffer
-rwxrwxr-x 1 admin admin 31850 Nov 12 21:26 NOTICE
drwxrwxr-x 2 admin admin 12288 Nov 12 21:43 oap-libs
-rw-rw-r-- 1 admin admin  1978 Nov 12 21:26 README.txt
drwxrwxr-x 3 admin admin    84 Apr  4 03:24 trace-buffer
drwxrwxr-x 2 admin admin    51 Apr  4 03:20 webapp
```

这个是我已经启动过一次的目录. 

我们只需要关注 :

`bin `  目录, 就是启动脚本了. 

`agent` 目录是Skywalking中的可以说是叫做探针的玩意, agent去实现客户端的探针.  对于客户端来说, 主要关注这里. 

`config` 目录, 主要是关于skywalking项目的配置  , 比如你的数据源存放在哪里, 默认是在H2 .

`webapp` 目录 , 就是一个springboot项目, 提供一个web-ui的功能.  可以通过配置告诉你web的端口号, 默认是8080 , 

基本就是这些了. 

其中注意的是 :  比如启动会启动俩服务, 其实就是为了方便罢了. ,web 和 服务是分开的. 

```java
[admin@hadoop1 bin]$ ./startup.sh 
SkyWalking OAP started successfully!
SkyWalking Web Application started successfully
```

我们通过 net 命令, 发现启动了三个监听的端口,.  然后jps 发下是俩引用, 

```java
[admin@hadoop1 bin]$ sudo netstat -nlp
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name    
tcp6       0      0 :::12800                :::*                    LISTEN      2234/java           
tcp6       0      0 :::8080                 :::*                    LISTEN      2241/java           
tcp6       0      0 :::22                   :::*                    LISTEN      1135/sshd   

tcp6       0      0 :::11800                :::*                    LISTEN      2234/java 

[admin@hadoop1 bin]$ jps
2241 skywalking-webapp.jar
2325 Jps
2234 OAPServerStartUp
```

那什么是 `OAP` 呢, 全程是 **观测分析平台(Observability Analysis Platform, OAP)**  , 这里有个文档介绍 : https://skyapm.github.io/document-cn-translation-of-skywalking/zh/6.2.0/concepts-and-designs/backend-overview.html

大致就是这些内容了, . 



## 2. Java-Web 应用, 快速开始

那我们快速开始吧 . 

我这里是一个  jpa + springboot的项目 .  由于我的SkyWalking服务端部署在远程, 此时需要我们整一些配置文件了. 

启动就是 加一个-javaagent:sky.jar 就可以了.  主要是利用RPC接口进行操作的, 性能很高 .  

其中 `restPort`是12800 , `gRPCPort` 是11800 端口.  每一个服务必须制定一个服务名称. 

```java
java -javaagent:/apache-skywalking-apm-incubating/agent/skywalking-agent.jar 
     -Dskywalking.agent.service_name=trace-api 
     -Dskywalking.collector.backend_service=localhost:11800 
     -jar xxxx.jar
```

使用起来很简单,  页面有吞吐量 ,  百分比的相应时间哇, 那个P50 , P99都是百分比的意思. 还有显示你的平均吞吐量

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-40-44/67afa269-2cac-44ae-8885-1e5aa0aafce9.png)

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-40-44/c8c933aa-3eda-4330-997d-661e49083939.png?x-oss-process=style/template01)

基本就是这些了. 

服务 - > 对应着你的服务名称 , 一个服务下可以有多个实例. 多个端点. 

端点 -> 对应着你的HTTP请求

实例 ->  对应着你服务多少台机器. 

Service 的含义就是服务 . 

Database 其实你Service中使用了数据库他就会帮你记录这个database. 包含查询的日志等等. 

这个是链路图 . 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-40-44/18a132c6-1069-4540-a78d-140fd014b692.png?x-oss-process=style/template01)







更多详细的可以自行百度. 

像客户端配置 ,  其实最好每一个客户端都配置一个 agent 配置. 这样使用起来也很方便. 就是把agent包的拷贝一下. 客户端都.  这个是官方给的agent的目录 , 最好每一个服务都拷贝一份 agent文件. 

```java
+-- agent
    +-- activations
         apm-toolkit-log4j-1.x-activation.jar
         apm-toolkit-log4j-2.x-activation.jar
         apm-toolkit-logback-1.x-activation.jar
         ...
    +-- config
         agent.config  
    +-- plugins
         apm-dubbo-plugin.jar
         apm-feign-default-http-9.x.jar
         apm-httpClient-4.x-plugin.jar
         .....
    +-- optional-plugins
         apm-gson-2.x-plugin.jar
         .....
    +-- bootstrap-plugins
         jdk-http-plugin.jar
         .....
    +-- logs
    skywalking-agent.jar
```



基本就是这两个 . 

```properties
# The service name in UI , 这个是你当前的服务ID ,这个是唯一值.默认走的是SW_AGENT_NAME
agent.service_name=${SW_AGENT_NAME:spring-boot}

# 这就是你trace的IP地址了.
collector.backend_service=${SW_AGENT_COLLECTOR_BACKEND_SERVICES:0.0.0.0:11800}
```

更多属性介绍 : https://github.com/apache/skywalking/blob/v6.6.0/docs/en/setup/service-agent/java-agent/README.md