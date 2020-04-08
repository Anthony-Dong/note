# JPA - 为何使用如此方便

> ​	JPA是Java Persistence API的简称，中文名Java持久层API，是JDK 5.0注解或XML描述对象－关系表的映射关系，并将运行期的实体[对象持久化](https://baike.baidu.com/item/对象持久化/7316192)到数据库中。 [1] 
>
> Sun引入新的JPA ORM规范出于两个原因：其一，简化现有Java EE和Java SE应用开发工作；其二，Sun希望整合ORM技术，实现天下归一。



> JPA是需要Provider来实现其功能的，Hibernate就是JPA Provider中很强的一个，应该说无人能出其右。从功能上来说，JPA就是Hibernate功能的一个子集。Hibernate 从3.2开始，就开始兼容JPA。Hibernate3.2获得了Sun TCK的JPA(Java Persistence API) 兼容认证。
>
> Spring + Hibernate 常常被称为 Java Web 应用人气最旺的框架组合。

所以JPA是规范 , Hibernate是实现 , 如果你用了JPA , 你会爱上他的, 太方便了 , 但是他学习起来可能需要一些成本 , 掌握了你会发现比Mybatis 方便一丢丢 , 

## 1. springboot - 快速开始

我的版本是 2.0.4.RELEASE   springboot 版本  , 版本不一致可能出现不同的问题 . 

```xml
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-data-jpa</artifactId>
</dependency>
<dependency>
    <groupId>mysql</groupId>
    <artifactId>mysql-connector-java</artifactId>
</dependency>
<!-- 方便测试 -->
<dependency>
    <groupId>com.h2database</groupId>
    <artifactId>h2</artifactId>
    <version>1.4.197</version>
    <scope>test</scope>
</dependency>
```

springboot的配置 , 其中没必要去设置 `@EnableJpaRepositories` 注解, 默认会自动注入的.  这个是测试环境的配置文件. 

```properties
# 使用H2为了测试方便.
spring.datasource.url=jdbc:h2:mem:jpa;DB_CLOSE_DELAY=-1;DB_CLOSE_ON_EXIT=FALSE;MODE=MYSQL
spring.datasource.driver-class-name=org.h2.Driver
spring.datasource.continue-on-error=false
spring.datasource.hikari.minimum-idle=2

# 就是每次运行完清空数据. 测试环境下, 最好使用这种.
spring.jpa.hibernate.ddl-auto=create-drop
spring.jpa.show-sql=true
# 修改数据库引擎.
spring.jpa.database-platform=org.hibernate.dialect.MySQL5InnoDBDialect
```

我们的pojo对象 , 按照规范,我们的对象应该是 `名字+Do` , 所以叫 `UserDo`

```java
@NoArgsConstructor
@AllArgsConstructor
@Builder
@Data
// name含义就是你生成表的table的名称.
// 默认是类名. 而且这个很有用处, 最好别自定义. 自定义的话 . JSQL就得按照这个名字来了.
// 所以一般也不加@Table注解.
@Entity
public class UserDo {

    @Id
    // 这个策略只是用与MySQL自带的那种自增策略.更多解释下文讲解.
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long userId;
    private String username;
    private String password;

}
```

mapper对象 - repository对象

```java
public interface UserMapper extends JpaRepository<UserDo, Long> {
    // 后面解释为什么这么写
    @Query(value = "select u from user u where u.userId=:userId")
    UserDo findByUserId(Long userId);
}
```

## 2. 测试

### 1. CRUD操作

```java
public interface UserMapper extends JpaRepository<UserDo, Long> {

    @Query(value = "select u from UserDo u where u.userId=:userId")
    UserDo findByUserId(Long userId);


    /**
     * 第一必须加 @Transactional 注解, 不然报错.
     * CUD 操作必须申明 @Modifying 注解, 不然报错 , 同时它的属性 clearAutomatically 可以帮助我们清空缓存, 最好设置为true.
     * 这就是申明这俩的原因. 
     */
    @Transactional
    @Modifying
    @Query("delete from UserDo u where u.userId=:userId")
    int deleteByUserId(Long userId);

    @Transactional
    @Modifying
    @Query("update UserDo u set u.password=:password where u.userId=:userId")
    int updateByUserId(Long userId, String password);
}
```



测试代码 : 

```java
@FixMethodOrder(value = MethodSorters.DEFAULT)
@RunWith(SpringRunner.class)
@SpringBootTest(webEnvironment = SpringBootTest.WebEnvironment.NONE)
public class UserMapperTest {

    @Autowired
    private UserMapper mapper;

    @Before
    public void init() {
        mapper.save(UserDo.builder().username("tom").password("123456").build());
        mapper.save(UserDo.builder().username("tony").password("123456").build());
    }

    @After
    public void drop() {
        mapper.deleteAll();
    }


    @Test
    public void findByUserId() {
        // 执行两次的目的是看看是否使用了缓存
        mapper.findByUserId(1L);
        mapper.findByUserId(1L);
    }

    @Test
    public void deleteByUserId() {
        mapper.deleteByUserId(2L);
    }

    @Test
    public void updateByUserId() {
        int i = mapper.updateByUserId(1L, "987654321");
        System.out.println(i);
        UserDo user = mapper.findByUserId(1L);
        System.out.println(user);
    }
}
```

我把结果大致放这里. 

```java
// 初始化流程 , 创建表
Hibernate: create table user_do (user_id bigint not null auto_increment, password varchar(255), username varchar(255), primary key (user_id)) engine=InnoDB


// 插入语句的流程
Hibernate: insert into user_do (password, username) values (?, ?)
Hibernate: insert into user_do (password, username) values (?, ?)
    
// 删除
Hibernate: delete from user_do where user_id=?
    
//查询
Hibernate: select userdo0_.user_id as user_id1_0_, userdo0_.password as password2_0_, userdo0_.username as username3_0_ from user_do userdo0_ where userdo0_.user_id=?
//查询
Hibernate: select userdo0_.user_id as user_id1_0_, userdo0_.password as password2_0_, userdo0_.username as username3_0_ from user_do userdo0_ where userdo0_.user_id=?


// 更新    
Hibernate: update user_do set password=? where user_id=?    
```

### 2. 问题

- 没有使用缓存, 
- 插入的流程是 ,先去sequence表中, 行锁查询ID字段. 然后根据ID去插入 , 没有使用数据库自带的自增策略. 
- 关于如何开启缓存 , 自行百度. 我觉得, 最好别从 dao层面开启缓存, 应该让我们从服务层手动写代码开启. 防止出现脏数据. 
- 关于 为何加 `@Transactional` 此注解的原因 , 以及 `@Modifying` 的原因.  上诉介绍了
- 关于 `@Entity` 注解的问题以及 JSQL的问题. 为何介意只使用 `@Entity` 注解, 不使用多余使用`@Table`注解呢, 

因为在我们写 jsql的时候, 是根据entity 注解的标记走的.  所以, 如果你要写. table和entity的name值必须相同, 而且表名默认就是 entity的name, 也就是类名. 

所以对于我上面的user对象

```java
@Entity(name = "user")
public class UserDo {}
```

这个查询语句

```sql
@Query(value = "select u from user u where u.userId=:userId")
UserDo findByUserId(Long userId);
```

此时就是, 使用了`user` 作为表的映射字段. 

如果我们将entity的name取消掉. 如何写sql呢

```java
@Query(value = "select u from UserDo u where u.userId=:userId")
UserDo findByUserId(Long userId);
```

就是上诉这种了, 结合这点. 所以最好别将UserDo 名字设置为这个, 最好直接是表名称最好. 

为了写起来方便, 因此我将 `@Entity(name = "user")` 改为了 `@Entity` 直接. 



### 3. 关于字段属性

发现了什么 , int类型,默认对应的字段时int(11) , 而long类型对应的是 bigint(20)  ,然后string对应的是 varchar(255) ,

很不友好

我们修改几个属性再 ,主要是 `@Column` 注解的属性

```java
public @interface Column {
	
    // 字段名
    String name() default "";

    // 是否唯一
    boolean unique() default false;

    // 是否可以为空
    boolean nullable() default true;

    // 是否可插入
    boolean insertable() default true;

    // 是否可更新
    boolean updatable() default true;

    // 字段定义 , 这个很好用
    String columnDefinition() default "";

    // table
    String table() default "";

    // 这个只能作用于String类型, 
    int length() default 255;

    int precision() default 0;

    int scale() default 0;
}
```



所以我们应该这么建表

```java
@ToString
@Setter
@Getter
@Entity
@Table(name = "jpa_user") // 默认是 user_do , 看自己业务需求,需不需要设置,感觉默认最好,不出事
public class UserDo {

    public UserDo() {

    }

    /**
     * 用户ID
     */
    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    @Column(name = "user_id")
    private int id;

    /**
     * 用户名
     */
    @Column(name = "user_name", nullable = false, length = 10)
    private String name;

    /**
     * 用户性别 , 0 none 1 female ,2 male  只展示一位数
     */
    @Column(name = "user_gender", nullable = false, columnDefinition = "tinyint (1) zerofill")
    private int gender;


    /**
     * 最长6位数 也就是最大是 999999.99
     */
    @Column(name = "user_salary", nullable = true, columnDefinition = "decimal(8,2)")
    private double salary;
}
```

### 4. 关于主键生成策略 

我这里留一篇文章 ,  [https://www.cnblogs.com/SummerinShire/p/7544897.html](https://www.cnblogs.com/SummerinShire/p/7544897.html)  , 大家自行去看, 很详细的. 

其中很多种方式, 比如 `org.hibernate.id.factory.internal.DefaultIdentifierGeneratorFactory#DefaultIdentifierGeneratorFactory` 这个定义了很多种 . 

## 3. 基本使用

#### 插入

```java
UserDo userDo = new UserDo();
userDo.setName("tom");
userDo.setSalary(1000.1);
repository.save(userDo);
```

#### 排序查找

```java
// 这里的字段名是和Java字段名相同  , 不是MYSQL的, 还有就是注意实例化放到外面, 不然每次掉用实例化浪费内存
final Sort sort = new Sort(Sort.Direction.DESC, "salary");
List<UserDo> salary = repository.findAll(sort);
salary.forEach(System.out::println);
```

#### 条件查询

```java
Optional<UserDo> tom = repository.findOne(new Example<UserDo>() {
    @Override
    public UserDo getProbe() {
        UserDo aDo = new UserDo();
        aDo.setName("tom");
        aDo.setSalary(1000.1);
        aDo.setId(1);
        return aDo;
    }

    @Override
    public ExampleMatcher getMatcher() {
        return ExampleMatcher.matchingAll();
    }
});


Optional<UserDo> one = repository.findOne(Example.of(userDo)); 这个
```

这里比较坑 , 有两种模式 MatchMode 中有ALL 和 ANY , all 就是全部是and连接, any全部是or



#### 分页查找

```java
Page<UserDo> all = repository.findAll(new PageRequest(0, 2));
all.forEach(System.out::println);
```

```java
Page<UserDo> list = repository.findAll(PageRequest.of(1, 2));
list.forEach(System.out::println);
```

`Pageable` 是一个接口类  , 显然我们不回去直接使用 , 一般使用 `PageRequest` 对象 , 他是一个现成的方法 , 还有就是对于其使用 , 可以直接new方法构造 和 of进行, 第一种方法被抛弃了 , 所以我们一般使用第二种 , 他需要三个参数 , page(从0开始) , size(页面大小)  , sort(排序) , 他不同于mysql的limit m n, 是通过计算得到的, 在mysql中 m指的是从第几行开始(不包含改行) , n指的是多少个 , 

#### 删除

```java
userRepository.deleteAllInBatch();  //删库跑路 , go go go ,delete from jpa_user

userRepository.deleteById(10); // 找不到抛出异常

userRepository.delete(userDo);  // 也是用的 删除ID的方法 , 需要提供ID ,先查再删


userRepository.deleteAll(); // 慢慢全部删除
```

还有其他

## 4. JPA 的 JQL

### 1. 认识JQL

其实跟SQL 么多大区别  ,这里注意下 , 我们的实体类上的 `@Entity` 注解, 别乱写基本么问题,默认就行了

```java
@Query(value = "select u from UserDo u where u.name= :name")
UserDo selectByJQL(@Param("name") String name);
```

测试

```java
UserDo tom3 = userRepository.selectByJQL("tom3");
System.out.println("tom3 = " + tom3);
```

输出结果 : 

```java
Hibernate: select userdo0_.user_id as user_id1_0_, userdo0_.user_gender as user_gen2_0_, userdo0_.user_name as user_nam3_0_, userdo0_.user_salary as user_sal4_0_ from jap_user userdo0_ where userdo0_.user_name=?
tom3 = UserDo(id=3, name=tom3, gender=1, salary=1003.0)
```

基本么问题 , 其中 ? index 也可以使用 , 看你自己咋用,下面就介绍了

### 2. 高级点的用法

首先定义一个构造方法

```java
public UserDo(String name, int gender) {
    this.name = name;
    this.gender = gender;
}
```

其次进行改造

```java
//select new com.example.springdatajpastart01.pojo.UserDo(name,gender) from UserDo where name=?1
// 或者这个都行,也可以是其他
@Query(value = "select new UserDo (name,gender) from UserDo where name=?1")
UserDo selectByJQLConstructor(String name);
```

输出 : 

```java
Hibernate: select userdo0_.user_name as col_0_0_, userdo0_.user_gender as col_1_0_ from jpa_user userdo0_ where userdo0_.user_name=?
tom3 = UserDo(id=0, name=tom3, gender=1, salary=0.0)
```

发现是不是查到了, 还是很强大的, 



再来个强大的

```java
@Query("select new map(name as myname,gender as myaddress) from UserDo where name = :name")
List<Map<String, Object>> selectByJQLMap(@Param("name") String name);
```

输出

```java
Hibernate: select userdo0_.user_name as col_0_0_, userdo0_.user_gender as col_1_0_ from jpa_user userdo0_ where userdo0_.user_name=?
[{myaddress=1, myname=tom3}]
```



## 4.  JPA 对原生SQL的支持

主要就是一个 `@Query`  对原生支持需要开启里面的属性 `nativeQuery` 改成true 就可以了, 

### 1. ? index 使用

> ​	这里的index 是我们参数的位置, 从0 开始 

```java
@Query(nativeQuery = true, value = "SELECT * FROM jap_user WHERE user_id=?1")
List<UserDo> selectById(String id);
```

测试一下

```java
// 插入语句
UserDo userDo = new UserDo();
for (int x = 1; x < 8; x++) {
    userDo.setId(0);
    userDo.setGender(x % 2);
    userDo.setName("tom" + x);
    userDo.setSalary(1000 + x);
    userRepository.save(userDo);
}

List<UserDo> dos = userRepository.selectById("2 or 1=1"); //看看是不是预编译
dos.forEach(System.out::println);
```

输出 : 

```java
Hibernate: SELECT * FROM jap_user WHERE user_id=?
UserDo(id=2, name=tom2, gender=0, salary=1002.0)
```

可以防止SQL 注入的问题

### 2.  @Param

```java
@Query(nativeQuery = true, value = "SELECT * FROM jap_user WHERE user_name = :user_name")
List<UserDo> selectByUserName(@Param("user_name") String userName);
```

我们还是测试是不是预编译, 防止SQL注入呢

```java
List<UserDo> dos = userRepository.selectByUserName("tom3");
dos.forEach(System.out::println);
List<UserDo> sql = userRepository.selectByUserName("tom3 or 1=1");
sql.forEach(System.out::println);
```

输出

```java
Hibernate: SELECT * FROM jap_user WHERE user_name = ?
UserDo(id=3, name=tom3, gender=1, salary=1003.0)
Hibernate: SELECT * FROM jap_user WHERE user_name = ?
```

很显然是可以防止的



### 3. Like 模糊查询

> 百分比(`%`)通配符允许匹配任何字符串的零个或多个字符。
>
> 下划线(`_`)通配符允许匹配任何单个字符。

```java
@Query(nativeQuery = true, value = "SELECT * FROM jap_user WHERE user_name like :user_name")
List<UserDo> selectByUserNames(@Param("user_name")String username);
```

测试

```java
List<UserDo> tom_ = userRepository.selectByUserNames("tom_");
tom_.forEach(System.out::println); // tom1 - tom9

System.out.println("================================");

List<UserDo> toms = userRepository.selectByUserNames("tom%");
toms.forEach(System.out::println); // tom1 - tom18
```

发现这里只是一个模式匹配 , 随便输入就行了

### 4. SPEL 表达式

```java
@Query(nativeQuery = true, value = "SELECT * FROM #{#entityName} WHERE user_name = :user_name")
UserDo selectBySPEL(@Param("user_name")String username);
```

`#{#entityName}` 这个对应的是实体类的 `@Entity(name = "jap_user")` 这里写的字段名

测试

```java
UserDo userDos = userRepository.selectBySPEL("tom1");
System.out.println("userDo = " + userDos);
```

```java
Hibernate: SELECT * FROM jap_user WHERE user_name = ?
userDo = UserDo(id=1, name=tom1, gender=1, salary=1001.0)
```

## 5. 分页

测试 : 

```java
@Query(value = "SELECT * FROM jap_user WHERE user_salary > ?1", countQuery = "SELECT count(*) FROM jap_user WHERE user_salary > ?1", nativeQuery = true)
Page<UserDo> selectBySalary(Double salary, Pageable pageable);
```

这里注意 需要写一个 `countQuery`  是你查询的总条数要告诉他  , 对于SQL查询他会自动给你整合

```java
Page<UserDo> salary = userRepository.selectBySalary(1000.0, PageRequest.of(0, 10, new Sort(Sort.Direction.ASC, "user_salary")));
System.out.println("salary.getTotalPages() = " + salary.getTotalPages());


List<UserDo> content = salary.getContent();
content.forEach(System.out::println);
```

**这里注意一点就是 , Sort字段的那个properties, 里面写的是表的字段名 , 这个需要注意** 

对比一下直接排序 ,区别就在这里

```java
List<UserDo> all = userRepository.findAll(new Sort(Sort.Direction.DESC, "salary"));
all.forEach(System.out::println);
```

## 6. DDL 策略

> ​	DDL mode. This is actually a shortcut for the "hibernate.hbm2ddl.auto" property. Defaults to "**create-drop**" when using an embedded database and no schema manager was detected. Otherwise, defaults to "none".

create---- 每次运行该程序，会先删除表格，然后建表

create-drop---- 在create模式中, 加入了每次程序运行结束删除表 (切入式数据库默认为这个)

update---   每次运行程序，没有表格会新建表格，如果表格以及存在了 , 啥事情也不做 , (感觉最合理)

validate---- 启动的时候会校验 ,如果数据库中没有表, 会直接报错 , 如果表存在会校验数据

none --- 默认为这个,就是啥事也不做 (和 Mybatis一样 , 需要我们人工建表,也不校验)