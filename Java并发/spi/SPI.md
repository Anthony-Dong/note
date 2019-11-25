# SPI

> ​	SPI全称Service Provider Interface，是Java提供的一套用来被第三方实现或者扩展的API，它可以用来启用框架扩展和替换组件。

**机构图 :** 

调用方 : 需要一个被实现的接口类 和 一个类加载器 , 

服务接口 : 就是你所需要实现的服务

服务提供方 ; 就是服务真正的实现类 

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-11-07/1d0135ff-e16d-4588-9c6b-d44e2c74185a.jpg?x-oss-process=style/template01)



## 快速使用

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

#### 问题一

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



#### 问题二

它是如何实现的呢 ?  

秘密就藏在迭代器里面  :  , 迭代器里面

![]( https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-11-07/9309e875-cde1-48ad-bac4-b6ac2c2be035.png?x-oss-process=style/template01)

我们发现每次迭代的时候都回去执行 Class.forName() 的方法 



#### 问题三

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







