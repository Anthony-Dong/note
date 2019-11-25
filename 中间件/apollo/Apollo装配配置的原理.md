

# Apollo客户端原理



## 1. Apollo官方介绍

### 1. 基本架构

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-08/b10e6287-c239-453c-9ce8-b6937848ed49.png)

上图简要描述了Apollo客户端的实现原理：

1. 客户端和服务端保持了一个长连接，从而能第一时间获得配置更新的推送。（通过Http Long Polling实现）
2. 客户端还会定时从Apollo配置中心服务端拉取应用的最新配置。
3. 客户端从Apollo配置中心服务端获取到应用的最新配置后，会保存在内存中
4. 客户端会把从服务端获取到的配置在本地文件系统缓存一份，以免服务器宕机
5. 应用程序可以从Apollo客户端获取最新的配置、订阅配置更新通知

### 2.和Spring集成的原理

Spring从3.1版本开始增加了`ConfigurableEnvironment`和`PropertySource`：

- ConfigurableEnvironment
  - Spring的ApplicationContext会包含一个Environment（实现ConfigurableEnvironment接口），包含
- PropertySource
  - 属性源，可以理解为我们的application.properties

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-08/11e976ca-f750-4889-93f1-5e19ae4194ed.png)

需要注意的是，PropertySource之间是有优先级顺序的，如果有一个Key在多个property source中都存在，那么在前面的property source优先。

所以对上图的例子：

- env.getProperty(“key1”) -> value1
- **env.getProperty(“key2”) -> value2**
- env.getProperty(“key3”) -> value4

在理解了上述原理后，Apollo和Spring/Spring Boot集成的手段就呼之欲出了：在应用启动阶段，Apollo从远端获取配置，然后组装成PropertySource并插入到第一个即可，如下图所示：



![]( https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-08/d093f6f8-dc93-4e74-9f75-071c8579bd97.png)



**知道设计思路了我们就开始吧**

## 2. Apollo-Client 源码

这里介意看看我的那一篇文章  , [本篇自己实现的源码链接](https://github.com/Anthony-Dong/spring-example/tree/master/Spring-Boot%E6%95%99%E7%A8%8B/spring-boot-di) , 可以说还是不错的 , 虽然写的不咋好

### 项目图

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-08/612a1d81-7400-470d-bbd1-1c3d850d9e52.png)

这个是一个大题的目录



### 整个设计思路

这个是我绘制的希望能看懂

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-08/f049026b-de86-4dde-a189-4a36ea1caf5f.jpg)



### 1. ApolloApplicationContextInitializer

```java
/**
  Initialize apollo system properties and inject the Apollo config in Spring Boot bootstrap phase Configuration example:
  
 app.id = 100004458
 apollo.bootstrap.enabled = true
 apollo.bootstrap.namespaces = application,FX.apollo
*/

public class ApolloApplicationContextInitializer implements
    ApplicationContextInitializer<ConfigurableApplicationContext> , EnvironmentPostProcessor, Ordered {
    // 在SPRING 启动的时候就加载了 ,最早加载的
     public void initialize(ConfigurableApplicationContext context) {
         ConfigurableEnvironment environment = context.getEnvironment();
         // 初始化环境
         initialize(environment);
     }
    
    // 为了在Spring加载日志系统阶段之前加载Apollo配置
      public void postProcessEnvironment(ConfigurableEnvironment configurableEnvironment, SpringApplication springApplication) {
      // 初始化环境
       initialize(configurableEnvironment);
  }
}
```

它启动的目的是 : 

1 .  初始化apollo系统属性，并在Spring Boot引导阶段注入apollo配置

2  . 为了 第一时间加载Config端远程的配置文件`Config config = ConfigService.getConfig(namespace);`这个类有一个方法可以获取,在初始化的时候执行的 , 用于 application.properties 里面写的那些namespace都加载进去, 用于启动Apollo配置 

### 2.ApolloAutoConfiguration

```java
@Configuration
// 条件注入 
@ConditionalOnProperty(PropertySourcesConstants.APOLLO_BOOTSTRAP_ENABLED)
@ConditionalOnMissingBean(PropertySourcesProcessor.class)
public class ApolloAutoConfiguration {

  @Bean
  public ConfigPropertySourcesProcessor configPropertySourcesProcessor() {
      // 主要是这个初始化 ConfigPropertySourcesProcessor
    return new ConfigPropertySourcesProcessor();
  }
}

```

