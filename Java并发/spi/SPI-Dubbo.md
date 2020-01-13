# SPI -Java & Dubbo

> ​	SPI全称`Service Provider Interface`，是Java提供的一套用来被第三方实现或者扩展的API，它可以用来启用框架扩展和替换组件。但是如果你了解Dubbo , 那么 Dubbo的那套 SPI  , 可算是最强的 . 我们后面会介绍到

**机构图 :** 

调用方 : 需要一个被实现的接口类 和 一个类加载器 , 

服务接口 : 就是你所需要实现的服务

服务提供方 ; 就是服务真正的实现类 

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-11-07/1d0135ff-e16d-4588-9c6b-d44e2c74185a.jpg?x-oss-process=style/template01)



## 1. 介绍

[链接](http://dubbo.apache.org/zh-cn/docs/source_code_guide/dubbo-spi.html)   : Dubbo官网讲的 ,所以多看看官方文档 , Dubbo的SPI 更加成熟 , 是 Dubbo的核心 ,源码我就不展示了,官网有讲解 .. . .

## 2. 快速开始

依赖

```xml
<dependency>
    <groupId>com.alibaba</groupId>
    <artifactId>dubbo</artifactId>
    <version>2.6.2</version>
</dependency>
```



1) **接口**   :  必须有 SPI 申明

```java
@SPI
public interface Robot {
    void sayHello();
}
```



2) **接口实现类**

```java
public class OptimusPrime implements Robot {
    
    @Override
    public void sayHello() {
        System.out.println("Hello, I am Optimus Prime.");
    }
}

public class Bumblebee implements Robot {

    @Override
    public void sayHello() {
        System.out.println("Hello, I am Bumblebee.");
    }
}
```

还需要申明一下接口文件 还有 实现类全限定类名

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-02/4c33852d-ba03-49cf-acb7-c9d4088a0e83.jpg?x-oss-process=style/template01)

测试一下

```java
public class DubboSPITest {

    @Test
    public void sayHello() throws Exception {
        // 启动一个 ExtensionLoader
        ExtensionLoader<Robot> extensionLoader =
                ExtensionLoader.getExtensionLoader(Robot.class);

        // 根据key值获取
        Robot optimusPrime = extensionLoader.getExtension("optimusPrime");
        optimusPrime.sayHello();

         // 根据key值获取
        Robot bumblebee = extensionLoader.getExtension("bumblebee");
        bumblebee.sayHello();
    }
}
```

输出

```java
Hello, I am Optimus Prime.
Hello, I am Bumblebee.
```

上诉只是Dubbo的一小部分 , 其中还有 IOC和AOP ,但是目前IOC只支持set方法注入

## 2. @SPI注解

### 1. 快速开始

```java
@Documented
@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.TYPE})
public @interface SPI {

    /**
     * default extension name
     */
    String value() default "";

}
```

以我们上面的为例子 ,其他都不变 ,我们在`Robot`接口的`@SPI`注解中加入了 bumblebee

```java
@SPI("bumblebee")
public interface Robot {
    void sayHello();
}
```

此时我们执行测试类 :  此时需要改为 `extensionLoader.getAdaptiveExtension();`

```java
public class DubboSPITest {

@Test
public void sayHello() throws Exception {
    ExtensionLoader<Robot> extensionLoader =
            ExtensionLoader.getExtensionLoader(Robot.class);
    
    // 获取一个自适应类
    Robot adaptiveRobot = extensionLoader.getAdaptiveExtension();
    adaptiveRobot.sayHello();
}
```

输出

```java
Hello, I am Bumblebee.
```

此时就引入了我们Dubbo最强大的功能 `Adaptive`

但是我们此时又改了 ,我们在`com.dubbo.spi.OptimusPrime` , 类上面加了一个 `@Adaptive` 注解 , 此时其他都不动继续执行 , 输出 

```java
Hello, I am Optimus Prime.
```

### 2. 注意和结论

所以结论就是 ,如果是自适应注入的话, `@Adaptive` 的优先级 高于 `@SPI(value="")` 显示声明高...

但是如果是默认注入的话 , 那么 `@SPI(value="")`才有用

