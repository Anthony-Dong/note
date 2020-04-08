# Spring的 BeanDefinition

> ​	`BeanDefinition` 顾名思义就是 Bean的定义, 那么他应该包含Bean的元信息. 所以就是这个意思.  对的他就是这么个意思. 

Spring中对于BeanFactory生成的Bean全部由这个去定义的. 

我们看看Spring提供了什么的BeanDefinition

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-30-33/58357a14-f6f6-4878-8fed-b5a33e1e1446.png?x-oss-process=style/template01)

基本就是上图所示的几种. 大部分都是由 abs组成的.  另一个是内部类. 我们不考虑 , 

那么我们就学学如何使用吧. 



### `GenericBeanDefinition`

它是个通用的. 

```java
public class SringApp {

    @Data
    static class Bean {
        String name;
        int age;
    }

    public static void main(String[] args) {
        AnnotationConfigApplicationContext context = new AnnotationConfigApplicationContext();

        GenericBeanDefinition definition = new GenericBeanDefinition();
        definition.setBeanClass(Bean.class);
        definition.getPropertyValues().add("name", "xiaoli");
        definition.getPropertyValues().add("age", 1);
        // 注册.
        context.registerBeanDefinition("bean1", definition);

        context.refresh();
        Bean bean = (Bean) context.getBean("bean1");
        System.out.println(bean);
    }
}
```

其实还可以继承的 

```java
public static void main(String[] args) {
    AnnotationConfigApplicationContext context = new AnnotationConfigApplicationContext();

    GenericBeanDefinition definition = new GenericBeanDefinition();
    definition.setBeanClass(Bean.class);
    definition.getPropertyValues().add("name", "xiaoli");
    definition.getPropertyValues().add("age", 1);
    context.registerBeanDefinition("bean1", definition);

    GenericBeanDefinition definition2 = new GenericBeanDefinition();
    definition2.setParentName("bean1");
    // bean2 的属性继承了 bean1
    context.registerBeanDefinition("bean2", definition2);

    context.refresh();

    Bean bean1 = (Bean) context.getBean("bean1");
    Bean bean2 = (Bean) context.getBean("bean2");
    // 虽然是这样,但是返回的false. 因为只是继承了属性.
    System.out.println(bean1==bean2);
}
```



那么它何时加载的.  显然是在  `context.refresh();` 的时候.  在fresh中的这个方法中. 初始化non-lazy.

```java
// Instantiate all remaining (non-lazy-init) singletons.
finishBeanFactoryInitialization(beanFactory);

// 然后进入在
// Instantiate all remaining (non-lazy-init) singletons.
beanFactory.preInstantiateSingletons();

// 主要流程就是去注册Bean. 到 , 详细代码可以看看. 其实很简单的. 
```





### `RootBeanDefinition`  和 `ChildBeanDefinition`

> ​	这俩成双成对的. 你说不是吗. root节点不能有父类 ,   其中儿子节点, 必须有父类  . 用法上和上面那个没啥区别. 

```java
public static void main(String[] args) {
    AnnotationConfigApplicationContext context = new AnnotationConfigApplicationContext();

    // root
    RootBeanDefinition definition = new RootBeanDefinition();
    definition.setBeanClass(Bean.class);
    definition.getPropertyValues().add("name", "xiaoli");
    definition.getPropertyValues().add("age", 1);
    context.registerBeanDefinition("bean1", definition);

    // child
    ChildBeanDefinition definition2 = new ChildBeanDefinition("bean1");
    context.registerBeanDefinition("bean2", definition2);

    // 刷新
    context.refresh();

    Bean bean1 = (Bean) context.getBean("bean1");
    Bean bean2 = (Bean) context.getBean("bean2");
    System.out.println(bean1==bean2);
}
```



### `BeanDefinitionBuilder`  工具

> ​	很显然就是一个Builder的工具类.

```java
BeanDefinitionBuilder builder = BeanDefinitionBuilder.genericBeanDefinition(Bean.class);
// lazy 的意思是. 你需要(调用get("beanname")方法)的时候才要实例化.
builder.setLazyInit(true);
// builder.getBeanDefinition() 其实是一个 GenericBeanDefinition
context.registerBeanDefinition("bean3", builder.getBeanDefinition());
```

### `BeanDefinitionHolder`  类

> ​	很显然是一个持有者 

```java
// 三个参数: 
// beanDefinition , bean_name, bean_alias(别名的意思就是小名,可以通过小名获取)
BeanDefinitionHolder holder = new BeanDefinitionHolder(definition, "bean1", new String[]{"bean2"});
context.registerBeanDefinition(holder.getBeanName(), holder.getBeanDefinition());
for (String alias : Objects.requireNonNull(holder.getAliases())) {
    context.registerAlias(holder.getBeanName(), alias);
}
```





### `BeanDefinitionParser` 类

> ​	这个接口就是一个对象. 是spring的xml配置中 解析xml需要使用到的. parser.  其实这个返回值没卵用. 有兴趣可以看看源码 , 分析一下为啥这个返回值没啥用. 所以返回null也行. 只要注册到context中就行了. 

```java
public interface BeanDefinitionParser {
	@Nullable
    // Element , 其实就是XML的一组标签
    // ParserContext  其实就是Spring的上下文,因为xmlcontext基础了这个.
	BeanDefinition parse(Element element, ParserContext  parserContext);
}
```



### `BeanDefinitionReader`  类

> ​	这个名字 , 很显然是从什么地方读的`BeanDefinition`  的.  

其实主要就是俩. `XmlBeanDefinitionReader`  他实现了 `BeanDefinition` 接口. 

但是 `AnnotatedBeanDefinitionReader` 并没有.  

其主要实现就是. 一下三个方法. 其实也简单. 因为需要`BeanDefinitionRegistry`  ,然后拿到`Resource `去注册就行了.

```java
BeanDefinitionRegistry getRegistry();

int loadBeanDefinitions(Resource resource) throws BeanDefinitionStoreException;

int loadBeanDefinitions(Resource... resources) throws BeanDefinitionStoreException;
```



我们先看最熟悉的 `AnnotatedBeanDefinitionReader`

```java
AnnotatedBeanDefinitionReader reader = new AnnotatedBeanDefinitionReader(context);
// SringApp类作为配置类. context作为register.
reader.registerBean(SringApp.class);
```



xml那个 也是 , 拿着source去做就行了. 

```java
XmlBeanDefinitionReader reader = new XmlBeanDefinitionReader(context);
reader.loadBeanDefinitions("user.xml");
// 刷新 . 获取就行了. 
context.refresh();
// 内部实现比较麻烦. 所以自行去了解. 
User bean = context.getBean(User.class);
System.out.println(bean);
```



### `ClassPathBeanDefinitionScanner`

> ​	这是一个根据类路径进行一个加载器

```java
// 我们调用scan方法 , 其实是调用的ClassPathBeanDefinitionScanner去加载的. 
public void scan(String... basePackages) {
    Assert.notEmpty(basePackages, "At least one base package must be specified");
    this.scanner.scan(basePackages);
}
```

他会扫描. 所有组件 .  默认过滤器是一下实现. 

```java
Candidate classes are detected through configurable type filters. The default filters include classes that are annotated with Spring's @Component, @Repository, @Service, or @Controller stereotype.
```

### `BeanDefinitionRegistry`

> ​	比较核心. 毕竟是注册BeanDefinition用的. 

核心接口方法

```java
void registerBeanDefinition(String beanName, BeanDefinition beanDefinition)
    throws BeanDefinitionStoreException;
```

一般我们常见的SpringApplication 默认就是一个实现好的register. 





