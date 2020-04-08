# Spring-xml的配置祖册流程

简单来上手一个例子. 

比如说. 我们注册一个Bean. 配置是. 



下面是最简单的一个. 那么它是如何做的呢. ? 为啥帮你解析了.  含义是啥各个字段

```xml
<bean id="user" name="user" class="com.example.springbeanconfig.bean.User">
    <property name="name" value="tom"></property>
    <property name="gender" value="1"></property>
</bean>
```

那么究竟是如何注入进去了呢 ? 

这个是启动程序. 

```java
public static void main(String[] args) {
    ClassPathXmlApplicationContext context = new ClassPathXmlApplicationContext("user.xml");
    User bean = context.getBean(User.class);
    System.out.println(bean);
}
```



我们看一下构造器. 就知道为啥不需要refresh了. 

```java
public ClassPathXmlApplicationContext(
        String[] configLocations, boolean refresh, @Nullable ApplicationContext parent)
        throws BeansException {
    super(parent);
    setConfigLocations(configLocations);
    if (refresh) {
        refresh();
    }
}
```

对于Spring的`Context`来说 ,  启动阶段就是 Refresh阶段

```java
public void refresh() throws BeansException, IllegalStateException {
    synchronized (this.startupShutdownMonitor) {
        // Prepare this context for refreshing.[前期准备.基本上啥也不做]
        prepareRefresh();

        // Tell the subclass to refresh the internal bean factory. (这里比较核心.刷新内部的bean-Factory , 其实基本大部分都在这里刷新了.[看下面第一个讲解])
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





```java
protected ConfigurableListableBeanFactory obtainFreshBeanFactory() {
    // 刷新
    refreshBeanFactory();
    ConfigurableListableBeanFactory beanFactory = getBeanFactory();
    return beanFactory;
}
```

```java
protected final void refreshBeanFactory() throws BeansException {
    if (hasBeanFactory()) {
        destroyBeans();
        closeBeanFactory();
    }
    try {
        // 
        DefaultListableBeanFactory beanFactory = createBeanFactory();
        beanFactory.setSerializationId(getId());
        customizeBeanFactory(beanFactory);
        // 主要看这里加载
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

`AbstractXmlApplicationContext`

```java
protected void loadBeanDefinitions(XmlBeanDefinitionReader reader) throws BeansException, IOException {
    Resource[] configResources = getConfigResources();
    if (configResources != null) {
        reader.loadBeanDefinitions(configResources);
    }
    String[] configLocations = getConfigLocations();
    if (configLocations != null) {
        // xml配置会走这里. 这个configLocations.使我们参数传入进去的那个.
        reader.loadBeanDefinitions(configLocations);
    }
}
```



```java
@Override
public int loadBeanDefinitions(String... locations) throws BeanDefinitionStoreException {
    Assert.notNull(locations, "Location array must not be null");
    int counter = 0;
    for (String location : locations) {
        // 加载loadBeanDefinitions . 加载每个location
        counter += loadBeanDefinitions(location);
    }
    return counter;
}
```



```java
public int loadBeanDefinitions(String location, @Nullable Set<Resource> actualResources) throws BeanDefinitionStoreException {
    // 其实就是classPathXmlContext . 他继承了ResourceLoader
    ResourceLoader resourceLoader = getResourceLoader();
    if (resourceLoader instanceof ResourcePatternResolver) {
        try {
            Resource[] resources = ((ResourcePatternResolver) resourceLoader).getResources(location);
            // 还是加载
            int loadCount = loadBeanDefinitions(resources);
            return loadCount;
        }
    }
}
```









`org.springframework.beans.factory.config.BeanDefinitionHolder#BeanDefinitionHolder(org.springframework.beans.factory.config.BeanDefinition, java.lang.String, java.lang.String[])`





`org.springframework.beans.factory.xml.BeanDefinitionParser`

