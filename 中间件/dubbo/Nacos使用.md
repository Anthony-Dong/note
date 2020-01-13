# Nacos

>  他和ZK一样都可以当做服务发现中心 , 但是他比ZK的优点就是他是AP,ZK是CP, 但是他没有ZK的ZKclient界面, 但是提供了一个web界面

## 1. 前期准备

下载链接 : https://github.com/alibaba/nacos/releases

解压找到启动脚本 , startup.sh -m standalone  , 单机模式

## 2. 配置管理

### 1. API - ConfigService 核心类

```java
public class NacosConfigApplicationTests {

    public static void main(String[] args) throws NacosException, InterruptedException {
        String serverAddr = "192.168.58.131";
        String dataId = "example";
        String group = "DEFAULT_GROUP";
        Properties properties = new Properties();
        properties.put(PropertyKeyConst.SERVER_ADDR, serverAddr);

        // 核心类
        ConfigService configService = NacosFactory.createConfigService(properties);
        String content = configService.getConfig(dataId, group, 5000);
		// 启动查到的配置
        System.out.println("content : "+content);

        configService.addListener(dataId, group, new Listener() {
            @Override
            public void receiveConfigInfo(String configInfo) {
                // 监听到变化
                System.out.println("recieve:" + configInfo);
            }

            @Override
            public Executor getExecutor() {
                return null;
            }
        });

        TimeUnit.SECONDS.sleep(Integer.MAX_VALUE);
    }
}
```



### 2. Spring 启动

```xml
<dependency>
    <groupId>com.alibaba.nacos</groupId>
    <artifactId>nacos-spring-context</artifactId>
    <version>${latest.version}</version>
</dependency>
// 版本 : 0.2.2-RC1
```

设置一个配置类

```java
@Configuration
@EnableNacosConfig(globalProperties = @NacosProperties(serverAddr = "192.168.58.131:8848"))
@NacosPropertySource(dataId = "example", autoRefreshed = true)
public class NacosConfiguration {

}
```

暴露接口

```java
@Controller
@RequestMapping("config")
public class ConfigController {

    @NacosValue(value = "${useLocalCache:false}", autoRefreshed = true)
    private boolean useLocalCache;

    @RequestMapping(value = "/get", method = GET)
    @ResponseBody
    public boolean get() {
        return useLocalCache;
    }
}
```

然后请求`curl -X POST "http://192.168.58.131:8848/nacos/v1/cs/configs?dataId=example&group=DEFAULT_GROUP&content=useLocalCache=true"` 这个发布配置, 然后访问我们的暴露接口

`curl http://localhost:8080/config/get` 会返回true

### 3. Spring-Boot 启动

> ​	由于Nacos遵循了spring-boot 自动装配的原理 , 不用我们手动 enable ,当加入依赖就会启动nacos

#### 1. 依赖

```java
<nacos-config-spring-boot.version>0.2.1</nacos-config-spring-boot.version>

<dependency>
    <groupId>com.alibaba.boot</groupId>
    <artifactId>nacos-config-spring-boot-actuator</artifactId>
    <version>${nacos-config-spring-boot.version}</version>
</dependency>
<dependency>
    <groupId>com.alibaba.boot</groupId>
    <artifactId>nacos-config-spring-boot-starter</artifactId>
    <version>${nacos-config-spring-boot.version}</version>
</dependency>
```

#### 2. 启动

你可以看看官网的例子

```java
@SpringBootApplication
@NacosPropertySource(dataId = "example", autoRefreshed = true)
public class NacosConfigApplication implements CommandLineRunner {

    public static void main(String[] args) {
        SpringApplication.run(NacosConfigApplication.class, args);
    }
}
```

测试接口

```java
@RestController
@RequestMapping("config")
public class ConfigController {

    @NacosValue(value = "${useLocalCache:false}", autoRefreshed = true)
    private boolean useLocalCache;

    @GetMapping(value = "/get")
    public boolean get() {
        return useLocalCache;
    }
}
```

启动springboot , 然后访问下面这个

