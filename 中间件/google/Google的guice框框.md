# Google的Guice框架 - 轻量级的Spring框架

> ​	对于DI 依赖注入. 是开发一个很麻烦的事情. 比如面向接口编程中, 我们只提供一个接口, 我们需要运行时,我们要用到此接口. 如果程序通过方法注入的话,那么会带来一堆的方法信息, 简而言之就是硬编码形式.  对于Java提供了SPI接口的形式可以实现运行时加载接口实现类.  Spring也是提供了一个容器. 可以通过几个配置或者注解便可以实现注入. 
>
> ​		对于今天的框架来说 , `Guice `也是一样, 他遵循了JSR规范, 实现了很好的解耦. 同时Guice也提供了AOP的功能, 其实使用的是CGLIB. 
>
> 我大部分内容参考自[官网](https://www.baeldung.com/guice)   : https://www.baeldung.com/guice

### 快速开始

```xml
<dependency>
  <groupId>com.google.inject</groupId>
  <artifactId>guice</artifactId>
  <version>4.2.0</version>
</dependency>
```



我们先来一个简单的例子 : 

比如最简单的web服务 , service层需要user服务, controller调用service的服务. 就这么一个例子. 

UserService 接口

```java
public interface UserService {
    boolean hasAuth(String name);
}
```

实现类 : 

```java
public class IUserService implements UserService {
    // 简单的实现. 
    @Override
    public boolean hasAuth(String name) {
        return name.length() % 2 == 0;
    }
}
```

UserController 实现类

```java
public class UserController {
    @Inject
    private UserService service;

    /**
     * 判断用户权限
     */
    public boolean userHasAuth(String name) {
        return service.hasAuth(name);
    }
}
```

程序如何启动 : 我们的App类

```java
public class App {
    
    public static void main(String[] args) {
        // 1. 创建一个Injector
        Injector injector = Guice.createInjector(new BasicModule());
		// 2. 实例化对象.
        UserController controller = injector.getInstance(UserController.class);
        // 3. 运行就行了
        controller.userHasAuth("ok");
    }
}
```

那么`BasicModule` 是啥呢, 其实这个就是Guice的核心. 

```java
public class BasicModule extends AbstractModule {

    @Override
    protected void configure() {
        // 表明：当需要 UserService 这个接口时，我们注入 IUserService 的实例作为依赖
        bind(UserService.class).to(IUserService.class);
    }
}
```

这就是一个简单的例子. .   那么到底注入的对象是单例还是原型呢. 我们试试 .

  ```java
public class App {
    public static void main(String[] args) {
        Injector injector = Guice.createInjector(new BasicModule());
        // 实例一个
        UserController controller = injector.getInstance(UserController.class);
        System.out.println(controller.service);
        // 实例两个
        UserController controller1 = injector.getInstance(UserController.class);
        System.out.println(controller1.service);
    }
}
  ```

输出 :  

```java
com.example.service.IUserService@eec5a4a
com.example.service.IUserService@2b2948e2
```

显然不是单例 , 其实他也考虑到了这点, 因此提供了几个方式. 

### 单例模式

我们修改 `AbstractModule` 类. 注释掉我们原来的写法. 我们继续使用 `@Provides` 和 `@Singleton` 注解修饰一下就可以了.  

```java
public class BasicModule extends AbstractModule {
    @Provides
    @Singleton
    public UserService userService() {
        return new IUserService();
    }
}
```

再次执行发现 : 

```java
com.example.service.IUserService@4b1c1ea0
com.example.service.IUserService@4b1c1ea0
```

完全符合预期. 

或者可以通过编程实现. 

```java
@Override
protected void configure() {
    bind(UserService.class).to(IUserService.class).in(Scopes.SINGLETON);
}
```



还可以直接在原实现类上直接标记 `@Singleton` . 

```java
@Singleton
public class IUserService implements UserService {
    @Override
    public boolean hasAuth(String name) {
        return name.length() % 2 == 0;
    }
}

```







### `Named` 来区别多个Bean 

**注解编程**

比如说我们现在有两个UserService的实现类. 此时我们继续 : 

```java
public class DefaultUserService implements UserService {
    @Override
    public boolean hasAuth(String name) {
        return false;
    }
}
```

然后我们再次注入 . 

```java
@Provides
@Singleton
public UserService defaultUserService() {
    return new DefaultUserService();
}
```

再次运行会发现错误. 那怎么办, 其实Spring中也是这样 , 两个Bean如果没有指定就会出现问题.  我们需要用 注解`@Named` 指定用谁 , 比如我们定义的两个对象. 

```java
public class BasicModule extends AbstractModule {
    // 对象一
    @Named("userService")
    @Provides
    @Singleton
    public UserService userService() {
        return new IUserService();
    }
    // 对象二
    @Named("defaultUserService")
    @Provides
    @Singleton
    public UserService defaultUserService() {
        return new DefaultUserService();
    }
}
```

此时我们需要在 `com.example.controller.UserController` 中告诉用谁也得. 

```java
@Named("defaultUserService")
@Inject
public UserService service;
```

此时运行. 

```java
com.example.service.DefaultUserService@76f2b07d
com.example.service.DefaultUserService@76f2b07d
```



**使用AIP注入** 

```java
public class BasicModule extends AbstractModule {

    @Override
    protected void configure() {
        bind(UserService.class).annotatedWith(Names.named("user")).to(IUserService.class).in(Scopes.SINGLETON);
        bind(UserService.class).annotatedWith(Names.named("default")).to(DefaultUserService.class).in(Scopes.SINGLETON);
    }

}
```





### 构造器注入

还有一种注入方式, 构造器注入方式.  不使用注解. 注解也可以的

```java
public class ClassController {

    private List<String> className;

    public ClassController(List<String> className) {
        this.className = className;
    }

    public List<String> getAllClass() {
        return className;
    }
}
```

此时我们写我们的`AbstractModule`

```java
public class ClassModule extends AbstractModule {
    @Override
    protected void configure() {
        bind(ClassController.class).toInstance(new ClassController(Arrays.asList("class1", "class2")));
    }
}
```

此时就成功注入了 .

```java
public static void main(String[] args) {
    Injector injector = Guice.createInjector(new ClassModule());
    ClassController instance = injector.getInstance(ClassController.class);
    System.out.println(instance.getAllClass());
}
```



也可以通过修改构造方法/可能出现构造方法重载的问题 :  `toConstructor()` 方法可以帮助注入构造方法. 

```java
// 需要Boolean就注入这个
bind(Boolean.class).toInstance(true);
// 需要Communication就注入下面这个.
bind(Communication.class).toConstructor(
  Communication.class.getConstructor(Boolean.TYPE));
```

还可以直接写一个单例对象. 

```java
bind(Communication.class).toInstance(new Communication(true));
```



### set / 构造方法注入

支持get,set方法注入. 

```java
@Inject
public void setEmailCommunicator(@Named("EmailComms") CommunicationMode emailComms) {
    this.emailComms = emailComms;
}
// 和下面是等效的.
@Inject @Named("SMSComms")
CommunicationMode smsComms;
```

支持构造方法注入 : 同上.

```java
@Inject
public Communication(@Named("IMComms") CommunicationMode imComms) {
    this.imComms= imComms;
}
```



### 支持Logger 隐式注入

支持 Logger隐式注入  , 不用申明. 

```java
@Inject
Logger logger;
```



### `Provider` 接口使用

可编程话注入 : 

```java
public class AopModule extends AbstractModule {
    @Override
    protected void configure() {
        // 如果需要Service回去找ServiceProvider实例化.
        bind(Service.class).toProvider(ServiceProvider.class).in(Scopes.SINGLETON);
    }
}
```

```java
public class ServiceProvider implements Provider<Service> {
    @Override
    public Service get() {
        Service service = new Service();
        return service;
    }
}
```

```java
public class App {

    public static void main(String[] args) throws InterruptedException {
        Injector injector = Guice.createInjector(new AopModule());
        Service service = injector.getInstance(Service.class);
    }
}
```





### AOP 功能 / MethodInterceptor接口使用

> ​	AOP功能.  底层是GJLIB . 和Spring一样, 

比如拦截这个标记的所有方法. 

```java
@Target({METHOD})
@Retention(RUNTIME)
public @interface LoggerAop {
}
```

我们的拦截器. 

```java
public class LoggerInterceptor implements MethodInterceptor {
    @Override
    public Object invoke(MethodInvocation invocation) throws Throwable {
        long start = System.currentTimeMillis();
        Object proceed = invocation.proceed();
        System.out.println(String.format("%s spend: %dms.", Thread.currentThread().getName(), System.currentTimeMillis() - start));
        return proceed;
    }
}
```

拦截的类 : 

```java
public class Service {
    @LoggerAop
    public String test() throws InterruptedException {
        TimeUnit.SECONDS.sleep(2);
        return "终于做完了";
    }
}
```

配置类:

```java
public class AopModule extends AbstractModule {
    @Override
    protected void configure() {
        bindInterceptor(Matchers.inPackage(Package.getPackage("com.example.apo2")), Matchers.annotatedWith(LoggerAop.class), new LoggerInterceptor());
    }
}
```

这个就是一个拦截器. 

```java
public class App {

    public static void main(String[] args) throws InterruptedException {
        Service service = Guice.createInjector(new AopModule()).getInstance(Service.class);
        String test = service.test();
        System.out.println(test);
    }
}
```

输出 : 

```java
main spend: 2015ms.
终于做完了
```



> ​	其实这里也跟Spring的AOP一样, 都是需要一个MethodInvocation , 只是Spring封装了一层罢了,应该有印象吧.  详情请看他的自己处理方式. 我感觉他封装了两层代理, 第一层代理在 `org.springframework.aop.framework.CglibAopProxy.DynamicAdvisedInterceptor`  , 然后就是 `org.aopalliance.intercept.MethodInterceptor` 做代理,实现回调接口, 基本是是基于aspectj的.  他封装太多了, 其实也不太好.  同时他也封装了一层 `MethodInvocation` 对应的是  `ReflectiveMethodInvocation` .  
>
> ​	但是Spring可以根据对象的特性 , 实现Java的代理和CGLIB的代理. 这个点赞. 
>
> ​	如果想学习Spring的AOP , 源码分析, 介意先学习CGLIB , 其次 ASPECTJ ,最后就好可以看懂了. 基本上就是这个. 



###  ProvisionListener使用

```java
public class MyProvisionListener implements ProvisionListener {
    @Override
    public <T> void onProvision(ProvisionInvocation<T> provision) {
        System.out.println("listener "+provision.getBinding());
    }
}
```

注入 : 

```java
public class AopModule extends AbstractModule {
    @Override
    protected void configure() {
        // 匹配全部
        bindListener(Matchers.any(), new MyProvisionListener());
    }
}
```



### 总结 : 

其中核心就是 `Guice` 对象. 他就是入口. 

其中还有就是 `Module`  , 他类似于Spring的Configuration类. 可以说是多个类 

还有一个就是 `Injector`  , 类似于Spring的ApplicationContext对象. 可以用它来实例化对象, 拿到Bean等. 

还有就是 `Binder`  , 我个人觉得就是Spring的Bean. 注入的Bean. 告诉他注入方式罢了.

还有 `Key` 和 `Binding`  是一对一的关系. 可以通过 `injector.getAllBindings()` 来获取 <key , binding>. 详细信息如下 : 

```java
Key[type=com.example.controller.ClassController, annotation=[none]]
InstanceBinding{key=Key[type=com.example.controller.ClassController, annotation=[none]], source=com.example.confif.BasicModule.configure(BasicModule.java:21), instance=com.example.controller.ClassController@2d6eabae}
```

那么 Binding则是将`Key` 和 `Provider`  和捆绑在一起了. 所以称为 `Binding` 

其实对于我们的代码: 

```java
ClassController instance = injector.getInstance(ClassController.class)
```

其实就是先实例化一个key  , 然后去表里找, 找到一个binding. 然后通过binding拿到Provider 对象. 可以通过provider对象的get方法获取到最终实例化的对象. (对于其核心就是生成一个Provider对象,上面我们代码也自己写了一个Provider对象) 



总体来说Guice遵循了JSR的标准, 进行开发, 所以很方便, 其实他是借鉴了Spring框架, 官网链接前面推荐了, 大家可以看看. 