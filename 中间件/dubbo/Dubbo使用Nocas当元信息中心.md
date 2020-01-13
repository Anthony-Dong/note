# Dubbo - 为什么选择Nacos

> 坑爹之路 , Nacos更新到1.1.4了 ,然后当你使用 1.1.4的客户端jar包 , 绝对注册不上 ,还有 Dubbo的版本不要太高 , Dubbo和Nacos的注册jar包版本 一致 , 



## 1. 为什么不用Zookeeper

[文章链接 :](http://jm.taobao.org/2018/06/13/%E5%81%9A%E6%9C%8D%E5%8A%A1%E5%8F%91%E7%8E%B0%EF%BC%9F/)  ,这个是阿里写的  ,主要是考虑 dubbo 节点太多的原因 , zk效率太低 , 还有一致性太强了

## 2. 快速开始

### 1. 下载Nacos 服务器 

 [下载链接](https://github.com/alibaba/nacos/releases)

### 2. pom文件, 版本别乱整 , 就写我这个

Dubbo 对于版本的把控真的实在是太坑了 , 动不动缺少东西 ,所以运行时如果出现什么加载错误, 可以直接去找是不是根本没有这个类 ,找个版本合适的就行了....

```xml
<dependencies>
    <dependency>
        <groupId>org.springframework.boot</groupId>
        <artifactId>spring-boot-starter-web</artifactId>
    </dependency>
    <dependency>
        <groupId>org.springframework.boot</groupId>
        <artifactId>spring-boot-starter</artifactId>
    </dependency>
    
 	  <!-- 如果不使用zk , 建议ZK也去了 ,其实这里就一个config依赖有用-->
       <dependency>
            <groupId>com.alibaba.boot</groupId>
            <artifactId>dubbo-spring-boot-starter</artifactId>
            <version>0.2.0</version>
            <exclusions>
                <exclusion>
                    <groupId>com.alibaba</groupId>
                    <artifactId>dubbo</artifactId>
                </exclusion>
            </exclusions>
      </dependency>
    
    <dependency>
        <groupId>com.alibaba</groupId>
        <artifactId>dubbo</artifactId>
        <version>2.6.6</version>
    </dependency>

    <!-- https://mvnrepository.com/artifact/com.alibaba/dubbo-registry-nacos -->
    <dependency>
        <groupId>com.alibaba</groupId>
        <artifactId>dubbo-registry-nacos</artifactId>
        <version>2.6.6</version>
    </dependency>

    <dependency>
        <groupId>com.alibaba.nacos</groupId>
        <artifactId>nacos-client</artifactId>
        <version>1.0.0</version>
    </dependency>

    <dependency>
        <groupId>io.netty</groupId>
        <artifactId>netty-all</artifactId>
        <version>4.1.36.Final</version>
    </dependency>
    
</dependencies>

```

### 3. 快速启动

服务器端配置 : 

```properties 
## application
dubbo.application.name = dubbo-provider-demo

## Nacos registry address
#dubbo.registry.address = nacos://47.93.xxx.xxx:2181
dubbo.registry.address = nacos://192.168.58.131:8848

## Dubbo Protocol
dubbo.protocol.name = dubbo
# 如果你写成 -1 , 默认是 20080 , 这个是netty服务器而已
dubbo.protocol.port = 20080

# Provider @Service version
demo.service.version=1.0.0
demo.service.name = demoService
```

启动类 和我dubbo入门一样 , 几乎不变直接启动就行了 , 然后进入 Nocas的服务器端界面就行了

