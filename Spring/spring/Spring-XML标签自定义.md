# Spring-XML标签自定义

有些时候, 比较老的项目 , 需要配置Spring-XML .  此时需要我们进行配置我们自己的标签. 其实很简单, 

## 快速开始

Spring的标签中.  

第一在 `META-INF/user.xsd` 文件

```java
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<xsd:schema xmlns="http://user.apache.org/schema/user"
            xmlns:xsd="http://www.w3.org/2001/XMLSchema"
            targetNamespace="http://user.apache.org/schema/user"
            elementFormDefault="qualified">

    <xsd:import namespace="http://www.w3.org/XML/1998/namespace"/>

    <xsd:element name="user">
        <xsd:complexType>
        // 必须有ID项. 就是你getBean(id) 这个传入的. 所以是必须配置的.
            <xsd:attribute name="id" type="xsd:string"/>
            <xsd:attribute name="name" type="xsd:string"/>
            <xsd:attribute name="gender" type="xsd:integer"/>
        </xsd:complexType>
    </xsd:element>
</xsd:schema>
```

第二在 `META-INF/spring.schemas` 文件中 , 导入你的XSD文件. 前面这个URL可以根据自己需求写. 

```java
http\://user.apache.org/schema/user/user.xsd=META-INF/user.xsd
```

第三在 `META-INF/spring.handlers` 中配置你的解析xsd的handler.

```java
http\://user.apache.org/schema/user=com.example.springbeanconfig.bean.UserNameSpaceHandler
```



第四. 我们的 Handler

```java
public class UserNameSpaceHandler extends NamespaceHandlerSupport {
    @Override
    public void init() {
        // 其实就是注册了一个Bean. 
        registerBeanDefinitionParser("user", new UserBeanDefinitionParser());
    }
}
```

第五. 设置我们的Parser

```java
public class UserBeanDefinitionParser implements BeanDefinitionParser {

    @Override
    public BeanDefinition parse(Element element, ParserContext parserContext) {
        String name = element.getAttribute("name");
        String gender = element.getAttribute("gender");
        BeanDefinitionBuilder builder = BeanDefinitionBuilder.genericBeanDefinition(User.class);

        if (name != null) {
            builder.addPropertyValue("name", name);
        }
        if (gender != null) {
            builder.addPropertyValue("gender", Integer.parseInt(gender));
        }
        parserContext.getRegistry().registerBeanDefinition("user", builder.getBeanDefinition());
        // 后期解释为啥不需要返回值. 其实返回值,没啥用.
        return null;
    }
}
```

这样就会注入我们的Bean. 

```java
@Data
public class User {

    String name;

    Integer gender;

}
```



虽然SpringBoot 后期提供了 注解配置. 但是其实这个也是很nice的.  我们的 user.xml

```java
<?xml version="1.0" encoding="UTF-8"?>
<beans xmlns="http://www.springframework.org/schema/beans"
       xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:user="http://user.apache.org/schema/user"
       xsi:schemaLocation="http://www.springframework.org/schema/beans http://www.springframework.org/schema/beans/spring-beans.xsd http://user.apache.org/schema/user http://user.apache.org/schema/user/user.xsd">

    <user:user id="user" name="xiaoli" gender="1"/>
</beans>
```

启动器

```java
public class SpringApp {
    public static void main(String[] args) {
        // 默认自动刷新的.
        ClassPathXmlApplicationContext context = new ClassPathXmlApplicationContext("user.xml");
        User bean = context.getBean(User.class);
        System.out.println(bean);
    }
}
```

输出 : 

```java
User(name=xiaoli, gender=1)
```



## 流程. 

