# Spring-Boot自动装配

> ​	我们在学习spring-boot的路上，是否看见一堆 `@Enable...`的注解类， 例如 `@EnableWebMvc` , `@EnableAutoConfiguration` , `@EnableDiscoveryClient`  ， 此时我们是否想过为什么 ？ 到底是做啥了？ 



## 1. 体验自动装配

### 1. 第一种实现方式

自定义一个Bean : 

```java
@Configuration("myConfigBean")
public class MyConfigBean {

    @Override
    public String toString() {
        return "This is MyConfigBean";
    }
}
```

自定义一个 @Enable 自动装配类 : 

```java
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
@Documented
// 导入自己想要加载的类 : 
@Import(MyConfigBean.class)
public @interface EnableConfigBean {

}
```

启动类 :  这里有些特殊 , 大家也可以学着点 . 

```java
// 导入自动装配
@EnableConfigBean
public class ConditionSpringbootApplication{

	public static void main(String[] args) {
		SpringApplicationBuilder builder = new SpringApplicationBuilder();
// 配置bean注入,此时就是ConditionSpringbootApplication,和原来spring的@Configuration类一样,叫配置Bean
		ConfigurableApplicationContext context = builder.sources(ConditionSpringbootApplication.class)
				.bannerMode(Banner.Mode.OFF)
            // spring-boot 模式有三种,我们选择NONE
				.web(WebApplicationType.NONE)
            // 必须run 来启动spring-boot
				.run(args);
        
        // 大家需要注意一下,如果是一个内部类注册Bean,获取bean的名字就不是首字母小写了.
//例如 com.spring.conditionspringboot.import_example.boot.ConditionSpringbootApplication$MyBean
//需要上面这种写法 . 
		// 打印自己注入的bean
		Object bean = context.getBean("myConfigBean");
		System.out.println(bean);
	}
}
```

运行结果 : 

```java
This is MyConfigBean
// 其他日志忽略
```

通过上面的代码我们已经大致了解了 `@Enable` 做了什么 ,他的主要关键点在哪 `@Import` 注解这,会自动注入一个Bean.



### 2. 第二种实现方式 

@Enable 实现类

```java
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
@Documented
@Import({MyImportSelector.class})
public @interface EnableConfigBean {

}
```

MyImportSelector 实现 方式 ,可以看看 `@EnableAutoConfiguration`这个注解, 他就用到了一个 Selector . 

```java
public class MyImportSelector implements ImportSelector {

    // AnnotationMetadata 是一个 注解元信息 ,可以获取注解信息,以及注解所在类的其他信息
    @Override
    public String[] selectImports(AnnotationMetadata importingClassMetadata) {
        return new String[]{MyConfigBean.class.getName()};
    }
}
```



```java
public class MyConfigBean {

    @Bean
    public String helloWorld(){
        return "Hello World";
    }
}
```





## 2. 条件装配

> ​	从spring Framework 3.1 开始的

### @Profile

接口类: 

```java
public interface CalculateService {

    Integer sum(Integer ...values);
}
```

@Profile注解 标注的实现类第一个 

```java
@Profile("Java7")
@Service
public class Java7CalculateService implements CalculateService{
    
    @Override
    public Integer sum(Integer... values) {
        Integer sum = 0;
        for (Integer value : values) {
            sum += value;
        }
        return sum;
    }
}
```

@Profile注解 标注的实现类第二个 

```java
@Profile("Java8")
@Service
public class Java8CalculateService implements CalculateService{

    @Override
    public Integer sum(Integer... values) {

        return Stream.of(values).reduce(Integer::sum).get();
    }
}
```

启动类 : 

```java
@ComponentScan("com.spring.conditionspringboot.condition_example.bean")
public class CalculateBootStrap {

    public static void main(String[] args) {

        SpringApplicationBuilder builder = new SpringApplicationBuilder();

        ConfigurableApplicationContext context = builder.sources(CalculateBootStrap.class)
                .bannerMode(Banner.Mode.OFF)
                .web(WebApplicationType.NONE)
           		// 这里需要指定profile ,不然会报错 ,
                .profiles("Java7")
                .run(args);


        CalculateService service = context.getBean(CalculateService.class);

        Integer sum = service.sum(1, 2, 3, 4);

        System.out.println(service+" : "+sum);

        context.close();
    }

}
```

