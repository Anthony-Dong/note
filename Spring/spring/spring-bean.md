# spring - bean

## ApplicationContext

> ​	Central interface to provide configuration for an application. This is read-only while the application is running. 
>
> **为应用程序提供配置的中央接口。在应用程序运行时，这是只读的，所以一旦启动我们就不能修改上下文了 .**

```java
SpringApplication #ConfigurableApplicationContext 方法: 

protected ConfigurableApplicationContext createApplicationContext() {
		Class<?> contextClass = this.applicationContextClass;
		....... 省略
		return (ConfigurableApplicationContext) BeanUtils.instantiateClass(contextClass);
}
```

所以Spring-Boot返回的 ApplicationContext 是一个 ConfigurableApplicationContext ,所以后期我们可以强转类型

## BeanDefinition

> ​	A BeanDefinition describes a bean instance, which has property values, constructor argument values, and further information supplied by concrete implementations.
>
> ​	**描述了一个bean实例，它具有属性值、构造函数参数值和由具体实现提供的进一步信息。就是一个Bean的元信息.**



## BeanDefinitionRegistry

> ​	Interface for registries that hold bean definitions, This is the only interface in Spring's bean factory packages that encapsulates registration of bean definitions. The standard BeanFactory interfaces only cover access to a fully configured factory instance.
>
> **这是Spring的bean factory包中惟一封装bean定义注册的接口。包含bean定义的注册中心的接口**

## BeanFactory

> ​	The root interface for accessing a Spring bean container. 
>
> ​	**用于访问Spring bean容器的根接口。**



## FactoryBean

> ​	Interface to be implemented by objects used within a BeanFactory which are themselves factories for individual objects. If a bean implements this interface, it is used as a factory for an object to expose, not directly as a bean instance that will be exposed itself.
>
> ​	**主要是用户来自定义实例化bean 的 , 可以自定义解析方式之类的**



## Environment

> ​	Interface representing the environment in which the current application is running. 
>
> 表示当前应用运行的环境 , 这个环境可以说是一大堆配置信息 , 包括我们的application.properties文件里的,和一堆系统的,和自定义实现的 

```java
public interface Environment extends PropertyResolver {}

```

**它继承了 PropertyResolver ,代表他可以设置 PropertyResolver ,就是一堆的配置信息 (环境信息).** 

## DefaultListableBeanFactory

> ​	Spring's default implementation of the ConfigurableListableBeanFactory and BeanDefinitionRegistry interfaces: a full-fledged bean factory based on bean definition metadata, extensible through post-processors.
>
> ​	Spring的ConfigurableListableBeanFactory和BeanDefinitionRegistry接口的默认实现 : 一个完全成熟的基于bean定义元数据的bean工厂，可通过后处理器扩展。
>
> ​	**它可以说我我们的 spring工厂 ,里面包含了所有的元信息** 
>



## EnvironmentPostProcessor

> ​	Allows for customization of the application's Environment prior to the application context being refreshed. EnvironmentPostProcessor implementations have to be registered in META-INF/spring.factories, using the fully qualified name of this class as the key.
>
> ​	**允许在刷新应用程序上下文之前自定义应用程序环境,启动的很早**必须在META-INF/spring中注册EnvironmentPostProcessor实现类，使用该类的完全限定名作为键。

```java
void postProcessEnvironment(ConfigurableEnvironment environment,SpringApplication application)

可以获取启动时的 environment 和 application  
```



## EnvironmentAware

> ​	Interface to be implemented by any bean that wishes to be notified of the Environment that it runs in.
>
> 这个接口可以被任何bean使用,希望通知env在运行中需要做什么

```java
void setEnvironment(Environment environment);
可以获取 Environment 对象 
```



## ApplicationContextInitializer

> ​	Callback interface for initializing a Spring ConfigurableApplicationContext prior to being refreshed.
>
> 用于在刷新(refresh())初始化Spring ConfigurableApplicationContext之前,的回调接口.

​	这是一个用来初始化Spring `ConfigurableApplicationContext`应用上下文的回调接口，设定的调用时机是在`ConfigurableApplicationContext#refresh()`调用之前。

```java
void initialize(C applicationContext);

可以获取 spring启动时的上下文对象 ,就是 ConfigurableApplicationContext  .
```



