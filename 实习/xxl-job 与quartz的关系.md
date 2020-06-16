# xxl-job和quartz的关系

## 问题

1、xxl-job 如何搭建集群，首先从设计架构上，它可以说是无server状态，也就是谁也可以是server，类似于我们项目中这种结构，每一服务都会去执行，但是因为redis分布式锁执行了一个，所以它也是可以把xxl-job想做是我们的服务节点，而我们的任务现在在我们的其他服务器上，它回去注册到xxl-job上，告诉地址就可以了。

那么有一种情况，我们在xxl-job-a上启动了X任务，如果此时a机器宕机了，此时b机器是不知道的，那么b机器如何继续运行X任务，这个就是quartz框架的魅力了，它回去解决这个问题，因为抽离了task层到db中，具体可以看[我的quartz文章](https://anthony-dong.github.io/post/yaoZolVYC/)，这里讲了为啥它这么设计。

2、xxl-job 我个人认为就是将quartz框架的注册层面抽离出来，同时将任务可视化，说白了就是做了个可视化界面，内部核心执行逻辑依赖于quartz

3、下一步就是研究quartz如何解决分布式调度的



## quartz 分布式调度

首先看看官方的一段话：

#### Quartz可以运行多少个作业？

这是一个很难回答的问题...答案基本上是``取决于''的。

我知道您讨厌这个答案，因此这里有一些有关它“取决于”的信息。

首先，您使用的JobStore起着重要的作用。基于RAM的JobStore比基于JDBC的JobStore快了（1000倍）。JDBC-JobStore的速度几乎完全取决于与数据库的连接速度，所使用的数据库系统以及数据库所运行的硬件。Quartz实际上很少进行处理，几乎所有时间都花在数据库中。当然，RAMJobStore对可以存储的作业和触发器的数量有更有限的限制，因为您肯定要拥有比数据库的硬盘空间少的RAM。您还可以查看常见问题解答“如何改善JDBC-JobStore的性能？”。

因此，触发器“石英”和“作业石英”可以“存储”和监视的数量的限制因素实际上是JobStore可用的存储空间量（RAM量或磁盘空间量）。

现在，除了“我可以存储多少？” 问题是“ Quartz可以同时运行多少个作业？”

可以降低石英本身速度的一件事是使用大量的侦听器（TriggerListeners，JobListeners和SchedulerListeners）。在每个侦听器中花费的时间显然会增加到“处理”作业执行的时间，而不是实际执行作业。这并不意味着您应该害怕使用侦听器，而只是意味着您应该明智地使用它们-如果您确实可以创建更专业的侦听器，则不要创建一堆“全局”侦听器。除非确实需要，否则也不要在侦听器中执行“昂贵”的操作。还请注意，许多插件（例如“ history”插件）实际上是侦听器。

随时可以运行的实际作业数受线程池大小的限制。如果池中有五个线程，则一次最多只能运行五个作业。但是，请注意不要创建大量线程，因为JVM，操作系统和CPU都很难处理大量线程，并且由于系统花费时间管理线程而导致性能下降。在大多数情况下，当您进入数百个线程时，性能就会开始下降（如果在线程上执行的代码非常密集，则性能会下降）。请注意，如果您在应用程序服务器中运行，它可能至少创建了几十个自己的线程！

除了这些因素，这实际上还取决于您的工作。如果您的工作需要很长时间才能完成工作，并且/或者他们的工作非常占用CPU，那么显然您将无法一次运行很多工作，也不会在给定的时间内运行很多工作。

最后，如果您无法从一个Quartz实例中获得足够的功率，则可以始终对多个Quartz实例进行负载平衡（在单独的计算机上）。每个人都将按照触发的先后顺序，以先到先得的方式在共享数据库中运行作业。

群集功能最适合扩展长期运行和/或CPU密集型作业（在多个节点上分配工作负载）。如果需要扩展以支持数千个短期运行（例如1秒）的作业，请考虑使用多个不同的调度程序对作业集进行分区。使用一个调度程序会强制使用群集范围的锁定，这种模式会在您添加更多客户端时降低性能。

因此，这里您对“有多少个”的答案还很遥远，而我仍然没有给您实际的数字。由于上述所有变量，我真的很讨厌。因此，我只想说说，有一部分Quartz正在管理成千上万的作业和触发器，并且在任何给定时间执行数十个作业-甚至没有利用Quartz的负载平衡功能。考虑到这一点，大多数人应该对自己可以从所需的Quartz中获得性能充满信心。

#### 如何提高JDBC-JobStore的性能？

有几种已知的方法可以加快JDBC-JobStore的速度，只有一种非常实用。

首先，显而易见但不实际的是：

- 在运行Quartz的计算机和运行RDBMS的计算机之间购买更好（更快）的网络。
- 购买一台更好（功能更强大）的计算机来运行您的数据库。
- 购买更好的RDBMS。

现在，执行简单但有效的操作：在Quartz表上建立索引。

大多数数据库系统会自动将索引放在主键字段上，许多数据库系统也会自动为外键字段添加索引。确保您执行此操作，或者手动在每个表的所有关键字段上建立索引。

接下来，手动添加一些其他索引：对索引最重要的是TRIGGER表的“ next_fire_time”和“ state”字段。最后（但不是很重要），将索引添加到FIRED_TRIGGERS表上的每一列。

```
在qrtz_triggers（NEXT_FIRE_TIME）上创建索引idx_qrtz_t_next_fire_time;
在qrtz_triggers（TRIGGER_STATE）上创建索引idx_qrtz_t_state；
在qrtz_triggers（TRIGGER_STATE，NEXT_FIRE_TIME）上创建索引idx_qrtz_t_nf_st;
在qrtz_fired_triggers（TRIGGER_NAME）上创建索引idx_qrtz_ft_trig_name；
在qrtz_fired_triggers（TRIGGER_GROUP）上创建索引idx_qrtz_ft_trig_group；
在qrtz_fired_triggers（TRIGGER_NAME）上创建索引idx_qrtz_ft_trig_name；
在\上创建索引idx_qrtz_ft_trig_n_g
    qrtz_fired_triggers（TRIGGER_NAME，TRIGGER_GROUP）;
在qrtz_fired_triggers（INSTANCE_NAME）上创建索引idx_qrtz_ft_trig_inst_name；
在qrtz_fired_triggers（JOB_NAME）上创建索引idx_qrtz_ft_job_name；
在qrtz_fired_triggers（JOB_GROUP）上创建索引idx_qrtz_ft_job_group
在\上创建索引idx_qrtz_t_next_fire_time_misfire
    qrtz_triggers（MISFIRE_INSTR，NEXT_FIRE_TIME）;
在\上创建索引idx_qrtz_t_nf_st_misfire
    qrtz_triggers（MISFIRE_INSTR，NEXT_FIRE_TIME，TRIGGER_STATE）;
在\上创建索引idx_qrtz_t_nf_st_misfire_grp
    qrtz_triggers（MISFIRE_INSTR，NEXT_FIRE_TIME，TRIGGER_GROUP，TRIGGER_STATE）;
```

群集功能最适合扩展长期运行和/或CPU密集型作业（在多个节点上分配工作负载）。如果您需要扩展以支持数千个短期运行（例如1秒）的作业，请考虑使用多个不同的调度程序（因此使用多个表集（具有不同的前缀））对作业集进行分区。使用一个调度程序会强制使用群集范围的锁定，这种模式会在您添加更多客户端时降低性能。

1. *如果重新启动数据库服务器，我的数据库连接将无法正确恢复。*

   如果您要使用Quartz创建连接数据源（通过在quartz properties文件中指定连接参数），请确保指定了连接验证查询，例如：

   ```
   org.quartz.dataSource.myDS.validationQuery =从双选0
   ```

   对于Oracle，此特定查询非常有效。对于其他数据库，您需要考虑一个有效的查询，只要连接良好即可，该查询始终有效。

   如果您的数据源是由应用程序服务器管理的，请确保以能够检测到失败连接的方式配置数据源。





### quartz的mysql核心逻辑

org.quartz.impl.jdbcjobstore.StdJDBCDelegate#selectTriggerToAcquire

大致流程:

获取trigger 

```java
triggers = qsRsrcs.getJobStore().acquireNextTriggers(now + idleWaitTime, Math.min(availThreadCount, qsRsrcs.getMaxBatchSize()), qsRsrcs.getBatchTimeWindow());
```

进去这里：

```java
public List<OperableTrigger> acquireNextTriggers(final long noLaterThan, final int maxCount, final long timeWindow)
    throws JobPersistenceException {
    String lockName;
    if(isAcquireTriggersWithinLock() || maxCount > 1) { 
        lockName = LOCK_TRIGGER_ACCESS;
    } else {
        lockName = null;
    }
  // 关键在这里
    return executeInNonManagedTXLock(lockName, 
            new TransactionCallback<List<OperableTrigger>>() {
               // 。。
            },
            new TransactionValidator<List<OperableTrigger>>() {
              // 。。
            });
}
```

然后关键是这个执行锁

```java
protected <T> T executeInNonManagedTXLock(
        String lockName, 
        TransactionCallback<T> txCallback, final TransactionValidator<T> txValidator) throws JobPersistenceException {
    boolean transOwner = false;
    Connection conn = null;
    try {
        if (lockName != null) {
            // If we aren't using db locks, then delay getting DB connection 
            // until after acquiring the lock since it isn't needed.
            if (getLockHandler().requiresConnection()) {
              // 这个锁，获取同步锁
                conn = getNonManagedTXConnection();
            }

		// 。。。 
}
```



```java
protected Connection getConnection() throws JobPersistenceException {
    Connection conn;
    try {
        conn = DBConnectionManager.getInstance().getConnection(
                getDataSource());

        if (!isDontSetAutoCommitFalse()) {
            conn.setAutoCommit(false);
        }

        if(isTxIsolationLevelSerializable()) {
          // TRANSACTION_SERIALIZABLE 事物隔离级别
            conn.setTransactionIsolation(Connection.TRANSACTION_SERIALIZABLE);
        }

    return conn;
}
```

sql 执行

```java
String SELECT_NEXT_TRIGGER_TO_ACQUIRE = "SELECT "
    + COL_TRIGGER_NAME + ", " + COL_TRIGGER_GROUP + ", "
    + COL_NEXT_FIRE_TIME + ", " + COL_PRIORITY + " FROM "
    + TABLE_PREFIX_SUBST + TABLE_TRIGGERS + " WHERE "
    + COL_SCHEDULER_NAME + " = " + SCHED_NAME_SUBST
    + " AND " + COL_TRIGGER_STATE + " = ? AND " + COL_NEXT_FIRE_TIME + " <= ? " 
    + "AND (" + COL_MISFIRE_INSTRUCTION + " = -1 OR (" +COL_MISFIRE_INSTRUCTION+ " != -1 AND "+ COL_NEXT_FIRE_TIME + " >= ?)) "
    + "ORDER BY "+ COL_NEXT_FIRE_TIME + " ASC, " + COL_PRIORITY + " DESC";
```



所以获取trigger的时候， 是可以保证同步的。但是也有并发问题，事物隔离级别是`SERIALIZABLE`  很差的效果。



释放trigger

```java
qsRsrcs.getJobStore().releaseAcquiredTrigger(triggers.get(i));
```

完成,这个是异常，释放

```java
qsRsrcs.getJobStore().triggeredJobComplete(triggers.get(i), bndle.getJobDetail(), CompletedExecutionInstruction.SET_ALL_JOB_TRIGGERS_ERROR);
```

