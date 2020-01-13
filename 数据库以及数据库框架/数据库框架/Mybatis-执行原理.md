# Mybatis-执行原理

## 1. 前期环境

> ​	研究原理不需要太高级的环境 , 直接参照官网Demo写 , [链接](https://mybatis.org/mybatis-3/zh/getting-started.html):https://mybatis.org/mybatis-3/zh/getting-started.html , 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/297c76b7-f864-43b6-bb5d-e37bae58ad14.jpg?x-oss-process=style/template01)

整体目录结构 . 就是这个 



## 2. 运行

```java
public class MybatisConf {


    public static void main(String[] args) throws IOException {
        // 1. 获取配置文件 , 主要目的就是为了一个 Configuration 对象的创建
        String resource = "mybatis-config.xml";
        InputStream inputStream = Resources.getResourceAsStream(resource);
        
        // 2. 拿到配置创建一个工厂
        SqlSessionFactory sqlSessionFactory = new SqlSessionFactoryBuilder().build(inputStream);
        
        // 3. 通过工厂创建一个 会话对象 SqlSession
        SqlSession sqlSession = sqlSessionFactory.openSession();
        
        // 4. 会话建立 执行SQL
        DeptMapper mapper = sqlSession.getMapper(DeptMapper.class);
        Dept dept = mapper.selectEmpByDeptId(10);
        System.out.println("dept = " + dept);
    }

}

```

### 1. SqlSessionFactoryBuilder

这是使用了建造者模式 , 可以根据不同的输入 , 创建不同的对象实例 ,主要方法就是一个  `SqlSessionFactoryBuilder#build` ,我们可以看一下,主要是返回了一个 `SqlSessionFactory` 对象

```java
public SqlSessionFactory build(InputStream inputStream, String environment, Properties properties) {
    try {
      XMLConfigBuilder parser = new XMLConfigBuilder(inputStream, environment, properties);
      return build(parser.parse());
    } catch (Exception e) {
      throw ExceptionFactory.wrapException("Error building SqlSession.", e);
    } finally {
      ErrorContext.instance().reset();
      try {
        inputStream.close();
      } catch (IOException e) {
        // Intentionally ignore. Prefer previous error.
      }
    }
  }

// 关键之处就是这一处需要一个 Configuration 对象
  public SqlSessionFactory build(Configuration config) {
    return new DefaultSqlSessionFactory(config);
  }
```

**Configuration对象**

`https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/f68ee2c7-3762-4367-a081-a9509228598d.jpg?x-oss-process=style/template01`

我们以API 的方式注入 一下试试 

```java
public static void main(String[] args) throws IOException, SQLException {

    // 1. 创建configuration 对象
    Configuration configuration = new Configuration(
            new Environment("env",
                    new JdbcTransactionFactory(),
                    new PooledDataSource("com.mysql.jdbc.Driver",
                            "jdbc:mysql://127.0.0.1:3306/tyut",
                            "root", "fhd19970516")));

    configuration.addMappers("com.mybatis.mapper");

    // 2. 拿到配置创建一个工厂
    SqlSessionFactory sqlSessionFactory = new SqlSessionFactoryBuilder().build(configuration);

    // 3. 通过工厂创建一个 会话对象 SqlSession
    SqlSession sqlSession = sqlSessionFactory.openSession();

    // 4. 会话建立 执行SQL
    DeptMapper mapper = sqlSession.getMapper(DeptMapper.class);

    mapper.selectAll();

    Dept dept = mapper.selectEmpByDeptId(10);
    System.out.println("dept = " + dept);
}

```

输出结果 , 此时需要注意 你的 mapper.xml 和你的 mapper 接口对象要放到一个包下面 , 输出一下结果 ...

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/1a14dfce-2105-416f-91a8-bfe9eb2fe79b.jpg?x-oss-process=style/template01)

```java
16:57:57.138 [main] DEBUG org.apache.ibatis.datasource.pooled.PooledDataSource - Created connection 1896294051.
16:57:57.138 [main] DEBUG org.apache.ibatis.transaction.jdbc.JdbcTransaction - Setting autocommit to false on JDBC Connection [com.mysql.jdbc.JDBC4Connection@710726a3]
16:57:57.146 [main] DEBUG com.mybatis.mapper.DeptMapper.selectAll - ==>  Preparing: select * from dept 
16:57:57.175 [main] DEBUG com.mybatis.mapper.DeptMapper.selectAll - ==> Parameters: 
16:57:57.194 [main] DEBUG com.mybatis.mapper.DeptMapper.selectAll - <==      Total: 4
16:57:57.197 [main] DEBUG com.mybatis.mapper.DeptMapper.selectEmpByDeptId - ==>  Preparing: select * from dept where deptno=? 
16:57:57.197 [main] DEBUG com.mybatis.mapper.DeptMapper.selectEmpByDeptId - ==> Parameters: 10(Integer)
16:57:57.198 [main] DEBUG com.mybatis.mapper.DeptMapper.selectEmpByDeptId - <==      Total: 1
```



