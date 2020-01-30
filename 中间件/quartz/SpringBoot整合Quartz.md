# SpringBoot整合Quartz

## 1. 基于Api的方式

```java
@SpringBootApplication(exclude = DataSourceAutoConfiguration.class)
public class SpringQuartzApplication implements CommandLineRunner{

    public static void main(String[] args) {
        SpringApplication.run(SpringQuartzApplication.class, args);
    }


    @Override
    public void run(String... args) throws Exception {
        Scheduler scheduler = StdSchedulerFactory.getDefaultScheduler();
        JobDetail job = JobBuilder.newJob(MyJob.class).build();
        Trigger trigger = TriggerBuilder.newTrigger().startNow().withSchedule(SimpleScheduleBuilder.repeatHourlyForever().withIntervalInMilliseconds(1000)).build();
        scheduler.scheduleJob(job, trigger);
        scheduler.start();
    }
}
```



其实可以实现接口 `EnvironmentAware` 接口, 拿到Environment对象, 然后注入进去. 

```java
StdSchedulerFactory factory = new StdSchedulerFactory(new Properties());
Scheduler scheduler = factory.getScheduler();
```



## 2. SpringBoot处理

`org.springframework.boot.autoconfigure.quartz.QuartzAutoConfiguration` 这个类实现了自动注入的功能 . 

他会有一个 `org.springframework.scheduling.quartz.SchedulerFactoryBean` 实例化, 同时 , 在返回之前, 会设置一个拦截器类似于 `org.springframework.boot.autoconfigure.quartz.SchedulerFactoryBeanCustomizer` , 可以设置一些`SchedulerFactoryBean` 的一些信息 . .... 

首先就是 application.properties ,前缀是 spring.quartz.properties , 后缀还是原来的写法 , 

```properties
spring.quartz.properties.org.quartz.scheduler.instanceName=MyScheduler
spring.quartz.properties.org.quartz.threadPool.threadCount=4
spring.quartz.properties.org.quartz.jobStore.class=org.quartz.simpl.RAMJobStore
spring.quartz.job-store-type=memory
```

### 1. 我们用SchedulerFactoryBeanCustomizer实现注入 . 

```java
@Bean
public SchedulerFactoryBeanCustomizer schedulerFactoryBeanCustomizer() {
    return new SchedulerFactoryBeanCustomizer() {
        @Override
        public void customize(SchedulerFactoryBean schedulerFactoryBean) {

            Properties properties = new Properties();
            try {
                properties.load(Thread.currentThread().getContextClassLoader().getResourceAsStream("quartz.properties"));
            } catch (IOException e) {
                e.printStackTrace();
            }
            schedulerFactoryBean.setQuartzProperties(properties);

            JobDetail job = JobBuilder.newJob(MyQuartzJobBean.class).storeDurably().build();
            Trigger trigger = TriggerBuilder.newTrigger().forJob(job).startNow()
                    .withSchedule(SimpleScheduleBuilder.repeatHourlyForever().withIntervalInMilliseconds(1000))
                    .build();
            // 
            schedulerFactoryBean.setJobDetails(job);
            // 
            schedulerFactoryBean.setTriggers(trigger);
        }
    };
}
```

### 2. 直接注入 

```java
@Bean
public Trigger schedulerFactoryBean() {
    return TriggerBuilder.newTrigger().forJob(jobDetail()).startNow()
            .withSchedule(SimpleScheduleBuilder.repeatHourlyForever().withIntervalInMilliseconds(1000))
            .build();
}

@Bean
public JobDetail jobDetail() {
    return JobBuilder.newJob(MyQuartzJobBean.class).storeDurably().build();
}
```



这里需要注意的是 需要将 `JobDetail` 设置storeDurably()为true , 不然启动会报错

```java
org.springframework.beans.factory.BeanCreationException: Error creating bean with name 'quartzScheduler' defined in class path resource [org/springframework/boot/autoconfigure/quartz/QuartzAutoConfiguration.class]: Invocation of init method failed; nested exception is org.quartz.SchedulerException: Jobs added with no trigger must be durable.
```



## 3. QuartzJobBean

`org.springframework.scheduling.quartz.QuartzJobBean`  注入Bean, 这个很有特点, 所以特别拿出来, 

他实现了 `org.quartz.Job` 接口, 实现了特定的方法

```java
public abstract class QuartzJobBean implements Job {

	@Override
	public final void execute(JobExecutionContext context) throws JobExecutionException {
		try {
            // 这里比较核心  ... 不好理解, 但是注释可以 , This implementation applies the passed-in job data map as bean property values, and delegates to executeInternal afterwards.
			BeanWrapper bw = PropertyAccessorFactory.forBeanPropertyAccess(this);
			MutablePropertyValues pvs = new MutablePropertyValues();
			pvs.addPropertyValues(context.getScheduler().getContext());
			pvs.addPropertyValues(context.getMergedJobDataMap());
			bw.setPropertyValues(pvs, true);
		}
		catch (SchedulerException ex) {
			throw new JobExecutionException(ex);
		}
		executeInternal(context);
	}

	protected abstract void executeInternal(JobExecutionContext context) throws JobExecutionException;

}
```

`assed-in job data map as bean property values` -> 将map中的内容设置为Bean的字段属性



```java
@PersistJobDataAfterExecution
@Component
public class MyQuartzJobBean extends QuartzJobBean {

    public static final String USER_NAME = "name";

    private String name;

    public void setName(String name) {
        this.name = name;
    }

    @Override
    protected void executeInternal(JobExecutionContext context) throws JobExecutionException {
        System.out.println("Name = " + name);
        JobDataMap map = context.getJobDetail().getJobDataMap();
        map.put(USER_NAME, UUID.randomUUID().toString());
    }
}
```

输出 : 

```java
Name = 29f5aff1-52f0-47eb-be2c-a84bbe638a7a
Name = 828b594e-f918-42ca-93fe-e2321b2bf312
Name = cb268907-ba2a-4bc7-8ca0-f0facd64a8e3
Name = a3553825-2e8b-4f50-b075-0402dfb1ee7a
Name = d0a95144-50e9-49ca-97c3-5730900180d2
Name = 7c6ff206-3611-4264-915a-f188f747bdd8
```



所以还是可以的 ... 



其实这个很好用 :  就这段代码 ....

```java
BeanWrapper bw = PropertyAccessorFactory.forBeanPropertyAccess(this);
MutablePropertyValues pvs = new MutablePropertyValues();
pvs.addPropertyValues(context.getScheduler().getContext());
pvs.addPropertyValues(context.getMergedJobDataMap());
bw.setPropertyValues(pvs, true);
```

