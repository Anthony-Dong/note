# 通过Quartz如何Debug , 修复Bug

> ​	我们以Quartz的用JDBC作为JobStore为例子



开始很简单绝对是 : 

`org.quartz.spi.JobStore` 找他的接口实现类 ->  通过包名字可以发现 -> `org.quartz.impl.jdbcjobstore` 原来是JDBC , 继续  -> 有三个实现

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-10-11/0bda2ed5-9bfb-42e7-9d8d-f5d989c2fad1.png?x-oss-process=style/template01)



很简单 我们绝对要使用实现类哇, 因为抽象类是不可能靠反射生成的 . 

这哦俩区别一个支持事务, 一个不支持事务  , TX transaction就是 .  我们选择事务的. 

```properties
org.quartz.jobStore.class=org.quartz.impl.jdbcjobstore.JobStoreTX
```

继续, 然后我们这个配置好了, 咋启动哇, 它又不知道JDBC配置是啥. 因此我们看我们上面的实现类  , 然后我们没有点开 `JobStore` 找哪里使用过, 因为这个是在初始化的时候实现的, 我们就判断在 `org.quartz.impl.StdSchedulerFactory` 这里 , 进去发现, 他使用了 

```java
js = (JobStore) loadHelper.loadClass(jsClass).newInstance();
```

那么 jsClass是咋获取的 ?  原来如此... 

```java
String jsClass = cfg.getStringProperty(PROP_JOB_STORE_CLASS,RAMJobStore.class.getName());
```

我们继续呆着问题走 /..... 

下面走着就看到一句

```java
// Set up any DataSources
```

继续看这里  , PROP_DATASOURCE_PREFIX=org.quartz.dataSource

```java
String[] dsNames = cfg.getPropertyGroups(PROP_DATASOURCE_PREFIX);
```

进去 ...  这个`org.quartz.utils.PropertiesParser#getPropertyGroups`

```java
public String[] getPropertyGroups(String prefix) {
    Enumeration<?> keys = props.propertyNames();
    HashSet<String> groups = new HashSet<String>(10);

    if (!prefix.endsWith(".")) {
        prefix += ".";
    }

    while (keys.hasMoreElements()) {
        String key = (String) keys.nextElement();
        if (key.startsWith(prefix)) {
            String groupName = key.substring(prefix.length(), key.indexOf(
                    '.', prefix.length()));
            groups.add(groupName);
        }
    }

    return (String[]) groups.toArray(new String[groups.size()]);
}
```

我再这里绕了很久, 咋获取了, 其实很简单, 就是获取前缀 org.quartz.dataSource.quartz.driver的前缀就是quartz他就是获取这个. 然后set去重  , 就获取到 group了 . 其实这里处理也很好 , 第一对于properties文件来说, 他的list是一个arg[1]=? , 对于spring来说 , 我个人比较喜欢properties和xml配置文件.

其实这个工具类真心不赖, 处理很方便, 这里告诉大家一个技巧, 可以引用别人的代码 , 但是我觉得尊重原创, 就是加个作者名字, 留个链接也行 . 



我们继续往下走  ... `StdSchedulerFactory ` 925行

```java
PropertiesParser pp = new PropertiesParser(cfg.getPropertyGroup(PROP_DATASOURCE_PREFIX + "." + dsNames[i], true));
// 很简单就是获取一个去了前缀的properties
```

```java
//  connectionProvider.class -> 这里就比较坑,往下走
String cpClass = pp.getStringProperty(PROP_CONNECTION_PROVIDER_CLASS, null);

// 默认构造器实例化, 显然是不行的.  ConnectionProvider , 实现类没有这么的
 cp = (ConnectionProvider) loadHelper.loadClass(cpClass).newInstance();
```

我们就放弃了  , 此时到了 Jndi ,这个我们也不考虑, 继续

```java
String poolingProvider = pp.getStringProperty(PoolingConnectionProvider.POOLING_PROVIDER); // provider
String dsDriver = pp.getStringProperty(PoolingConnectionProvider.DB_DRIVER); //  driver
String dsURL = pp.getStringProperty(PoolingConnectionProvider.DB_URL); // URL


// 这里就是我们注意的是使用 hikaricp / c3p0 ,注意不是写全名,
// we load even these "core" providers by class name in order to avoid a static dependency on
// the c3p0 and hikaricp libraries
if(poolingProvider != null && poolingProvider.equals(PoolingConnectionProvider.POOLING_PROVIDER_HIKARICP)) {
    cpClass = "org.quartz.utils.HikariCpPoolingConnectionProvider";
}
else {
    cpClass = "org.quartz.utils.C3p0PoolingConnectionProvider";
}
```

继续

