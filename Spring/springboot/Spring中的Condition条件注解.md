# Spring - Conditional条件注解

## 1. Conditional注解 和 Condition接口

Conditional注解

> ​	Indicates that a component is only eligible for registration when all specified conditions match.
>
> ​	当所有指定的Condition接口类都匹配的时候才会将Bean注入

Condition 接口

> ​	A single condition that must be matched in order for a component to be registered. 
>
> ​	一个component 要注册所必须匹配的单个条件。

```java
boolean matches(ConditionContext context, AnnotatedTypeMetadata metadata);
// metadata是标记类的元信息,包含该注解标记类的注解信息还有类信息等等
```

返回false 不允许注册

### 3. 基本使用

比如说我们要注册一下这个类 , 此时

```java
@Component
@Conditional(value = {MyCondition.class})
public class MyConditionalOnMissingBean {
    @Override
    public String toString() {
        return "MyConditionalOnMissingBean";
    }
}
```

Condition实现类

```java
public class MyCondition implements Condition {

    @Override
    public boolean matches(ConditionContext context, AnnotatedTypeMetadata metadata) {
        return false;
    }
}
```

启动类启动会发现此对象未注册 . 

### 4. 高级用法

> ​	下面的几个内置的ConditionOnxxxx 其实都是基于这个实现的 .... 

我们有一个`MyConditional` , 需要版本匹配才能使用

```java
@Target({ElementType.TYPE, ElementType.METHOD})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Conditional(value = {MyCondition.class})
public @interface MyConditional {

    String version() default "";
}
```

此时我们用注解标记一下类 `@MyConditional(version = "2.0.0")`

```java
@Component
@MyConditional(version = "2.0.0")
public class MyConditionalOnMissingBean {
    @Override
    public String toString() {
        return "MyConditionalOnMissingBean";
    }
}
```

我们的`MyCondition`类

```java
public class MyCondition implements Condition {

    @Override
    public boolean matches(ConditionContext context, AnnotatedTypeMetadata metadata) {
        Map<String, Object> annotationAttributes = metadata.getAnnotationAttributes(MyConditional.class.getName());
        // 版本如果不为1.0.0 ,则抛出异常
        String version = (String) annotationAttributes.get("version");
        Assert.isTrue(version.equals("1.0.0"), "版本号不匹配");
        return true;
    }
}
```

启动 ... 发现出现异常

```java
org.springframework.beans.factory.BeanDefinitionStoreException: Failed to read candidate component class: file [D:\代码库\spring教程\Spring-Boot教程\springboot-enable\target\classes\com\example\springbootenable\bean\MyConditionalOnMissingBean.class]; nested exception is java.lang.IllegalArgumentException: 版本号不匹配
```







## 2. ConditionalOnBean

> ​	Conditional that only matches when the specified bean classes and/or names are already contained in the BeanFactory.
>
> ​	在BeanFactory中**匹配到**指定的Bean的类或者名字时

### 1. 使用

```java
@Configuration
public class MyAutoConfiguration {

    @ConditionalOnBean
    @Bean
    @Primary
    public MyConditionalOnMissingBean myService() {
        System.out.println("BeanFactory已经有MyConditionalOnMissingBean,我来注入一个");
        MyConditionalOnMissingBean bean = new MyConditionalOnMissingBean();
        bean.name = "hello  ConditionalOnBean";
        return bean;
    }
}
```

```java
@Component
public class MyConditionalOnMissingBean {
    @Override
    public String toString() {
        return "MyConditionalOnMissingBean";
    }

    public String name = "default";
}
```

此时输出 ; 

```java
hello  ConditionalOnBean
```

### 2. 注意

当你注入的Bean中,比我我上面是 `MyConditionalOnMissingBean` , 如果我没有标记`@Component` 注解,此时会启动失败 , 

如果我方法中, 未修饰 `@Primary`注解,此时 启动失败, 因为有俩类 , 所以我不知道这个条件注解究竟要做啥

