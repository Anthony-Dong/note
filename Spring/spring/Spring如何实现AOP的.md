# Spring的AOP实现

> ​	Spring的AOP确实大家都会使用, 那么它是如何实现的呢, 那么是不是需要我们深入的研究一下. 本文从开始简单的使用. 到源码分析 . 最后依靠Spring提供的AOP框架的接口, 我们自己实现一套AOP逻辑.  深入了解一下他的设计模式和业务流程. 

## 1. 快速使用Spring的AOP

这个是注解 , 让记录调用的日志信息.  

```java
@Target(ElementType.METHOD)
@Retention(RetentionPolicy.RUNTIME)
public @interface LogAop {
}
```

这个是AOP配置类 , 环绕通知.

```java
@Component
@Aspect
public class Logger {
    static final org.slf4j.Logger logger = LoggerFactory.getLogger(Logger.class);
    
    @Around("@annotation(com.example.springbeanconfig.springboot_bean.LogAop)")
    public Object around(ProceedingJoinPoint point) throws Throwable {
        logger.info("thread:{}, method:{}, args:{}.", Thread.currentThread().getName(), point.getSignature().getName(), point.getArgs());
        return point.proceed(point.getArgs());
    }
}
```

我们的启动类

```java
@EnableAspectJAutoProxy
@Configuration
@ComponentScan
public class SringApp {

    public static void main(String[] args) {
        AnnotationConfigApplicationContext context = new AnnotationConfigApplicationContext(SringApp.class);
        UserService bean = context.getBean(UserService.class);
        System.out.println(bean);
        // 调用
        bean.say();
    }

    @Component
    public class UserService {
        @LogAop
        void say() {
            System.out.println("ok");
        }
    }
}
```

输出: 

```java
18:28:55.411 [main] INFO com.example.springbeanconfig.springboot_bean.Logger - thread:main, method:say, args:[].
ok
```

## 2. 查看AOP的实现原理, 注入过程.

前面主要的注入流程. 

```java
flush()方法
->
finishBeanFactoryInitialization(beanFactory);
-> 
beanFactory.preInstantiateSingletons();
-> 
org.springframework.beans.factory.support.DefaultListableBeanFactory#preInstantiateSingletons
->
getBean(beanName);方法去生成Bean.
->
org.springframework.beans.factory.support.AbstractBeanFactory#doGetBean
->
org.springframework.beans.factory.support.AbstractAutowireCapableBeanFactory#createBean
->    
org.springframework.beans.factory.support.AbstractAutowireCapableBeanFactory#doCreateBean
->
org.springframework.beans.factory.support.AbstractAutowireCapableBeanFactory#applyMergedBeanDefinitionPostProcessors
->BeanPostProcessor->postProcessMergedBeanDefinition
->org.springframework.aop.framework.autoproxy.AbstractAutoProxyCreator#wrapIfNecessary
->org.springframework.aop.framework.autoproxy.AbstractAutoProxyCreator#createProxy   
```

基本就是这个情况, 实现原理是. 根据`BeanPostProcess`实现. 也就是当对象生成的时候, 依靠after事件触发的时候,代理生成一个对象. 

其中`org.springframework.aop.framework.autoproxy.AbstractAutoProxyCreator#createProxy   ` 的逻辑. 

```java
protected Object createProxy(Class<?> beanClass, @Nullable String beanName,
        @Nullable Object[] specificInterceptors, TargetSource targetSource) {

    // ProxyFactory 继承了AdvisedSupport , 其实就是元信息/
    ProxyFactory proxyFactory = new ProxyFactory();
    proxyFactory.copyFrom(this);
    Advisor[] advisors = buildAdvisors(beanName, specificInterceptors);
    // 添加拦截器
    proxyFactory.addAdvisors(advisors);
    // 这是被增强的对象
    proxyFactory.setTargetSource(targetSource);
    // 
		... 
    //最后就直接拿到代理对象了.
    return proxyFactory.getProxy(getProxyClassLoader());
}
```

这个 `proxyFactory.getProxy(getProxyClassLoader());`  执行一下方法. 主要就是两步, 拿到`AopProxy` , 第二步然后调用`getProxy`方法

```java
public Object getProxy(@Nullable ClassLoader classLoader) {
    return createAopProxy().getProxy(classLoader);
}
```