`ApolloAutoConfiguration`他的主要目的就是为了初始化 `ConfigPropertySourcesProcessor`

### 3. ConfigPropertySourcesProcessor

```java
public class ConfigPropertySourcesProcessor extends PropertySourcesProcessor
    implements BeanDefinitionRegistryPostProcessor {

  private ConfigPropertySourcesProcessorHelper helper = ServiceBootstrap.loadPrimary(ConfigPropertySourcesProcessorHelper.class);

  @Override
  public void postProcessBeanDefinitionRegistry(BeanDefinitionRegistry registry) throws BeansException {
  // 这里的目的是为了 
    helper.postProcessBeanDefinitionRegistry(registry);
  }
}
```

`ConfigPropertySourcesProcessor` 他的目的有两个 :

1 .  就是初始化 PropertySourcesProcessor

2 .  就是初始化 Processor ,Apollo的一堆监听器处理器`ApolloProcessor`

### 4. PropertySourcesProcessor

说到这个我们必须要说一下 : `@EnableApolloConfig` , 先去看看它去 ,看完几乎就懂了 , 

```java
// Apollo Property Sources processor for Spring Annotation Based Application 
// 文档上说他主要是为了处理基于spring的注解
public class PropertySourcesProcessor implements BeanFactoryPostProcessor, EnvironmentAware, PriorityOrdered {
    
   // 处理逻辑和ApolloApplicationContextInitializer 它一目一样
}
```

所以 看起来真的很简单

### 5. @EnableApolloConfig

```java
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
@Documented
@Import(ApolloConfigRegistrar.class)
public @interface EnableApolloConfig {

  String[] value() default {ConfigConsts.NAMESPACE_APPLICATION};

  int order() default Ordered.LOWEST_PRECEDENCE;
}
```

它呢 , 引入了一个 `ApolloConfigRegistrar` 主要目的就是那`String[] value()` , 其实就是引入多个namespace , 这个很人性化 , 其实就是让注解导入 , 脱离配置文件(例如 application.properties) , 

### 6. ApolloConfigRegistrar

```java
public class ApolloConfigRegistrar implements ImportBeanDefinitionRegistrar {

  // SPI 使用的是
  private ApolloConfigRegistrarHelper helper = ServiceBootstrap.loadPrimary(ApolloConfigRegistrarHelper.class);

  @Override
  public void registerBeanDefinitions(AnnotationMetadata importingClassMetadata, BeanDefinitionRegistry registry) {
    helper.registerBeanDefinitions(importingClassMetadata, registry);
  }
}

# registerBeanDefinitions() 方法中 : 
    AnnotationAttributes attributes = AnnotationAttributes     .fromMap(importingClassMetadata.getAnnotationAttributes(EnableApolloConfig.class.getName()));
    String[] namespaces = attributes.getStringArray("value");
// 关键在这里 
PropertySourcesProcessor.addNamespaces(Lists.newArrayList(namespaces), order);
```

这个类目的也是两个 : 

1 .  将`@EnableApolloConfig`注解上写的 `namespace`注入到 `PropertySourcesProcessor`中

2 . 也是初始化一堆 `ApolloProcessor` ,他也是做了一些判断 , 我也不懂他为啥重复做同样的事情 ,可能考虑到加载时机的问题 .

## 3. 简单使用

我这里直接复制我写的代码了  , 我感觉注释很详细

### 1. 启动时为bean设置属性

