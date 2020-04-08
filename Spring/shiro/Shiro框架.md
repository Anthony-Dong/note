# Shiro  VS Spring Security VS Oauth2.0

> ​	直接拿来说三块 文章内容很长, 主要是我觉得安全/授权/权限校验/权限认证 只是一个思想罢了. 所以我们来看看这三个框架的做法是什么. 
>
> ​	三者难度层次递进. 本文会涉及到部分代码./ 大量代码.



## Shiro框架

官方链接 :  https://github.com/apache/shiro

Github啥也没有, wiki也没有, 介绍也没有就官网有, 其实这种框架很简单. 我简单的给大家介绍一下. 



首先看一个入门例子. 先不是web项目,主要看他的流程和组件. 

依赖 :  去maven随便找一个就可以了

```xml
<!-- https://mvnrepository.com/artifact/org.apache.shiro/shiro-core -->
<dependency>
    <groupId>org.apache.shiro</groupId>
    <artifactId>shiro-core</artifactId>
    <version>1.4.0</version>
</dependency>
```



测试用例 . 

```java
@Test
public void test() {
    //1. SecurityManager , 这个就是一个安全管理器. 配置你所有的信息
    DefaultSecurityManager manager = new DefaultSecurityManager();

    SimpleAccountRealm realm = new SimpleAccountRealm();
    // name=tom , password=123 , role=admin
    realm.addAccount("tom", "123", "admin");
    // 添加一个角色权限
    manager.setRealm(realm);

    // 2.这个是一个我感觉很low的做法. 他利用这个就设置进去了, 其实为了简化罢了.(因此可能后来的覆盖前面的,多线程共同设置不安全)
    SecurityUtils.setSecurityManager(manager);

    // 3.获取 Subject , 这个就是指的一次会话. 是一个域对象.用ThreadLocal管理.所以单个线程随便拿都是一个对象.
    Subject subject = SecurityUtils.getSubject();

    // 4. 登录
    try {
        subject.login(new UsernamePasswordToken("tom", "456"));
        System.out.println("登录成功");
    } catch (AuthenticationException e) {
        System.out.println("密码错误 : " + e.getMessage());
    }finally{
        subject.logout();
    }
}
```

上诉就是一个最简单的例子 . 

输出: 

```java
密码错误
```

主要就是利用ThreadLocal做的, 很好地控制了上下文. 



全局包含了 `SecurityManager`  和 `Subject`  , 这个就是一个最简单的流程.   其中还有很多. 

或者可以已经过时的方式注入, 需要编写一个简单的INI文档. 

```java
// 1.读取配置文件,写法和Git的config一样.
Ini ini = Ini.fromResourcePath("classpath:shiro.ini");
// 2.传入配置信息
Factory<SecurityManager> factory = new IniSecurityManagerFactory(ini);
// 3.构建一个SecurityManager,这个是单例对象.不管你执行多少次,都是一个.
SecurityManager securityManager = factory.getInstance();
```



整合一下SpringBoot吧. 其实我不推荐.  因为真的没啥用.

```java
<!-- https://mvnrepository.com/artifact/org.apache.shiro/shiro-spring -->
<dependency>
    <groupId>org.apache.shiro</groupId>
    <artifactId>shiro-spring</artifactId>
    <version>1.4.0</version>
</dependency>
```

其实Shiro就是用了Java的Filter接口拓展的. 有兴趣的可以看看去. `org.apache.shiro.web.filter.authc.BasicHttpAuthenticationFilter`



我写了简单的Demo 

