# Spring-MVC核心思想

> ​	我们都知道Spring-Mvc的搭建环境的环节, 就是注册一个`DispatcherServlet` 的Servlet , 然后servlet-mapping拦截所有的请求 , 此时我们是否先应该学习一下Servlet呢 ?  学习玩Servlet 再看看就很简单了 ........

## 1. Servlet的生命周期

### 1. 简单的Demo

这是我的web.xml配置文件

```xml
  <servlet>
    <servlet-name>myServlet</servlet-name>
    <servlet-class>com.example.web.MyServlet</servlet-class>
    <init-param>
      <param-name>contextConfigLocation</param-name>
      <param-value>classpath*:springMVC.xml</param-value>
    </init-param>
  </servlet>
  <servlet-mapping>
    <servlet-name>myServlet</servlet-name>
    <url-pattern>/my</url-pattern>
  </servlet-mapping>
```

这是我的Servlet类

```java
public class MyServlet extends HttpServlet {
    @Override
    protected void service(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
        System.out.println("service");
    }

    @Override
    public void destroy() {
        System.out.println("destroy");
    }
    @Override
    public void init(ServletConfig config) throws ServletException {
        String contextConfigLocation = config.getInitParameter("contextConfigLocation");
        System.out.println("InitParameter : " + contextConfigLocation);
        System.out.println("init");
    }
}
```

第一次请求输出的结果 : 

```java
InitParameter : classpath*:springMVC.xml
init
service
```

第二次请求输出的结果 : 

```java
service
```

第三次  ..  以后呢 ? 

```java
service
```

当我们把Tomcat服务器关闭呢 ? 

```java
destroy
```

### 2. 通过Demo,我们知道了什么 ? 

#### 1. init(..) 方法

他只会执行一次 , 就是当第一个请求到达或者Servlet容器启动的时候执行 . 

**补充 :** `<load-on-startup>0</load-on-startup>` 这个标签控制着Servlet加载的时机,默认是-1 ,就是需要用户去访问才会初始化 , 当大于等于0时 ,Servlet容器启动的时候就会帮助我们初始化

#### 2. service() 方法

他会在每次请求到达的时候都会去执行 .

#### 3. destroy() 方法

他会在Servlet容器关闭的时候执行 ,也是只会执行一次

#### 4. 总结

通过上诉我们知道了Servlet的生命周期 ,那么我们就开始吧 , 看看`DispatcherServlet`是如何执行的

## 2. DispatcherServlet 的核心

配置项 web.xml 的

```xml
  <servlet>
    <servlet-name>dispatcherServlet</servlet-name>
    <servlet-class>org.springframework.web.servlet.DispatcherServlet</servlet-class>
    <init-param>
      <param-name>contextConfigLocation</param-name>
      <param-value>classpath*:springMVC.xml</param-value>
    </init-param>
  </servlet>
  <servlet-mapping>
    <servlet-name>dispatcherServlet</servlet-name>
    <url-pattern>/</url-pattern>
  </servlet-mapping>
// 没有配置初始化启动是为了方便Debug 后期
```

springMVC.xml 配置

```xml
    <!--开启注解扫描-->
    <context:component-scan base-package="com.example.web">
        <!--只扫描controller注解下的对象-->
        <context:include-filter type="annotation" expression="org.springframework.stereotype.Controller"/>
    </context:component-scan>


    <!--视图解析器-->
    <bean id="internalResourceViewResolver" class="org.springframework.web.servlet.view.InternalResourceViewResolver">
        <property name="prefix" value="/WEB-INF/pages/"></property>
        <property name="suffix" value=".jsp"></property>
    </bean>

    <!--开启MVC注解驱动-->
    <mvc:annotation-driven ></mvc:annotation-driven>
```

Controller 配置 

```java
@Controller
public class TestController {

    @GetMapping("/get")
    public String get(){
        return "hello";
    }
}
```



### 1. 认识DispatcherServlet 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/7f83b9b8-07eb-417d-adbb-636f456ad513.jpg?x-oss-process=style/template01)

首先我们看蓝色部分 , 他继承自 `FrameworkServlet` , 然后FrameworkServlet继承了`HttpServletBean`,然后就到了我们常见的 `HttpServlet`和`GenericServlet` . 

### 1. init() 方法启动过程

首先我们去访问`http://localhost:8080/get` 

首先由于 `HttpServletBean`重写了 `init()`方法 , 第一步就是获取我们` <init-param>`传入的配置信息 . 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/cbf8dea6-aa18-4232-8e4c-c0e6c3c323b0.jpg?x-oss-process=style/template01)

第二步 : 执行 init()中的 `initServletBean()` 方法 ,

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/7f038918-9c33-441b-96b7-1f410faf59f4.jpg?x-oss-process=style/template01)

所以 init执行完毕以后 以及将 ApplicationContext 注册成功了  , 所有的元信息都在里面

### 2. service()方法 执行流程

#### 1. 第一步  执行service() 方法

方法看似很简单 ,实际上包含了全部流程 , 就是进去`service()` 方法