```java
Constructor constructor = loadHelper.loadClass(cpClass).getConstructor(Properties.class);
cp = (ConnectionProvider) constructor.newInstance(pp.getUnderlyingProperties());
```

继续走`HikariCpPoolingConnectionProvider`  ,这个连接池性能高  , 比C3P0强一点 , 这里很显然一堆配置

```java
public HikariCpPoolingConnectionProvider(Properties config) throws SchedulerException, SQLException {
    PropertiesParser cfg = new PropertiesParser(config);
    initialize(
            cfg.getStringProperty(DB_DRIVER),
            cfg.getStringProperty(DB_URL),
            cfg.getStringProperty(DB_USER, ""),
            cfg.getStringProperty(DB_PASSWORD, ""),
            cfg.getIntProperty(DB_MAX_CONNECTIONS, DEFAULT_DB_MAX_CONNECTIONS),
            cfg.getStringProperty(DB_VALIDATION_QUERY),
            cfg.getIntProperty(DB_DISCARD_IDLE_CONNECTIONS_SECONDS, 0));
}
```

这里就实例化好了 数据库连接池 , 



其实他的整体设计很好 , 第一我们作为存储方 , 不用管理连接方 ,所以提供了两个, 一个是 `org.quartz.spi.JobStore `  ,然后就是负责连接的是 `DBConnectionManager` , 然后这个管理者管理的是 `ConnectionProvider`  , 所以很好的解耦, 这个设计很棒 . 拓展点多 , 





其实问题还没有结束, 如果我们直接通过 ... 

```properties
org.quartz.scheduler.instanceName=MyScheduler
org.quartz.threadPool.threadCount=4
org.quartz.jobStore.class=org.quartz.impl.jdbcjobstore.JobStoreTX


org.quartz.dataSource.jpa.provider=hikaricp
org.quartz.dataSource.jpa.driver=com.mysql.jdbc.Driver
org.quartz.dataSource.jpa.URL=jdbc:mysql://localhost:3306/quartz?useSSL=false
org.quartz.dataSource.jpa.user=root
org.quartz.dataSource.jpa.password=123456
org.quartz.dataSource.jpa.maxConnections=5
```

一我们目前的认真案例来说写应该这么写, 足够了 ,因此启动 ///

结果报错, 说 : 

```java
org.quartz.SchedulerConfigException: DataSource name not set.
```

我们因此去到了 ... 

```java
public void initialize(ClassLoadHelper loadHelper,
        SchedulerSignaler signaler) throws SchedulerConfigException {

    // 为啥么设置 , 继续找
    if (dsName == null) { 
        throw new SchedulerConfigException("DataSource name not set."); 
    }
    //// .... 
}
```

我们找到字段的方法  ,  然后通过查找是否被调用, 发现没有被调用过 ? ,怎么办, 我们在这里打断点, 一定有调用, 

```java
public void setDataSource(String dsName) {
    this.dsName = dsName;
}
```

发现是在 `org.quartz.impl.StdSchedulerFactory#setBeanProps` 这里被调用的, 原来是反射哇  ,茅舍顿开, 继续

```java
tProps = cfg.getPropertyGroup(PROP_JOB_STORE_PREFIX, true, new String[] {PROP_JOB_STORE_LOCK_HANDLER_PREFIX});
// 这里调用, jobstore
setBeanProps(js, tProps);
```

很显然是 `org.quartz.jobStore` 这里设置两个 tablePrefix 和  dsName 其实还有很多可以设置 , 只要是遵循了他的使用原则 .  基本就是 比如说 `setDataSource` 我们就配置中写 , org.quartz.jobStore.dataSource , 主要第一个字母小写 . 就是这些. ...  所以问题处理了, 有些时候么有直接被调用基本上使用的是反射. 



所以我们配置文件是? 

```java
org.quartz.scheduler.instanceName=MyScheduler
org.quartz.threadPool.threadCount=4
#org.quartz.jobStore.class = org.quartz.simpl.RAMJobStore
#org.quartz.impl.jdbcjobstore.JobStoreTX
org.quartz.jobStore.class=org.quartz.impl.jdbcjobstore.JobStoreTX


// 这里才算完整了..... 
org.quartz.jobStore.tablePrefix = QRTZ_
org.quartz.jobStore.dataSource = quartz
org.quartz.dataSource.quartz.provider=hikaricp
org.quartz.dataSource.quartz.driver=com.mysql.jdbc.Driver
org.quartz.dataSource.quartz.URL=jdbc:mysql://localhost:3306/quartz?useSSL=false
org.quartz.dataSource.quartz.user=root
org.quartz.dataSource.quartz.password=123456
org.quartz.dataSource.quartz.maxConnections=5
```



表我就直接告诉大家了, 这个可以去官网找, 他不会自动建表, 像JPA那样 ,所以很坑这点, 