其中我们看看我们config对象做了什么 ,无非就是数据源的配置 , 有了所有的配置信息一切都好说



## 3. SqlSessionFactory 对象

> ​	Creates an {@link SqlSession} out of a connection or a DataSource , 从connection 或者  DataSource  获取一个 SqlSession对象

它是一个接口  我们看一下 `DefaultSqlSessionFactory` 的默认实现类

执行`OpenSession`方法 ,其实就是创建一个 `SqlSession` 对象 , 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/bc5a460b-b400-4e2d-85ab-a52df1cfaa3a.jpg?x-oss-process=style/template01)

`openSessionFromDataSource` 方法, 创建一个`SqlSession` 对象需三个参数 `Configuration` , `Executor` , `autoCommit` ,  其中 `Executor` 是执行的关键 , 他分为几个不同的执行器 , 后续讲到

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/24b14f8f-4eed-4968-9af8-ce4f1361c94d.jpg?x-oss-process=style/template01)

```java
  public DefaultSqlSession(Configuration configuration, Executor executor, boolean autoCommit) {
    this.configuration = configuration;
    this.executor = executor;
    this.dirty = false;
    this.autoCommit = autoCommit;
  }

```

其中 finally中 执行了一个 `ErrorContext.instance().reset();` 其实就是一个异常清除

```java
  public static ErrorContext instance() {
    ErrorContext context = LOCAL.get();
    if (context == null) {
      context = new ErrorContext();
      LOCAL.set(context);
    }
    return context;
  }
```

从`ThreadLocal<ErrorContext>` 中实例一个 `ErrorContext`对象 , 防止多线程出现的并发问题, 最后从`ThreadLocal` 中移除 , 其实这一步操作我也不知道是做啥 , 后续可能会讲到

## 4. SqlSession - 核心

通过上面的 `SqlSessionFactory` 我们已经拿到了`SqlSession` 对象, 此时我们可以进行一切操作了其实

以 `DeptMapper mapper = sqlSession.getMapper(DeptMapper.class);` 这个代码为例子我们继续讲解 , 

![]( https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/272207fc-2d53-4fcf-bb72-ece0ef0ef0d8.jpg?x-oss-process=style/template01)

他会去执行 `Configuration.getMapper(type, this);` 我们知到上一步我们已经将`Configuration`传给了`SqlSession` 对象 ,我们继续往下走进去

![]( https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/cf2bc9e5-49e8-4e79-9a12-2d4f66b08f74.jpg?x-oss-process=style/template01)

它又丢给了一个 `MapperRegistry`对象 , 我们发现,他在实例化这个对象的时候已经创建了它 , 

```java
protected final MapperRegistry mapperRegistry = new MapperRegistry(this);
```

我们继续跟进

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/80455184-57b3-414f-8312-5e0db9da2f48.jpg?x-oss-process=style/template01)

第一步是 

```java
final MapperProxyFactory<T> mapperProxyFactory = (MapperProxyFactory<T>) knownMappers.get(type);
```

```java
private final Map<Class<?>, MapperProxyFactory<?>> knownMappers = new HashMap<>();
```

此时我们发现knownMappers对象什么时候创建的哇 , 此时我们应该往上看

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/d148b0b0-2fa1-4224-aa44-053d3b3f0bfb.jpg?x-oss-process=style/template01)

此时就到我们下一节了 ...............

## 4.configuration.addMappers() 方法全过程 

你还记得这行代码吗 , 我们执行了一句 `configuration.addMappers("com.mybatis.mapper");` 这个 , 当时我们一笔带过没说 , 此时我们先看这个 

![]( https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/3ec504f3-c6c9-4aaf-a9c2-f942751e286a.jpg?x-oss-process=style/template01)

我们发现了一个这个 `mapperRegistry.addMappers(packageName);`这个方法 , 其实还是调用了 `MapperRegistry`

下面我们就讲一下 `MapperRegistry` 类

此时第一步 ,  创建了一个`ResolverUtil` 类 ,我们看一下这个类可以做啥 . 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/a489f7b3-af11-4fbb-9251-a8c7b9d18722.jpg?x-oss-process=style/template01)

我们进入 源码发现他可以 

```java
The standard usage pattern for the ResolverUtil class is as follows:
   ResolverUtil<ActionBean> resolver = new ResolverUtil<ActionBean>();
   resolver.findImplementation(ActionBean.class, pkg1, pkg2);
   resolver.find(new CustomTest(), pkg1);
   resolver.find(new CustomTest(), pkg2);
   Collection<ActionBean> beans = resolver.getClasses();
```

