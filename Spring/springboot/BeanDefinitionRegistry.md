# Spring - BeanDefinitionRegistry

Spring中保存`BeanDefinition`的一个注册中心

>Interface for registries that hold bean definitions, for example RootBeanDefinition and ChildBeanDefinition instances. 
>
>注册接口用于保存bean definitions , 
>
>Spring's bean definition readers expect to work on an implementation of this interface. Known implementors within the Spring core are DefaultListableBeanFactory and GenericApplicationContext.
>
>常见的实现接口有 `DefaultListableBeanFactory ` 和 `GenericApplicationContext` , 其实还有一个测试类`SimpleBeanDefinitionRegistry`也可以使用



## 1. Spring中如何使用BeanDefinitionRegistry

```java
@Test
public void springTest(){
    // 他实现了BeanDefinitionRegistry
    GenericApplicationContext applicationContext = new GenericApplicationContext();
	
    // 类路径扫描器
    ClassPathBeanDefinitionScanner scanner = new ClassPathBeanDefinitionScanner(applicationContext);

    scanner.scan("com.example.springbootenable.boot");

    // 注册启动事件
    applicationContext.addApplicationListener(new ApplicationListener<ContextStartedEvent>() {
        @Override
        public void onApplicationEvent(ContextStartedEvent event) {
            System.out.println("启动上下文成功");
        }
    });

    // 必须刷新,才能实现实例化
    applicationContext.refresh();
    // 发布一个启动事件
    applicationContext.start();


    /**
     * {@link org.springframework.stereotype.Component} 可以注入 , 还有他的派生注解
     */
    TestCommons testCommons = applicationContext.getBean(TestCommons.class);
    System.out.println(testCommons);


    /**
     * {@link javax.annotation.ManagedBean} 也可以注入
     */
    TestManagedBean testManagedBean = applicationContext.getBean(TestManagedBean.class);
    System.out.println(testManagedBean);

}
```

## 2. SpringBoot 开发中如何使用BeanDefinitionRegistry

springboot中默认的BeanFactory是 `org.springframework.beans.factory.support.DefaultListableBeanFactory` 这个类 . 

### 1. @Import

> 1. Indicates one or more @Configuration classes to import.
>
> 2. Provides functionality equivalent to the <import/> element in Spring XML. Allows for importing @Configuration classes, ImportSelector and ImportBeanDefinitionRegistrar implementations, as well as regular component classes (as of 4.2; analogous to AnnotationConfigApplicationContext.register).
> 3. 总结一下就是spring.xml中的`<import/>`标签的作用, 他可以使用API的方式进行注入, 主要是分为三类: `@Configuration`, `ImportSelector` , 第三类`ImportBeanDefinitionRegistrar` , 综合一下第三者功能性强一些

### 2. ImportBeanDefinitionRegistrar 注入

@Import注解可以导入一个 `com.spring.context.annotation.DubboConfigConfigurationRegistrar `接口的实现类,

```java
@Target({ElementType.TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Inherited
@Documented
@Import(DubboConfigConfigurationRegistrar.class)
@DubboComponentScan
public @interface EnableDubbo {

    @AliasFor(annotation = DubboComponentScan.class, attribute = "basePackages")
    String[] scanBasePackages() default {};

    @AliasFor(annotation = DubboComponentScan.class, attribute = "basePackageClasses")
    Class<?>[] scanBasePackageClasses() default {};
}
```

```java
public class DubboConfigConfigurationRegistrar implements ImportBeanDefinitionRegistrar {
	// BeanDefinitionRegistry 可以获取这个对象
    @Override
    public void registerBeanDefinitions(AnnotationMetadata importingClassMetadata, BeanDefinitionRegistry registry) {
		
        // 获取EnableDubbo注解上的元信息
        AnnotationAttributes attributes = AnnotationAttributes.fromMap(
                importingClassMetadata.getAnnotationAttributes(EnableDubbo.class.getName()));

        String[] scanBasePackages = (String[]) attributes.get("scanBasePackages");
        System.out.println(scanBasePackages[0]);

        ClassPathBeanDefinitionScanner scanner = new ClassPathBeanDefinitionScanner(registry);
        scanner.scan("com.spring.context.classpathbeandefinitioscanner");


        AnnotatedBeanDefinitionReader reader = new AnnotatedBeanDefinitionReader(registry);
        reader.registerBean(Util.Bean.class);

        BeanDefinition beanDefinition = BeanDefinitionBuilder.genericBeanDefinition(BuilderBean.class).getBeanDefinition();

        registry.registerBeanDefinition("builderBean", beanDefinition);
    }
}
```

## 3. BeanDefinitionBuilder

> ​	你可以叫他为一个BeanDefinition的工具类 , 可以快速构建一个bean的定义

```java
BeanDefinition beanDefinition = BeanDefinitionBuilder.genericBeanDefinition(BuilderBean.class).getBeanDefinition();

// BeanDefinitionRegistry 注入一个Bean的定义
registry.registerBeanDefinition("builderBean", beanDefinition);
```

## 4. ClassPathBeanDefinitionScanner

> ​	`org.springframework.context.annotation.ClassPathBeanDefinitionScanner` , 
>
> ​	A bean definition scanner that detects bean candidates on the classpath, registering corresponding bean definitions with a given registry (BeanFactory or ApplicationContext).
>
> ​	一个bean definition扫描器,可以在classpath路径下检测Bean候选人 , 将它注册到给定一个`BeanFactory `或者`ApplicationContext`

可以扫描带着`org.springframework.stereotype.Component`注解和 `javax.annotation.ManagedBean`他的类

```java
ClassPathBeanDefinitionScanner scanner = new ClassPathBeanDefinitionScanner(applicationContext);
scanner.scan("com.example.springbootenable.boot");
```



## 5. AnnotatedBeanDefinitionReader

> ​	Convenient adapter for programmatic registration of annotated bean classes. This is an alternative to ClassPathBeanDefinitionScanner, applying the same resolution of annotations but for explicitly registered classes only.
>
> ​	可以很方便的适配类,可以编程的方式注册带有注解的Bean , 这是`ClassPathBeanDefinitionScanner`的另一种选择，它应用了相同的注释解析，但只针对显式注册的类。

```java
AnnotatedBeanDefinitionReader reader = new AnnotatedBeanDefinitionReader(registry);
reader.registerBean(Util.Bean.class);
```

这里的`Util.Bean.class` 的类 , 此时可以通过这样子实现注册 ,但是他还不是重点 ,其中 `@AutoBeans`才是

```java
public class Util {

    @AutoBeans
    public static class Bean{

        @Override
        public String toString() {
            return "Util.Bean";
        }
    }
}
```

`@AutoBeans` 我们可以这么做 , 此时我们发现... 他可以使用 `@Import`注入一个Bean

```java
@Target({ElementType.TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Inherited
@Documented
@Import(ReaderBean.class)
public @interface AutoBeans {

}
```

```java
public class ReaderBean {

    @Override
    public String toString() {
        return "ReaderBean";
    }
}
```

总结一下这个工具类 , 他可以实现注解方式注入Bean, 其中Dubbo在做springboot整合的时候就用的这个