```shell
curl -X GET "http://localhost:8811/config/get"
```

### 4. ConfigService使用

ConfigService 接口提供的抽象方法 ,几乎是nacos的全部 , 所以基本会这个就行了

```java
public interface ConfigService {

	public String getConfig(String dataId, String group, long timeoutMs) throws NacosException;

	public void addListener(String dataId, String group, Listener listener) throws NacosException;

	public boolean publishConfig(String dataId, String group, String content) throws NacosException;

	public boolean removeConfig(String dataId, String group) throws NacosException;

	public void removeListener(String dataId, String group, Listener listener);

	public String getServerStatus();

}
```

#### 1. 注入 ConfigService

```java
@NacosInjected(properties = @NacosProperties(encode = "UTF-8"))
private ConfigService configService;
```

#### 2. 启动监听

```java
/**
* 方式二 需要注入一个ConfigService对象
* @param args
* @throws Exception
*/
configService.addListener("example", "DEFAULT_GROUP", new AbstractListener() {
    @Override
    public void receiveConfigInfo(String config) {
        System.out.println("configService 监听到变化 : "+config);
    }
});
```

或者,这里可以定义接收类型 , 如果是Properties ,就可以取值了

```java
@NacosConfigListener(dataId = "example",groupId = "DEFAULT_GROUP",timeout = 2000)
public void onMessage(Properties config) {
    System.out.println("监听到变化 : "+config);
}
```

#### 3. 注入Value

```java
/**
* 方式一
* @param config
 */
@NacosValue(value = "${useLocalCache:false}", autoRefreshed = true)
private boolean useLocalCache;
```

#### 4. 发布配置

```java
configService.publishConfig(PROPERTIES_DATA_ID, DEFAULT_GROUP,builder.toString());
```

#### 5. 获取状态

```java
configService.getServerStatus()  //  UP or DOWN
```



#### 4. 总结

发现还可以哇 .... 

### 5. 通过API接口设置配置信息

#### 1. 发布配置信息

```java
curl -X POST "http://192.168.58.131:8848/nacos/v1/cs/configs?dataId=nacos.cfg.dataId&group=test&content=HelloWorld"
```

#### 2. 获取配置信息

```java
curl -X GET "http://192.168.58.131:8848/nacos/v1/cs/configs?dataId=nacos.cfg.dataId&group=test"
```

其实这种接口看看源码 很容易发现 ,,,, 官网提供的以上的几个



## 3. 服务发现

### 1 Spring-Boot 服务

核心类 : 

```java
public static void main(String[] args) throws NacosException {
    String serverAddr = "192.168.58.131";
    String serverName = "example";
    Properties properties = new Properties();
    properties.put(PropertyKeyConst.SERVER_ADDR, serverAddr);
    NamingService namingService = NacosFactory.createNamingService(properties);
	
    // NamingService 就是服务发现的管理
    namingService.registerInstance(serverName, "127.0.0.1", 8811);
	//有一些方法进行选择
    Instance example = namingService.selectOneHealthyInstance(serverName);
}
```

这里我要吐槽一下了 , 大家看

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-17/371549e1-fb9d-4937-8fd1-97241849f6cb.jpg?x-oss-process=style/template01)

这里走的是 PUT方法 ,我们再看看nacos 提供的请求方法 `curl -X POST "http://192.168.58.131:8848/nacos/v1/ns/instance?serviceName=nacos.naming.serviceName&ip=20.18.7.10&port=8080"` 显然是一个PUT方法 , 但是post也支持 , 哈哈哈哈 , 我感觉设计上有问题哇 ,因为Put是幂等性 , 而post不是的, 



整个springboot

```xml
<dependency>
    <groupId>com.alibaba.boot</groupId>
    <artifactId>nacos-discovery-spring-boot-starter</artifactId>
    <version>0.2.1</version>
</dependency>
```

配置

```properties
nacos.discovery.server-addr=192.168.58.131:8848
```

启动