输出结果 : 

```java
com.spring.conditionspringboot.condition_example.bean.Java7CalculateService@3e6ef8ad : 10
```



### @Conditional

注解实现类  :  主要是实现一个 @Conditional 这个注解,指向一个Condition的实现类 . 

```java
@Target({ElementType.TYPE, ElementType.METHOD})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Conditional(ConditionProfile.class)
public @interface MyConditional {


    String key();

    String value();

}
```

ConditionProfile 实现类 : 

```java
// 这个类可以获取所有被 @MyConditional 注解所标记类的 , 注解信息 . 判断是否可以注入bean
public class ConditionProfile implements Condition {

    @Override
    public boolean matches(ConditionContext context, AnnotatedTypeMetadata metadata) {
        
        Map<String, Object> attributes = metadata.getAnnotationAttributes(MyConditional.class.getName());

        Object key = attributes.get("key");

        Object value = attributes.get("value");

        if (key.equals("name")) {

            return true;
        }
        return false;
    }
}
```

启动类 : 

```java
public class ConditionBootstrap {

    @MyConditional(key = "name",value = "sb")
    @Bean
    public String helloWorld(){
        return "Hello world";
    }


    public static void main(String[] args) {
        SpringApplicationBuilder builder = new SpringApplicationBuilder();


        ConfigurableApplicationContext context = builder.sources(ConditionBootstrap.class)
                .bannerMode(Banner.Mode.OFF)
                .web(WebApplicationType.NONE)
                .run(args);


        Object bean = context.getBean("helloWorld");

        System.out.println(bean);
    }

}

```

输出结果 : 

```java
Hello world
```





## 3. 走向 @EnableAutoConfiguration

首先实现一个 MyAutoConfiguration ,

这个类需要实现你想加载的Bean ,我们注入了  @Enable 模块 和  @Conditional 模块 , 其实不用配置 @Configuration ,但是源码中好多都加了  , 可以去`spring-boot-autoconfigure-2.0.4.RELEASE.jar` 这个包中有大量的自动加载的类 

```java
@Configuration
@MyConditional(key = "name",value = "key")
@EnableConfigBean
public class MyAutoConfiguration {

}
```

在resource /META-INF/spring.factories 中添加

```java
# Auto Configure  其中 \是换行符,不能有乱带空格之类的,
org.springframework.boot.autoconfigure.EnableAutoConfiguration=\
com.spring.conditionspringboot.autoconfiguration.bean.MyAutoConfiguration
```



我们的启动类 :  只要加上 @EnableAutoConfiguration 的注解轻轻松松 ,其实 @SpringBootApplication就是里面包含了@EnableAutoConfiguration  ,所以可以自动装配实现

```java
@EnableAutoConfiguration
public class AutoBootStrap {

    public static void main(String[] args) {

        SpringApplicationBuilder builder = new SpringApplicationBuilder();

        ConfigurableApplicationContext context = builder.sources(AutoBootStrap.class)
                .bannerMode(Banner.Mode.OFF)
                .web(WebApplicationType.NONE)
                .run(args);

        Object bean = context.getBean("helloWorld");

        System.out.println(bean);

        context.close();
    }

}
```

输出  

```java
Hello World
```





> 加载机制分析 :  
>
> 1  .  SpringApplication 启动的时候去找运行环境,此时就会去找SpringFactoriesLoader,  SpringFactoriesLoader 会加载 "META-INF/spring.factories" 下面的文件 
>
> 2 . org.springframework.boot.autoconfigure.EnableAutoConfiguration= 会自动加载你写的那些自动装配的自动装配类 ,例如本例子中的 com.spring.conditionspringboot.autoconfiguration.bean.MyAutoConfiguration 类 , 
>
> 3 .  第一步 条件加载 @MyConditional
>
> 4  . 第二步 模式注解 : 这个类须被 @Configuration 修饰 , 其实没必要 
>
> 5 .  @Enable 模块 :  @EnableConfigBean的自动装配的类





