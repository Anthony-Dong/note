# Servlet 学习(全文贯穿spring-mvc,spring boot)

## 1. Servlet 

> ​	生命周期 :
>
> - init  
> - service (执行 请求)
> - destroy

- 当我们使用 : (servlet 3.0 加入注解开发)

```java
@WebServlet(
name="doServlet",
value={"/do"},
loadOnStartup=0,
initParams={@WebInitParam(name = "para1",value = "para1"),@WebInitParam(name = "para2",value = "para2")}
)

与之对应的 xml配置 是 
  <servlet>
    <servlet-name>doServlet</servlet-name>
    <servlet-class>com.web.DoServlet</servlet-class>
    <init-param>
      <param-name>para1</param-name>
      <param-value>name2</param-value>
    </init-param>
    <init-param>
      <param-name>para2</param-name>
      <param-value>name2</param-value>
    </init-param>
    <load-on-startup>1</load-on-startup>
  </servlet>
  <servlet-mapping>
    <servlet-name>doServlet</servlet-name>
    <url-pattern>/do</url-pattern>
  </servlet-mapping>
  
 

```

- 当我们去看 springmvc的 配置时: 我们便可以看懂了 为什么 ?

```java
	 <!--前端控制器--> 
	<servlet>
    <servlet-name>dispatcherServlet</servlet-name>
    <servlet-class>org.springframework.web.servlet.DispatcherServlet</servlet-class>
    <init-param>
      <param-name>contextConfigLocation</param-name>
      <param-value>classpath:/config/springMVC.xml</param-value>
    </init-param>
    <load-on-startup>1</load-on-startup>
  </servlet>
  <servlet-mapping>
    <servlet-name>dispatcherServlet</servlet-name>
    <url-pattern>/</url-pattern>
  </servlet-mapping>
```

- spring boot 中如何注入一个 servlet (这个比较坑爹,为啥我只能注入一个servlet呢 ,求解 )

```java
    @Bean
    public ServletRegistrationBean<HttpServlet> servletServletRegistrationBean(){
        ServletRegistrationBean<HttpServlet> registrationBean = new 			ServletRegistrationBean<>(new MyServlet(), "/hello");
        return registrationBean;
    }
```



### 异步的 Servlet 以及异步的 listener 

​	下面的注释已经够详细了我就不过多解释了 , servlet 3.0 引入的新功能,需要tomcat7 以上支持, 他可以异步处理请求,缓解 请求处理线程的压力,他会调用新的线程执行任务 .

​	其中 spring的 webflux 也是应用了 这个玩意,不过他更多的是使用了 reactor框架

```java

1. 需要加入注解 asyncSupported = true 开启支持异步处理

@WebServlet(asyncSupported = true,urlPatterns = {"/index"},name = "asyncServlet")
public class AsyncServlet extends HttpServlet {

    @Override
    protected void doGet(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {

        // 如果不去执行  req.startAsync(req,resp); 此时就算你开启了,也是返回false

        /**
         *      * <p>This method returns <tt>false</tt> if this request was
         *      * put into asynchronous mode, but has since been dispatched using
         *      * one of the {@link AsyncContext#dispatch} methods or released
         *      * from asynchronous mode via a call to {@link AsyncContext#complete}.
         */
        这里还没有开启异步处理
        boolean asyncStarted = req.isAsyncStarted();
		
        // 开启 异步处理
        final AsyncContext asyncContext = req.startAsync(req,resp);

        // 添加 listener 需要实现 AsyncListener 接口
        asyncContext.addListener(new AsyncListener());
		
        req.setAttribute("thread1", Thread.currentThread().getName());


        //Sets the timeout (in milliseconds) for this AsyncContext.  显然是 ms
        asyncContext.setTimeout(5000);

        // 启动异步任务 
        asyncContext.start(new Runnable() {
            @Override
            public void run() {
                try {
                    Thread.sleep(300);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                req.setAttribute("thread2", Thread.currentThread().getName());
                asyncContext.dispatch("/WEB-INF/hello.jsp");
                // dispatch 和 complete 只能二选一 ,处理请求完成,需要结束
               // asyncContext.complete();
            }

        });
    }

    @Override
    protected void doPost(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
        super.doPost(req, resp);
    }
}


不需要注册到listener中,需要注册到 异步的servlet中 
public class AsyncListener implements javax.servlet.AsyncListener {


    @Override
    public void onComplete(AsyncEvent event) throws IOException {
        System.out.println("onComplete");
    }

    @Override
    public void onTimeout(AsyncEvent event) throws IOException {
        System.out.println("onTimeout");
    }

    @Override
    public void onError(AsyncEvent event) throws IOException {
        System.out.println("onError");
    }

    @Override
    public void onStartAsync(AsyncEvent event) throws IOException {
        System.out.println("onStartAsync");
    }
}

```



