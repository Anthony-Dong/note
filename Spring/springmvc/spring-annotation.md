# spring 常用注解

> ​	https://docs.spring.io/spring/docs/current/spring-framework-reference/web.html
>
> 官方文档



### 通用注解

```java
@Component

@Configuration
```



### dao层

```java
@Repository
```



### service 层

```java
@Service
```



### map

```java
1.  自定义返回类型 xml  json 		
       <dependency>
            <groupId>com.fasterxml.jackson.dataformat</groupId>
            <artifactId>jackson-dataformat-xml</artifactId>
        </dependency>
2. 响应  produces       
    json : @GetMapping(path = "/get1",produces = MediaType.APPLICATION_JSON_VALUE)
	xml  : @GetMapping(path = "/get2",produces = MediaType.APPLICATION_XML_VALUE)

3. 请求  consumes 
 @GetMapping(path = "/get1",consumes = MediaType.APPLICATION_JSON_VALUE)
 @GetMapping(path = "/get2",consumes = MediaType.APPLICATION_XML_VALUE)   
```







### controller 类的: 

```java
1. @Controller 
2. @RestController  = @Controller +  @ResponseBody

3. 通知类,切面编程,其实就是给@RequestMapping 上加了多元注解,
@ControllerAdvice(assignableTypes = {MyController.class}) 
    内部常用注解
3.1  @ExceptionHandler({Throwable.class}) 
  @ExceptionHandler({Throwable.class})
    public ResponseEntity<String> res(Throwable throwable){
        return ResponseEntity.status(HttpStatus.BAD_REQUEST).body(throwable.getMessage());
    }        
        
        
3.2  @ModelAttribute("acceptLanguage") 
     @ModelAttribute
     public void addAttribute(Model model) {
        model.addAttribute("attribute",  "The Attribute");
    }
        
3.3 @CookieValue
3.4 @InitBinder  : 应用到所有@RequestMapping注解方法，在其执行之前初始化数据绑定器 ,比如:
	 @InitBinder
     public void initBinder(WebDataBinder binder) {
        binder.addCustomFormatter(new DateFormatter("yyyy-MM-dd"));
        
    }
   
3.5 @ResponseStatus(HttpStatus.INTERNAL_SERVER_ERROR) 返回状态
		
     
4. @RequestMapping 
 
    
5. ResponseEntity 对象 
```





### 注入属性

```java
@Data
@ConfigurationProperties(prefix = "anthony")
public class MyProperties {

	@value(${"anthony.name"})
    String name;

    String age;

    String info;

    
    //在对象初始化之后立        马执行 
    @PostConstruct
    public void doFirst() {
        System.out.println("name = " + name);

        System.out.println("age = " + age);

        System.out.println("info = " + info);

        this.info = "modify";
    }

}

// 如果不用EnableConfigurationProperties 需要将MyProperties加上一个@Configuration最好
@EnableConfigurationProperties(MyProperties.class) 
@RestController
public class MyController {

    @Autowired
    MyProperties properties;
}    
```





### 单元测试类

```java
@RunWith(SpringRunner.class)
@SpringBootTest
class SpringValidatorApplicationTests {

    @Test
    @After
    @Before
    public void test(){
        
    }

}
```





### validate  数据校验

```java
@Data
public class MYData {


    @NotNull(message = "状态不能为空")
    @Min(value = 18,groups = Adult.class)
    @PositiveOrZero(message = "正数或者0")
    private Integer age;

    public interface Adult{}

    public interface Minor{}
    
}

    @RequestMapping("/live")
    public String live(@Validated({MYData.Adult.class}) MYData foo, BindingResult bindingResult) {
        if(bindingResult.hasErrors()){
            for (FieldError fieldError : bindingResult.getFieldErrors()) {
                //...
            }
            return "fail";
        }
        return "success";
    }

    @NotBlank
    @NotEmpty
    @NotNull(message = "状态不能为空")
    @Min(value = 18,message = "错了")
    @PositiveOrZero(message = "正数或者0")


@Validated({MYData.Adult.class}) 只会对 groups标记的做校验 其他不会进行 ,所以要么你注意一点
@Valid 不可以进行分组检测
BindingResult 可以获取失败结果
```





### aop 