```java
@Override
public void refresh() throws BeansException, IllegalStateException {
    synchronized (this.startupShutdownMonitor) {
        // Prepare this context for refreshing.
        prepareRefresh();

        // Tell the subclass to refresh the internal bean factory.
        ConfigurableListableBeanFactory beanFactory = obtainFreshBeanFactory();

        // Prepare the bean factory for use in this context.
        prepareBeanFactory(beanFactory);

        try {
            // Allows post-processing of the bean factory in context subclasses.
            postProcessBeanFactory(beanFactory);

            // Invoke factory processors registered as beans in the context.
            invokeBeanFactoryPostProcessors(beanFactory);

            // Register bean processors that intercept bean creation.
            registerBeanPostProcessors(beanFactory);

            // Initialize message source for this context.
            initMessageSource();

            // Initialize event multicaster for this context.
            initApplicationEventMulticaster();

            // Initialize other special beans in specific context subclasses.
            onRefresh();

            // Check for listener beans and register them.
            registerListeners();

            // Instantiate all remaining (non-lazy-init) singletons.
            finishBeanFactoryInitialization(beanFactory);

            // Last step: publish corresponding event.
            finishRefresh();
        }

        catch (BeansException ex) {
            if (logger.isWarnEnabled()) {
                logger.warn("Exception encountered during context initialization - " +
                        "cancelling refresh attempt: " + ex);
            }

            // Destroy already created singletons to avoid dangling resources.
            destroyBeans();

            // Reset 'active' flag.
            cancelRefresh(ex);

            // Propagate exception to caller.
            throw ex;
        }

        finally {
            // Reset common introspection caches in Spring's core, since we
            // might not ever need metadata for singleton beans anymore...
            resetCommonCaches();
        }
    }
}
```



在 `ConfigurableListableBeanFactory beanFactory = obtainFreshBeanFactory();`

```java
protected final void refreshBeanFactory() throws BeansException {
    if (hasBeanFactory()) {
        destroyBeans();
        closeBeanFactory();
    }
    try {
        DefaultListableBeanFactory beanFactory = createBeanFactory();
        beanFactory.setSerializationId(getId());
        customizeBeanFactory(beanFactory);
        // 这里加载
        loadBeanDefinitions(beanFactory);
        synchronized (this.beanFactoryMonitor) {
            this.beanFactory = beanFactory;
        }
    }
    catch (IOException ex) {
        throw new ApplicationContextException("I/O error parsing bean definition source for " + getDisplayName(), ex);
    }
}
```



然后`loadBeanDefinitions(beanFactory);`  在 xml这个实现类中

```java
@Override
protected void loadBeanDefinitions(DefaultListableBeanFactory beanFactory) throws BeansException, IOException {
    // Create a new XmlBeanDefinitionReader for the given BeanFactory.
    XmlBeanDefinitionReader beanDefinitionReader = new XmlBeanDefinitionReader(beanFactory);

    // Configure the bean definition reader with this context's
    // resource loading environment.
    beanDefinitionReader.setEnvironment(this.getEnvironment());
    beanDefinitionReader.setResourceLoader(this);
    beanDefinitionReader.setEntityResolver(new ResourceEntityResolver(this));

    // Allow a subclass to provide custom initialization of the reader,
    // then proceed with actually loading the bean definitions.
    initBeanDefinitionReader(beanDefinitionReader);
    // 还是这里加载
    loadBeanDefinitions(beanDefinitionReader);
}

```



```java
protected void loadBeanDefinitions(XmlBeanDefinitionReader reader) throws BeansException, IOException {
    Resource[] configResources = getConfigResources();
    if (configResources != null) {
        reader.loadBeanDefinitions(configResources);
    }
    // 我们传入的 localconfig 就是配置文件,
    String[] configLocations = getConfigLocations();
    if (configLocations != null) {
        reader.loadBeanDefinitions(configLocations);
    }
}
```



继续`reader.loadBeanDefinitions(configLocations);`

```java
@Override
public int loadBeanDefinitions(String... locations) throws BeanDefinitionStoreException {
    Assert.notNull(locations, "Location array must not be null");
    int counter = 0;
    for (String location : locations) {
        // 还是加载. 一个个加载.
        counter += loadBeanDefinitions(location);
    }
    return counter;
}
```



最后在这里 

```java
protected void doRegisterBeanDefinitions(Element root) {

    preProcessXml(root);
    // 就是在这里 注册了我们的Handler.
    parseBeanDefinitions(root, this.delegate);
    postProcessXml(root);
    this.delegate = parent;
}
```

然后再这里 `parseBeanDefinitions`

```java
protected void parseBeanDefinitions(Element root, BeanDefinitionParserDelegate delegate) {
    if (delegate.isDefaultNamespace(root)) {
        NodeList nl = root.getChildNodes();
        for (int i = 0; i < nl.getLength(); i++) {
            Node node = nl.item(i);
            if (node instanceof Element) {
                Element ele = (Element) node;
                if (delegate.isDefaultNamespace(ele)) {
                    parseDefaultElement(ele, delegate);
                }
                else {
                    // 委托执行解析. 但是. 我们发现返回值根本没啥用
                    delegate.parseCustomElement(ele);
                }
            }
        }
    }
    else {
        delegate.parseCustomElement(root);
    }
}
```