![]( https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/021f91dd-0154-4b5f-ab69-3feb26322c3d.jpg?x-oss-process=style/template01)

#### 2. 第二步  processRequest()方法

就是 `processRequest()` 方法上 , 我测试有BUG不能直接进去这个方法 , 也不懂为啥,我们就先看吧,其主要

首先就是这个`initContextHolders()` 方法, 我们去看源码,发现他的意思就是 : **将LocaleContext实例与当前线程相关联**,类似于一种环境,就是你在中国,中国就是一个环境,这里可以取到 你所有的本地信息 ,可以看看java的`Local`类 

其次就是调用 doService() 方法 ,由于他是抽象类并未实现 所以我们需要找子类 `DispatcherServlet`他的实现方法

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/1fc4501b-3405-4362-92ee-74af97030cc9.jpg?x-oss-process=style/template01)

#### 3. 第三步 调用doService() 方法

主角开始 `DispatcherServlet#doService()` , 

首先就是将我们的 ApplicationContext 注册到 request中 ,方便以后拿到 

然后他使用了一种适配器模式 , 执行了 doDispatch() 方法

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/e47e6440-862a-44d5-9c57-f07bc2c5d87e.jpg?x-oss-process=style/template01)

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/04bf6891-1c3b-48e3-8277-8300571be99a.jpg?x-oss-process=style/template01)

#### 4. 第四步  调用doDispatch()方法

大致流程就是通过 `handlerMappings`获取`HandlerExecutionChain`中的`handler`对象, 然后拿`handler`对象去找到他匹配的`HandlerAdapter`对象 ,

还有一个细节就是`mappedHandler.applyPreHandle(processedRequest, response)`这个方法的执行其实就是执行拦截器`HandlerInterceptor`, 如果拦截了直接就返回了, 没有拦截继续调用`HandlerAdapter`的`handle()`方法去做后续操作.....

`HandlerMapping` 是什么 ? 

> ​	Interface to be implemented by objects that define a mapping between requests and handler objects.
>
> 定义请求和处理程序对象之间的映射关系, 他就一个方法 `HandlerExecutionChain getHandler(HttpServletRequest request) throws Exception;` 获取 HandlerExecutionChain  , 



`HandlerExecutionChain` 是什么 ? 

> ​	Handler execution chain, consisting of handler object and any handler interceptors. Returned by HandlerMapping's HandlerMapping.getHandler method.
>
> 包含了 handler对象和拦截器 , 可以通过 `HandlerMapping`的 `getHandler ()`方法获取



`handler `是什么 ? 

> ​	HandlerMethod 对象 ,就是我们的Controller对象和他的方法组成的一个对象 , 元信息



`HandlerAdapter `是什么 ? 

> ​	MVC framework SPI, allowing parameterization of the core MVC workflow.
>
> MVC框架SPI，允许参数化的核心MVC工作流。 主要方法 `ModelAndView handle(HttpServletRequest request, HttpServletResponse response, Object handler) throws Exception;`  根据方法我们就可以知道 他的作用是什么 ,



所以三者的关系是`  HandlerMapping` 是一个菜单 , 每个请求进来会根据请求,可以找到它对应的`HandlerExecutionChain  `进而拿到`HandlerMethod `对象  , 

当我们拿到 `HandlerMethod` 对象 , 此时做的不是简单的调用 ,Method.invoke() 就完了 .... ,他使用了适配器模式 ,所以我们需要通过 `HandlerMethod` 对象找到他何时的适配器 `HandlerAdapter` ,然后适配器执行他的 handle 方法帮助我们返回 `ModelAndVive` . 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/4f0c6c39-ab9d-4d6d-b319-380587b1a5e1.png?x-oss-process=style/template01)

#### 5. 第五步  handler() 方法 

我们进入`mv = ha.handle(processedRequest, response, mappedHandler.getHandler())` 方法继续执行

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/8f4d5c3d-1259-476c-acc6-55baf413a537.jpg?x-oss-process=style/template01)

这时候我们需要看看什么是 `HandlerMethod` ,官方给的意思是 : **此对象由方法和bean组成的处理程序方法的信息 ,提供对方法参数、方法返回值、方法注释等的方便访问。就是我们的TestController对象和get方法 封装了起来,类似于元信息**.

#### 6. 第六步 继续跟进

执行 `handleInternal(request, response, (HandlerMethod) handler)`方法 ,

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/1d37604c-c91d-47c3-9d6e-cc1293d633ca.jpg?x-oss-process=style/template01)

#### 7. 第七步  继续跟进

执行 `mav = invokeHandlerMethod(request, response, handlerMethod);` 方法我们继续跟进

代码太长 我就截取一部分 :通过 HandlerMethod调用 返回 ModelAndVive

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/024062cd-ae6f-4aa8-a6bd-940035d17e05.jpg?x-oss-process=style/template01)

`invocableMethod.invokeAndHandle(webRequest, mavContainer);`方法执行 : 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/3fdcb64c-021d-49c2-8fff-400491e04675.jpg?x-oss-process=style/template01)

