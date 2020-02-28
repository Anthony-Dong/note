# Dubbo 和 SpringBoot

这个版本是 dubbo的 2.7.1的版本. 跟dubbo版本适配 . 

```xml
<dependency>
    <groupId>org.apache.dubbo</groupId>
    <artifactId>dubbo-spring-boot-starter</artifactId>
    <version>2.7.1</version>
</dependency>
<dependency>
    <groupId>org.apache.dubbo</groupId>
    <artifactId>dubbo</artifactId>
    <version>2.7.1</version>
</dependency>
```



## Dubbo-注解驱动

> ​	是基于`@DubboComponentScan`  实现的. 

文章链接 : [https://mercyblitz.github.io/2018/01/01/Dubbo-%E6%B3%A8%E8%A7%A3%E9%A9%B1%E5%8A%A8/](https://mercyblitz.github.io/2018/01/01/Dubbo-注解驱动/)

```java

```





## Dubbo-外部化配置

> ​	是基于 `@EnableDubboConfig` 注解实现的

文章链接 : [https://mercyblitz.github.io/2018/01/18/Dubbo-外部化配置/](https://mercyblitz.github.io/2018/01/18/Dubbo-外部化配置/)





dubbo提供了两种注入方式.  一种是配置文件, 一种是bean

这么就注入了三个bean .  需要开启 `@EnableDubbo` 注解. 他等同于 `@EnableDubboConfig ` 和 `@DubboComponentScan`

```properties
dubbo.application.name=dubbo-provider-demo
dubbo.registry.address=multicast://224.5.6.7:1234
dubbo.protocol.name=dubbo
dubbo.protocol.port=20088
```