```java
@Configuration
public class ShiroConfig {
    @Bean
    public ShiroFilterFactoryBean shiroFilterFactoryBean(SecurityManager securityManager) {
        ShiroFilterFactoryBean filter = new ShiroFilterFactoryBean();
        filter.setSecurityManager(securityManager);
        // 添加一个filter
        Map<String, Filter> map = new HashMap<>(1);
        map.put("filter", new MyFileter());
        filter.setFilters(map);
        // 设置拦截. 表示为啥不能用这个多方便. 一个url - 对应一个filter.
        filter.setFilterChainDefinitionMap(Collections.singletonMap("/*", "filter"));
        return filter;
    }

    /**
     * @param realm Realm
     * @return SecurityManager
     */
    @Bean
    public SecurityManager securityManager(Realm realm) {
        // SecurityManager  核心组件
        DefaultWebSecurityManager manager = new DefaultWebSecurityManager();
        manager.setRealm(realm);
        return manager;
    }

    /**
     * @return Realm
     */
    @Bean
    public Realm realm() {
        // real 核心组件. 
        return new AuthorizingRealm() {
            // 认证
            @Override
            protected AuthenticationInfo doGetAuthenticationInfo(AuthenticationToken token) throws AuthenticationException {
                System.out.println("username : " + token.getPrincipal());
                System.out.println("password : " + new String((char[]) token.getCredentials()));
                // 这个是数据库的密码
                String auth = "1234567";
                // 需要校验的认证信息 ,  第三个参数+第一个参数是真实名字. 没啥用
                return new SimpleAuthenticationInfo(token.getPrincipal(), auth, "");
            }

            // 授权
            @Override
            protected AuthorizationInfo doGetAuthorizationInfo(PrincipalCollection principals) {
                System.out.println("AuthorizationInfo : " + principals.asList());
                return new SimpleAuthorizationInfo();
            }
        };
    }
    static class MyFileter extends BasicHttpAuthenticationFilter {

        @Override
        protected boolean executeLogin(ServletRequest request, ServletResponse response) throws Exception {
            System.out.println("executeLogin");
            return super.executeLogin(request, response);
        }

        @Override
        protected boolean isAccessAllowed(ServletRequest request, ServletResponse response, Object mappedValue) {
            System.out.println("isAccessAllowed : " + mappedValue);
            // 后期解释
            return false;
        }
        @Override
        protected boolean preHandle(ServletRequest request, ServletResponse response) throws Exception {
            System.out.println("preHandle");
            return true;
        }
    }
}
```



我们看到我们注入了一个Filter . 

我们查看日志后发现, 

```java
2020-03-01 14:44:28.315  INFO 4980 --- [ost-startStop-1] o.s.b.w.servlet.ServletRegistrationBean  : Servlet dispatcherServlet mapped to [/]
2020-03-01 14:44:28.321  INFO 4980 --- [ost-startStop-1] o.s.b.w.servlet.FilterRegistrationBean   : Mapping filter: 'characterEncodingFilter' to: [/*]
2020-03-01 14:44:28.322  INFO 4980 --- [ost-startStop-1] o.s.b.w.servlet.FilterRegistrationBean   : Mapping filter: 'hiddenHttpMethodFilter' to: [/*]
2020-03-01 14:44:28.322  INFO 4980 --- [ost-startStop-1] o.s.b.w.servlet.FilterRegistrationBean   : Mapping filter: 'httpPutFormContentFilter' to: [/*]
2020-03-01 14:44:28.322  INFO 4980 --- [ost-startStop-1] o.s.b.w.servlet.FilterRegistrationBean   : Mapping filter: 'requestContextFilter' to: [/*]
2020-03-01 14:44:28.322  INFO 4980 --- [ost-startStop-1] o.s.b.w.servlet.FilterRegistrationBean   : Mapping filter: 'shiroFilterFactoryBean' to: [/*]
```

最后一个就是我们的过滤器. 

所以基本可以确定是过滤器的作用. 

我们跟踪代码. 也就是过滤器. 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-30-33/a3e6fb96-94c8-423f-9254-34a566b8f1bf.png?x-oss-process=style/template01)

结构图很简单, 

`org.apache.shiro.web.servlet.OncePerRequestFilter#doFilter`

```java
public final void doFilter(ServletRequest request, ServletResponse response, FilterChain filterChain)
        throws ServletException, IOException {
    String alreadyFilteredAttributeName = getAlreadyFilteredAttributeName();
    if ( request.getAttribute(alreadyFilteredAttributeName) != null ) {
        log.trace("Filter '{}' already executed.  Proceeding without invoking this filter.", getName());
        filterChain.doFilter(request, response);
    } else //这些地方我们都可以重写.
        if (!isEnabled(request, response) ||
            shouldNotFilter(request) ) {
        log.debug("Filter '{}' is not enabled for the current request.  Proceeding without invoking this filter.",
                getName());
        filterChain.doFilter(request, response);
    } else {
        // Do invoke this filter...
        log.trace("Filter '{}' not yet executed.  Executing now.", getName());
        request.setAttribute(alreadyFilteredAttributeName, Boolean.TRUE);
        try {
            // 执行真正的逻辑.
            doFilterInternal(request, response, filterChain);
        } finally {
            // Once the request has finished, we're done and we don't
            // need to mark as 'already filtered' any more.
            request.removeAttribute(alreadyFilteredAttributeName);
        }
    }
}
```



