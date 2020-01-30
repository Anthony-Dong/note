# Spring中BeanFactory和FactoryBean的使用

## BeanFactory

其实对于Spring来说, 其实记住他的名词很简单, 咱们从一个Bean的生命周期来讲, 第一步需要将Bean,生成一个BeanDefinition , 其次就是将 BeanDefinition 给 BeanDefinitionRegistry, 然后交给统一的 BeanFactory生成, 

所以这个BeanFactory就是Spring帮助我们自动生成的Bean的工厂, 所以对于用户来说是不需要管理的 . 对于SpringBoot中, 他的BeanFactory是 `DefaultListableBeanFactory` 



显然对于Spring自动帮我们生成的我们没必要研究, 需要研究的是 我们可以手动控制的, 比如MyBatis整合Spring框架, 他需要考虑什么. 第一不能让Spring给他生成Mapper对象, 这时候就需要引入, FactoryBean. 



## FactoryBean

> ​	Interface to be implemented by objects used within a BeanFactory which are themselves factories for individual objects. If a bean implements this interface, it is used as a factory for an object to expose, not directly as a bean instance that will be exposed itself.
>
> 由BeanFactory中使用的对象实现的接口，这些对象本身就是各个对象的工厂。



此时有个简单的例子 ,  比如说我们有个接口的对象, 需要实例化一个Spring实现不了的Bean, 比如Mapper , 

假如就是这个

```java
public interface MyMapper {
    void say(String msg);

    default void talk() {
        System.out.println(this+"talk");
    }
}
```



```java
@Configuration(value = MyBeanFactory.FACTORY_NAME)
public class MyBeanFactory<T> implements FactoryBean<T> {
    static final String FACTORY_NAME = "MyBeanFactory";

    private Class<T> mapperInterface;

    // 问题就是这里如何处理, 因为Spring对于Bean的生成, 会首先执行默认构造器
    public MyBeanFactory(Class<T> mapperInterface) {
        this.mapperInterface = mapperInterface;
    }

    // 生成代理对象, 这个回调方法会被Spring启动过程中执行, 
    @Override
    public T getObject() throws Exception {
        T say = getProxy();
        return say;
    }

    // 获取代理对象
    private T getProxy() {
        return (T) Proxy.newProxyInstance(Thread.currentThread().getContextClassLoader(), new Class[]{mapperInterface}, (proxy, method, args) -> {
            if (method.getDeclaringClass() == Object.class) {
                return method.invoke(this, args);
            }
            if (method.isDefault()) {
                return invokeDefaultMethod(proxy, method, args);
            }
            if (method.getName().equals("say")) {
                System.out.println("输出 : " + args[0]);
            }
            return null;
        });
    }


    @Override
    public Class<?> getObjectType() {
        return MyMapper.class;
    }


    private Object invokeDefaultMethod(Object proxy, Method method, Object[] args)
            throws Throwable {
		...// 可以参考MapperProxy 的实现
    }
}
```



设置一个拦截器 . 

```java
@Configuration
public class MyBeanFactoryPostProcessor implements BeanFactoryPostProcessor {

    @Override
    public void postProcessBeanFactory(ConfigurableListableBeanFactory beanFactory) throws BeansException {
        BeanDefinition beanDefinition = beanFactory.getBeanDefinition(MyBeanFactory.FACTORY_NAME);
        ConstructorArgumentValues values = beanDefinition.getConstructorArgumentValues();
        // 注入构造器参数
        values.addGenericArgumentValue(MyMapper.class);
    }
}
```

此时就OK了 

```java
@SpringBootApplication
public class SpringFactoryBeanApplication implements CommandLineRunner {

    public static void main(String[] args) {
        SpringApplication.run(SpringFactoryBeanApplication.class, args);
    }

    @Autowired
    private MyMapper service;

    @Override
    public void run(String... args) throws Exception {
        service.say("hello world");
        service.talk();
    }
}
```



```java
输出 : hello world
com.example.springfactorybean.config.MyBeanFactory$$EnhancerBySpringCGLIB$$ef09ee61@37d80fe7talk
```



其实这就是 Mybatis整合的原理 



其中就是 `org.mybatis.spring.mapper.MapperFactoryBean`  , 他就是用来实例化每一个Mapper对象的 , 每一个Mapper对象会对应一个MapperfactoryBean. 

```java
@Override
public T getObject() throws Exception {
	return getSqlSession().getMapper(this.mapperInterface);
}
```

其实这里就是对应着我们手动写的 

```java
SqlSession.getMapper("....")
```

其实对于Spring中 , Mybatis 实现了一个核心对象, 是 `SqlSessionTemplate` 对应着我们的 `SqlSession`  

还有一个是 `SqlSessionFactoryBean` 