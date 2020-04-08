# lombox

> ​	Java中添加maven依赖, IDEA添加lombox插件. 就可以使用了. 

## 介绍

 [官网地址 ](https://projectlombok.org/)  :  https://projectlombok.org/

> 	    ​	Project Lombok is a java library that automatically plugs into your editor and build tools, spicing up your java. Never write another getter or equals method again, with one annotation your class has a fully featured builder, Automate your logging variables, and much more.

在日常开发中会经常遇见， 为此我专门写了一下它主要的涉及到的点， 

## `@Data`

```java
public @interface Data {
	String staticConstructor() default "";
}
```

这个有一个 `staticConstructor` , 可以生成一个静态的构造方法, 默认是没有的. 

同时 `@Data` 等效于 `@Getter`  `@Setter` `@RequiredArgsConstructor` `@ToString` `@EqualsAndHashCode`.  这五个注解的效果.  

默认情况下， 如果普通字段有加final关键字， `@Data` 将final关键字的字段放入构造器中（因为final关键字必须在对象实例化的时候初始化）， 也就是出现覆盖无参构造器。

## `@RequiredArgsConstructor`

> ​	它一共三个属性

```java
public @interface RequiredArgsConstructor {
    // 一个静态的构造器.是否需要生成
	String staticName() default "";
    // 生成构造器上需要标记什么注解. JDK1.8可以通过`onConstructor_`来加入
	AnyAnnotation[] onConstructor() default {};
    // 构造器的域
	AccessLevel access() default lombok.AccessLevel.PUBLIC;
}
```

既然上面说了这个玩意. 那么一定有它的含义. 

比如常见的. 我们Spring中注入 `@Autowired` 字段, 确实不方便. 但是如果加了这个. 我们可以省略不写.  **但是这个作为构造器参数的前提是我们的字段必须被 `final`所修饰.** 

 像在spring中, 我们可以这么愉快的注入. 由于Spring的构造器自动发现. **其实可以不加`@Autowired`注解给构造器.** 

```java
@RequiredArgsConstructor(onConstructor_ = {@Autowired})
@RestController
@RequestMapping(path = "/v1/user")
public class UserController {
    private final UserService userService;
}
```

## `@Accessors `

@Accessors(存取器)用于配置getter和setter方法的生成结果，下面介绍三个属性 

### fluent

**fluent的中文含义是流畅的，设置为true，则getter和setter方法的方法名都是基础属性名，且setter方法返回当前对象**。如下

```java
// 生成的getter和setter方法如下，方法体略
public Long id() {}
public User id(Long id) {}
public String name() {}
public User name(String name) {}
```

### chain

**chain的中文含义是链式的，设置为true，则setter方法返回当前对象**。如下

```java
// 生成的setter方法如下，方法体略
public User setId(Long id) {}
public User setName(String name) {}
```

### prefix

**prefix的中文含义是前缀，用于生成getter和setter方法的字段名会忽视指定前缀（遵守驼峰命名）**。如下

```java
@Data
@Accessors(prefix = "p")
class User {
	private Long pId;
	// 生成的getter和setter方法如下，方法体略
	public Long getId() {}
	public void setId(Long id) {}
}
```

## `@Builder` 

当加入builder注解后. 会默认生成一个`OrdersBuilder`内部类, 去构建对象. 这个我最喜欢。

```java
public static void main(String[] args) {
    Orders build = Orders.builder().count(1).name("1").id(1).build();
}
```

> ​	需要特别注意的是, 如果加入`@Builder`注解后. 那么此时你的无参构造器会消失. 而且存在的构造器会私有化了. 所以还需要配合  `@AllArgsConstructor`   和 `@NoArgsConstructor` 

因此如果你喜欢使用 `@Builder` , 最好多个注解一起配合。也就是四者合一， 天下无敌 

```java
@NoArgsConstructor
@AllArgsConstructor
@Builder
@Data
```

## `@NonNull`

> ​	会自动生成NUll监测的代码.  同时抛出`NullPointerException` 运行时异常. 

```java
public void test(@NonNull String msg) {
    System.out.println(msg.getBytes().length);
}
```

结果 : 

```java
public void test(@NonNull String msg) {
    if (msg == null) {
        throw new NullPointerException("msg");
    } else {
        System.out.println(msg.getBytes().length);
    }
}
```



## `@SneakyThrows`

就是帮你写 try-catch   , 比如下列代码. 

```java
@SneakyThrows({NullPointerException.class})
public void test(String msg) {
    System.out.println(msg.getBytes().length);
}
```

生成 : 

```java
public void test(String msg) {
    try {
        System.out.println(msg.getBytes().length);
    } catch (NullPointerException var3) {
        throw var3;
    }
}
```



## `@Synchronized`

生成一个lock对象. 然后sync给你套住. 玩了. 就是加个锁 . 也不建议使用. 

## `@Cleanup`

try - resource  , 其实也不是, 就是他会帮助你调用close. 方法. 我觉得没啥必要. 用这个. 