我们继续跟进 就是一个执行链模式了. 所以我也不想写了. 基本上就是执行链不断的走.

```java
protected void doFilterInternal(ServletRequest servletRequest, ServletResponse servletResponse, final FilterChain chain)
        throws ServletException, IOException {

    Throwable t = null;

    try {
        // 包装一下.
        final ServletRequest request = prepareServletRequest(servletRequest, servletResponse, chain);
        final ServletResponse response = prepareServletResponse(request, servletResponse, chain);

        // 这就是Shrio的核心. 一次会话对象.
        final Subject subject = createSubject(request, response);

        // 核心执行逻辑.
        subject.execute(new Callable() {
            public Object call() throws Exception {
                updateSessionLastAccessTime(request, response);
                // 主要是这个. 执行链. 有兴趣的可以看看. 反正就是走完这个执行链.
                // 最后会走一个默认实现.
                executeChain(request, response, chain);
                return null;
            }
        });
	.........// 不写了. 
}

```





主要逻辑再 `org.apache.shiro.web.servlet.AdviceFilter` 这个实现 

```java
public void doFilterInternal(ServletRequest request, ServletResponse response, FilterChain chain)
        throws ServletException, IOException {

    Exception exception = null;

    try {
        // true
        boolean continueChain = preHandle(request, response);
        if (log.isTraceEnabled()) {
            log.trace("Invoked preHandle method.  Continuing chain?: [" + continueChain + "]");
        }
        // 继续执行调用
        if (continueChain) {
            // 继续执行. 
            executeChain(request, response, chain);
        }
        // 事后通知. 跟Spring一样.
        postHandle(request, response);
        if (log.isTraceEnabled()) {
            log.trace("Successfully invoked postHandle method");
        }
    } catch (Exception e) {
        exception = e;
    } finally {
        cleanup(request, response, exception);
    }
}
```





我们上面Filter重写的方法逻辑  executeChain-> preHandle

```java
protected boolean preHandle(ServletRequest request, ServletResponse response) throws Exception {
    if (this.appliedPaths == null || this.appliedPaths.isEmpty()) {
        if (log.isTraceEnabled()) {
            log.trace("appliedPaths property is null or empty.  This Filter will passthrough immediately.");
        }
        return true;
    }
    for (String path : this.appliedPaths.keySet()) {
        if (pathsMatch(path, request)) {
            Object config = this.appliedPaths.get(path);
            // 继续判断
            return isFilterChainContinued(request, response, path, config);
        }
    }
    return true;
}
```

```java
@SuppressWarnings({"JavaDoc"})
private boolean isFilterChainContinued(ServletRequest request, ServletResponse response,
                                       String path, Object pathConfig) throws Exception {
    	// 这里就会执行前置处理器.
        return onPreHandle(request, response, pathConfig);
    if (log.isTraceEnabled()) {
    }
    return true;
}
```

```java
public boolean onPreHandle(ServletRequest request, ServletResponse response, Object mappedValue) throws Exception {
    // 这个控制权限. isAccessAllowed-> 走我们重写的方法
    // onAccessDenied 走我们的login登录逻辑.这个我们都可以重写的.
    return isAccessAllowed(request, response, mappedValue) || onAccessDenied(request, response, mappedValue);
}
```





`preHandle`   true -> `isAccessAllowed `   -> false->  `onAccessDenied` ->  `executeLogin`  -> `Realm`  -> 	

`return new SimpleAuthenticationInfo("token", "123456", "name");` -> 密码一样OK 继续.





### 总结

不谈web的整合, 基本就是构建一次会话 Subject ,有些人叫角色其实感觉不咋对, 因为传统的web编程都是同步的, 使得单个线程执行. 这个线程生命周期就是一次会话. 默认tomcat是开启300个线程好像. 

而subject 就是入口, 可以进行权限校验. 这个校验就是因为我们的securitymanager.对象的设置的信息. 

而 web 则是采用了Java的拦截器.  不过他的执行链模式值得学习一下. 

## Spring-Security 框架



我直接看的官方的例子,  我很早以前学过一次  https://github.com/spring-projects/spring-boot/blob/v2.1.6.RELEASE/spring-boot-samples/spring-boot-sample-secure/pom.xml 

当时是传统的web 项目, 前后端不分离的.  所以通常是jsp页面.