```java
1. 自定义log注解
@Target(ElementType.METHOD)
@Retention(RetentionPolicy.RUNTIME)
@Documented
public @interface Log {
    String value() default "";
}

2. 自定义 log aop 类

@Aspect
@Component
public class LogAspect {


    private static final Logger logger = LoggerFactory.getLogger(LogAspect.class);

    // 可以将 加上 @log 注解的类 环绕通知
    @Pointcut("@annotation(com.spring.springvalidator.aop.Log)")
    public void logPointCut() {
    }


    //环绕通知
    @Around("logPointCut()")
    public Object around(ProceedingJoinPoint point) throws Throwable {
        Object result =null;
        try {
            ServletRequestAttributes attributes = (ServletRequestAttributes) RequestContextHolder.getRequestAttributes();
            HttpServletRequest request = attributes.getRequest();

            // 记录下请求内容
            logger.info("doBefore : 请求地址 : " + request.getRequestURL().toString());
            logger.info("doBefore :  HTTP METHOD : " + request.getMethod());
            logger.info("doBefore :  CLASS_METHOD : " + point.getSignature().getDeclaringTypeName() + "."
                    + point.getSignature().getName());
            // 执行方法
            result = point.proceed();

            logger.info("doBefore  :  方法参数 : " + Arrays.toString(point.getArgs()));

            logger.info("doBefore :  方法返回值 : " + result);

            logger.info("doAfterReturning");
            return result;

        } catch (Exception e) {

            logger.error("doAfterThrowing :  {} ", e.getMessage());
            throw new RuntimeException("runtime  exception");
        } finally {
            logger.info("doAfter");
        }
        
    }

```





```java
@Aspect
@Component
public class WebLogAspect2 {

    private static final Logger logger = LoggerFactory.getLogger(WebLogAspect2.class);

    //切点
    //两个..代表所有子目录，最后括号里的两个..代表所有参数
    @Pointcut("execution( * com.spring.springvalidator.web.*.*(..))")
    public void logPointCut() {
    }

    //前置通知 
    @Before("logPointCut()")
    public void doBefore(JoinPoint joinPoint) throws Throwable {
        logger.info("doBefore");

    }
    
    // 成功返回
    @AfterReturning("logPointCut()")
    public void doAfterReturning(JoinPoint joinPoint){
        logger.info("doAfterReturning");
    }
    // 失败返回
    @AfterThrowing("logPointCut()")
    public void doAfterThrowing(JoinPoint joinPoint){
        logger.info("doAfterThrowing");
    }

	//前置通知	
    @After("logPointCut()")
    public void doAfter(JoinPoint joinPoint){
        logger.info("doAfter");
    }
}


执行顺序 一般是 :
doBefore  do(controller)  doAfter  doAfterReturning\doAfterThrowing



或者 环绕通知, 

    @Around("logPointCut()")
    public Object aroundLog(ProceedingJoinPoint pjp) {
        Object reValue = null;
        try {
            // 请求参数 : 
            Object[] args = pjp.getArgs();
            logger.info("doBefore");

            reValue = pjp.proceed(args);

            logger.info("成功返回值 : {} ", reValue);
            logger.info("doAfterReturning");
            return reValue;
        } catch (Throwable throwable) {
            logger.info("doAfterThrowing");
            throw new RuntimeException(throwable);
        } finally {
            logger.info("doAfter");
        }
    }

  doBefore  doAfterReturning/doAfterThrowing   doAfter
```





### 拦截器

```java
@Configuration
public class WebConfig implements WebMvcConfigurer {

    /**
     * 添加拦截器
     * @param registry
     */
    @Override
    public void addInterceptors(InterceptorRegistry registry) {
        registry.addInterceptor(new HandlerInterceptor() {

            @Override
            public boolean preHandle(HttpServletRequest request, HttpServletResponse response, Object handler) throws Exception {
                // 设置
                System.out.println("前置拦截1");
                return true;
            }
        });

        registry.addInterceptor(new HandlerInterceptor() {

            @Override
            public boolean preHandle(HttpServletRequest request, HttpServletResponse response, Object handler) throws Exception {
                // 设置

                System.out.println(handler);

                System.out.println("前置拦截2");
                response.setStatus(400);
                return false;
            }
        });
    }

}
```