原来就是发现一堆类对象哇 .... 其实我们可以用用的 , 这个工具类蛮好用的 ,我们可以

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/fe10a279-ac69-494c-b154-9e2987f71518.jpg?x-oss-process=style/template01)

这个工具类的作用就是获取指定包下所有的类对象 . 

所以回到我们上面的 `public void addMappers(String packageName, Class<?> superType) {_}` 方法中 , 继续执行 

<span id="jump">关键图</span >

![link](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/d1073248-df2a-4f0b-9461-0f3b90f35a3c.jpg?x-oss-process=style/template01)

其中主要关键的地方在 `knownMappers.put(type, new MapperProxyFactory<>(type));` 和 `parser.parse();` , 第一个是创建了一个代理对象的工厂 , 后期加工, 第二个是解析 , 我们跟进去第二个`parser.parse()` 方法进去

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/840ba000-05ad-4544-acde-df02418112d1.jpg?x-oss-process=style/template01)

我们先看 `MapperAnnotationBuilder#loadXmlResource();`

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/a12494b0-637c-4752-8b93-b5f68f7ff174.jpg?x-oss-process=style/template01)

解析XML的过程过于复杂化 ,其实就是将XML的信息全部存入Configuration中,方便后期使用 , 

我们继续到了 `MapperAnnotationBuilder#parse#parseStatement(method);` 方法 进入

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/1463ac0f-544e-4db2-965c-d93a1cebca39.jpg?x-oss-process=style/template01)

这个目的就是创建一个 `MappedStatement` 为每一个方法

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/afbf8aeb-86ca-4033-9940-13689ed57880.jpg?x-oss-process=style/template01)

**这个对象包含的信息很多 , 可以称之为 元信息** 



其实到这里 已经结束了 , 我们发现 `configuration.addMappers()` 的过程就是在`Configuration`中添加了`MapperStatement`  

 还有一点是 `MapperStatement` 其实一个方法 有两个映射 , 一个全限定名 ,一个只有方法名 ,后面我会讲到,这样的话可能出现两个mapper 方法重复的问题 , 后期研究 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/6dda815b-3b7c-474b-8f0e-698b27ff32b0.jpg?x-oss-process=style/template01)

 

## 5.  重新回到getMapper(Class<T> type) 上来

![]( https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/fd6a1ef6-8a39-4feb-a141-538bc689253f.jpg?x-oss-process=style/template01)

其中主要就是冲我们上面的那个  , 是否还记得[那张图](#jump),我上面写的, 在 `addMapper()` 第一步就是创建了一个代理工厂, 然后插入到了这个`knownMappers` 这个map中

我们return跟进去继续 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/62a27735-8c07-43d7-9ff3-0bdb7f202d14.jpg?x-oss-process=style/template01)

其实最后返回的是一个代理对象 , 好了就此我们回忆一下动态代理 的基本流程. ....

```java
public static Object newProxyInstance(ClassLoader loader,
                                      Class<?>[] interfaces,
                                      InvocationHandler h)
```

这里需要传入三个参数 , 

第一个是一个类加载器, 目的就是找到你需要代理的类, 其实就是**通过类加载器可以找到你的类的跟路径** , 所以他可以找到代理对象 ..... 

第二个是 被代理的接口 ,

第三个是 一个代理实现 , 我们自己实现需要继承 `InvocationHandler `接口

> ​	InvocationHandler is the interface implemented by the invocation handler of a proxy instance.
>
> InvocationHandler  是一个接口可以通过一个代理实例调用方法#invoke

他就一个方法

```java
public Object invoke(Object proxy, Method method, Object[] args)
        throws Throwable;
```

参数一 : proxy  方法所调用的代理实例

参数二 : method 代理的方法 

参数三 : args  就是参数的值

返回值 : Object

对于 Mybatis给我们执行的是什么 , 和我们人知的是有区别的

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/1430beb5-1f90-4f93-aa97-143da5ac987d.jpg?x-oss-process=style/template01)

```java
输出: 
proxy = com.proxy.DynamicProxy@776ec8df
2003749087
```

这就是 Mybatis的动态代理 ,和你认知的一样吗 ............. 好多教程都是返回一个`method.invoke(object, args);` 他用了吗,没有,我前面代码这个object指的就是接口的实例化对象,  有时候因为我们根本拿不到一个接口的实例化对象 ,mybatis也做不到, 那怎么办 , 所以取巧和我上面的一样 , 只要返回值一致就可以

**这个图就是 Mybatis的代理接口, 他做了什么 ? 我们分析一下**

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/bc4ce797-d6ec-4ed3-b7dd-cf1303c70705.jpg?x-oss-process=style/template01)

