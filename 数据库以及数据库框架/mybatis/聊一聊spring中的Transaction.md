# 聊一聊Spring中的 Transaction基本实现

我们知道加一个 `org.springframework.transaction.annotation.Transactional` 注解便可以让改方法实现事务管理(提交/回滚),  但是我们知道一个如何用编码的方式实现事务如何回滚的 . 抛开动态代理 , 其实事务就是动态代理实现, 但是不是我们的重点 , 我们要讲讲他为啥只支持单个数据源 还有不支持 异步操作(这里的意思是在事务里开启一个新线程执行业务)

## 前言

一般操作都是下面这样子 , 基本流程

```java
// 获取连接
Connection connection = dataSource1.getConnection();
try {
    // 设置事务隔离等级
    connection.setTransactionIsolation(3);
    // 设置自动提交为false
    connection.setAutoCommit(false);
    
    // 业务
    PreparedStatement statement = connection.prepareStatement();
    statement.execute();
    
    // 成功提交
    connection.commit();
} catch (Exception e) {    
    // 异常rollback
    connection.rollback();
} finally {
    // 设置为true
    connection.setAutoCommit(true);
    // 关闭
    connection.close();
}
```

所以Spring帮助我们做的就是这个, 那么它一定也是这么实现的 . 

如何启动事务呢 , 

首先需要加入依赖 

```xml
<dependency>
    <groupId>org.springframework</groupId>
    <artifactId>spring-tx</artifactId>
    <version>5.0.8.RELEASE</version>
</dependency>
```

SpringBoot 可以使用这个 `org.springframework.transaction.annotation.EnableTransactionManagement`  ,对于XML配置可以去看看这个类, 他会教你咋配置 . 

方法控制是依靠 `@Transactional(transactionManager = "TransactionManager")`  ,

然后我们需要自己自定义实现一个 `org.springframework.transaction.PlatformTransactionManager` Bean , 告诉spring事务处理的使用哪个

那么为啥加了个注解就会自动给你管理事务了呢 , 他难道不是这么做的吗.  

其实看 `org.springframework.boot.autoconfigure.jdbc.DataSourceTransactionManagerAutoConfiguration` 这个地方

```java
@Configuration
// 前提你只有一个数据源
@ConditionalOnSingleCandidate(DataSource.class)
static class DataSourceTransactionManagerConfiguration {
	...........
    // 这里PlatformTransactionManager前提你不实现
    @Bean
    @ConditionalOnMissingBean(PlatformTransactionManager.class)
    public DataSourceTransactionManager transactionManager(DataSourceProperties properties) {
        DataSourceTransactionManager transactionManager = new DataSourceTransactionManager(this.dataSource);
        return transactionManager;
    }
}
```

在你单数据源的情况下, 同时自动注入情况下, 他会默认给你生成一个`DataSourceTransactionManager`

所以默认情况下, 就是走这个, 现在我们绝地不看这个 . 我们要自己实现 . 

## 正文'

我们的问题 . 

```java
@Configuration
public class DataConfig {

    @Bean("datasource1")
    public DataSource dataSource() {
        try {
            Class.forName("com.mysql.jdbc.Driver");
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }
        MysqlDataSource dataSource = new MysqlDataSource();
        dataSource.setURL("jdbc:mysql://localhost:3306/jpa?useSSL=false");
        dataSource.setUser("root");
        dataSource.setPassword("123456");
        return dataSource;
    }
    // 还有一个 datasource2
    @Bean(name = "TransactionManager1")
    public DataSourceTransactionManager clusterTransactionManager(@Qualifier("datasource1") DataSource dataSource) {
        return new DataSourceTransactionManager(dataSource);
    }
    // 还有一个 TransactionManager2,地方有限
}
```

我们定义俩数据源, 其中就需要俩 `DataSourceTransactionManager`



那么写我们的service 

```java
@Service
public class TestService {
    @Autowired
    @Qualifier("datasource1")
    private DataSource dataSource1;

    @Transactional
    public void test() throws SQLException {
        Connection connection1 = dataSource1.getConnection();
        Statement statement1 = connection1.createStatement();
        int i = statement1.executeUpdate("INSERT INTO go_user (`name`,`password`) VALUES ('name12345','p5')");
        System.out.println("成功 : " + i);
        int x = 1 / 0;
        // 连接二也是如此 . 懒得写 , 我测试的时候写的俩数据源
    }
}
```

这样子启动绝对会GG , 因为你没指定有哪个`TransactionManager` . 

那么我们怎么做, 

```java
@Transactional(transactionManager = "TransactionManager2")
```

但是事务处理的时候 `org.springframework.jdbc.datasource.DataSourceTransactionManager#doRollback`

```java
@Override
protected void doRollback(DefaultTransactionStatus status) {
    DataSourceTransactionObject txObject = (DataSourceTransactionObject) status.getTransaction();
    Connection con = txObject.getConnectionHolder().getConnection();
    try {
        con.rollback();
    }
    catch (SQLException ex) {
    }
}
```

他有一个`Connection`  , 那么不和我们的connection一样 , 那么绝对回滚不了哇.  因为你不是一个连接, 所以根本题绝对提交成功了 , 那么为啥`Mybatis `可以呢 . 所以借鉴一下, 

我们看MyBatis的实现  `org.mybatis.spring.transaction.SpringManagedTransaction` 在这里, 这里缩进不了了 , 他的tab是俩空格, 我是四个, 哎 . 就这么看吧

