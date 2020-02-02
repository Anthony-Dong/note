# 重新认识Servlet

对于[Servlet 讲的文章](https://www.ibm.com/developerworks/cn/java/j-lo-servlet/index.html)比较好的是 https://www.ibm.com/developerworks/cn/java/j-lo-servlet/index.html , 大家可以看看这篇文章 . 

对于[Servlet深入理解](https://blog.csdn.net/qq_25933249/article/details/94172473)可以看看 : https://blog.csdn.net/qq_25933249/article/details/94172473

很明确 , Servlet他其实由两部分 , 第一部分就Servlet容器, 第二部分就是我们编写的Servlet接口. 

本文以SpringBoot进行阐述 . 

## Servlet

注册一个 Servlet  ,这个注册机制其实设计到tomcat的内部实现`org.springframework.boot.web.embedded.tomcat.TomcatStarter#onStartup` 这里进行注册. 

```java
@Bean
public ServletRegistrationBean<Servlet> servletRegistrationBean() {
    ServletRegistrationBean<Servlet> registrationBean = new ServletRegistrationBean<>();
    registrationBean.addUrlMappings("/get");
    registrationBean.setName("servlet 1");
    registrationBean.addInitParameter("port", "8080");
    registrationBean.setOrder(Integer.MAX_VALUE);
    registrationBean.setServlet(new MyServlet());
    return registrationBean;
}

public static class MyServlet implements Servlet {
    @Override
    public void init(ServletConfig config) throws ServletException {
        String conf = config.getInitParameter("port");
        System.out.printf("init config  k : %s, v: %s.\n", "port", conf);
    }

    @Override
    public ServletConfig getServletConfig() {
        return null;
    }

    @Override
    public void service(ServletRequest req, ServletResponse res) throws ServletException, IOException {
        res.getWriter().write("HELLO WORLD");
    }

    @Override
    public String getServletInfo() {
        return null;
    }

    @Override
    public void destroy() {
        System.out.println("destroy ");
    }
}
```



Servlet有 5个方法, `init `  ,  `getServletConfig()` , `service` , `destroy` `getServletInfo` 之类的. 

init 和 destroy 只会在服务初始化和销毁执行一次.  

service 方法则是在每次服务都会被调用的 . 

SpringBoot中Servlet注册是在 `org.springframework.boot.web.servlet.RegistrationBean#onStartup`  , 其实对于`org.springframework.web.servlet.DispatcherServlet` 也是在这里注册的.  这个是Spring基于Tomcat的回调机制实现的一种 注入方式. 

```java
@Override
public final void onStartup(ServletContext servletContext) throws ServletException {
    String description = getDescription();
    if (!isEnabled()) {
        logger.info(StringUtils.capitalize(description)
                + " was not registered (disabled)");
        return;
    }
    // 注册 , 其实
    register(description, servletContext);
}
```



## Session 与 Cookie

Session 与 Cookie 的作用都是为了保持访问用户与后端服务器的交互状态。

我们以简单的例子 , 看看Session和Cookie是如何运作的.  (测试前清空cookie)

第一我们不关闭浏览的cookie . 

```java
@RestController
public class Test {

    @GetMapping("/test")
    public Object get(HttpServletRequest request, HttpServletResponse response) {
        System.out.println("cookie" + " : " + request.getHeader("cookie"));
        Cookie cookie = new Cookie("time", "" + System.currentTimeMillis());
        response.addCookie(cookie);
        HttpSession session = request.getSession();
        Object user = session.getAttribute("user");
        if (user == null) {
           session.setAttribute("user", UUID.randomUUID().toString());
        }
        return user;
    }
}
```



第一次请求响应是 : 

```java
HTTP/1.1 200
Set-Cookie: time=1580539709072
Set-Cookie: JSESSIONID=AE10A3E001E312061E071234ACDBA36A; Path=/; HttpOnly
Content-Length: 0
Date: Sat, 01 Feb 2020 06:48:29 GMT
```

我们发现除了我们自己定义的还有一个 `JSESSIONID=AE10A3E001E312061E071234ACDBA36A`  这个是什么 ? 

我们发现我们本地多了俩 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/b826bc1a-24da-44c1-a44b-3b7be2222a43.png?x-oss-process=style/template01)

这个就是我们存入的cookie . 所以 session其实就是cookie的表示 . 作为用户的key , **他的生命周期只有一次会话**.  也就是我们关闭浏览器, 这个cookie就会被删除.  这个是Session的默认实现 . 他不允许在浏览器保存 . 

Session可以设置 存活时间. 

```java
HttpSession session = request.getSession();
// 设置为10S, 当用户10S未请求客户端的时候, 会自动认为这个session无效
session.setMaxInactiveInterval(10);
```



而且如果我们不关闭浏览器, 那么每次访问响应给我们的永远是不变的 `JSESSIONID`



所以我们看一下流程其实就是 . 

当用户发送一个请求给服务器时,  用户会携带cookie去访问服务器. 这个是浏览器默认实现的. 当服务器拿到cookie发现Cookie中没有`JSESSIONID`  , 会给用户创建一个 `JSESSIONID` 的cookie . 这个cookie在浏览器保存最长时间只能是一次会话(就是浏览器打开关闭代表一次会话).   所以用户可以在浏览器未关闭之前一直拿着这个 `JSESSIONID`作为身份认证.   当浏览器关闭后这个cookie也会被删除. 此时下次打开浏览器再请求又循环到开始了.  

其实这里有一个BUG , 就是我可以伪装用户的`JSESSIONID` 访问浏览器 . 这样子很能出现危险 . 所以不要在 session中存入用户认证信息. 这样子会很危险. 

![]( https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/8d8dda8b-b87c-4dcb-9b96-249963c1298a.png?x-oss-process=style/template01)



根据这个图. 我们可以发现我们很轻松的伪装信息.  所以不要在 Session中存入敏感信息或者认证信息. 



但是当我们把Cookie禁用了 会怎么办呢 ? 

那么 浏览器的Cookie失效了, 那么代表Session也失效了(因为Session也是基于Cookie). .最简单的测试就是

`curl  http://localhost:8080/test`  会发现永远返回空 . 

但是我们要记住 , 还有一种方式可以传递 cookie. 那就是 url传递. 

如何传递呢 .  `http://localhost:8080/test;use=493B013E8DC852A7BFDE40DCDFAD06B5`  

比如说我们访问 `http://localhost:8080/test` 将 cookie写入到服务器, 需要 `http://localhost:8080/test;auth=D92A968FA16D00F511F5B8EECB8733A6` 这么写  , 但是问题你咋获取这个SessionID , 那么就需要参数返回了.       

springboot开启只需要

```properties
# 设置session模式 , 运行时是不能修改的. 
server.servlet.session.tracking-modes=url

# 设置session 的name
server.servlet.session.cookie.name=auth
```



关于Session的具体实现问题 , 可以看看[这篇文章](https://www.cnblogs.com/chenpi/p/5434537.html)  https://www.cnblogs.com/chenpi/p/5434537.html



## Listener  监听器

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/d538de33-a5d3-4cc1-832e-e219ded80173.png?x-oss-process=style/template01)

其实 `ServletContextListener` 这个也挺有用的 . 

我们来使用 `HttpSessionListener` 测试一下

```java
// 注册Listener
@Bean
public ServletListenerRegistrationBean<HttpSessionListener> sessionListenerServletListenerRegistrationBean() {
    ServletListenerRegistrationBean<HttpSessionListener> registrationBean = new ServletListenerRegistrationBean<>();
    registrationBean.setListener(new MyHttpSessionListener());
    return registrationBean;
}

// 这里是触发事件
public static class MyHttpSessionListener implements HttpSessionListener {
    @Override
    public void sessionCreated(HttpSessionEvent se) {
        System.out.println("创建 session 成功");
    }

    @Override
    public void sessionDestroyed(HttpSessionEvent se) {
        System.out.println("销毁 session 成功");
    }
}
```

可以监听session的创建和销毁.  

Session好像默认存活时间是20mine . 



## Filter  过滤器

类似于Spring的 `org.springframework.web.servlet.HandlerInterceptor`  , 只不过 Filer是事情通知. 不过他要比 `HandlerInterceptor` 早.

其中 `chain.doFilter(request, response);` 主要是用来放行的 . 

```java
@Bean
public FilterRegistrationBean<Filter> filterRegistrationBean() {

    FilterRegistrationBean<Filter> registrationBean = new FilterRegistrationBean<>();

    registrationBean.setFilter(new Filter() {
        @Override
        public void init(FilterConfig filterConfig) throws ServletException {
			//
        }

        @Override
        public void doFilter(ServletRequest request, ServletResponse response, FilterChain chain) throws IOException, ServletException {
            System.out.println("doFilter");
            chain.doFilter(request, response);
        }

        @Override
        public void destroy() {
			// 
        }
    });

    registrationBean.setName("filter 1");

    // 拦截所有请求
    registrationBean.addUrlPatterns("/*");
    return registrationBean;
}
```



## ServletContext

SpringBoot注入可以使用 , 可以获取ServletContext  , 但是我感觉没啥用, 毕竟现在是前后端分离

```java
@Configuration
public class Config implements ServletContextAware{

    public ServletContext servletContext;

    @Override
    public void setServletContext(ServletContext servletContext) {
        this.servletContext = servletContext;
    }
}
```

