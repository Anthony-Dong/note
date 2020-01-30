# Quartz 任务调度框架

> ​	Quartz is a richly featured, open source job scheduling library that can be integrated within virtually any Java application - from the smallest stand-alone application to the largest e-commerce system.
>
> Quartz 是一个功能丰富,开源的任务调度的库, 可以和任何Java应用整合 . 

[项目地址](https://github.com/quartz-scheduler/quartz) : https://github.com/quartz-scheduler/quartz

[快速开始地址](https://github.com/quartz-scheduler/quartz/blob/master/docs/quick-start-guide.adoc) : https://github.com/quartz-scheduler/quartz/blob/master/docs/quick-start-guide.adoc

[Demo地址](https://github.com/quartz-scheduler/quartz/tree/master/docs/examples) :  https://github.com/quartz-scheduler/quartz/tree/master/docs/examples



## 1. 快速开始

```java
public class StdSchedulerTest {
    public static void main(String[] args) {
        try {
            //1. StdSchedulerFactory工厂机制加载一个Scheduler
            Scheduler scheduler = StdSchedulerFactory.getDefaultScheduler();

            // 2. JobBuilder 定义一个JobDetail , 工作信息
            JobDetail job = JobBuilder.newJob(HelloJob.class)
                    .withIdentity("job1", "group1")
                    .build();

            // 3. TriggerBuilder定义一个trigger , 触发器 , 告诉你这个Job何时触发
            Trigger trigger = TriggerBuilder.newTrigger()
                    .withIdentity("trigger1", "group1")
                    .startNow()
                    .withSchedule(SimpleScheduleBuilder.simpleSchedule()
                            .withIntervalInMilliseconds(1000)
                            .repeatForever())
                    .build();
            
            // 4.告诉Scheduler , 我这个工作需要这个trigger
            scheduler.scheduleJob(job, trigger);

            //5. 启动
            scheduler.start();

            // 这里我们先阻塞着.. 
            System.in.read();
            // 6. 关闭
            scheduler.shutdown();
        } catch (SchedulerException | IOException se) {
            se.printStackTrace();
        }
    }
}
```

其中 `com.example.springquartz.HelloJob`   需要实现`org.quartz.Job` 此接口 

```java
public class HelloJob implements Job {
    @Override
    public void execute(JobExecutionContext jobExecutionContext) throws JobExecutionException {
        JobKey key = jobExecutionContext.getJobDetail().getKey();
        System.out.printf("group  %s , name : %s ,\t", key.getGroup(), key.getName());
        System.out.printf("Thread : %s - %s\n", Thread.currentThread().getName(), "Hello Job !");
    }
}
```

启动 :  日志信息 :   会告诉你任务何时调用的. 

```java
11:12:09.682 [main] INFO org.quartz.impl.StdSchedulerFactory - Quartz scheduler 'DefaultQuartzScheduler' initialized from default resource file in Quartz package: 'quartz.properties'
11:12:09.682 [main] INFO org.quartz.impl.StdSchedulerFactory - Quartz scheduler version: 2.3.0
11:12:09.697 [main] INFO org.quartz.core.QuartzScheduler - Scheduler DefaultQuartzScheduler_$_NON_CLUSTERED started.
11:12:09.697 [DefaultQuartzScheduler_QuartzSchedulerThread] DEBUG org.quartz.core.QuartzSchedulerThread - batch acquisition of 1 triggers
11:12:09.697 [DefaultQuartzScheduler_QuartzSchedulerThread] DEBUG org.quartz.simpl.PropertySettingJobFactory - Producing instance of Job 'group1.job1', class=com.example.springquartz.HelloJob
11:12:09.713 [DefaultQuartzScheduler_QuartzSchedulerThread] DEBUG org.quartz.core.QuartzSchedulerThread - batch acquisition of 1 triggers
11:12:09.713 [DefaultQuartzScheduler_Worker-1] DEBUG org.quartz.core.JobRunShell - Calling execute on job group1.job1
group  group1 , name : job1 ,	Thread : DefaultQuartzScheduler_Worker-1 - Hello Job !
```

## 2. JobData 同步问题

可以通过 `org.quartz.JobBuilder#usingJobData(java.lang.String, java.lang.Long)` 给Job赋值参数. 对于我们这个使用的如果是静态的数据, 比如说数据一层不变, 或者说任务前后关系不是依赖的. 可以不考虑同步问题.  

```java
JobDetail job = JobBuilder.newJob(HelloJob.class)
        .withIdentity("job1", "group1")
        .usingJobData(HelloJob.START_TIME, System.currentTimeMillis())
        .build();
```

如果选择了同步, 请选择在你的Job中这俩注解  , 保持其同步关系. 

```java
@PersistJobDataAfterExecution // 这个希望刷新JobData,所以多线程环境下共享是安全
@DisallowConcurrentExecution // 这个保证其同步执行, 其实就是任务挨着任务.
```

我们对比使用一下. 不使用同步

```java
public class HelloJob implements Job {
    public static final String START_TIME = "START_TIME";

    @Override
    public void execute(JobExecutionContext jobExecutionContext) throws JobExecutionException {
        long start = jobExecutionContext.getMergedJobDataMap().getLong(START_TIME);
        long flag = System.currentTimeMillis()-start;
        System.out.printf("Thread-%d start : %s - %dms\n", tag, Thread.currentThread().getName(), System.currentTimeMillis() - start);
        sleep(2000);
        System.out.printf("Thread-%d  end  : %s - %dms\n", tag, Thread.currentThread().getName(), System.currentTimeMillis() - start);
    }
}
```

输出 :  所以他们并不会因为任务延时导致同步执行, 这些任务之间都是不互相依赖的.  

```java
Thread-15 start : DefaultQuartzScheduler_Worker-1 - 15ms
Thread-1002 start : DefaultQuartzScheduler_Worker-2 - 1002ms
Thread-2001 start : DefaultQuartzScheduler_Worker-3 - 2001ms
Thread-15  end  : DefaultQuartzScheduler_Worker-1 - 2016ms
Thread-1002  end  : DefaultQuartzScheduler_Worker-2 - 3010ms
Thread-2001  end  : DefaultQuartzScheduler_Worker-3 - 4002ms
```

但是, 当我们加入了 

```java
@DisallowConcurrentExecution
@PersistJobDataAfterExecution
public class HelloJob implements Job {
    public static final String START_TIME = "START_TIME";
    @Override
    public void execute(JobExecutionContext jobExecutionContext) throws JobExecutionException {
        long start = jobExecutionContext.getMergedJobDataMap().getLong(START_TIME);
        long flag = System.currentTimeMillis()-start;
        System.out.printf("Thread-%d start : %s - %dms\n", flag, Thread.currentThread().getName(), System.currentTimeMillis() - start);
        sleep(2000);
        System.out.printf("Thread-%d  end  : %s - %dms\n", flag, Thread.currentThread().getName(), System.currentTimeMillis() - start);
    }
}
```

输出 :  所以同步执行 , 

```java
Thread-27 start : DefaultQuartzScheduler_Worker-1 - 28ms
Thread-27  end  : DefaultQuartzScheduler_Worker-1 - 2032ms
Thread-2033 start : DefaultQuartzScheduler_Worker-2 - 2033ms
Thread-2033  end  : DefaultQuartzScheduler_Worker-2 - 4034ms
```

总结一下 , 他的功能性, 以及特性都十分的强.  可以保证其同步也可以不同步, 这里就要对比一下 . `ScheduledExecutorService` ->  `java.util.concurrent.ScheduledThreadPoolExecutor`  -> `java.util.concurrent.ScheduledExecutorService`   他的任务调度全部是同步执行的. 第二个任务必须等待第一个任务执行完毕才行.  我后面给大家展示

他的颗粒度掌握的非常的好 . 



## 4. Trigger 

### 1. CronScheduleBuilder - 支持 Cron

> ​	计划任务，是任务在约定的时间执行已经计划好的工作，这是表面的意思。在Linux中，我们经常用到 cron 服务器来完成这项工作。cron服务器可以根据配置文件约定的时间来执行特定的任务。

怎么使用?  , 当然是触发器里面了 , 我们任务调度是靠的Triggers 

// 这里是每2S执行一次

```java
Trigger trigger = TriggerBuilder.newTrigger()
        .startNow()
        .withSchedule(CronScheduleBuilder.cronSchedule("0/2 * * * * ?"))
        .build();
```

输出:

```java
DefaultQuartzScheduler_Worker-1 : Echo com.example.springquartz.EchoJob@5f02d7f6
DefaultQuartzScheduler_Worker-2 : Echo com.example.springquartz.EchoJob@1d653213
DefaultQuartzScheduler_Worker-3 : Echo com.example.springquartz.EchoJob@7d2e9543
```

其实**发现Quartz框架对于每一个任务对象来说,他不是单例的, 每次都会依靠反射生成一个**

### 2. SimpleScheduleBuilder 

> ​	简单的编程实现

```java
SimpleScheduleBuilder.simpleSchedule()
    // 2S 一次
                    .withIntervalInMilliseconds(2000)
    // 永远执行下去
                    .repeatForever()
    // 0 代表执行一次, 1代表执行两次
                //    .withRepeatCount(1)
```

### 3. JobExecutionException 控制处理异常

```java
JobExecutionException e = new JobExecutionException("exception");

//e.refireImmediately(); // 失败了可以重复执行 , This will force quartz to run this job over and over and over and over again. 
e.setUnscheduleAllTriggers(true); // 失败了立马停止trigger ,This will force quartz to shutdown this job so that it does not run again.
```

这个异常需要放到 Job中抛出去, 只能抛出这个异常才能控制是否执行

### 4. Trigger优先级 Priority

> ​	Set the Trigger's priority. When more than one Trigger have the same fire time, the scheduler will fire the one with the highest priority first.

俩任务一块触发, 优先触发优先级高的任务  . 

```java
TriggerBuilder.newTrigger()
                    .usingJobData(EchoJob.FAVORITE_COLOR,"RED")
                    .forJob("JOB")
                    .startNow()
        			// 设置优先级
                    .withPriority(0)
                    .withSchedule(CronScheduleBuilder.cronSchedule("0/2 * * * * ?"))
                    .build();
```



### 5. 误触发 

> ​	Instructs the Scheduler that upon a mis-fire situation, the SimpleTrigger wants to be fired now by Scheduler.

有几种, 我也不知道他这个有啥用 , 误触发. 