然后再这里

```java
public BeanDefinition parseCustomElement(Element ele, @Nullable BeanDefinition containingBd) {
    String namespaceUri = getNamespaceURI(ele);
    if (namespaceUri == null) {
        return null;
    }
    //  拿到 NamespaceHandler
    NamespaceHandler handler = this.readerContext.getNamespaceHandlerResolver().resolve(namespaceUri);
    if (handler == null) {
        error("Unable to locate Spring NamespaceHandler for XML schema namespace [" + namespaceUri + "]", ele);
        return null;
    }
    // 通过它parse 对象. 得到BeanDefinition
    return handler.parse(ele, new ParserContext(this.readerContext, this, containingBd));
}
```



所以parser 最终的返回值是无效的. 所以更像是是一种回调. 所以. 我们并不需要返回 . 需要拿到Register 去注册bean. 



## Dubbo 做代理. 是如何做的. 



第一步也是 创建一个 handler `org.apache.dubbo.config.spring.schema.DubboNamespaceHandler`

```java
public class DubboNamespaceHandler extends NamespaceHandlerSupport {

    static {
        Version.checkDuplicate(DubboNamespaceHandler.class);
    }

    @Override
    public void init() {
        registerBeanDefinitionParser("application", new DubboBeanDefinitionParser(ApplicationConfig.class, true));
        registerBeanDefinitionParser("module", new DubboBeanDefinitionParser(ModuleConfig.class, true));
        registerBeanDefinitionParser("registry", new DubboBeanDefinitionParser(RegistryConfig.class, true));
        registerBeanDefinitionParser("config-center", new DubboBeanDefinitionParser(ConfigCenterBean.class, true));
        registerBeanDefinitionParser("metadata-report", new DubboBeanDefinitionParser(MetadataReportConfig.class, true));
        registerBeanDefinitionParser("monitor", new DubboBeanDefinitionParser(MonitorConfig.class, true));
        registerBeanDefinitionParser("metrics", new DubboBeanDefinitionParser(MetricsConfig.class, true));
        registerBeanDefinitionParser("provider", new DubboBeanDefinitionParser(ProviderConfig.class, true));
        registerBeanDefinitionParser("consumer", new DubboBeanDefinitionParser(ConsumerConfig.class, true));
        registerBeanDefinitionParser("protocol", new DubboBeanDefinitionParser(ProtocolConfig.class, true));
        registerBeanDefinitionParser("service", new DubboBeanDefinitionParser(ServiceBean.class, true));
        registerBeanDefinitionParser("reference", new DubboBeanDefinitionParser(ReferenceBean.class, false));
        registerBeanDefinitionParser("annotation", new AnnotationBeanDefinitionParser());
    }

}
```



这个里面解析太麻烦. 所以不做解释了.  反正就是 

原理就是 注入了一个FactoryBean.   然后这个玩意继承了 ReferenceConfig. 当调用他的get方法的时候, 然后此时就回去调用dubbo的 ReferenceConfig的get方法. 这个类是 `org.apache.dubbo.config.spring.ReferenceBean`

那么这个怎么写, 我大致写一个. 



像Spring没有提供Spring那种活性. 只能这么注入. 

```java
public class MyBeanDefinitionParser implements BeanDefinitionParser {

    @Override
    public BeanDefinition parse(Element element, ParserContext parserContext) {
        String name = element.getAttribute("name");
        String gender = element.getAttribute("gender");

        BeanDefinitionRegistry registry = parserContext.getRegistry();
        BeanDefinitionBuilder builder = BeanDefinitionBuilder.genericBeanDefinition(MyFactoryBean.class);
        User user = new User();
        user.setName(name);
        user.setGender(Integer.parseInt(gender));
        builder.addConstructorArgValue(user);
        registry.registerBeanDefinition("user-factoryBean", builder.getBeanDefinition());
        return null;
    }
}
```

这个是我的 FactoryBean

```java
public class MyFactoryBean implements FactoryBean<User> {
    private User user;
    public MyFactoryBean(User user) {
        this.user = user;
    }
    @Override
    public User getObject() throws Exception {
        user.setAge(1);
        // dubbo的org.apache.dubbo.config.spring.ReferenceBean这里是使用的是调用referenceConfig的get方法. 
        return user;
    }
    @Override
    public Class<?> getObjectType() {
        return User.class;
    }
}
```

所以 基本就是这么实现的.



