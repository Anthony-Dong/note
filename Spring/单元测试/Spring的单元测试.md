# SpringBoot 单元测试之Junit & Mockito

关于`junit`和`mockito `的相关知识, 如果说junit提供了测试方法, 那么mockito则是提供了bean的管理以及bean的方法调用  , 如果这两块某些注解和方法的用法不熟悉的话 , 介意看看这篇文章 : [https://www.jianshu.com/p/ecbd7b5a2021](https://www.jianshu.com/p/ecbd7b5a2021)  内容很全, 必须先铺垫一下. 

> ​	下面就开始本文内容了  .  spring-test的话主要依赖于 `mockito` 和 `junit` 这两个库. 

## Controller层

> ​	Controller测试一般是web接口测试  , 但是往往依赖于service层,所以不适合做测试. 但是提供了mock 很好的帮助了我们测试.

我们这里就拿登录来说吧  , 模拟这个是用户

```java
@Data
public class UserDto {
    private String username;
    private String password;
}
```

其次就是用户服务

```java
@Service
public class UserService {

    /**
     * 验证用户密码是否正确
     * @param userDto 用户
     * @return 验证成功 返回 false
     */
    public boolean verify(UserDto userDto) {
        // 模拟异常 , 表示次业务线跑不通, 目前还没有写完
        throw new RuntimeException();
    }
}
```

其次就是我们的Controller层 , 

```java
@Slf4j
@RestController
@RequiredArgsConstructor
@RequestMapping(path = "/v1/user")
public class UserController {

    private final UserService userService;

    /**
     * 登录
     * @param userDto 用户
     * @return 登录成功返回 true
     */
    @PostMapping("/login")
    public boolean login(UserDto userDto) {
        // 记录日志
        log.info("login username: {}, password :{}.", userDto.getUsername(), userDto.getPassword());
        return userService.verify(userDto);
    }
}
```

此时拿 postman绝对跑不通. 怎么办呢. 

主角来了

```java
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;

@RunWith(SpringRunner.class)
@SpringBootTest
// 会自动注入 mockmvc对象
@AutoConfigureMockMvc
public class UserControllerTest {

    @Autowired
    private MockMvc mockMvc;

    // 标记service表示无效,走我们的service
    @MockBean
    private UserService service;

    @Test
    public void findAddr() throws Exception {
        // 当访问service的 verify方法时. 任何对象都返回true
        when(service.verify(any(UserDto.class))).thenReturn(true);
        MvcResult result = mockMvc.perform(
                post("/v1/user/login")
                // application/x-www-form-urlencoded , 一般都是选择这个
                .contentType(MediaType.APPLICATION_FORM_URLENCODED_VALUE)
                // 输入参数
                .param("username", "root")
                .param("password", "123456")
        )
            // 打印一些必要的请求信息.
                .andDo(MockMvcResultHandlers.print())
                .andReturn();

        String content = result.getResponse().getContentAsString();
        Assert.assertEquals(content, "true");
    }
}
```

我们最终启动跑一下 

```java
2020-04-01 19:56:58.054  INFO 17140 --- [           main] c.e.s.controller.UserController          : login username: root, password :123456.
```

结果完全OK . 是不是很神奇呢. 



注意这里的 `@MockBean`  是Spring提供的. 同时还有, mock官方提供了 `@Mock` 和 `@InjectMocks `  , 具体可以看看这篇文章 , 怕使用的时候出错误 , 记录一下 : [https://www.jianshu.com/p/c68ee5d08fdd]( https://www.jianshu.com/p/c68ee5d08fdd)



## Service层

service层 往往依赖于dao层 

### 1. 关闭web 环境

所以解决方式也是 , 但是不是web层, 所以往往不需要web环境. `@SpringBootTest(webEnvironment = SpringBootTest.WebEnvironment.NONE)` 这个可以关闭web环境. 

```java
@RunWith(SpringRunner.class)
@SpringBootTest(webEnvironment = SpringBootTest.WebEnvironment.NONE)
public class UserServiceTest {

    @Autowired
    UserService service;
    
    @Test
    public void testUser() {
        service.verify(UserDto.builder().password("111").build());
    }
}
```

### 2. 测试用例 , 每个都需要单独的context

```java
@RunWith(SpringRunner.class)
@SpringBootTest(webEnvironment = SpringBootTest.WebEnvironment.NONE)
public class UserServiceTest {

    @Autowired
    private UserService service;

    @DirtiesContext
    @Test
    public void test1() {
        System.out.println("test-1 " + service.hashCode());
    }

    @DirtiesContext
    @Test
    public void test2() {
        System.out.println("test-2 " + service.hashCode());
    }
}
```

我们发现输出的是 

```java
test-1 2125470482
test-2 1846539844
```

这个 `@DirtiesContext` 有两种

第一种就是方法隔离 . 

第二种就是类隔离.  使用场景的话各有差异 .. 

测试用例的运行 . 可以直接通过 mvn  test  就可以了 , 重定向输出可以用 >> log.log ,这里切记一点. `mvn test -Dtest=*Test -DfailIfNoTests=true`  , 这个是默认的缺省值.   也就是只会监测到Test后缀的文件进行测试. 







## DAO层面

其中 `@MockBean` 可以帮助我们实例化一个mapper对象, 同时我们可以直接调用. 此时就可以直接隔离数据库访问层面.

```java
@RunWith(SpringRunner.class)
@SpringBootTest(webEnvironment = SpringBootTest.WebEnvironment.NONE)
public class UserMapperTest {

    @MockBean
    private UserMapper mapper;

    @Test
    public void findAddr() {
        when(mapper.findAddr(anyInt())).thenReturn("北京");
        String addr = mapper.findAddr(100);
        assertEquals("北京", addr);
        verify(mapper).findAddr(100);
    }
}
```



### H2数据库 方便测试 

加入依赖, 

```xml
<!-- 方便测试 -->
<dependency>
    <groupId>com.h2database</groupId>
    <artifactId>h2</artifactId>
    <version>1.4.197</version>
    <scope>test</scope>
</dependency>
```

配置文件的话, 很简单, 我是用的是JPA. 

```properties
# H2的配置. 就两行其实. 其他我多余加上的. 
spring.datasource.url=jdbc:h2:mem:jpa;DB_CLOSE_DELAY=-1;DB_CLOSE_ON_EXIT=FALSE;MODE=MYSQL
spring.datasource.driver-class-name=org.h2.Driver
spring.datasource.continue-on-error=false
spring.datasource.hikari.minimum-idle=2

# 没次启动创建, 每次结束进程删除
spring.jpa.hibernate.ddl-auto=create-drop
# SQL打印开启
spring.jpa.show-sql=true
# 数据库引擎修改一下
spring.jpa.database-platform=org.hibernate.dialect.MySQL5InnoDBDialect
```

这个就是一个完整的测试环境.  soeasy . 



## swagger-ui

首先呢  , 依赖少不了 . 

```xml
<dependency>
    <groupId>io.springfox</groupId>
    <artifactId>springfox-swagger-ui</artifactId>
    <version>2.9.2</version>
</dependency>
<dependency>
    <groupId>io.springfox</groupId>
    <artifactId>springfox-swagger2</artifactId>
    <version>2.9.2</version>
</dependency>
```

其次呢  , 加入配置.  

```xml
@Configuration
@EnableSwagger2
public class SwaggerConfig {

    /**
     * 访问接口在
     * @return
     */
    @Bean
    public Docket api() {
        return new Docket(DocumentationType.SWAGGER_2)
                .select()
                .apis(RequestHandlerSelectors.basePackage("com.example.springswagger.controller"))
                .paths(PathSelectors.any())
                .build()
                .apiInfo(apiEndPointsInfo())
                .useDefaultResponseMessages(false);
    }

    private ApiInfo apiEndPointsInfo() {
        return new ApiInfoBuilder().title("REST API")
                .description("Spring Swaager2 REST API")
                .contact(new Contact("anthony", "https://github.com/Anthony-Dong", "574986060@qq.com"))
                .license("The Apache License")
                .licenseUrl("https://opensource.org/licenses/MIT")
                .version("V1")
                .build();
    }
}
```

其他就直接映射好了 

直接访问 [http://localhost:8080/swagger-ui.html](http://localhost:8080/swagger-ui.html)  既可

比较坑的点 . 第一点, 要写注释就写清楚点像`@ApiParam(value = "用户ID",required = true,example = "1")`  , 如果不加 example会抛出异常, 比如`java.lang.NumberFormatException:For input string:""` , 因为我们format的时候失败了. 默认是空. 所以比较坑. 

```java
@Slf4j
@RestController
@RequestMapping(path = "/v1/user")
public class UserController {

    @GetMapping("/info/{id}")
    @ApiOperation("根据ID获取用户信息")
    public Map<String, UserDo> userInfo(@ApiParam(value = "用户ID",required = true,example = "1") @PathVariable("id") Long id) {
        return Collections.singletonMap("user-info", UserDo.builder().userId(id).username("tom").password("123").build());
    }
} 
```

具体的解决方案就是 如果写 就要加上最好了. 

```java
syms x;
a = -15;
b = -a;
step = 0.01;
gap = 15;
e = 1e-3;
while (1)
    for w= a : step : b
            s1 = int(exp(-(x-1000)^2/50), -inf, (16-w)/18);
            s2 = exp(-((16 - w) / 18 - 1000) ^ 2  / 50);
            u = 5 * sqrt(2 * pi) / s1 + 36 * (16 - w) + 250 + 5 * sqrt(2 * pi) * (w - 5)  / s2 * ( - 1 / 18);
            if abs(u) < gap
                display(['gap=', num2str(gap), ' w=', num2str(w)])
                step = gap / 100;
                a = w - gap;
                b = w + gap;
                gap = gap / 10;
                break;
            end
    end
    if gap < e
        break;
    end
end
```







## MockITO  的使用

第一种  使用 , 很方便

```java
public class MockTest {
    @Test
    public void test(){
        UserMapper mapper = mock(UserMapper.class);
        UserDo tom = UserDo.builder().userId(1L).username("tom").password("123").build();
        when(mapper.findByUserId(anyLong())).thenReturn(tom);
        // 第一次调用
        System.out.println(mapper.findByUserId(1L));
        // 第二次调用
        System.out.println(mapper.findByUserId(2L));

        // 校验
        verify(mapper).findByUserId(1L);
        verify(mapper).findByUserId(2L);
    }
}
```



第二种 , 更方便

```java
@RunWith(MockitoJUnitRunner.class)
public class MockTest {

    @Mock
    private UserMapper mapper;

    @Test
    public void test(){
        UserDo byUserId = mapper.findByUserId(1L);
        System.out.println(byUserId);
    }
}
```



第三种 , 一般吧, 其实和第一种相似 . 

```java
public class MockTest {

    @Mock
    private UserMapper mapper;

    public MockTest() {
        mapper = Mockito.mock(UserMapper.class);
    }

    @Test
    public void test(){
        UserDo byUserId = mapper.findByUserId(1L);
        System.out.println(byUserId);
    }
}
```



## SQL脚本的执行

### 1. Spring自带的 注意坑 

第一种就是在配置文件中告诉, 他, 你要启动时注入的脚本

```properties
# 必须设置这个.原因可以看 org.springframework.boot.autoconfigure.jdbc.DataSourceInitializer.isEnabled , 像H2是可以不设置的,默认是只有切入式数据库才可以加载启动脚本.
spring.datasource.initialization-mode=always
# 建表语句
spring.datasource.schema=classpath:schema.sql
# 多个的时候可以如下这么写.
# spring.datasource.schema[0]=classpath:schema.sql
# 插入数据语句
spring.datasource.data=classpath:schema.sql
```

其中Spring加载的原理是  , 相当的简单. 反正. 因此我写了个脚本.  反正底层原理也是解析SQL语句 , 然后执行. 很简单的. 直接调用它的就可以了. 

```java
/**
 * spring的模式 , 可以执行脚本 , 这个脚本在classpath下面.
 */
public static void runSql(DataSource dataSource, String... fileInClassPathResources) {
    if (fileInClassPathResources == null || fileInClassPathResources.length == 0) return;
    ResourceDatabasePopulator populator = new ResourceDatabasePopulator();
    for (String source : fileInClassPathResources) {
        populator.addScript(new ClassPathResource(source));
    }
    DatabasePopulatorUtils.execute(populator, dataSource);
}
```



### 2. Mybatis的工具.

利用Mybatis的工具 , 它可以展示SQL. 这个最好了

```xml
<dependency>
    <groupId>org.mybatis</groupId>
    <artifactId>mybatis</artifactId>
    <version>3.5.2</version>
</dependency>
```

执行脚本 

```java
public class RunSqlScript {
    /**
     * <p>运行指定的sql脚本
     * @param sqlStream 需要执行的sql脚本的数据流
     */
    public static void run(DataSource dataSource, InputStream sqlStream) throws SQLException {
        // try - with - resource
        try (Connection conn = dataSource.getConnection()) {
            // 创建ScriptRunner，用于执行SQL脚本
            ScriptRunner runner = new ScriptRunner(conn);
            runner.setErrorLogWriter(new PrintWriter(System.err));
            runner.setLogWriter(new PrintWriter(System.out));
            // 执行SQL脚本
            runner.runScript(new InputStreamReader(sqlStream));
            // 成功就输出成功
            System.out.println("load sql script successful");
        }
    }
}
```

SpringBoot整合的话. 可以通过

```java
@SpringBootApplication
public class SpringTestApplication implements CommandLineRunner {

    public static void main(String[] args) {
        SpringApplication.run(SpringTestApplication.class, args);
    }

    @Autowired
    private DataSource dataSource;

    @Override
    public void run(String... args) throws Exception {
        InputStream stream = Thread.currentThread().getContextClassLoader().getResourceAsStream("schema.sql");
        RunSqlScript.run(dataSource, stream);
        stream.close();
    }
}
```