```java
/**
 * {@link BeanFactoryPostProcessor} 可以获取  BeanFactory 对象 操作元信息 {@link BeanDefinition} , 在 bean 初始化之前操作的
 *
 * {@link EnvironmentAware} 可以获取环境上下文 , 其实就是我们的配置文件的信息
 *
 * {@link PriorityOrdered}  设置Bean的加载顺序 , 我们设置的是最高优先级
 * @date:2019/11/6 16:26
 * @author: <a href='mailto:fanhaodong516@qq.com'>Anthony</a>
 */
public class MyBeanFactoryPostProcessor implements EnvironmentAware, PriorityOrdered, BeanFactoryPostProcessor {

    private ConfigurableEnvironment environment;

    // source name
    private final String proName = "AnthonyPropertySources";


    private final Map<String, Object> map = new ConcurrentHashMap<>();


    public Map<String, Object> getMap() {
        return map;
    }

    private static final Multimap<Integer, String> NAMESPACE_NAMES = LinkedHashMultimap.create();


    public static boolean addNamespaces(Collection<String> namespaces, int order) {

        return NAMESPACE_NAMES.putAll(order,namespaces);
    }


    /**
     * Modify the application context's internal bean factory after its standard
     * initialization. All bean definitions will have been loaded, but no beans
     * will have been instantiated yet. This allows for overriding or adding
     * properties even to eager-initializing beans.
     */
    @Override
    public void postProcessBeanFactory(ConfigurableListableBeanFactory beanFactory) throws BeansException {

        Object bean = beanFactory.getBean("noRegisterService");

        if (bean instanceof NoRegisterService) {
            NoRegisterService noRegisterService = (NoRegisterService) bean;
            noRegisterService.setValue("hhhhhhhhhhhhhhh");
        }


        System.out.println("==========PropertySourcesProcessor    postProcessBeanFactory=============");

        Collection<String> strings = NAMESPACE_NAMES.get(Ordered.LOWEST_PRECEDENCE);

        // namespace 输出
        strings.forEach(e->{
//            System.out.println("namespace : "+e);
        });

        // 获取pro
        getPro(beanFactory);

        // 设置配置属性 , 可以直接设置 对象的属性  , 不依靠spring
        setPro(beanFactory);

        
        // 初始化 bean , 用存在的source 初始化对象
        initWithHivingSource();


        // 可以自己创建一个  source  , 这里有问题, 好多source不可以使用 ,会出现一堆问题
        initWittCustomSource();


        // 判断类型
        /**
         * {@link DefaultListableBeanFactory}  存储了所有的元信息  spring的大家族所有存储的信息 都在里面
         */
        if (beanFactory instanceof DefaultListableBeanFactory) {
            System.out.println("-----YES , is  DefaultListableBeanFactory--------");
        }

    }


    /**
     * 初始化
     * {@link OriginTrackedMapPropertySource} 是  application.properties 的默认加载配置 ,它是最低优先级 ,
     *
     * 其实我们可以在第一个设置 ,可惜是私有的 : {@link ConfigurationPropertySourcesPropertySource} 它是最高优先级 , {name='configurationProperties'} ,但是人家是私有的
     *
     *  {@link MapPropertySource}   {name='systemProperties'}
     *
     * // spring - boot
     *  OriginTrackedMapPropertySource {name='applicationConfig: [classpath:/application.properties]'}
     *
     *  // spring -cloud
     *  OriginTrackedMapPropertySource {name='applicationConfig: [classpath:/bootstrap.properties]'}
     */
    void initWithHivingSource(){

        System.out.println("--------initWithHivingSource-----------");

        PropertySource<?> applicationConfig = environment.getPropertySources().get("applicationConfig: [classpath:/application.properties]");

/*
        // 这个优先级高
        PropertySource<?> systemProperties = environment.getPropertySources().get("systemProperties");
        if (systemProperties instanceof MapPropertySource) {
            MapPropertySource propertySource = (MapPropertySource) systemProperties;
            propertySource.getSource().put("test.value", "我又改了");
        }*/


        // 这个优先级低
        if (applicationConfig instanceof OriginTrackedMapPropertySource) {
            OriginTrackedMapPropertySource source = (OriginTrackedMapPropertySource) applicationConfig;
            source.getSource().put("test.value", "333333333333");
        }
    }


    /**
     * 定义自己的 source , 这里很有点坑 , {@link PropertySource}不能随意的定义 ,
     * 我这里使用的是 {@link MapPropertySource} 可以使用, 我使用 Apollo开发的时候,
     * 发现 {@link CompositePropertySource} 不能使用
     */
    private void initWittCustomSource() {

        System.out.println("----------initWittCustomSource------------");

        // 存在我们就不操作了
        if (environment.getPropertySources().contains(proName)) {
            //already initialized
            return ;
        }


        /**
         * 创建一个   {@link MapPropertySource}  这个可以么问题 ,但是其他的却有问题 .
         * CompositePropertySource
         */
        // name 是这个 map的名字 ---- , key 是这个map的 属性 , 跟name 么联系 ,
        // 比如 test.key=key  就是这么用 , 不需要加 proName.test.key=key

        MapPropertySource propertySource = new MapPropertySource(proName,map);

        map.put("test.value", "我比你application.properties优先级高");
        map.put("test.key", "key");

        map.put("server.port", 8098);


        // 设置环境 , 可以随意设置
        // environment.getPropertySources().addBefore("applicationConfig: [classpath:/application.properties]", propertySource);

        // 设置环境 , she置在第一位
        environment.getPropertySources().addFirst(propertySource);
    }


    /**
     * 获取 bean 的元信息
     * {@link BeanDefinition}
     * @param beanFactory
     */
    private void getBeanDefinition(ConfigurableListableBeanFactory beanFactory) {
        // 获取名字
        String[] definitionNames = beanFactory.getBeanDefinitionNames();

        for (String definitionName : definitionNames) {
            // 通过名字获取 definition
            BeanDefinition definition = beanFactory.getBeanDefinition(definitionName);
        }
    }




    /**
     * 过滤 bean 的字段
     * @param beanFactory
     */
    private void filterBean(ConfigurableListableBeanFactory beanFactory) {
        String[] beanNames = beanFactory.getBeanDefinitionNames();
        for (String beanName : beanNames) {
            BeanDefinition definition = beanFactory.getBeanDefinition(beanName);
            StringValueResolver valueResolver = new StringValueResolver() {
                @Override
                public String resolveStringValue(String strVal) {
                    return strVal;
                }
            };
            BeanDefinitionVisitor visitor = new BeanDefinitionVisitor(valueResolver);
            // 处理 bean 的配置信息
            visitor.visitBeanDefinition(definition);
        }
    }



    /**
     * 设置属性 , 已知 bean 才用
     * @param beanFactory
     */
    private void setPro(ConfigurableListableBeanFactory beanFactory) {

        AbstractBeanDefinition abstractBeanDefinition = (AbstractBeanDefinition) beanFactory.getBeanDefinition("calculateService");

        System.out.println("BeanFactoryPostProcessor  : setPro");

        MutablePropertyValues pv =  abstractBeanDefinition.getPropertyValues();

        pv.addPropertyValue("desc", "Desc is changed from bean factory post processor");


        abstractBeanDefinition.setScope(BeanDefinition.SCOPE_SINGLETON);
    }



    /**
     * 获取属性 , 已知 bean 才用
     * @param beanFactory
     */
    private void getPro(ConfigurableListableBeanFactory beanFactory) {
        BeanDefinition beanDefinition = beanFactory.getBeanDefinition("calculateService");
        System.out.println("BeanFactoryPostProcessor  : getPro");


        MutablePropertyValues propertyValues = beanDefinition.getPropertyValues();
        PropertyValue[] propertyValues1 = propertyValues.getPropertyValues();

        for (PropertyValue propertyValue : propertyValues1) {
           //
        }
    }


    /**
     * 设置环境属性   你要知道 spring由于是 单例对象 , 你的私有成员变量的 改变也会改变源对象的.
     * @param environment
     */
    @Override
    public void setEnvironment(Environment environment) {
        System.out.println("==========PropertySourcesProcessor    setEnvironment=============");

        if (environment instanceof ConfigurableEnvironment) {
            this.environment = (ConfigurableEnvironment) environment;
        }
    }


    /**
     * 设置优先级 , 不管最高最低都会加载的
     * @return
     */
    @Override
    public int getOrder() {
        return Ordered.HIGHEST_PRECEDENCE;
    }

}

```



### 2. 启动后修改bean

```java
@Component
public class MyBeanPostProcessor implements BeanPostProcessor {

    // 实例化、依赖注入完毕，在调用显示的初始化之前完成一些定制的初始化任务
    @Override
    public Object postProcessBeforeInitialization(Object bean, String beanName) throws BeansException {

        if (beanName.equals("calculateService")) {
            if (bean instanceof CalculateService) {
                System.out.println("==========MyBeanPostProcessor    postProcessBeforeInitialization=============");
                CalculateService bean1 = (CalculateService) bean;
                bean1.setDesc("MyBeanPostProcessor 设置的属性");
            }
        }
        return bean;
    }


    //实例化、依赖注入、初始化完毕时执行 
    @Override
    public Object postProcessAfterInitialization(Object bean, String beanName) throws BeansException {

        if (beanName.equals("calculateService")) {
            if (bean instanceof CalculateService) {
                System.out.println("==========MyBeanPostProcessor    postProcessAfterInitialization=============");
            }
        }
        return bean;
    }
}

```