```java
  @Override
  public Connection getConnection() throws SQLException {
      // 连接为null
    if (this.connection == null) {
        // 打开一个
      openConnection();
    }
    return this.connection;
  }
  private void openConnection() throws SQLException {
     // 获取Connection , 原来如此 这么获取
    this.connection = DataSourceUtils.getConnection(this.dataSource);
    this.autoCommit = this.connection.getAutoCommit();
    this.isConnectionTransactional = DataSourceUtils.isConnectionTransactional(this.connection, this.dataSource);
  }
```

所以就是在获取这里 `DataSourceUtils` -> `org.springframework.jdbc.datasource.DataSourceUtils#getConnection`

```java
public static Connection getConnection(DataSource dataSource) throws CannotGetJdbcConnectionException {
    try {
        return doGetConnection(dataSource);
    }
    catch (SQLException ex) {
        ///
    }
}
```

`doGetConnection(dataSource)`->`org.springframework.jdbc.datasource.DataSourceUtils#doGetConnection`

```java
public static Connection doGetConnection(DataSource dataSource) throws SQLException {
    // 获取一个 ConnectionHolder 持有对象 . 奥原来这样子 . 
    // 通过 TransactionSynchronizationManager
    ConnectionHolder  conHolder = (ConnectionHolder) TransactionSynchronizationManager.getResource(dataSource);
    if (conHolder != null && (conHolder.hasConnection() || conHolder.isSynchronizedWithTransaction())) {
        conHolder.requested();
        if (!conHolder.hasConnection()) {
            conHolder.setConnection(fetchConnection(dataSource));
        }
        return conHolder.getConnection();
    }
	/// .... 
}
```

`TransactionSynchronizationManager.getResource(dataSource);` ->   `org.springframework.transaction.support.TransactionSynchronizationManager#getResource` 

```java
@Nullable
public static Object getResource(Object key) {
    // key 
    Object actualKey = TransactionSynchronizationUtils.unwrapResourceIfNecessary(key);
    // 获取值
    Object value = doGetResource(actualKey);
    //...
    return value;
}
```

`doGetResource(actualKey);`->`org.springframework.transaction.support.TransactionSynchronizationManager#doGetResource`

```java
@Nullable
private static Object doGetResource(Object actualKey) {
    // 获取map
    Map<Object, Object> map = resources.get();
    // map拿value
    Object value = map.get(actualKey);
   	// ...
    return value;
}
```

其实 `resources` 就是啥, 就是一个 ThreadLocal 维护的一个线程对象, 

```java
private static final ThreadLocal<Map<Object, Object>> resources =
        new NamedThreadLocal<>("Transactional resources");
```

所以这就是为啥说事务不支持异步的原因了, 只能一个线程执行一个事务  , 不能俩线程执行 , 比如说这种,绝对不行.

```java
new Thread(() -> mapper.findByName("tom")).start();
```

其实到这里我们就拿到了 `ConnectionHolder`了   , `org.springframework.jdbc.datasource.ConnectionHolder`其实就是一个连接持有者, 一层封装罢了  , 多了一些额外信息. 



那么到这里他是如何加入进去的呢, 那就是 

`org.springframework.jdbc.datasource.DataSourceTransactionManager#doBegin` 这个方法了

```java
@Override
protected void doBegin(Object transaction, TransactionDefinition definition) {
    // 无所谓..
    DataSourceTransactionObject txObject = (DataSourceTransactionObject) transaction;
    Connection con = null;
    try {
        if (!txObject.hasConnectionHolder() ||
                txObject.getConnectionHolder().isSynchronizedWithTransaction()) {
            // 没有就newCon
            Connection newCon = obtainDataSource().getConnection();
            txObject.setConnectionHolder(new ConnectionHolder(newCon), true);
        }
        txObject.getConnectionHolder().setSynchronizedWithTransaction(true);
        // 获取连接
        con = txObject.getConnectionHolder().getConnection();
	
        // 获取事务隔离等级
        Integer previousIsolationLevel = DataSourceUtils.prepareConnectionForTransaction(con, definition);
        txObject.setPreviousIsolationLevel(previousIsolationLevel);
		// 自动提交关闭
        if (con.getAutoCommit()) {
            txObject.setMustRestoreAutoCommit(true);
            con.setAutoCommit(false);
        }

       // ... 这里就是保存对象了 .  
        if (txObject.isNewConnectionHolder()) {
            //  这里就是添加进去
            TransactionSynchronizationManager.bindResource(obtainDataSource(), txObject.getConnectionHolder());
        }
    }
}
```

`org.springframework.transaction.support.TransactionSynchronizationManager#bindResource`

```java
public static void bindResource(Object key, Object value) throws IllegalStateException {
    // key
    Object actualKey = TransactionSynchronizationUtils.unwrapResourceIfNecessary(key);
    Map<Object, Object> map = resources.get();
    if (map == null) {
        map = new HashMap<>();
        resources.set(map);
    }
    // 添加进去
    Object oldValue = map.put(actualKey, value);
}
```



所以前前后后就是这个, 所以懂了吗 

```java
// 通过 DataSourceUtils.getConnection(dataSource1); 获取当前线程的连接
Connection connection2 = DataSourceUtils.getConnection(dataSource1);
// 执行 ...
Statement statement1 = connection1.createStatement();
int i = statement1.executeUpdate("INSERT INTO go_user (`name`,`password`) VALUES ('name12345','p5')");
```





所以这就是  transaction 的内容 , 

## 总结

我们观察源码 发现 Transaction 不支持异步操作 ,  不支持单任务(单个请求)多数据源,  所以不能保证多数据源的事务相关问题 , 

因此下一期我讲解多数据源 (分库分表其实也是这样子) , 或者 微服务下多数据源问题 . 