`Object returnValue = invokeForRequest(webRequest, mavContainer, providedArgs);` 方法执行 : 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/9f391b0d-d375-4eb5-b220-438256a43570.jpg?x-oss-process=style/template01)

#### 8. 第八步  终于执行真正的逻辑了

`Object returnValue = doInvoke(args);` 终于完成了 ............. 执行了我们的写的方法

![]( https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/02cc31c0-67f8-4b86-86d2-935ed11f3ec0.jpg?x-oss-process=style/template01)

此时就已经 讲我们写的:

```java
    @GetMapping("/get")
    public String get(){
        return "hello";
    }
```

返回了 , 我们可以看看这个对象是什么 :  确实使我们的返回的结果 : 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/7c9cb801-406a-4496-96e7-ab3f20f05d5b.jpg?x-oss-process=style/template01)

此后的过程就是一个 逆向过程往回返了 ....

#### 9. 第九步  根据返回的vive 渲染视图

到了我们的 `DispatcherServlet` 的`mv = ha.handle(processedRequest, response, mappedHandler.getHandler());` 这个方法执行完毕了 ............................... 然后开始渲染  ... ,执行的是 `processDispatchResult(processedRequest, response, mappedHandler, mv, dispatchException);`

![]( https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/edf083c2-7800-44ae-ace0-7486308b623d.jpg?x-oss-process=style/template01)

渲染视图 , 调用其他方法 继续追

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/4f424a62-dadf-410d-bbc6-26893210a830.jpg?x-oss-process=style/template01)

继续追 : 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/84e88eeb-3fc5-4bfc-8cc4-34829b09b1d2.jpg?x-oss-process=style/template01)

就到了我们配置的视图解析器 , 渲染完成 .

#### 10. 第十步  结束processRequest() 方法,发布事件

然后最后就到了processRequest() 方法的终点 ,发布了一个事件 , 就完成了

![]( https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-15/6916ae0a-41b6-4f6c-bb7e-b4752d66874b.jpg?x-oss-process=style/template01)

这个最后的事件发布有点骚哇 ,发布以后并无卵用 ,spring框架 并没有监听该事件 .... ,此时需要我们自己去监听 , 所以下面我自己去实现了一个简单的监听流程 .... 



### 3. destroy() 方法

```java
	@Override
	public void destroy() {
		getServletContext().log("Destroying Spring FrameworkServlet '" + getServletName() + "'");
		// Only call close() on WebApplicationContext if locally managed...
		if (this.webApplicationContext instanceof ConfigurableApplicationContext && !this.webApplicationContextInjected) {
			((ConfigurableApplicationContext) this.webApplicationContext).close();
		}
	}
```

> ​	Close the WebApplicationContext of this servlet.

就是将 ApplicationContext关闭了 ..............



## 3. ServletRequestHandledEvent 使用

写一个 Listener 实现自 ApplicationListener , 

```java
@Component
public class ServletRequestHandledEventListener implements ApplicationListener<ServletRequestHandledEvent> {

    /**
     * Handle an application event.
     *
     * @param event the event to respond to
     */
    @Override
    public void onApplicationEvent(ServletRequestHandledEvent event) {

        System.out.println(Thread.currentThread().getName()+" 收到事件 : " + event);
    }
}
```

配置一个spring的上下文配置 applicationContext.xml

```xml
<?xml version="1.0" encoding="UTF-8"?>
<beans xmlns="http://www.springframework.org/schema/beans"
       xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
       xmlns:context="http://www.springframework.org/schema/context"
       xsi:schemaLocation="http://www.springframework.org/schema/beans
        http://www.springframework.org/schema/beans/spring-beans.xsd
        http://www.springframework.org/schema/context
        http://www.springframework.org/schema/context/spring-context.xsd">


    <!--开启扫描-->
    <context:component-scan base-package="com.example.listener"></context:component-scan>

</beans>
```

注入到 环境中web.xml中 , 监听器实现了ServletContextListener,所以当容器启动会自动加载

```xml
  <!--监听器 , 加载contextConfigLocation属性的配置文件-->
  <listener>
    <listener-class>org.springframework.web.context.ContextLoaderListener</listener-class>
  </listener>
  <!--设施spring配置文件的文件路径,不能改名字-->
  <context-param>
    <param-name>contextConfigLocation</param-name>
    <param-value>classpath:applicationContext.xml</param-value>
  </context-param>
```



所以运行结果 ,当我们访问  `http://localhost:8080/get` 链接时 ..

```java
http-nio-8080-exec-6 收到事件 : ServletRequestHandledEvent: url=[/get]; client=[0:0:0:0:0:0:0:1]; method=[GET]; servlet=[dispatcherServlet]; session=[B2595F9D400922D5A4EC7FA55162349C]; user=[null]; time=[217ms]; status=[OK]
http-nio-8080-exec-9 收到事件 : ServletRequestHandledEvent: url=[/get]; client=[0:0:0:0:0:0:0:1]; method=[GET]; servlet=[dispatcherServlet]; session=[B2595F9D400922D5A4EC7FA55162349C]; user=[null]; time=[4ms]; status=[OK]
```