## 2. Filter

创建一个 filter 对象

```java
@WebFilter(urlPatterns={"/*"})
public class MyFilter implements Filter {}

	或者

  <filter>
    <filter-name>myFilter</filter-name>
    <filter-class>com.web.MyFilter</filter-class>
  </filter>
  <filter-mapping>
    <filter-name>myFilter</filter-name>
    <url-pattern>/*</url-pattern>
  </filter-mapping>
  
  xml配置会出现执行顺序的问题,配置在前面的先执行,后面的后执行, 而 filter注解版 么有,是随机的
  
```

spring-mvc中的使用 :

```java
    <!--制作过滤器 解决中文乱码的问题-->
    <filter>
        <filter-name>characterEncodingFilter</filter-name>
        <filter-class>org.springframework.web.filter.CharacterEncodingFilter</filter-class>
        <init-param>
            <param-name>encoding</param-name>
            <param-value>utf-8</param-value>
        </init-param>
    </filter>
    <filter-mapping>
        <filter-name>characterEncodingFilter</filter-name>
        <url-pattern>/*</url-pattern>
    </filter-mapping>
```



spring-boot中注入 一个 filter

```java
    @Bean
    public FilterRegistrationBean<HttpFilter> filterFilterRegistrationBean(){
        FilterRegistrationBean<HttpFilter> registrationBean = new FilterRegistrationBean<>();
        registrationBean.setFilter(new MyFilter());
        registrationBean.setUrlPatterns(Arrays.asList("/*"));
        return registrationBean;
    }



	或者直接来个给力的
	@Component
	@WebFilter(filterName="myFilter",value={"/*"})
	public class MyFilter extends HttpFilter {}
```



## 3. spring-mvc 拦截器 (interceptor)

​	-	**他和 filter的关系,区别是什么**

- filter是属于 servlet规范的,只能用于web程序中,而 interceptor是 spring规范的,可以用于spring框架内部
- **interceptor 归于spring内部管理 ,所以他只能拦截spring管理的的请求(@Controller注解修饰的bean),**
- **而 filter 他可以过滤所有请求 ,包括spring管理的请求**
- filter只能在请求前执行,而interceptor功能更强大,弹性大



  --  **实现interceptor的两种方式:**

- 实现 HandlerInterceptor 接口

```java
public class MyInterceptor implements HandlerInterceptor {

    @Override
    public boolean preHandle(HttpServletRequest request, HttpServletResponse response, Object handler) throws Exception {

        return false;
    }

    @Override
    public void postHandle(HttpServletRequest request, HttpServletResponse response, Object handler, ModelAndView modelAndView) throws Exception {

    }

    @Override
    public void afterCompletion(HttpServletRequest request, HttpServletResponse response, Object handler, Exception ex) throws Exception {

    }
}
```

- 实现 WebRequestInterceptor 接口

```java
public class MyInterceptor02 implements WebRequestInterceptor {


    @Override
    public void preHandle(WebRequest request) throws Exception {

    }

    @Override
    public void postHandle(WebRequest request, ModelMap model) throws Exception {
			
    }

    @Override
    public void afterCompletion(WebRequest request, Exception ex) throws Exception {

    }
}


注册到 spring-MVC中

    <mvc:interceptors>
        <mvc:interceptor>
            <mvc:mapping path="/user/login/*"/>
            <mvc:exclude-mapping path="/user/login/do" />
            <bean class="com.mvc.MyInterceptor"></bean>
        </mvc:interceptor>
    </mvc:interceptors>
```



- **HandlerInterceptor与WebRequestInterceptor的异同**

  >1. WebRequestInterceptor的入参WebRequest是包装了HttpServletRequest 和HttpServletResponse的，通过WebRequest获取Request中的信息更简便。
  >2. WebRequestInterceptor的preHandle是没有返回值的，说明该方法中的逻辑并不影响后续的方法执行，所以这个接口实现就是为了获取Request中的信息，或者预设一些参数供后续流程使用。
  >3. HandlerInterceptor的功能更强大也更基础，可以在preHandle方法中就直接拒绝请求进入controller方法。



**springboot 中 注入 interceptor (这里不能放入 实现 WebRequestInterceptor 接口的 拦截器)**