```java
1. com.dubbo.spi.Robot 类加入 @SPI("bumblebee")
2. com.dubbo.spi.OptimusPrime 加入  @Adaptive
3. com.dubbo.spi.Bumblebee 什么也不加    

@Test
public void testDefault(){
    ExtensionLoader<Robot> loader = ExtensionLoader.getExtensionLoader(Robot.class);
    Robot robot = loader.getDefaultExtension();
    robot.sayHello();
}
```

输出

```java
Hello, I am Bumblebee.
```

## 3. IOC 

### 1. 使用

其实IOC 依赖于 Adaptive的 , 因为注入就是要区分优先级的  ,举个例子

我们有一个 `com.dubbo.spi.Store` 接口 , 

```java
@SPI
public interface Store {
    void sell(String name);
}
```

还有一个实现类

```java
public class AliBaBaStore implements Store {

    private Robot robot;

    public void setRobot(Robot robot) {
        this.robot = robot;
    }

    @Override
    public void sell(String name) {
        System.out.println(name+" 卖了 : " + robot);
    }
}
```

还需要在 `META-INF/dubbo/com.dubbo.spi.Store` 中添加`alibaba = com.dubbo.spi.AliBaBaStore` 这句话 ,

启动测试类 ....

```java
public class DubboSPITest {
    @Test
    public void sayHello() throws Exception {
        ExtensionLoader<Store> loader = ExtensionLoader.getExtensionLoader(Store.class);
        Store ali = loader.getExtension("alibaba");
        ali.sell("alibaba");
    }
}
```

输出 ....

```java
alibaba 卖了 : com.dubbo.spi.OptimusPrime@23223dd8
```

确实如此 , 我们的`com.dubbo.spi.OptimusPrime` 类上面申明了注解 `@Adaptive` ,优先级最高 , **所以set注入是一种自适应注入 ,需要注意一下  , 而且Dubbo 会自动帮助我们实现加载的**

我们可以在启动参数上加入 `-XX:+TraceClassLoading` 看一下

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-02/e725d75b-9ae9-48b1-b541-6786e3d5b798.jpg?x-oss-process=style/template01)

我们发现确实都加载了 ...

### 2. 注意点 

```java
@Test
public void testLoad(){
    ExtensionLoader<Robot> loader = ExtensionLoader.getExtensionLoader(Robot.class);
    Robot robot = loader.getAdaptiveExtension();

    ExtensionLoader<Store> loader2 = ExtensionLoader.getExtensionLoader(Store.class);
    Store store = loader2.getAdaptiveExtension();

    // 这是因为确定是这个实现类 ....
    AliBaBaStore baStore = (AliBaBaStore) store;
    System.out.println("baStore.getRobot()==robot = " + (baStore.getRobot() == robot));
}
```

输出结果 : 

```java
baStore.getRobot()==robot = true
```

所以确实是单例 , 维护的很好 

## 4. @Adaptive 注解

```java
@Documented
@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.TYPE, ElementType.METHOD})
public @interface Adaptive {
    String[] value() default {};
}
```

我们看一下官方解释

> ​	Decide which target extension to be injected. The name of the target extension is decided by the parameter passed in the URL , and the parameter names are given by this method.

其实`value()`  的作用就是决定哪个实现类可以被注入进去 , 这个实现类的名字通过参数`URL`决定 , 参数名称有`value()`方法给出 

### 1. 测试一下 ,其实主要决定因素在于 URL

```java
@Test
public void testRobot(){
    ExtensionLoader<Robot> extensionLoader = ExtensionLoader.getExtensionLoader(Robot.class);

    List<Robot> load = extensionLoader.getActivateExtension(URL.valueOf("test://localhost/test?load=bumblebee"), "load");

    for (Robot robot : load) {
        robot.sayHello();
    }
}
```

输出

```java
Hello, I am Bumblebee.
```



### 2. 骚操作 

我们对项目加以改造 ... 

```java
@SPI
public interface Robot {
    @Adaptive({"load"})
    void sayHello(URL url);
}
```

实现类 不变 ,只是参数添加一下就行 

测试类 : 

```java
@Test
public void testAdaptive(){
    ExtensionLoader<Robot> loader = ExtensionLoader.getExtensionLoader(Robot.class);

    Robot extension = loader.getAdaptiveExtension();

    extension.sayHello(URL.valueOf("test://localhost/test?load=bumblebee"));

}
```

输出 : 

```java
Hello, I am Bumblebee.
```

结果还是对的 , 所以 , 这个自动注入蛮厉害的. 

