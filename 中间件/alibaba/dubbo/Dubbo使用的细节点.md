# Dubbo 使用中发现的三个问题

> ​	本文中使用的dubbo的版本是2.7.3 , springboot分支也是2.7.3 , springboot版本是2.0.4.release , 所以先确定环境是否一样. 

## 1. Dubbo - Mock  的使用注意点

> ​	mock如果有效,必须使用注册中心,不能使用N/A模式,不然mock无效的.  文章链接 : [http://dubbo.apache.org/zh-cn/docs/user/demos/local-mock.html](http://dubbo.apache.org/zh-cn/docs/user/demos/local-mock.html)    , mock的颗粒度最低只能停留在接口上,不能细化到具体方法上. 还有mock一般用在测试方向.

由于dubbo的RPC使用的是动态代理么 ， 所以默认会走到这里 . 

```java
public class InvokerInvocationHandler implements InvocationHandler {
    private static final Logger logger = LoggerFactory.getLogger(InvokerInvocationHandler.class);
    private final Invoker<?> invoker;

    public InvokerInvocationHandler(Invoker<?> handler) {
        this.invoker = handler;
    }

    @Override
    public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
        String methodName = method.getName();
        Class<?>[] parameterTypes = method.getParameterTypes();
        // 看这里
        return invoker.invoke(new RpcInvocation(method, args)).recreate();
    }
}
```



继续

```java
@Override
public Result invoke(Invocation invocation) throws RpcException {
    Result result = null;

    String value = directory.getUrl().getMethodParameter(invocation.getMethodName(), MOCK_KEY, Boolean.FALSE.toString()).trim();
    // 默认,或者false.
    if (value.length() == 0 || value.equalsIgnoreCase("false")) {
        //no mock
        result = this.invoker.invoke(invocation);
        // 强制.直接调用本地服务
    } else if (value.startsWith("force")) {
        if (logger.isWarnEnabled()) {
            logger.warn("force-mock: " + invocation.getMethodName() + " force-mock enabled , url : " + directory.getUrl());
        }
        //force:direct mock
        result = doMockInvoke(invocation, null);
    } else {
        //fail-mock
        try {
            // 如果是异步,这里显然没有错误,除非连不上
            // 如果同步,那么失败一定会走mock的.
            result = this.invoker.invoke(invocation);
        } catch (RpcException e) {
            if (e.isBiz()) {
                throw e;
            }

            if (logger.isWarnEnabled()) {
                logger.warn("fail-mock: " + invocation.getMethodName() + " fail-mock enabled , url : " + directory.getUrl(), e);
            }
            // 返回mock结果.
            result = doMockInvoke(invocation, e);
        }
    }
    return result;
}
```



那是为什么不能使用直连呢.  关键代码在 `ReferenceConfig` 里

```java
private T createProxy(Map<String, String> map) {
    if (shouldJvmRefer(map)) {
        URL url = new URL(LOCAL_PROTOCOL, LOCALHOST_VALUE, 0, interfaceClass.getName()).addParameters(map);
        invoker = REF_PROTOCOL.refer(interfaceClass, url);
        if (logger.isInfoEnabled()) {
            logger.info("Using injvm service " + interfaceClass.getName());
        }
    } else {
        urls.clear(); // reference retry init will add url to urls, lead to OOM
        // 如果我们的@Reference没有设置url,这里走不通,那代表去注册中心拿
        if (url != null && url.length() > 0) { // user specified URL, could be peer-to-peer address, or register center's address.
            String[] us = SEMICOLON_SPLIT_PATTERN.split(url);
            if (us != null && us.length > 0) {
                for (String u : us) {
                    URL url = URL.valueOf(u);
                    if (StringUtils.isEmpty(url.getPath())) {
                        url = url.setPath(interfaceName);
                    }
                    if (REGISTRY_PROTOCOL.equals(url.getProtocol())) {
                        urls.add(url.addParameterAndEncoded(REFER_KEY, StringUtils.toQueryString(map)));
                    } else {
                        urls.add(ClusterUtils.mergeUrl(url, map));
                    }
                }
            }
        } else { // assemble URL from register center's configuration
            // if protocols not injvm checkRegistry
            // 否则,没有写URL,注册中心去拿url.
            if (!LOCAL_PROTOCOL.equalsIgnoreCase(getProtocol())){
                // 监测注册中心
                checkRegistry();
                List<URL> us = loadRegistries(false);
                if (CollectionUtils.isNotEmpty(us)) {
                    for (URL u : us) {
                        URL monitorUrl = loadMonitor(u);
                        if (monitorUrl != null) {
                            map.put(MONITOR_KEY, URL.encode(monitorUrl.toFullString()));
                        }
                        urls.add(u.addParameterAndEncoded(REFER_KEY, StringUtils.toQueryString(map)));
                    }
                }
                if (urls.isEmpty()) {
                    throw new IllegalStateException("No such any registry to reference " + interfaceName + " on the consumer " + NetUtils.getLocalHost() + " use dubbo version " + Version.getVersion() + ", please config <dubbo:registry address=\"...\" /> to your spring config.");
                }
            }
        }

        // 调用这里是关键.
        if (urls.size() == 1) {
            invoker = REF_PROTOCOL.refer(interfaceClass, urls.get(0));
        } else {
            List<Invoker<?>> invokers = new ArrayList<Invoker<?>>();
            URL registryURL = null;
            for (URL url : urls) {
                invokers.add(REF_PROTOCOL.refer(interfaceClass, url));
                if (REGISTRY_PROTOCOL.equals(url.getProtocol())) {
                    registryURL = url; // use last registry url
                }
            }
            if (registryURL != null) { // registry url is available
                // use RegistryAwareCluster only when register's CLUSTER is available
                URL u = registryURL.addParameter(CLUSTER_KEY, RegistryAwareCluster.NAME);
                // The invoker wrap relation would be: RegistryAwareClusterInvoker(StaticDirectory) -> FailoverClusterInvoker(RegistryDirectory, will execute route) -> Invoker
                invoker = CLUSTER.join(new StaticDirectory(u, invokers));
            } else { // not a registry url, must be direct invoke.
                invoker = CLUSTER.join(new StaticDirectory(invokers));
            }
        }
    }

    if (shouldCheck() && !invoker.isAvailable()) {
        throw new IllegalStateException("Failed to check the status of the service " + interfaceName + ". No provider available for the service " + (group == null ? "" : group + "/") + interfaceName + (version == null ? "" : ":" + version) + " from the url " + invoker.getUrl() + " to the consumer " + NetUtils.getLocalHost() + " use dubbo version " + Version.getVersion());
    }
    if (logger.isInfoEnabled()) {
        logger.info("Refer dubbo service " + interfaceClass.getName() + " from url " + invoker.getUrl());
    }
    /**
     * @since 2.7.0
     * ServiceData Store
     */
    MetadataReportService metadataReportService = null;
    if ((metadataReportService = getMetadataReportService()) != null) {
        URL consumerURL = new URL(CONSUMER_PROTOCOL, map.remove(REGISTER_IP_KEY), 0, map.get(INTERFACE_KEY), map);
        metadataReportService.publishConsumer(consumerURL);
    }
    // create service proxy
    return (T) PROXY_FACTORY.getProxy(invoker);
}
```

我们看看这个实现

```java
@Override
public <T> Invoker<T> refer(Class<T> type, URL url) throws RpcException {
    // 如果走注册中心会走这一步.
    if (REGISTRY_PROTOCOL.equals(url.getProtocol())) {
        return protocol.refer(type, url);
    }
    // 直接连接会走这.
    return new ListenerInvokerWrapper<T>(protocol.refer(type, url),
            Collections.unmodifiableList(
                    ExtensionLoader.getExtensionLoader(InvokerListener.class)
                            .getActivateExtension(url, INVOKER_LISTENER_KEY)));
}

```

所以就是这个问题.   如果玩mock 千万要记住不能直连 , 其实真实环境也不存在直接连接的问题.

## 2. `@Reference` 无法配置`methods`项

> ​	这个是为什么呢, 如果认真敲过的小虎斑, 一定发现这个问题了. 

```java
@Reference(timeout = 100
    , methods = {
    @Method(name = "echo", timeout = 1000, retries = 0)
})
private IEchoService service;
```

此时timeout = 100 , retries为3 , 为什么呢,  `Method`的重写没啥用,还有我不知道为啥这个重试为啥无效. 哈哈哈哈. BUG ? , 方法级别控制无效的 .  其实原因是`methods`在2.7.1忽略掉了 .  根本配置了也不起效果.



其实我们直接核心在 `ReferenceConfig` 这个类上, 那么他一定有它的问题, 我们先着手解决一下, 第一在`ReferenceAnnotationBeanPostProcessor#doGetInjectedBean` 这里调用了 `buildReferenceBeanIfAbsent` , 这里实例化的. 那么传入的参数`attributes` 就是我们想要的

```java
@Override
protected Object doGetInjectedBean(AnnotationAttributes attributes, Object bean, String beanName, Class<?> injectedType,
                                   InjectionMetadata.InjectedElement injectedElement) throws Exception {

    String referencedBeanName = buildReferencedBeanName(attributes, injectedType);

    // 这里去构建一个对象,也就是
    ReferenceBean referenceBean = buildReferenceBeanIfAbsent(referencedBeanName, attributes, injectedType);

    registerReferenceBean(referencedBeanName, referenceBean, attributes, injectedType);

    cacheInjectedReferenceBean(referenceBean, injectedElement);

    return buildProxy(referencedBeanName, referenceBean, injectedType);
}
```

我们现在往上走 , 其实标注的很清晰, 忽略了注解.  上面写了在2.7.1版本做的. 在 `AnnotationUtils`这个类里,这个方法调用. 

```java
public static Map<String, Object> getAttributes(Annotation annotation, PropertyResolver propertyResolver,
                                                boolean ignoreDefaultValue, String... ignoreAttributeNames) {

    if (annotation == null) {
        return emptyMap();
    }

    Map<String, Object> attributes = getAnnotationAttributes(annotation);

    Map<String, Object> actualAttributes = new LinkedHashMap<>();

    for (Map.Entry<String, Object> entry : attributes.entrySet()) {

        String attributeName = entry.getKey();
        Object attributeValue = entry.getValue();

        // ignore default attribute value
        if (ignoreDefaultValue && nullSafeEquals(attributeValue, getDefaultValue(annotation, attributeName))) {
            continue;
        }

        /**
         * @since 2.7.1
         * ignore annotation member
         */
        if (attributeValue.getClass().isAnnotation()) {
            continue;
        }
        if (attributeValue.getClass().isArray() && attributeValue.getClass().getComponentType().isAnnotation()) {
            continue;
        }
        actualAttributes.put(attributeName, attributeValue);
    }


    return resolvePlaceholders(actualAttributes, propertyResolver, ignoreAttributeNames);
}
```

那这就是为什么 我们的methods对象注入不进去的原因; . 

但是当你下载了 2.7.0版本去看的时候, 会发现 , 奥 , 竟然没有`methods` 标签,  其实dubbo原来的版本也没有实现该标签. 

### 解决方案一 : 

> ​	我提交在 [https://github.com/apache/dubbo-spring-boot-project/issues/694](https://github.com/apache/dubbo-spring-boot-project/issues/694)  这个问题里.

目前的做法可能为了向下兼容,  其实可以这么设置就可以触发了 

```java
@Reference(
        timeout = 5000,
        retries = 1,
        parameters = {"echo.retries", "0", "echo.timeout", "1000"}
)
private IEchoService service;
```

这个可以设置echo方法的调用, 可以保证方法级别配置.  这样子就是方法级别优先.

### 解决方案二 : 

把出问题的地方注释掉就行了. `org.apache.dubbo.config.spring.util.AnnotationUtils#getAttributes` 

```java
public static Map<String, Object> getAttributes(Annotation annotation, PropertyResolver propertyResolver,
                                                boolean ignoreDefaultValue, String... ignoreAttributeNames) {

    if (annotation == null) {
        return emptyMap();
    }

    Map<String, Object> attributes = getAnnotationAttributes(annotation);

    Map<String, Object> actualAttributes = new LinkedHashMap<>();

    for (Map.Entry<String, Object> entry : attributes.entrySet()) {

        String attributeName = entry.getKey();
        Object attributeValue = entry.getValue();

        // ignore default attribute value
        if (ignoreDefaultValue && nullSafeEquals(attributeValue, getDefaultValue(annotation, attributeName))) {
            continue;
        }

//            /**
//             * @since 2.7.1
//             * ignore annotation member
//             */
//            if (attributeValue.getClass().isAnnotation()) {
//                continue;
//            }
//            if (attributeValue.getClass().isArray() && attributeValue.getClass().getComponentType().isAnnotation()) {
//                continue;
//            }
        actualAttributes.put(attributeName, attributeValue);
    }


    return resolvePlaceholders(actualAttributes, propertyResolver, ignoreAttributeNames);
}
```

## SpringBoot项目进行Dubbo的Mock测试



第一存在的问题,  进行Dubbo测试, Dubbo环境在我们配置中加入 `@DubboComponentScan` 会自动启动 , 如果我们注册中心都没有配置呢.  是不是很坑

显然, 第一步就要取消 `@DubboComponentScan` 注解 

第二步就是如何隔离接口,   `@MockBean` 可以自动注入 `@Autowired`的未实例化对象, 那么他是不能实现`@Reference` 所加入的对象.  所以我们需要,在原先的地方加一个`@Reference` 加上`@Autowired` 配置. 

```java
@RunWith(SpringRunner.class)
@SpringBootTest(webEnvironment = SpringBootTest.WebEnvironment.NONE, classes = {DubboConsumerConfig.class}
)
public class ConsumerServiceTest {
    @MockBean
    private IEchoService service;

    @Autowired
    private ConsumerService consumerService;

    @Test
    public void echo() {
        when(service.echo(anyString())).thenReturn("hello mock");
        consumerService.echo("xiaoli");
    }
}
```

此时就可以跑了. 完全隔离dubbo的环境.