```java
@Configuration
public class InterceptorConfig extends WebMvcConfigurerAdapter {

    @Override
    public void addInterceptors(InterceptorRegistry registry) {
        registry.addInterceptor(new MyInterceptor()).addPathPatterns("/**");
        super.addInterceptors(registry);
    }
}

```





## 3.Listener

**监听器的实现类有哪些? **

- javax.servlet.AsyncListener - 如果在添加了侦听器的ServletRequest上启动的异步操作已完成，超时或导致错误，将会通知侦听器。
- javax.servlet.ServletContextListener - 用于接收关于ServletContext生命周期更改的通知事件的接口。
- javax.servlet.ServletContextAttributeListener - 接收关于ServletContext属性更改的通知事件的接口。
- javax.servlet.ServletRequestListener - 用于接收关于进入和超出Web应用程序范围的请求的通知事件的接口。
- javax.servlet.ServletRequestAttributeListener - 接收关于ServletRequest属性更改的通知事件的接口。
- javax.servlet.http.HttpSessionListener - 接收关于HttpSession生命周期更改的通知事件的接口。
- javax.servlet.http.HttpSessionBindingListener - 使对象从会话绑定到绑定或从其绑定时被通知。
- javax.servlet.http.HttpSessionAttributeListener - 用于接收关于HttpSession属性更改的通知事件的接口。
- javax.servlet.http.HttpSessionActivationListener - 绑定到会话的对象可能会侦听容器事件，通知他们会话将被钝化，该会话将被激活。需要在VM或持久化会话之间迁移会话的容器来通知绑定到HttpSessionActivationListener的会话的所有属性。



**作用 :**

- 第一种是监听应用启动和关闭，需要实现ServletContextListener接口；
- 第二种是监听session的创建与销毁；属性的新增、移除和更改，需要实现HttpSessionListener和HttpSessionAttributeListener接口。

**实现一个 filter** 

```java
public class MyListener implements ServletContextListener {

    @Override
    public void contextInitialized(ServletContextEvent sce) {
        System.out.println("服务器启动");
    }

    @Override
    public void contextDestroyed(ServletContextEvent sce) {
        System.out.println("服务器销毁");

    }
}

```



spring-mvc中使用 的listener (ContextLoaderListener)

```java

Create a new {@code ContextLoaderListener} that will create a web application context based on the "contextClass" and "contextConfigLocation" servlet context-params. See {@link ContextLoader} superclass documentation for details on default values for each. 

class ContextLoader {
    ....
    public static final String CONFIG_LOCATION_PARAM = "contextConfigLocation";
    ...
    protected void configureAndRefreshWebApplicationContext(ConfigurableWebApplicationContext wac, ServletContext sc){
        ....
         String configLocationParam = sc.getInitParameter(CONFIG_LOCATION_PARAM);
        ....
    }    	
   
}

   

<listener>
        <listener-class>org.springframework.web.context.ContextLoaderListener</listener-class>
    </listener>
    <!--设施spring配置文件的文件路径-->
    <context-param>
        <param-name>contextConfigLocation</param-name>
        <param-value>classpath:applicationContext.xml</param-value>
    </context-param>
```



我们可以发现 spring-mvc 中 其实就是一个 简单的 servlet 工程, 只是帮助我们做了许多事情 , 在这里我推荐一下我手写springmvc的一个 小demo的[链接](https://github.com/Anthony-Dong/javapractice/tree/master/javaee-mvc)









## 4. HttpServletRequestWrapper 很强大

作用 :

- 他实现了 **HttpServletRequest**  所以我们可以过滤 request中 的请求参数 ,
- 他可以对 **HttpServletRequest**   进行进一步的加工 
- 他属于 一个 包装类
- **HttpServletResponseWrapper** 都一样
- 如下 :

```java
public class MyWrapper extends HttpServletRequestWrapper {

    public MyWrapper(HttpServletRequest request) {
        super(request);
    }
    
    @Override
    public String getParameter(String name) {
        System.out.println(name);
        System.out.println("执行了");
        String value = super.getParameter(name);
        if (null == value || value.equals("")) {
            value = "hello";
        }
        return value;
    }
}

然后注册到 filter 中 

@Override
public void doFilter(ServletRequest request, ServletResponse response, FilterChain chain) throws IOException, ServletException {
        request = new MyWrapper((HttpServletRequest) request);
        System.out.println("过滤器");
        chain.doFilter(request, response);
    }
```

