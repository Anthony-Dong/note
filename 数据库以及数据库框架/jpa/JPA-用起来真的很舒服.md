# JPA - 为何使用如此方便

> ​	JPA是Java Persistence API的简称，中文名Java持久层API，是JDK 5.0注解或XML描述对象－关系表的映射关系，并将运行期的实体[对象持久化](https://baike.baidu.com/item/对象持久化/7316192)到数据库中。 [1] 
>
> Sun引入新的JPA ORM规范出于两个原因：其一，简化现有Java EE和Java SE应用开发工作；其二，Sun希望整合ORM技术，实现天下归一。



> JPA是需要Provider来实现其功能的，Hibernate就是JPA Provider中很强的一个，应该说无人能出其右。从功能上来说，JPA就是Hibernate功能的一个子集。Hibernate 从3.2开始，就开始兼容JPA。Hibernate3.2获得了Sun TCK的JPA(Java Persistence API) 兼容认证。
>
> Spring + Hibernate 常常被称为 Java Web 应用人气最旺的框架组合。



所以JPA是规范 , Hibernate是实现 , 如果你用了JPA , 你会爱上他的, 太方便了 , 但是他学习起来可能需要一些成本 , 掌握了你会发现比Mybatis 方便一丢丢 , 

我把jpa写成jap , 注意错误

## 1. springboot - 快速开始

我的版本是 2.0.4.RELEASE   springboot 版本  , 版本不一致可能出现不同的问题

```xml
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-data-jpa</artifactId>
</dependency>
<dependency>
    <groupId>mysql</groupId>
    <artifactId>mysql-connector-java</artifactId>
</dependency>
```

springboot的配置

```properties
spring.datasource.driver-class-name=com.mysql.jdbc.Driver
spring.datasource.username=root
spring.datasource.password=123456
spring.datasource.url=jdbc:mysql://localhost:3306/jpa?useSSL=false
spring.datasource.hikari.maximum-pool-size=2

# 展示SQL
spring.jpa.show-sql=true
# DDL模式
spring.jpa.hibernate.ddl-auto=create
# 数据库引擎
spring.jpa.database-platform=org.hibernate.dialect.MySQL5InnoDBDialect
```

我们的pojo对象 , 按照规范,我们的对象应该是 `名字+Do` , 所以叫 `UserDo`

```java
@ToString
@Setter
@Getter
@Entity
@Table(name = "jap_user")
public class UserDo {

    public UserDo() {

    }
    /**
     * 用户ID
     */
    @Id //主键
    @GeneratedValue(strategy = GenerationType.AUTO)//主键生成策略
    @Column(name = "user_id")//字段名
    private long id;


    /**
     * 用户名
     */
    @Column(name = "user_name",nullable = false)
    private String name;


    /**
     * 用户性别 , 0 none 1 female ,2 male
     */
    @Column(name = "user_gender",nullable = false)
    private int gender;

}
```

mapper对象 - repository对象

```java
@Repository
public interface UserRepository extends JpaRepository<UserDo,Integer> {

}
```

启动测试

```java
UserDo userDo = new UserDo();
userDo.setName("tom");
repository.save(userDo);
repository.findAll().forEach(System.out::println);
```

我们进入数据库查看 , 发现多了两张表

```sql
mysql> show tables;
+--------------------+
| Tables_in_jpa      |
+--------------------+
| hibernate_sequence |
| jap_user           |
+--------------------+
2 rows in set (0.00 sec)
```

我们查看 jpa_user 表信息

```java
mysql> desc jap_user;
+-------------+--------------+------+-----+---------+-------+
| Field       | Type         | Null | Key | Default | Extra |
+-------------+--------------+------+-----+---------+-------+
| user_id     | bigint(20)   | NO   | PRI | NULL    |       |
| user_gender | int(11)      | NO   |     | NULL    |       |
| user_name   | varchar(255) | NO   |     | NULL    |       |
+-------------+--------------+------+-----+---------+-------+
3 rows in set (0.00 sec)
```

## 2. 问题

### 1. 关于save 方法问题

```java
@Test
public void testCRUD() {
    UserDo userDo = new UserDo();
    for (int x = 1; x < 10; x++) {
        userDo.setGender(x % 2);
        userDo.setName("tom" + x);
        userDo.setSalary(1000 + x);
        userRepository.save(userDo);
    }

    List<UserDo> all = userRepository.findAll();
    System.out.println("all.size() = " + all.size());
}
```

大家可以猜猜输出结果 : 

```java
all.size() = 1
```

为什么呢 ?  我们试着打印我们每次的 UserDo  , 所以他的逻辑就是

```java
userDo = UserDo(id=1, name=tom1, gender=1, salary=1001.0)
userDo = UserDo(id=1, name=tom2, gender=0, salary=1002.0)
....
userDo = UserDo(id=1, name=tom9, gender=1, salary=1009.0)   
```

首先由于我们主键生成策略里是自增的  , 所以他建立了一个 `hibernate_sequence` 表, 来记录主键ID , 初始化的时候是 1 , 

```sql
Hibernate: create table hibernate_sequence (next_val bigint) engine=InnoDB
Hibernate: insert into hibernate_sequence values ( 1 )
select next_val as id_val from hibernate_sequence for update
update hibernate_sequence set next_val= ? where next_val=?
insert into jap_user (user_gender, user_name, user_salary, user_id) values (?, ?, ?, ?)
```

他会在插入的时候判断是不是新对象 , 执行一些判断逻辑

```java
@Transactional
public <S extends T> S save(S entity) {
    // 这里会判断ID是不是0 ,是的话会走这个
    if (entityInformation.isNew(entity)) {
        em.persist(entity);
        return entity;
    } else {
        // 不是的话走这个
        return em.merge(entity);
    }
}
```

很显然`persist`是持续的意思 , persistent一样 , 就是继续吧 ,主键自增

而 `merge` 就是在原来的基础上合并的意思 ,  

所以总结一下就是我们主键自增需要将对象的主键值设置成0, 然后就别管了 , 

### 2. 关于字段属性

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