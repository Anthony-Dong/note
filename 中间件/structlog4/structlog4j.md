# StructLog4J  - 日志插件

> ​	顾名思义 结构化日志  .  链接 : [https://github.com/jacek99/structlog4j](https://github.com/jacek99/structlog4j)

首先安装依赖 

```xml
<!-- Structured logging -->
<dependency>
    <groupId>structlog4j</groupId>
    <artifactId>structlog4j-api</artifactId>
    <version>1.0.0</version>
</dependency>
```

## 是什么

```java
public class Demo {
    private static final ILogger logger = SLoggerFactory.getLogger(Demo.class);

    public static void main(String[] args) {
        logger.info("user-info", "name", "tom", "age", 10,"name");
    }
}
```

输出

```java
17:15:12.658 [main] INFO com.example.springtest.log.Demo - user-info name=tom age=10
```

发现多余的最后一个会舍弃掉, 

## 支持JSON 

需要安装依赖 

```xml
<!-- Optional JSON formatter -->
<dependency>
    <groupId>structlog4j</groupId>
    <artifactId>structlog4j-json</artifactId>
    <version>1.0.0</version>
</dependency>
<!--还依赖与javax拓展包-->
<dependency>
    <groupId>org.glassfish</groupId>
    <artifactId>javax.json</artifactId>
    <version>1.1</version>
</dependency>
```

快速开始 . 

```java
public class Demo {
    private static final ILogger logger = SLoggerFactory.getLogger(Demo.class);

    static {
        StructLog4J.setFormatter(JsonFormatter.getInstance());
    }

    public static void main(String[] args) {
        logger.info("user-info", "name", "tom", "age", 10,"home", Arrays.asList("上海","北京"));
    }
}
```

会输出日志 

```java
17:17:08.071 [main] INFO com.example.springtest.log.Demo - {"message":"user-info","name":"tom","age":10,"home":"[上海, 北京]"}
```

很显然. 每一个都会有"message" , 后面就是根据我们的个数进行配对了.  多余的会舍弃掉. 

基本就是个这,  还支持 yaml 等等. 