#### 充当eureka的登陆校验工具.

不过eureka的界面登录 用security做其实不错. 

我先附赠一段代码 , 他其实适合走这种玩意. 

```java
@EnableEurekaServer
@EnableWebSecurity
public class WebSecurityConfig extends WebSecurityConfigurerAdapter {
    @Override
    protected void configure(HttpSecurity http) throws Exception {
        http.csrf().disable();
        super.configure(http);
    }
}
```

配置文件加入两行就行了

```properties
spring.security.user.name=user
spring.security.user.password=password
```

这就是一个拦截. 



快速如么嗯一个把. 

```properties
spring.security.user.name=user
spring.security.user.password=1234
# 权限是,一般是ROLE_USER. 这么判断的.
spring.security.user.roles=USER
```

```java
@RestController
@SpringBootApplication
// 开启Secured.
@EnableGlobalMethodSecurity(securedEnabled = true)
public class SpringSecuritySpringbootApplication {

    public static void main(String[] args) {
        SpringApplication.run(SpringSecuritySpringbootApplication.class);
    }

    // 必须有USER权限.
    @Secured("ROLE_USER")
    @GetMapping("/go")
    public String redirect() {
        return "hello world!";
    }
}
```

这就是一个最简单的例子. 我们发现他的权限也是死的. 不是活的. 





我么测试一下SpringSecurity 的原理 , 官方提供的例子.

```ava
public class SpringSecuritySpringbootApplicationTest {

    @Autowired
    SampleService service;

    @Test
    public void test() {
        UsernamePasswordAuthenticationToken token = new UsernamePasswordAuthenticationToken("user", "N/A",
                AuthorityUtils.commaSeparatedStringToAuthorityList("ROLE_USER"));
        SecurityContextHolder.getContext().setAuthentication(token);
        try {
        // 验证权限.
            System.out.println(this.service.secure());
        } finally {
            SecurityContextHolder.clearContext();
        }
    }
}
```

显然写死了不好.  其实由改进方式就是加入一个配置. 重写`org.springframework.security.config.annotation.web.configuration.WebSecurityConfigurerAdapter`  他的配置信息. 注入`org.springframework.security.core.userdetails.UserDetailsService` 接口. 我随便写了个 . 

```java
public class MyUserDetailsService implements UserDetailsService {

    // 模拟数据库
    private static final Map<String, String> upmap = new HashMap<>();
    private static final Map<String, List<String>> roles = new HashMap<>();
    private static final String admin = "ADMIN";
    private static final String user = "USER";
    static {
        upmap.put("user1", "p1");
        roles.put("user1", Collections.singletonList(user));
        upmap.put("user2", "p2");
        roles.put("user2", Arrays.asList(admin, user));
    }

    @Override
    public UserDetails loadUserByUsername(String username) throws UsernameNotFoundException {
        String pass = upmap.get(username);
        // 用户名不对直接pass掉
        if (pass == null) {
            throw new UsernameNotFoundException("username : " + username);
        }

        // 添加权限
        Collection<GrantedAuthority> croles = new ArrayList<>();
        List<String> list = roles.get(username);
        list.forEach(e -> {
            GrantedAuthority grantedAuthority = new SimpleGrantedAuthority(e);
            croles.add(grantedAuthority);
        });
        return new User(username, pass, croles);
    }
}
```

配置 : 

```java
@EnableWebSecurity
@Configuration
public class SpringSecurityConfig extends WebSecurityConfigurerAdapter {
    @Override
    protected void configure(AuthenticationManagerBuilder auth) throws Exception {
        // 必须配置解码.
        auth.userDetailsService(new MyUserDetailsService())
                .passwordEncoder(new PasswordEncoder() {
                    @Override
                    public String encode(CharSequence rawPassword) {
                        // 解码
                        return (String) rawPassword;
                    }
                    @Override
                    public boolean matches(CharSequence rawPassword, String encodedPassword) {
                        // 字节返回true
                        return true;
                    }
                });
    }
}
```

这里有个BUG 就是权限失效的问题. Debug找不到解决入口. 难受. 主要入口在 `org.springframework.security.access.intercept.aopalliance.MethodSecurityInterceptor` 这里这个就是实现方法级控制的 . 和 `org.springframework.security.access.intercept.AbstractSecurityInterceptor` 这个实现. 

主要问题在`org.springframework.security.access.vote.AffirmativeBased#decide` .  我不知道问题出现在哪. 

