其中`AopProxy`具体代理实现有两种 : 

`org.springframework.aop.framework.CglibAopProxy` 和 `org.springframework.aop.framework.JdkDynamicAopProxy`   这两个

同时我们可以依靠 `AopProxyFactory`  拿到一个 `AopProxy` 对象. 

```java
public interface AopProxyFactory {
	AopProxy createAopProxy(AdvisedSupport config) throws AopConfigException;
}
```

`createAopProxy` 的 具体实现如下 : 

```java
public class DefaultAopProxyFactory implements AopProxyFactory, Serializable {
	@Override
	public AopProxy createAopProxy(AdvisedSupport config) throws AopConfigException {
        // 是否需要优化.  是否直接需要代理 , 还有就是是不是这个是一个普通类.
		if (config.isOptimize() || config.isProxyTargetClass() || hasNoUserSuppliedProxyInterfaces(config)) {
            // config中获取代理接口.
			Class<?> targetClass = config.getTargetClass();
            // 空抛出异常
			if (targetClass == null) {
				// 
			}
             // targetClass.isInterface()它是一个接口. 就走JDK代理
			if (targetClass.isInterface() || Proxy.isProxyClass(targetClass)) {
				return new JdkDynamicAopProxy(config);
			}
            // 否则走CGLIB.
			return new ObjenesisCglibAopProxy(config);
		}
		else {
			return new JdkDynamicAopProxy(config);
		}
	}
}
```

具体就是上诉讲解的.   其实`AdvisedSupport`就是一个配置类. 没了 . 

我们分析一下CGLIB的代理

```java
@Override
public Object getProxy(@Nullable ClassLoader classLoader) {

    try {
        // 拿到被代理的类信息
        Class<?> rootClass = this.advised.getTargetClass();
        Class<?> proxySuperClass = rootClass;
        // 额外判断.是不是一个CGLIB已经代理的类. 也就是类名中带有$$.
        if (ClassUtils.isCglibProxyClass(rootClass)) {
            // 
        }
        // Configure CGLIB Enhancer...
        Enhancer enhancer = createEnhancer();
        if (classLoader != null) {
            // 空监测
        }
        // 设置代理类的父类
        enhancer.setSuperclass(proxySuperClass);
        // 设置我们额代理类需要实现的接口. 
        enhancer.setInterfaces(AopProxyUtils.completeProxiedInterfaces(this.advised));
        // 设置命名规则.
        enhancer.setNamingPolicy(SpringNamingPolicy.INSTANCE);
        // 设置字节码生成策略. 其中它多实现了两个接口,所以需要自定义实现一些代码. 这个不好操作. 
        enhancer.setStrategy(new ClassLoaderAwareUndeclaredThrowableStrategy(classLoader));
	    // 设置Callback , 也就是业务逻辑
        Callback[] callbacks = getCallbacks(rootClass);
        Class<?>[] types = new Class<?>[callbacks.length];
        for (int x = 0; x < types.length; x++) {
            types[x] = callbacks[x].getClass();
        }
        // 设置CallbackFilter , 因为有多个设置Callback.
        enhancer.setCallbackFilter(new ProxyCallbackFilter(
                this.advised.getConfigurationOnlyCopy(), this.fixedInterceptorMap, this.fixedInterceptorOffset));
        enhancer.setCallbackTypes(types);

        // 最后生成代理对象.
        return createProxyClassAndInstance(enhancer, callbacks);
    }
    catch (CodeGenerationException | IllegalArgumentException ex) {
        throw .. 
    }
    catch (Throwable ex) {
        throw new AopConfigException("Unexpected AOP exception", ex);
    }
}
```

## 3. 如何不借助Spring, 使用它的AOP框架呢.

我大致写了个Demo. 大家可以来看看Spring的主要AOP的代理逻辑吧. 其实它的设计蛮好的. 

我们先定义一个接口 `EchoService`

```java
public interface EchoService {
    String echo();
}
```

`EchoServiceImpl` 实现了 `EchoService` 接口 . 

```java
public class EchoServiceImpl implements EchoService {
    @Override
    public String echo() {
        return "hello world";
    }
}
```

我们的`Main` 主方法的业务逻辑, 注释很详细. 可以看看主要流程. 

