# Apollo 配置服务中心



## 介绍

​      Apollo（阿波罗）是携程框架部门研发的分布式配置中心，能够集中化管理应用不同环境、不同集群的配置，配置修改后能够实时推送到应用端，并且具备规范的权限、流程治理等特性，适用于微服务配置管理场景。服务端基于Spring Boot和Spring Cloud开发，打包后可以直接运行，不需要额外安装Tomcat等应用容器。Java客户端不依赖任何框架，能够运行于所有Java运行时环境，同时对Spring/Spring Boot环境也有较好的支持。

了解 apollo  可以去 github 上 ,地址 https://github.com/ctripcorp/apollo 

## Quick start

快速开始 : https://github.com/ctripcorp/apollo/wiki/Quick-Start

### Prerequisite

1. 项目下载地址,貌似只能在linux上跑,没有bat脚本   https://github.com/nobodyiam/apollo-build-scripts 这个链接下载就行了
2. 直接  在 linux/mac上用 ` git clone git@github.com:nobodyiam/apollo-build-scripts.git`  ,如果你下载在windows上还打开修改 发布到linux上 注意 换行符的不同 , 建议使用  `yum install dos2unix`  下载这个工具
3. 下载安装mysql 版本 ,并且 导入sql脚本
4.  建议内网使用开发 , 不建议远程服务器开发 ,
5. 修改 demo.sh  脚本 中 两个数据库配置信息
6. 启动 脚本就ok了



## Future

1. apollo集群的注册中心是eureka , 默认端口是8080 
2. 数据库 : mysql
3. AppId  项目名称
4. **namespace**   就是 每一个 配置的空间名  ,默认是application
5. 不会因为配置中心宕机,而无法读取配置服务,会本地缓存一份 , 地址` /opt/data/{*appId*}/config-cache`  ,也可以自定义缓存路径 `apollo.cacheDir=/opt/data/some-cache-dir`
6. 支持本地开发  只需要 将配置 `env=Local`  





## Java Development

启动配置参数 :  优先级从高到低

1. 第一个 jvm启动参数 ` -Dapollo.meta=http://1.1.1.1:8080,http://2.2.2.2:8080`
2. Spring Boot配置文件  application.yml 或 bootstrap.yml  指定 `apollo.meta=http://config-service-url`
3. 系统环境变量 : System Environment  `APOLLO_META`
4. server.properties 配置文件   , linux地址 : opt/settings/server.properties ,win : C:\opt\settings\server.properties
5. app.properties  配置文件 , 地址 :  classpath:/META-INF/app.properties



其他参数 打通小异 : 

```
env=DEV
app.id=test
// placeholder在运行时自动更新功能
apollo.autoUpdateInjectedSpringProperties=false
```



### maven依赖

```
<dependency>
        <groupId>com.ctrip.framework.apollo</groupId>
        <artifactId>apollo-client</artifactId>
        <version>1.1.0</version>
    </dependency>
```

### api开发

```java
// 打开 config
Config config = ConfigService.getConfig("springboot.application");
//Config config = ConfigService.getAppConfig(); 
//监听配置变化事件只在应用真的关心配置变化，需要在配置变化时得到通知时使用
config.addChangeListener(configChangeEvent -> {
    Set<String> strings = configChangeEvent.changedKeys();

});

String key = "key1";
String value = "someDefaultValueForTheKey";
// 获取值
String outvalue = config.getProperty(key, value);

System.out.println(outvalue);
```

### spring  快速开始

#### 配置文件开发

```properties
1. 默认只有 application 这个namespace
# will inject 'application' namespace in bootstrap phase
apollo.bootstrap.enabled = true
    

2. 有多个namespace (加载顺序按照 写的先后顺序)
apollo.bootstrap.enabled = true
# will inject 'application', 'springboot.application'namespaces in bootstrap phase
apollo.bootstrap.namespaces = application,springboot.application
# put apollo initialization before logging system initialization
```

#### Spring Placeholder的使用

如果需要关闭placeholder在运行时自动更新功能，可以通过以下方式关闭 `apollo.autoUpdateInjectedSpringProperties=false`

```java
// key 为 timeout ,默认值为100
@Value("${timeout:100}")
private int timeout;
```



#### @ConfigurationProperties使用

例如 `redis.cache.expireSeconds` 这样的key 存在 apollo服务器中 ,  下面例子会自动将  expireSeconds 注入进去 , 但是这样有个问题就是 , 不会自动刷新配置 ..........

```java
@ConfigurationProperties(prefix = "redis.cache")
public class SampleRedisConfig {
  private int expireSeconds;
  private int commandTimeout;
}    
```

自动刷新 需要手动设置

```java
    @ApolloConfigChangeListener(value = {
            ConfigConsts.NAMESPACE_APPLICATION, "springboot.application"},
            interestedKeyPrefixes = {"dev."},
            interestedKeys = {"dev.test"})
    public void onChange(ConfigChangeEvent changeEvent) {
        logger.info("before refresh {}", myConfig.toString());

        if (changeEvent.isChanged("dev.test")) {
            ConfigChange change = changeEvent.getChange("dev.test");
            String newValue = change.getNewValue();
            myConfig.setTest(newValue);
        }
        logger.info("after refresh {}", myConfig.toString());
    }
```





### annotation 使用

1. @ApolloConfig
   - 用来自动注入Config对象
2. @ApolloConfigChangeListener
   - 用来自动注册ConfigChangeListener
3. @ApolloJsonValue
   - 用来把配置的json字符串自动注入为对象



```java
@ApolloConfig("FX.apollo")
private Config yetAnotherConfig; //inject config for namespace FX.apollo
@ApolloConfig("application.yml")
private Config ymlConfig; //inject config for namespace application.yml


// 原数据 jsonBeanProperty=[{"someString":"hello","someInt":100},{"someString":"world!","someInt":200}]
@ApolloJsonValue("${jsonBeanProperty:[]}")
private List<JsonBean> anotherJsonBeans;
```



### 已有配置迁移

可以将 application.yml 或 bootstrap.yml  换成 properties文件 

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-10/561af482-0c27-4d63-a22b-44c25eb505de.png?x-oss-process=style/template01)



### 灰度发布

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-10/03950468-a391-4c3a-a6b6-a4ffdc17acfd.png?x-oss-process=style/template01)



  可以看出 有个灰度列表 , 可以作为测试 发布出去 , 也可以取消