# SPI

> ​	SPI全称Service Provider Interface，是Java提供的一套用来被第三方实现或者扩展的API，它可以用来启用框架扩展和替换组件。可以理解为一种容器 , 我们可以依靠容器获取我们想要的实例化对象 . 

**机构图 :** 

调用方 : 需要一个被实现的接口类 和 一个类加载器 , 

服务接口 : 就是你所需要实现的服务

服务提供方 ; 就是服务真正的实现类 

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-11-07/1d0135ff-e16d-4588-9c6b-d44e2c74185a.jpg?x-oss-process=style/template01)



## 1. 快速使用

### 1. 简单使用

这个是一个目录结构 : 

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-11-07/a7d26511-e805-48e7-8ea3-4e555a386bc4.jpg?x-oss-process=style/template01)

我们需要在classpath 下面创建一个 META-INF/services目录 , 新建一个接口的全限定名称的文件

文件内容 : 

```java
com.example.spi.FinalEchoService
com.example.spi.AdvancedEchoService
```

类的具体实现 : 

```java
// 1. 接口
public interface EchoService {
    String echo(String msg);
}

// 2. 实现类一
public class FinalEchoService implements EchoService {
    @Override
    public String echo(String msg) {
        return "FinalEchoService : "+msg;
    }
}

//3. 实现类二
public class AdvancedEchoService implements EchoService{
    @Override
    public String echo(String msg) {
        return "AdvancedEchoService :" + msg;
    }
}
```

启动 : 

```java
public class RunDemo {

    public static void main(String[] args) {
        // 默认传入俩参数 , 一个是服务接口, 一个是类加载器, 因为他要去寻找你的classpath
        ServiceLoader<EchoService> services = ServiceLoader.load(EchoService.class, RunDemo.class.getClassLoader());

        Iterator<EchoService> iterator = services.iterator();
        iterator.forEachRemaining(echoService -> {
            System.out.println("echoService.echo(\"hello\") = " + echoService.echo("hello"));
        });
    }
}
```

输出结果: 

```java
echoService.echo("hello") = FinalEchoService : hello
echoService.echo("hello") = AdvancedEchoService :hello
```



### 2. 查找问题 

#### 问题一 : 类加载器

```java
// 1. 实现1 
ServiceLoader<EchoService> services = ServiceLoader.load(EchoService.class);
// 2. 实现2
ServiceLoader<EchoService> services = ServiceLoader.load(EchoService.class, RunDemo.class.getClassLoader());
```

我们发现第一个不需要类加载器 ,这里就有一个技巧,可以快速获取当前线程的类加载器

```java
Thread.currentThread().getContextClassLoader();  //或者是 : 
ClassLoader.getSystemClassLoader();

输出是 : 
Thread.currentThread().getContextClassLoader();
```



#### 问题二 : 如何实现

它是如何实现的呢 ?  

秘密就藏在迭代器里面  :  , 迭代器里面

![]( https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-11-07/9309e875-cde1-48ad-bac4-b6ac2c2be035.png?x-oss-process=style/template01)

我们发现每次迭代的时候都回去执行 Class.forName() 的方法 



#### 问题三 : 构造器

由于是反射所以只能呢个实现默认的构造方法,所以我们试试 ; 

```java
public class FinalEchoService implements EchoService {

    public FinalEchoService() {
        System.out.println("FinalEchoService 无参构造器");
    }
    
    private int x;

    public FinalEchoService(int x) {
        this.x = x;
    }

    @Override
    public String echo(String msg) {
        return "FinalEchoService : "+msg;
    }
}
```

输出结果 : 

```java
FinalEchoService 无参构造器
echoService.echo("hello") = FinalEchoService : hello
```



## 2. Spring中的使用

### 1. SpringServletContainerInitializer

> 1 . `Servlet 3.0`引入的接口，用于在web应用启动时动态添加`servlet`、`filter`和`listener`；
>
> 2 . 基于`spi`机制，`META-INF/services/javax.servlet.ServletContainerInitializer`文件中存放实现该接口的类，这些类会被容器调用 ；
>
> 3 . 只能使用在`Jar`文件中，不能使用在`web`项目中 ；

### 2. 分析机制

我将 SpringServletContainerInitializer 自己实现了一个 , 没有过多的封装 . 其基本原理基本上是一致的 . 

```java
@HandlesTypes({WebApplicationInitializer.class}) // 感兴趣的类 或者 叫做 处理的类
public class MyServletContainerInitializer implements ServletContainerInitializer {

    @Override
    public void onStartup(Set<Class<?>> c, ServletContext ctx) throws ServletException {
        // c 是你感兴趣的类{HandlesTypes} : WebApplicationInitializer 的实现类 ,都存里面了 , 是一些字节码 ,

        // 我们将我们感兴趣的类 , 实例化 , 首先创建一个空的
        List<WebApplicationInitializer> initializers = new LinkedList<>();

        // 遍历所有的 类 {WebApplicationInitializer 的子类 } , 然后实例化 ,放入initializers中
        for (Class<?> aClass : c) {
            try {
                initializers.add((WebApplicationInitializer) aClass.newInstance());
            } catch (InstantiationException e) {
                e.printStackTrace();
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            }
        }

        // 全部调用WebApplicationInitializer 的 onStartup() 方法 , 传入 参数 ServletContext , 初始化web 容器 
        for (WebApplicationInitializer initializer : initializers) {
            initializer.onStartup(ctx);
        }
    }
}
```

```java
public interface WebApplicationInitializer {
	// 需要传入一个 ServletContext 
	void onStartup(ServletContext  servletContext) throws ServletException;
}
```



这个案例的 [Demo](https://github.com/Anthony-Dong/servletContainerInitializer) 在 : https://github.com/Anthony-Dong/servletContainerInitializer