```java
public class Main {
    public static void main(String[] args) {
        // 1.CGLIB的话, 可以将字节码保存在指定目录, 方便我们查看字节码
        System.setProperty(DebuggingClassWriter.DEBUG_LOCATION_PROPERTY, "D:/test");

        // 2.创建 AopProxyFactory
        AopProxyFactory factory = new DefaultAopProxyFactory();

        // 3.AopProxyFactory 需要传入一个 AdvisedSupport 的config类.
        AdvisedSupport support = new AdvisedSupport();

        // 4.配置source. 也就是定义一些元信息,其实Spring这里注入的其实是Context对象.
        support.setTargetSource(new TargetSource() {
            // 单例对象.
            final EchoService echoService = new EchoServiceImpl();

            // 这个就是设置代理的类信息
            @Override
            public Class<?> getTargetClass() {
                return EchoService.class;
            }

            // true的话, 他不会去调用releaseTarget方法.
            // false的话, 这个会去调用releaseTarget方法.
            @Override
            public boolean isStatic() {
                return false;
            }

            // 设置代理的对象. 也就是需要一个原对象, 调用方法的时候走这个.
            @Override
            public Object getTarget() throws Exception {
                // 这个每次方法调用都回去执行 getTarget , 所以最好单例.
                return echoService;
            }

            @Override
            public void releaseTarget(Object target) throws Exception {
                System.out.println("调用释放方法");
            }
        });

        // 4.如果你学过 aop aliens . 那么一定会发现这个是啥. 这个就是一个拦截器
        support.setAdvisorChainFactory((config, method, targetClass) -> {
            MethodInterceptor interceptor = invocation -> {
                System.out.println("调用代理方法");
                return invocation.proceed();
            };
            return Collections.singletonList(interceptor);
        });

        // 5.拿工厂创建一个 AopProxy对象.
        AopProxy proxy = factory.createAopProxy(support);
        System.out.println("代理模式 : "+proxy);

        // 6.直接调用getProxy方法. 会获取一个实例化对象. 改对象实现了 SpringProxy, Advised, Factory 和继承了EchoServiceImpl类.
        EchoService service = (EchoService) proxy.getProxy();

        System.out.println(service.echo());
    }
}
```

以上会输出 : 

```java
代理模式 : org.springframework.aop.framework.JdkDynamicAopProxy@484990cc
调用代理方法
调用释放方法
hello world   
```

所以上面这个写法走得是JDK的代理逻辑. 

那么如何转成CGLIB呢   , 其实我们将代码做一下改动就行了 .也就是`EchoServiceImpl` 不实现`EchoService`  就会自动走CGLIB代理了, 所以还是很方便的.  

```java
public class EchoServiceImpl {
    public String echo() {
        return "hello world";
    }
}
```

主方法 : 

```java
public class Main {
    public static void main(String[] args) {
        System.setProperty(DebuggingClassWriter.DEBUG_LOCATION_PROPERTY, "D:/test");
        AopProxyFactory factory = new DefaultAopProxyFactory();
        AdvisedSupport support = new AdvisedSupport();
        support.setTargetSource(new TargetSource() {
            final EchoServiceImpl echoService = new EchoServiceImpl();
            @Override
            public Class<?> getTargetClass() {
                return EchoServiceImpl.class;
            }
            @Override
            public boolean isStatic() {
                return false;
            }

            @Override
            public Object getTarget() throws Exception {
                return echoService;
            }
            @Override
            public void releaseTarget(Object target) throws Exception {
                System.out.println("调用释放方法");
            }
        });
        support.setAdvisorChainFactory((config, method, targetClass) -> {
            MethodInterceptor interceptor = invocation -> {
                System.out.println("调用代理方法");
                return invocation.proceed();
            };
            return Collections.singletonList(interceptor);
        });
        AopProxy proxy = factory.createAopProxy(support);
        System.out.println("代理模式 : "+proxy);
        EchoServiceImpl service = (EchoServiceImpl) proxy.getProxy();
        System.out.println(service.echo());
    }
}
```

输出 : 

```java
代理模式 : org.springframework.aop.framework.ObjenesisCglibAopProxy@127a6246
调用代理方法
调用释放方法
hello world    
```