## 2. ConditionalOnMissingBean

> Conditional that only matches when the specified bean classes and/or names are not already contained in the BeanFactory.
>
> 在BeanFactory中**尚未匹配到**指定的Bean的类或者名字 ,  所以这个有可能出现阶段问题, 加载早与迟,所以一般推荐自动装配的时候使用 , 与上诉`ConditionalOnBean`的正好相反 , 

用法, 文档推荐用法

配置类

```java
@Configuration
public class MyAutoConfiguration {

    @ConditionalOnMissingBean
    @Bean
    public MyConditionalOnMissingBean myService() {
        System.out.println("BeanFactory中没有MyConditionalOnMissingBean,我来注入一个");
        return new MyConditionalOnMissingBean();
    }
}
```



```java
public class MyConditionalOnMissingBean {
    @Override
    public String toString() {
        return "MyConditionalOnMissingBean";
    }
}
```

启动类

```java
@SpringBootApplication
public class SpringbootEnableApplication implements ApplicationContextAware, CommandLineRunner {
    public static void main(String[] args) {
        SpringApplication.run(SpringbootEnableApplication.class, args);
    }
    private ApplicationContext context;

    @Override
    public void setApplicationContext(ApplicationContext applicationContext) throws BeansException {
        this.context = applicationContext;
    }

    @Override
    public void run(String... args) throws Exception {
        Object bean = context.getBean(MyConditionalOnMissingBean.class);
        System.out.println(bean);
    }
}
```

输出

```java
BeanFactory中没有MyConditionalOnMissingBean,我来注入一个
MyConditionalOnMissingBean
```

当我们将 `MyConditionalOnMissingBean` 使用 `@Component`注入的时候会发现, 不出出现我们配置类中的话.

## 4. ConditionalOnClass

> ​	{@link Conditional} that only matches when the specified classes are on the classpath.
>
> ​	就是这个类必须在classpath下面 , 才会成功



## 5. ConditionalOnExpression

> ​	Configuration annotation for a conditional element that depends on the value of a SpEL expression.
>
> 取决于这个SPEL表达式

我们的使用方式,注入条件

```java
@ConditionalOnExpression("${enable:false}")
@Bean
public MyConditionalOnExpression myConditionalOnExpression() {
    return new MyConditionalOnExpression();
}
```

类

```java
public class MyConditionalOnExpression {

    @Override
    public String toString() {
        return "MyConditionalOnExpression";
    }
}
```

启动的时候VM参数加入 `-Denable=true` 就可以启动成功了 ,获取其他环境配置注入也行

## 6. ConditionalOnProperty

> Conditional that checks if the specified properties have a specific value.
>
> ​	检查指定属性是否具有特定值的条件

> ​	类似于上面的ConditionalOnExpression , 他只不过不要求属性值

```java
@ConditionalOnProperty(prefix = "spring",name = "example.values")
@Bean
public MyConditionalOnProperty myConditionalOnProperty(){
    return new MyConditionalOnProperty();
}
```

```java
public class MyConditionalOnProperty {

    @Override
    public String toString() {
        return "MyConditionalOnProperty";
    }
}
```

application.properties中加入

```properties
spring.example.values=name
```

就可以正确注入了,其实默认写一个value值 `spring.example.values` ,就行了



## 7.总结

剩下的就不介绍了 ........................... 还有一种特殊的是Profile 

## 8. Profile 注解

他需要制定一个描述,必须有描述信息才能注入Bean成功, 

```java
@Profile("springboot")
@Bean
public MyConditionalOnProperty myConditionalOnProperty(){
    return new MyConditionalOnProperty();
}
```

我们需要如何加入profiles , 需要这么配置....

```java
public static void main(String[] args) {
    new SpringApplicationBuilder()
            .sources(SpringbootEnableApplication.class)
            .web(WebApplicationType.NONE)
            .profiles("springboot")
            .run(args);
}
```