```java
@Controller
@RequestMapping("discovery")
public class DiscoveryController {

    @NacosInjected
    private NamingService namingService;

    @RequestMapping(value = "/get", method = GET)
    @ResponseBody
    public List<Instance> get(@RequestParam String serviceName) throws NacosException {
        return namingService.getAllInstances(serviceName);
    }
}
```

```ssh
crul -PUT "http://192.168.58.131:8848/nacos/v1/ns/instance?serviceName=example&ip=127.0.0.1&port=8811"
```

然后 获取服务节点

```ssh
crul -GET "http://localhost:8888/discovery/get?serviceName=example"
```

我们发现并没有啥用 ....  ,所以还需要走springcloud



### 2. Spring-Cloud 服务

感觉 springboot中并不需要 服务发现, 做微服务的话就转向springcloud了 , 所以这里展示springcloud,还是以springboot建立工程

服务提供方和消费方都需要的依赖 , 同时启动类上都需要加上 `@EnableDiscoveryClient` 注解

```java
<dependency>
    <groupId>com.alibaba.cloud</groupId>
    <artifactId>spring-cloud-starter-alibaba-nacos-discovery</artifactId>
    <version>2.1.0.RELEASE</version>
</dependency>
```

#### 1. 服务消费方

服务提供方启动类

```java
@EnableDiscoveryClient
@SpringBootApplication
public class NacosProviderApplication {

    public static void main(String[] args) {
        SpringApplication.run(NacosProviderApplication.class, args);
    }

    @RestController
    class EchoController {

        @GetMapping(value = "/echo/{string}")
        public String echo(@PathVariable String string) {
            return string;
        }
    }
}
```

服务提供方配置:

```properties
spring.cloud.nacos.discovery.server-addr=192.168.58.131:8848
spring.application.name=service-provider
server.port=18082
```

#### 2. 服务提供方

服务消费方启动类 :

```java
@Slf4j
@EnableDiscoveryClient
@SpringBootApplication
public class SpringConsumerApplication implements ApplicationRunner {

    public static void main(String[] args) {
        new SpringApplicationBuilder().bannerMode(Banner.Mode.OFF).sources(SpringConsumerApplication.class)
                .web(WebApplicationType.SERVLET).run(args);
    }

    @Autowired
    private RestTemplate restTemplate;

	// 必须实现 @LoadBalanced , 不然会无法找到service-provider的地址
    @Bean
    @LoadBalanced
    public RestTemplate restTemplate(RestTemplateBuilder builder){
        return builder.build();
    }

    @Override
    public void run(ApplicationArguments args) throws Exception {
        URI uri = UriComponentsBuilder.fromUriString("http://service-provider/echo/{id}").build("hello");
        ResponseEntity<String> forEntity = restTemplate.getForEntity(uri, String.class);
        log.info("data : {}", forEntity.getBody());
    }
}
```

```properties
server.port=9200
spring.application.name=server-consumer
spring.cloud.nacos.discovery.server-addr=192.168.58.131:8848
```



[![CircleCI](https://circleci.com/gh/alibaba/spring-cloud-alibaba/tree/master.svg?style=svg)](https://circleci.com/gh/alibaba/spring-cloud-alibaba/tree/master)[![Maven Central](https://img.shields.io/maven-central/v/com.alibaba.cloud/spring-cloud-alibaba-dependencies.svg?label=Maven%20Central)](https://search.maven.org/search?q=g:com.alibaba.cloud%20AND%20a:spring-cloud-alibaba-dependencies)[![Codecov](https://codecov.io/gh/alibaba/spring-cloud-alibaba/branch/master/graph/badge.svg)](https://codecov.io/gh/alibaba/spring-cloud-alibaba)[![License](https://img.shields.io/badge/license-Apache%202-4EB1BA.svg)](https://www.apache.org/licenses/LICENSE-2.0.html)



![](https://circleci.com/gh/alibaba/spring-cloud-alibaba/tree/master.svg?style=svg)

![CircleCI](https://circleci.com/gh/alibaba/spring-cloud-alibaba/tree/master.svg?style=svg)