前两步, 第一个判断这个申明的方法是不是`Object`的方法 , 第二个就是因为JDK1.8引入了 default 关键字 , 此时我们不需要进行加强实现 , 我测试的 Mybatis版本有点高哇 尽然基于1.8写的 ...

如果你懂了我上面说的  , 就可以继续看下面了 , 就是所有Mybatis的执行流程了..........



## 6. MapperMethod.	() 核心执行流程

### 1. 创建一个  MapperMethod 

```java
  public MapperMethod(Class<?> mapperInterface, Method method, Configuration config) {
      // 创建 SqlCommand , 这个主要是确定方法的 Type的 
    this.command = new SqlCommand(config, mapperInterface, method);
      // 创建 MethodSignature , 这个主要是确定方法的 返回类型 
    this.method = new MethodSignature(config, mapperInterface, method);
  }
```

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/0b7cedf7-1e7a-45b7-bff6-c2c9b107570b.jpg?x-oss-process=style/template01)

这个`Type`有用的 ,所以我才要说 

### 2. 执行

我们跟进来 , 第一步就是执行选择器  , 选择合适的入口 .... 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/e9a4cb4d-f624-4116-bbbe-33eb30ee14c2.jpg?x-oss-process=style/template01)

第二步 ... 继续

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/bdb8c81a-6919-470e-ae8e-2664aee92595.jpg?x-oss-process=style/template01)

由于我展示演示效果我执行的是一个比较复杂的方法 `List<Dept> selectAll(@Param("count") int count);` 这个方法 ,返回时List 属于Many , 所以此时执行的是`result = executeForMany(sqlSession, args);` 方法,我们跟进去看看 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/5c376bd5-96da-4cdc-80ac-628b85cefc65.jpg?x-oss-process=style/template01)

此时到了 `DefaultSqlSession#selectList` 方法

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/f3f1c125-dfe2-4023-937e-857bd797a932.jpg?x-oss-process=style/template01)

此时我们发现其实 `SqlStatement`的查询其实是委托给了 `Executor`这个接口的实现类的, 一种委托代理关系,目的就是为了解耦 , 我们继续往里跟 , 执行 `executor.query()` 方法

此时进入了 `CachingExecutor#query` 方法

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/ad8284dc-3679-4a6c-aae1-27b0570a816e.jpg?x-oss-process=style/template01)

 继续往下走 ...到了 `CachingExecutor#query`重载方法,看我注释就行

![]( https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/51aaa886-ebe5-4632-add2-6757cc283174.jpg?x-oss-process=style/template01)

此时我们继续往委托的executor执行 , 其实这个委托类是 `BaseExecutor` ... 我们进到里面,此时执行的就是 `BaseExecutor#query` 方法 , 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/d1d5d275-011e-45f3-a16f-0caa217886cd.jpg?x-oss-process=style/template01)

如果缓存里没有就执行 JDBC 查询  , 执行 `BaseExecutor#queryFromDatabase`

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/2d7ce646-e515-4580-8a7a-6f242dc2fb46.jpg?x-oss-process=style/template01)

继续往下执行 `SimpleExecutor#doQuery` 方法

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/ba1bb97f-282d-40ec-b18e-f41df75aa2e5.jpg?x-oss-process=style/template01)

先看第一步的 `stmt = prepareStatement(handler, ms.getStatementLog());` 我们需要执行 ... 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/566c8c1a-1531-4e97-876b-da6eb15dc19e.jpg?x-oss-process=style/template01)

- 我们跟进去看看 我写的第一步 ...`Connection connection = getConnection(statementLog);`

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/62efa0c6-953d-483e-aa18-a074e422fa8b.png?x-oss-process=style/template01)

- 再看第二步 ... `stmt = handler.prepare(connection, transaction.getTimeout());`

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/928d6af9-2637-4ec8-979a-b3733bd131eb.jpg?x-oss-process=style/template01)

- 我们看看这里的 `statement = instantiateStatement(connection);`

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-01/4685ce00-ff74-4854-8226-207ee66eec47.jpg?x-oss-process=style/template01)

最后看一下 `SimpleExecutor#doQuery` 中的 `handler.query(stmt, resultHandler)` 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-30/cfa59981-ec81-4af2-bdb5-04fcaa7b147e.jpg?x-oss-process=style/template01)

终于到终点了 , 我们往回走 ............ 其实已经结束了 , 返回的就是 List 也对着了 ...............



## 6. 总结一下

JDBC的操作很重要 , MyBatis的设计原则也很重要 , 尽管是动态代理 ,但是他的前期环境准备使用的建造者模式, 后期使用的委托代理模式 ,都是十分重要的 , 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-01/a8cd8b65-828e-4950-9ef2-d4f21eca186d.jpg?x-oss-process=style/template01)