```java
#
# Quartz seems to work best with the driver mm.mysql-2.0.7-bin.jar
#
# PLEASE consider using mysql with innodb tables to avoid locking issues
#
# In your Quartz properties file, you'll need to set 
# org.quartz.jobStore.driverDelegateClass = org.quartz.impl.jdbcjobstore.StdJDBCDelegate
#

DROP TABLE IF EXISTS QRTZ_FIRED_TRIGGERS;
DROP TABLE IF EXISTS QRTZ_PAUSED_TRIGGER_GRPS;
DROP TABLE IF EXISTS QRTZ_SCHEDULER_STATE;
DROP TABLE IF EXISTS QRTZ_LOCKS;
DROP TABLE IF EXISTS QRTZ_SIMPLE_TRIGGERS;
DROP TABLE IF EXISTS QRTZ_SIMPROP_TRIGGERS;
DROP TABLE IF EXISTS QRTZ_CRON_TRIGGERS;
DROP TABLE IF EXISTS QRTZ_BLOB_TRIGGERS;
DROP TABLE IF EXISTS QRTZ_TRIGGERS;
DROP TABLE IF EXISTS QRTZ_JOB_DETAILS;
DROP TABLE IF EXISTS QRTZ_CALENDARS;


CREATE TABLE QRTZ_JOB_DETAILS
  (
    SCHED_NAME VARCHAR(120) NOT NULL,
    JOB_NAME  VARCHAR(200) NOT NULL,
    JOB_GROUP VARCHAR(200) NOT NULL,
    DESCRIPTION VARCHAR(250) NULL,
    JOB_CLASS_NAME   VARCHAR(250) NOT NULL,
    IS_DURABLE VARCHAR(1) NOT NULL,
    IS_NONCONCURRENT VARCHAR(1) NOT NULL,
    IS_UPDATE_DATA VARCHAR(1) NOT NULL,
    REQUESTS_RECOVERY VARCHAR(1) NOT NULL,
    JOB_DATA BLOB NULL,
    PRIMARY KEY (SCHED_NAME,JOB_NAME,JOB_GROUP)
);

CREATE TABLE QRTZ_TRIGGERS
  (
    SCHED_NAME VARCHAR(120) NOT NULL,
    TRIGGER_NAME VARCHAR(200) NOT NULL,
    TRIGGER_GROUP VARCHAR(200) NOT NULL,
    JOB_NAME  VARCHAR(200) NOT NULL,
    JOB_GROUP VARCHAR(200) NOT NULL,
    DESCRIPTION VARCHAR(250) NULL,
    NEXT_FIRE_TIME BIGINT(13) NULL,
    PREV_FIRE_TIME BIGINT(13) NULL,
    PRIORITY INTEGER NULL,
    TRIGGER_STATE VARCHAR(16) NOT NULL,
    TRIGGER_TYPE VARCHAR(8) NOT NULL,
    START_TIME BIGINT(13) NOT NULL,
    END_TIME BIGINT(13) NULL,
    CALENDAR_NAME VARCHAR(200) NULL,
    MISFIRE_INSTR SMALLINT(2) NULL,
    JOB_DATA BLOB NULL,
    PRIMARY KEY (SCHED_NAME,TRIGGER_NAME,TRIGGER_GROUP),
    FOREIGN KEY (SCHED_NAME,JOB_NAME,JOB_GROUP)
        REFERENCES QRTZ_JOB_DETAILS(SCHED_NAME,JOB_NAME,JOB_GROUP)
);

CREATE TABLE QRTZ_SIMPLE_TRIGGERS
  (
    SCHED_NAME VARCHAR(120) NOT NULL,
    TRIGGER_NAME VARCHAR(200) NOT NULL,
    TRIGGER_GROUP VARCHAR(200) NOT NULL,
    REPEAT_COUNT BIGINT(7) NOT NULL,
    REPEAT_INTERVAL BIGINT(12) NOT NULL,
    TIMES_TRIGGERED BIGINT(10) NOT NULL,
    PRIMARY KEY (SCHED_NAME,TRIGGER_NAME,TRIGGER_GROUP),
    FOREIGN KEY (SCHED_NAME,TRIGGER_NAME,TRIGGER_GROUP)
        REFERENCES QRTZ_TRIGGERS(SCHED_NAME,TRIGGER_NAME,TRIGGER_GROUP)
);

CREATE TABLE QRTZ_CRON_TRIGGERS
  (
    SCHED_NAME VARCHAR(120) NOT NULL,
    TRIGGER_NAME VARCHAR(200) NOT NULL,
    TRIGGER_GROUP VARCHAR(200) NOT NULL,
    CRON_EXPRESSION VARCHAR(200) NOT NULL,
    TIME_ZONE_ID VARCHAR(80),
    PRIMARY KEY (SCHED_NAME,TRIGGER_NAME,TRIGGER_GROUP),
    FOREIGN KEY (SCHED_NAME,TRIGGER_NAME,TRIGGER_GROUP)
        REFERENCES QRTZ_TRIGGERS(SCHED_NAME,TRIGGER_NAME,TRIGGER_GROUP)
);

CREATE TABLE QRTZ_SIMPROP_TRIGGERS
  (          
    SCHED_NAME VARCHAR(120) NOT NULL,
    TRIGGER_NAME VARCHAR(200) NOT NULL,
    TRIGGER_GROUP VARCHAR(200) NOT NULL,
    STR_PROP_1 VARCHAR(512) NULL,
    STR_PROP_2 VARCHAR(512) NULL,
    STR_PROP_3 VARCHAR(512) NULL,
    INT_PROP_1 INT NULL,
    INT_PROP_2 INT NULL,
    LONG_PROP_1 BIGINT NULL,
    LONG_PROP_2 BIGINT NULL,
    DEC_PROP_1 NUMERIC(13,4) NULL,
    DEC_PROP_2 NUMERIC(13,4) NULL,
    BOOL_PROP_1 VARCHAR(1) NULL,
    BOOL_PROP_2 VARCHAR(1) NULL,
    PRIMARY KEY (SCHED_NAME,TRIGGER_NAME,TRIGGER_GROUP),
    FOREIGN KEY (SCHED_NAME,TRIGGER_NAME,TRIGGER_GROUP) 
    REFERENCES QRTZ_TRIGGERS(SCHED_NAME,TRIGGER_NAME,TRIGGER_GROUP)
);

CREATE TABLE QRTZ_BLOB_TRIGGERS
  (
    SCHED_NAME VARCHAR(120) NOT NULL,
    TRIGGER_NAME VARCHAR(200) NOT NULL,
    TRIGGER_GROUP VARCHAR(200) NOT NULL,
    BLOB_DATA BLOB NULL,
    PRIMARY KEY (SCHED_NAME,TRIGGER_NAME,TRIGGER_GROUP),
    FOREIGN KEY (SCHED_NAME,TRIGGER_NAME,TRIGGER_GROUP)
        REFERENCES QRTZ_TRIGGERS(SCHED_NAME,TRIGGER_NAME,TRIGGER_GROUP)
);

CREATE TABLE QRTZ_CALENDARS
  (
    SCHED_NAME VARCHAR(120) NOT NULL,
    CALENDAR_NAME  VARCHAR(200) NOT NULL,
    CALENDAR BLOB NOT NULL,
    PRIMARY KEY (SCHED_NAME,CALENDAR_NAME)
);

CREATE TABLE QRTZ_PAUSED_TRIGGER_GRPS
  (
    SCHED_NAME VARCHAR(120) NOT NULL,
    TRIGGER_GROUP  VARCHAR(200) NOT NULL, 
    PRIMARY KEY (SCHED_NAME,TRIGGER_GROUP)
);

CREATE TABLE QRTZ_FIRED_TRIGGERS
  (
    SCHED_NAME VARCHAR(120) NOT NULL,
    ENTRY_ID VARCHAR(95) NOT NULL,
    TRIGGER_NAME VARCHAR(200) NOT NULL,
    TRIGGER_GROUP VARCHAR(200) NOT NULL,
    INSTANCE_NAME VARCHAR(200) NOT NULL,
    FIRED_TIME BIGINT(13) NOT NULL,
    SCHED_TIME BIGINT(13) NOT NULL,
    PRIORITY INTEGER NOT NULL,
    STATE VARCHAR(16) NOT NULL,
    JOB_NAME VARCHAR(200) NULL,
    JOB_GROUP VARCHAR(200) NULL,
    IS_NONCONCURRENT VARCHAR(1) NULL,
    REQUESTS_RECOVERY VARCHAR(1) NULL,
    PRIMARY KEY (SCHED_NAME,ENTRY_ID)
);

CREATE TABLE QRTZ_SCHEDULER_STATE
  (
    SCHED_NAME VARCHAR(120) NOT NULL,
    INSTANCE_NAME VARCHAR(200) NOT NULL,
    LAST_CHECKIN_TIME BIGINT(13) NOT NULL,
    CHECKIN_INTERVAL BIGINT(13) NOT NULL,
    PRIMARY KEY (SCHED_NAME,INSTANCE_NAME)
);

CREATE TABLE QRTZ_LOCKS
  (
    SCHED_NAME VARCHAR(120) NOT NULL,
    LOCK_NAME  VARCHAR(40) NOT NULL, 
    PRIMARY KEY (SCHED_NAME,LOCK_NAME)
);


commit;
```

