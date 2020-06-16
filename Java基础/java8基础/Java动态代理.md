# Java - 动态代理实现原理以及ASM技术

> ​	Java的动态代理是基于字节码技术实现的, 主要是运行时生成类字节码, 然后加载, 主要难点就是如何生成这个字节码, 运行时自动给我们去写



Java的伟大之处就是在于动态代理 ,走向后期的框架, AOP实现都是基于动态代理的, 事务哇,基本都是这玩意 . 



## 1. Proxy

> Proxy provides static methods for creating dynamic proxy classes and instances, and it is also the superclass of all dynamic proxy classes created by those methods.



> ​	Proxy 是Java动态代理的类的父类, 基本动态代理就是围绕着他来的

怎么来实现一个动态代理的类呢 , 有两种方法 ,都是Proxy里的API提供的

### 1. 构造方法实例化

1) 我们需要代理实现的接口

```java
public interface EchoService {
    String echo(String name);
}
```

2) 生成代理类

就是下面这行代码 , 调用 `java.lang.reflect.Proxy#getProxyClass`此方法便可以生成一个代理类, 获取他的类对象 . 

```java
Class<?> proxyClass = Proxy.getProxyClass(Demo.class.getClassLoader(), EchoService.class);
```

2) 实例化对象

我们知道实例化对象无非是 `Constructor.newInstance()` 么 , 但是我们不知道我们的构造方法是啥哇, 好难哇 , Java中规定如果你实现了一个构造方法,不会帮你生成无参的构造方法, 此时我们就需要知道这个参数是啥 , 

这里不卖关子了, 上面不是说`Proxy`是多有代理对象的父类么 ,所以他绝对继承他的构造方法哇. 此时一看, 奥原来如此,竟然是`InvocationHandler` 对象哇 . 

```java
protected Proxy(InvocationHandler h) {
    Objects.requireNonNull(h);
    this.h = h;
}
```

3) 实例化一个 `InvocationHandler` 对象

```java
/**
 * proxy 代理对象, 是我们生成的代理类的实例化对象,比如动态生成了`Proxy$1`类,他就是这个类的实例化对象
 * method 当前调用的方法对象
 * args 当前方法的参数
 * @return Object 方法的返回值
 */
InvocationHandler handler = new InvocationHandler() {
    @Override
    public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {

        // 1. 如果方法是继承自父类Object的方法, 我们就交给代理对象实现
        if (method.getDeclaringClass() == Object.class) {
            return method.invoke(this, args);
        }

        // 2. 如果不是呢, 我们就自己判断执行
        if (method.getName().equals("echo")) {
            return "Hello Proxy";
        }

        // 3. 如果不符合我们的意思, 我们就返回空值
        return null;
    }
};
```

4) 实例化终于到了

```java
EchoService service = (EchoService) proxyClass.getConstructor(InvocationHandler.class).newInstance(handler);
```

此时就拿到了我们的实例化对象

测试一下

```java
System.out.println("toString : "+service);

System.out.println("echo : "+service.echo("name"));
```

输出

```java
toString : com.java.proxy.Demo$1@682a0b20
echo : Hello Proxy
```



### 2. 静态方法

这个依赖于 `java.lang.reflect.Proxy#newProxyInstance` 这个方法

```java
EchoService echoService = (EchoService) Proxy.newProxyInstance(Demo.class.getClassLoader(), new Class[]{EchoService.class}, handler);
```

此时就直接实现了 , 其实跟上面原理一样 ,只不过他封装了一个方法罢了

```java
public static Object newProxyInstance(ClassLoader loader,
                                      Class<?>[] interfaces,
                                      InvocationHandler h)
    throws IllegalArgumentException
{
    final Class<?>[] intfs = interfaces.clone();

	// 这个就是获取代理类 , 为JNI接口
    Class<?> cl = getProxyClass0(loader, intfs);

   
    try {

		// 获取构造方法
        final Constructor<?> cons = cl.getConstructor(constructorParams);
        final InvocationHandler ih = h;
       
        // 构造方法实例化对象, 返回
        return cons.newInstance(new Object[]{h});
    } catch (IllegalAccessException|InstantiationException e) {
       ....
    } 
}

```

所以万变不离其宗 , 



## 2. Proxy 原理

其实就是生成了一个代理类 , 这个`代理类`是继承了 `Proxy` 类 , 同时实现了我们的代理接口 `EchoService` , 所以很是方便 , 

但是我们知道利用这种方式实现效率并不高 , 为什么呢 ,因为所有逻辑都是基于`InvocationHandler.invoke()`. 实现的 , 同时层层封装效率也低 . 

通过`javap -p` 我们发现确实如此 

```java
public final class $Proxy0 extends java.lang.reflect.Proxy {
  private static java.lang.reflect.Method m1;
  private static java.lang.reflect.Method m2;
  private static java.lang.reflect.Method m0;
  public $Proxy0(java.lang.reflect.InvocationHandler) throws ;
  public final boolean equals(java.lang.Object) throws ;
  public final java.lang.String toString() throws ;
  public final int hashCode() throws ;
  static {} throws ;
}
```

我找了一张比较清晰的图, 对于我们这些想学这玩意的人来说 , 其实这些都并不难的. 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-01-06/6baee407-f057-494c-90b4-4fe162c13e3c.jpg?x-oss-process=style/template01)

## 3. CGLIB

> ​	`CGLIB`其实是基于 `ASM` 实现的 , `ASM`框架可以直接生成一个字节码文件(类) 在运行时 , 所以他就是利用这个实现的, 
>
> ​	对于普通的开发者,在不了解Java字节码规范的情况下基本不可能会使用`ASM` 框架, 所以`CGLIB` 做了个封装, 简单轻巧, 
>
> ​	

我们来说说他的核心思想 , 基本就是基于Java的继承来实现的 , 所以我们可以对他进行拓展 ,  多以代理对象不能被`final`修饰

核心对象就是 `Enhancer` ,中文意思就是增强剂 , 增强字节码, 进行完成 . 

```java
public  class SampleClass {
    public void test() {
        System.out.println("hello world");
    }

    public static void main(String[] args) {
        
        Enhancer enhancer = new Enhancer();
        
        enhancer.setSuperclass(SampleClass.class);
        
        enhancer.setCallback(new MethodInterceptor() {
            @Override
            public Object intercept(Object obj, Method method, Object[] args, MethodProxy proxy) throws Throwable {
                System.out.println("before method run...");
                Object result = proxy.invokeSuper(obj, args);
                System.out.println("after method run...");
                return result;
            }
        });
        
        
        SampleClass sample = (SampleClass) enhancer.create();
        sample.test();
    }
}
```

```java
before method run...
hello world
after method run...
```





```xml
<dependency>
    <groupId>cglib</groupId>
    <artifactId>cglib</artifactId>
    <version>3.2.10</version>
</dependency>
```