## ImportBeanDefinitionRegistrar

> Interface to be implemented by types that register additional bean definitions when processing @Configuration classes. Useful when operating at the bean definition level (as opposed to @Bean method/instance level) is desired or necessary.
>
> Along with @Configuration and ImportSelector, classes of this type may be provided to the @Import annotation (or may also be returned from an ImportSelector).

通常与 `@Import` 注释使用 , 下面举个栗子 

```java
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
@Documented
@Import(ApolloConfigRegistrar.class)
public @interface EnableApolloConfig {
  String[] value() default {ConfigConsts.NAMESPACE_APPLICATION};
  int order() default Ordered.LOWEST_PRECEDENCE;
}
```

```java
public class ApolloConfigRegistrar implements ImportBeanDefinitionRegistrar {

  @Override
  public void registerBeanDefinitions(AnnotationMetadata importingClassMetadata, BeanDefinitionRegistry registry) {
     // 1. importingClassMetadata 可以获取 @EnableApolloConfig 的注解的元信息 , 比如字段属性
      //  例如 : 
     importingClassMetadata.getAnnotationAttributes(EnableApolloConfig.class.getName())
       
    // 2.registry可以手动注册一个bean ,需要BeanDefinition,和类信息
     BeanDefinition beanDefinition = BeanDefinitionBuilder.genericBeanDefinition(NoRegisterService.class).getBeanDefinition();
     registry.registerBeanDefinition("noRegisterService", beanDefinition);     
  }
}
```



// 主要方法

```java
public void registerBeanDefinitions(
			AnnotationMetadata importingClassMetadata, BeanDefinitionRegistry registry);
可以获取 注解类的元信息 , 自定义注解可以
```





## BeanDefinitionRegistryPostProcessor

> ​	Extension to the standard BeanFactoryPostProcessor SPI, allowing for the registration of further bean definitions before regular BeanFactoryPostProcessor detection kicks in. 
>
> ​	扩展标准的BeanFactoryPostProcessor SPI，允许在常规的BeanFactoryPostProcessor检测开始之前注册更多的bean定义。

```java
void postProcessBeanDefinitionRegistry(BeanDefinitionRegistry registry) throws BeansException;
```

​	在应用程序上下文的标准初始化之后修改它的内部bean定义注册表。所有常规bean定义都已加载，但还没有实例化任何bean。这允许在下一个后处理阶段开始之(BeanFactoryPostProcessor)前添加更多的bean定义.



## BeanFactoryPostProcessor

> ​	Allows for custom modification of an application context's bean definitions, adapting the bean property values of the context's underlying bean factory.
>
> **允许自定义修改应用程序上下文的bean定义，调整上下文的底层bean工厂的bean属性值。** 此时已经上下文启动好了.但是bean还没有被实例化 , 可以获取bean的 BeanDefinition , 

```java
void postProcessBeanFactory(ConfigurableListableBeanFactory beanFactory) throws BeansException;
```

他的实现类 : `PropertyResourceConfigurer` 可以快速的使用,并且简单的使用





## BeanPostProcessor

> ​	Factory hook that allows for custom modification of new bean instances,
>
> **允许自定义修改新的bean实例， **可以对bean 进行一些动态的修改 , 在程序运行后 , 

```java
public interface BeanPostProcessor {
    
    default Object postProcessBeforeInitialization(Object bean, String beanName) throws BeansException {
		return bean;
	}
    
    	default Object postProcessAfterInitialization(Object bean, String beanName) throws BeansException {
		return bean;
	}
}
```



## @Import 

> Indicates one or more @Configuration classes to import.
>
> Provides functionality equivalent to the element in Spring XML. Allows for importing @Configuration classes, ImportSelector and ImportBeanDefinitionRegistrar implementations, as well as regular component classes (as of 4.2; analogous to AnnotationConfigApplicationContext.register).

指示要导入的一个或多个@Configuration类。提供与Spring XML中的<import/>元素相同的功能。允许导入@Configuration类，ImportSelector和ImportBeanDefinitionRegistrar实现，以及常规组件类(从4.2;类似于AnnotationConfigApplicationContext.register)。也就四种 .. .. . . .

