# xxl-job 与 go-cron的源码学习

## 单机 go-cron

> ​	cron 是一个任务调度常用的表达是，由七位组成，但是我们只需要写6位即可，省略年份，其中详细的讲解可以自行百度。

### 源码分析

首先，项目中使用的是2.0版本好像，对于3.0 改进了下面我说的很多问题。

```go
	c := cron.New()
	c.AddFunc("0 01 0 * * *", func)
	c.Start()
```

很简单，无非就是三行，一行创建，一行注册，一行启动。

```go
// New returns a new Cron job runner, in the Local time zone.
func New() *Cron {
	return NewWithLocation(time.Now().Location())
}

// NewWithLocation returns a new Cron job runner.
func NewWithLocation(location *time.Location) *Cron {
	return &Cron{
		entries:  nil,
    // 添加信号 ， 在运行时使用
		add:      make(chan *Entry),
    // 停止信号
		stop:     make(chan struct{}),
    // 创建副本信号
		snapshot: make(chan []*Entry),
    // 启动标识
		running:  false,
		ErrorLog: nil,
		location: location,
	}
}
```



基本根据结构体就知道，大致是一个chan，

```go
// AddFunc adds a func to the Cron to be run on the given schedule.
func (c *Cron) AddFunc(spec string, cmd func()) error {
  // 添建任务
	return c.AddJob(spec, FuncJob(cmd))
}

// AddJob adds a Job to the Cron to be run on the given schedule.
func (c *Cron) AddJob(spec string, cmd Job) error {
  // 解析schedule ， 其实就是解析我们的cron表达式
	schedule, err := Parse(spec)
	if err != nil {
		return err
	}
  // 
	c.Schedule(schedule, cmd)
	return nil
}
```



其实对于parse 这个流程，网上所有的解析cron的做法几乎一模一样，所以不做解释了，可以看看quartz 、 go-cron 、 xxl-job-2.2.x 基本都是一样的， 大致就是解析一下

其中schedule 是啥

```go
type SpecSchedule struct {
	Second, Minute, Hour, Dom, Month, Dow uint64
}
```

看一下功能， 其实这个就是获取一下时间罢了，无非就是根据cron ，cur-time 这两个值计算出下一次的运行时间。计算逻辑百度可以看一下。

```go
type Schedule interface {
	// Return the next activation time, later than the given time.
	// Next is invoked initially, and then each time the job is run.
	Next(time.Time) time.Time
}
```

这里我们知道了 schedule 是一个计时器， 也就是trigger， 



其次就是看`c.Schedule(schedule, cmd)` ,  它是做啥的呢？ 其实就是添加到我们的任务队列中，将每一个任务封装，主要是由job 和 trigger组成。

```go
func (c *Cron) Schedule(schedule Schedule, cmd Job) {
  // 创建一个 job + trigger 
	entry := &Entry{
		Schedule: schedule,
		Job:      cmd,
	}
  // 如果没有启动，直接添加到队列（数组）中
	if !c.running {
		c.entries = append(c.entries, entry)
		return
	}

  // 启动了， 就靠chan 进行通知（其实go的chan特别像Java的blockqueue），阻塞的一个个添加进去
	c.add <- entry
}
```



其次就是看启动事件， Start（） 启动 ，主要是修改状态量，以及运行

```go
func (c *Cron) Start() {
  // 逻辑判断 ， go里面好像不需要管可见性，Java的volatile。可能是它的tripper（goroutine）是基于线程搭建的。
	if c.running {
		return
	}
  // 修改状态，这里竟然没有加锁，无语，万一有两个同时执行呢？
	c.running = true
  // 启动
	go c.run()
}
```



启动运行，run方法

```go
// Run the scheduler. this is private just due to the need to synchronize
// access to the 'running' state variable.
func (c *Cron) run() {
	// Figure out the next activation times for each entry.
  // cur-time
	now := c.now()
  // 全部去拉取计算一下-  触发时间
	for _, entry := range c.entries {
		entry.Next = entry.Schedule.Next(now)
	}

  // 死循环，不断轮训
	for {
		// Determine the next entry to run.
    // 根据触发时间进行排序，最先触发的排在前面
		sort.Sort(byTime(c.entries))

		var timer *time.Timer
    // 如果是空的话，防止下面空转，所以直接wait了。还很久
		if len(c.entries) == 0 || c.entries[0].Next.IsZero() {
			// If there are no entries yet, just sleep - it still handles new entries
			// and stop requests.
			timer = time.NewTimer(100000 * time.Hour)
		} else {
      // 创建一个计时器，go的计时器，可以通过chan发送信号
			timer = time.NewTimer(c.entries[0].Next.Sub(now))
		}

    // 选择触发，注意break，continue，return这几个的用法。
    // break  是跳出select分支，但是分支阻塞 , 这里是在select外部，所以是停止当前循环
    // defaule 是分支不阻塞，
    // continue 继续循环
		for {
			select {
			case now = <-timer.C:
				now = now.In(c.location)
				// Run every entry whose next time was less than now
        // 运行每一个task的trigger时间，小于当前时间。
				for _, e := range c.entries {
          // 1.判断trigger是否在当前时间之后，是的话，break，为啥不是continue，
					if e.Next.After(now) || e.Next.IsZero() {
						break
					}
          // 如果trigger在当前时间之前，所以就运行了
					go c.runWithRecovery(e.Job)
          // pre记录上次执行时间 ，next记录下次执行时间
					e.Prev = e.Next
					e.Next = e.Schedule.Next(now)
				}

        // 添加job， 运行时添加
			case newEntry := <-c.add:
        // 这个不理解，为啥stop，网上说，如果stop，timer的chan是不会接受到消息的，导致一个chan被死锁了，但是这里的话，执行完这个，直接break了，跳出这个for循环了。
        // 这里的做法其实隐式的做了一个同步工作
				timer.Stop()
        // 添加进去
				now = c.now()
				newEntry.Next = newEntry.Schedule.Next(now)
				c.entries = append(c.entries, newEntry)

			case <-c.snapshot:
        // 复制了一个当前的副本
				c.snapshot <- c.entrySnapshot()
				continue

			case <-c.stop:
        // 停止直接stop
				timer.Stop()
				return
			}

			break
		}
	}
}
```





其次就是看程序运行逻辑了， 

```go
func (c *Cron) runWithRecovery(j Job) {
  // 记录panic 
	defer func() {
		if r := recover(); r != nil {
			const size = 64 << 10
			buf := make([]byte, size)
			buf = buf[:runtime.Stack(buf, false)]
			c.logf("cron: panic running job: %v\n%s", r, buf)
		}
	}()
  // 我们的func转换成了job
	j.Run()
}
```



### 单机存在的问题

那么分布式任务调度中心，需要做啥 ？ 

- 上面我们手动注册任务 ， 无法灵活控制
- 其次比如10台机器， 每台机器都执行运行，那么无法做到单个执行，业务中加入了分布式锁，
-  其次是执行脚本是一个很消耗资源的东西，那么就算可以单个执行，也是随机的，无法做到一个选择性的调用，比如这个机器负载低，选择这台机器调用 
- 其次就是调度策略的选择，可能是执行几台，可能是需要动态调参等，也能修改脚本（go这种编译性语言基本无缘）
- 脚本的控制能力差，写死了，无法动态修改，应急能力差，比如时间段，我可以只执行一次脚本等。
- 日志记录差，需要第三方记录脚本运行日志



## xxl-job 

首先抛弃它所说的， 一个分布式调度中心， 我们这里只关注它分布式的能力，官网的图我也放一下。那么对比go-cron的实现，究竟区别在哪

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-40-44/dad3e196-634c-4077-9350-f6aca3f0747e.png)

对于一个分布式调度中心， 主要核心的是任务管理，执行器管理，日志管理，其他，数据中心。

任务管理，也就是task注册在我们这里 -> 服务注册

执行期管理，就是调度运行资源 -> 服务注册

日志管理，记录调度日志 -> 日志监控

其他服务-> 任务检测 ， 脚本执行等。

| 组件                       | go-cron                | xxl-job-2.0.x                                            | xxl-job-2.2.x                                            |
| -------------------------- | ---------------------- | -------------------------------------------------------- | -------------------------------------------------------- |
| 调度器                     | go-cron的run方法       | quartz框架                                               | 自己写的一个调度逻辑                                     |
| 日志监控                   | 无，手动记录           | 有，mysql记录，无结构化日志                              | 有，mysql记录，无结构化日志                              |
| 任务检测（执行时间等记录） | 无，手动记录           | 有，rpc回掉                                              | 有，http轮训                                             |
| 注册服务                   | 硬编码注册             | 有，基于http方式，注册到quartz，然后存储到mysql          | 有，基于http方式，注册到调度线程，然后存储到mysql        |
| 动态脚本                   | 无，编译性语言无法实现 | 有，基于Groovy框架，可以运行时编译Java文件，编译成字节码 | 有，基于Groovy框架，可以运行时编译Java文件，编译成字节码 |



### 2.0.X版本的架构

2.0.x版本， 主要是封装了quartz框架，也就是xxl-job的本地，也就是我们的调度中心，本身就是一个quartz框架，在调度中心统一调度资源，数据存储主要是在mysql，比如说日志，资源地址，任务详情等

#### 源码 

> ​	源码挺简单的

首先我们找到入口，所有的job，都是在 jobinfo这个路由下面, 主要有 add 、 update 、 remove 、 stop 、start、 trigger四个操作。

先看运行逻辑吧。

```java
	@Override
	public ReturnT<String> start(int id) {
    // 获取job-info，查询数据库
		XxlJobInfo xxlJobInfo = xxlJobInfoDao.loadById(id);
		String name = String.valueOf(xxlJobInfo.getId());
		String cronExpression = xxlJobInfo.getJobCron();

		try {
      // 添加job
			boolean ret = XxlJobDynamicScheduler.addJob(name, cronExpression);
			return ret?ReturnT.SUCCESS:ReturnT.FAIL;
		} catch (SchedulerException e) {
			logger.error(e.getMessage(), e);
			return ReturnT.FAIL;
		}
	}
```



下面部分的业务逻辑，就是涉及到quartz框架了， [https://anthony-dong.gitee.io/search/index.html?q=quartz](https://anthony-dong.gitee.io/search/index.html?q=quartz)  这个我博客里以前写了几片文章关于quartz的

```java
	public static boolean addJob(String jobName, String cronExpression) throws SchedulerException {
    	// 1、job key
    //  job 和 key的一个双向绑定的过程
        TriggerKey triggerKey = TriggerKey.triggerKey(jobName);
        JobKey jobKey = new JobKey(jobName);

        // 2、valid
        if (scheduler.checkExists(triggerKey)) {
            return true;    // PASS
        }

        // 3、corn trigger，创建trigger
        CronScheduleBuilder cronScheduleBuilder = CronScheduleBuilder.cronSchedule(cronExpression).withMisfireHandlingInstructionDoNothing();   // withMisfireHandlingInstructionDoNothing 忽略掉调度终止过程中忽略的调度
        CronTrigger cronTrigger = TriggerBuilder.newTrigger().withIdentity(triggerKey).withSchedule(cronScheduleBuilder).build();

        // 4、job detail ， 这个RemoteHttpJobBean 实现了 QuartzJobBean ，会触发trigger，然后还是trigger的流程
		Class<? extends Job> jobClass_ = RemoteHttpJobBean.class;   // Class.forName(jobInfo.getJobClass());
		JobDetail jobDetail = JobBuilder.newJob(jobClass_).withIdentity(jobKey).build();

        /*if (jobInfo.getJobData()!=null) {
        	JobDataMap jobDataMap = jobDetail.getJobDataMap();
        	jobDataMap.putAll(JacksonUtil.readValue(jobInfo.getJobData(), Map.class));	
        	// JobExecutionContext context.getMergedJobDataMap().get("mailGuid");
		}*/
        
        // 5、schedule job ， 这里是使用的quartz框架，job+trigger
        Date date = scheduler.scheduleJob(jobDetail, cronTrigger);

        logger.info(">>>>>>>>>>> addJob success(quartz), jobDetail:{}, cronTrigger:{}, date:{}", jobDetail, cronTrigger, date);
        return true;
    }

```



所以start流程，就是注册到quartz中 ， 运行流程我们看看com.xxl.job.admin.core.thread.JobTriggerPoolHelper#addTrigger , 其实quartz也有线程池，这里也使用了线程池，我觉得没啥必要，配置的时候可以将quarzt那个配小点，几个就可以主要是负责调度，不执行业务逻辑

```java
public void addTrigger(final int jobId, final TriggerTypeEnum triggerType, final int failRetryCount, final String executorShardingParam, final String executorParam) {

    // choose thread pool，它这里有个快慢线程池，不懂，可能是想让短资源使用快线程池
    ThreadPoolExecutor triggerPool_ = fastTriggerPool;
    AtomicInteger jobTimeoutCount = jobTimeoutCountMap.get(jobId);
    if (jobTimeoutCount!=null && jobTimeoutCount.get() > 10) {      // job-timeout 10 times in 1 min
        triggerPool_ = slowTriggerPool;
    }

    // trigger
    triggerPool_.execute(new Runnable() {
        @Override
        public void run() {

            long start = System.currentTimeMillis();

            try {
                // do trigger ， 核心流程
                XxlJobTrigger.trigger(jobId, triggerType, failRetryCount, executorShardingParam, executorParam);
            } catch (Exception e) {
                logger.error(e.getMessage(), e);
            } finally {

                // check timeout-count-map
                long minTim_now = System.currentTimeMillis()/60000;
                if (minTim != minTim_now) {
                    minTim = minTim_now;
                    jobTimeoutCountMap.clear();
                }

                // incr timeout-count-map
                long cost = System.currentTimeMillis()-start;
                if (cost > 500) {       // ob-timeout threshold 500ms
                    AtomicInteger timeoutCount = jobTimeoutCountMap.put(jobId, new AtomicInteger(1));
                    if (timeoutCount != null) {
                        timeoutCount.incrementAndGet();
                    }
                }

            }

        }
    });
}

```



```java
public static void trigger(int jobId, TriggerTypeEnum triggerType, int failRetryCount, String executorShardingParam, String executorParam) {
    // 查询数据库，看看jobinfo
    XxlJobInfo jobInfo = XxlJobAdminConfig.getAdminConfig().getXxlJobInfoDao().loadById(jobId);
    if (jobInfo == null) {
        logger.warn(">>>>>>>>>>>> trigger fail, jobId invalid，jobId={}", jobId);
        return;
    }
    if (executorParam != null) {
        jobInfo.setExecutorParam(executorParam);
    }
  // 失败的重试次数
    int finalFailRetryCount = failRetryCount>=0?failRetryCount:jobInfo.getExecutorFailRetryCount();
    XxlJobGroup group = XxlJobAdminConfig.getAdminConfig().getXxlJobGroupDao().load(jobInfo.getJobGroup());

    // 运行参数
    int[] shardingParam = null;
    if (executorShardingParam!=null){
        String[] shardingArr = executorShardingParam.split("/");
        if (shardingArr.length==2 && isNumeric(shardingArr[0]) && isNumeric(shardingArr[1])) {
            shardingParam = new int[2];
            shardingParam[0] = Integer.valueOf(shardingArr[0]);
            shardingParam[1] = Integer.valueOf(shardingArr[1]);
        }
    }
    // 这里有个分片的策略，我也不了解这个策略，所以我们看调度
    if (ExecutorRouteStrategyEnum.SHARDING_BROADCAST==ExecutorRouteStrategyEnum.match(jobInfo.getExecutorRouteStrategy(), null)
            && group.getRegistryList()!=null && !group.getRegistryList().isEmpty()
            && shardingParam==null) {
        for (int i = 0; i < group.getRegistryList().size(); i++) {
            processTrigger(group, jobInfo, finalFailRetryCount, triggerType, i, group.getRegistryList().size());
        }
    } else {
        if (shardingParam == null) {
            shardingParam = new int[]{0, 1};
        }
        // 其次就是走这里
        processTrigger(group, jobInfo, finalFailRetryCount, triggerType, shardingParam[0], shardingParam[1]);
    }

}
```



```java
private static void processTrigger(XxlJobGroup group, XxlJobInfo jobInfo, int finalFailRetryCount, TriggerTypeEnum triggerType, int index, int total){

    // param
    ExecutorBlockStrategyEnum blockStrategy = ExecutorBlockStrategyEnum.match(jobInfo.getExecutorBlockStrategy(), ExecutorBlockStrategyEnum.SERIAL_EXECUTION);  // block strategy
    ExecutorRouteStrategyEnum executorRouteStrategyEnum = ExecutorRouteStrategyEnum.match(jobInfo.getExecutorRouteStrategy(), null);    // route strategy
    String shardingParam = (ExecutorRouteStrategyEnum.SHARDING_BROADCAST==executorRouteStrategyEnum)?String.valueOf(index).concat("/").concat(String.valueOf(total)):null;

    // 1、save log-id ，保存日志，调用mysql
    XxlJobLog jobLog = new XxlJobLog();
		// ...
    XxlJobAdminConfig.getAdminConfig().getXxlJobLogDao().save(jobLog);

    // 2、init trigger-param ， 初始化一些信息
    TriggerParam triggerParam = new TriggerParam();
    triggerParam.setJobId(jobInfo.getId());
		//  ...

    // 3、init address ， 查看一下remote地址
    String address = null;
    ReturnT<String> routeAddressResult = null;
  // 组的概念，一次调度几台机器，上个流程去数据库里查了
    if (group.getRegistryList()!=null && !group.getRegistryList().isEmpty()) {
        // 逻辑判断，
    }

    // 4、trigger remote executor
    ReturnT<String> triggerResult = null;
    if (address != null) {
        // 调用一下 ，核心逻辑
        triggerResult = runExecutor(triggerParam, address);
    } else {
        triggerResult = new ReturnT<String>(ReturnT.FAIL_CODE, null);
    }

    // 5、collection trigger info
    StringBuffer triggerMsgSb = new StringBuffer();
    // 日志记录

    // 6、save log trigger-info
    jobLog.setExecutorAddress(address);
    XxlJobAdminConfig.getAdminConfig().getXxlJobLogDao().updateTriggerInfo(jobLog);

    logger.debug(">>>>>>>>>>> xxl-job trigger end, jobId:{}", jobLog.getId());
}

```



```java
public static ReturnT<String> runExecutor(TriggerParam triggerParam, String address){
    ReturnT<String> runResult = null;
    try {
        // biz其实一个job的核心，逻辑，run start kill 等,这个做了代理
        ExecutorBiz executorBiz = XxlJobDynamicScheduler.getExecutorBiz(address);
        // 这里做了代理了，实际上是com.xxl.rpc.remoting.invoker.reference.XxlRpcReferenceBean
        runResult = executorBiz.run(triggerParam);
    } catch (Exception e) {
        logger.error(">>>>>>>>>>> xxl-job trigger error, please check if the executor[{}] is running.", address, e);
        runResult = new ReturnT<String>(ReturnT.FAIL_CODE, ThrowableUtil.toString(e));
    }

    StringBuffer runResultSB = new StringBuffer(I18nUtil.getString("jobconf_trigger_run") + "：");
    runResultSB.append("<br>address：").append(address);
    runResultSB.append("<br>code：").append(runResult.getCode());
    runResultSB.append("<br>msg：").append(runResult.getMsg());

    runResult.setMsg(runResultSB.toString());
    return runResult;
}
```



```java
public static ExecutorBiz getExecutorBiz(String address) throws Exception {
		// 。。。
  // set-cache
  	// 做了个代理，可以看到超时时间是5s,而且是http的方式
    executorBiz = (ExecutorBiz) new XxlRpcReferenceBean(
            NetEnum.NETTY_HTTP,
            Serializer.SerializeEnum.HESSIAN.getSerializer(),
            CallType.SYNC,
            LoadBalance.ROUND,
            ExecutorBiz.class,
            null,
            5000,
            address,
            XxlJobAdminConfig.getAdminConfig().getAccessToken(),
            null,
            null).getObject();
    executorBizRepository.put(address, executorBiz);
    return executorBiz;
}
```



rpc调用

```java
public Object getObject() {
  return Proxy.newProxyInstance(Thread.currentThread()
      .getContextClassLoader(), new Class[] { iface },
      new InvocationHandler() {
        @Override
        public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
          // method param
          String className = method.getDeclaringClass().getName();	// iface.getName()
          String varsion_ = version;
          String methodName = method.getName();
          Object[] parameters = args;

          // address
          String finalAddress = address;

          // request
          XxlRpcRequest xxlRpcRequest = new XxlRpcRequest();

                    // send
          if (CallType.SYNC == callType) {
            // future-response set
            XxlRpcFutureResponse futureResponse = new XxlRpcFutureResponse(invokerFactory, xxlRpcRequest, null);
            try {
              // do invoke
              client.asyncSend(finalAddress, xxlRpcRequest);

              // future get
              XxlRpcResponse xxlRpcResponse = futureResponse.get(timeout, TimeUnit.MILLISECONDS);
              if (xxlRpcResponse.getErrorMsg() != null) {
                throw new XxlRpcException(xxlRpcResponse.getErrorMsg());
              }
              // 同步发送
              return xxlRpcResponse.getResult();
            } catch (Exception e) {
              throw (e instanceof XxlRpcException)?e:new XxlRpcException(e);
            } finally{
              // future-response remove
              futureResponse.removeInvokerFuture();
            }
            // 异步发送
          } else if (CallType.FUTURE == callType) {
       			// 。。
          } else if (CallType.CALLBACK == callType) {
          		// 回掉
        }
      });
}
```

发送 ,使用netty， 创建一个http连接，发送出去，而且这个是一个长连接，在pool中拿的

```java
@Override
public void send(XxlRpcRequest xxlRpcRequest) throws Exception {
  //  hessian2序列号
    byte[] requestBytes = serializer.serialize(xxlRpcRequest);

    DefaultFullHttpRequest request = new DefaultFullHttpRequest(HttpVersion.HTTP_1_1, HttpMethod.POST, new URI(address).getRawPath(), Unpooled.wrappedBuffer(requestBytes));
    request.headers().set(HttpHeaderNames.HOST, host);
    request.headers().set(HttpHeaderNames.CONNECTION, HttpHeaderValues.KEEP_ALIVE);
    request.headers().set(HttpHeaderNames.CONTENT_LENGTH, request.content().readableBytes());

    this.channel.writeAndFlush(request).sync();
}

```





#### 总结

其实框架很简单 ， 第一在服务端做了一个调度框架，而我们的资源更多的是暴漏成了 http 或者 rpc方法等， 所以也就是将比如我们的 go-cron 将其抽离成服务， 而我们的资源确是在我们的各个服务器上， 传输层可以使用rpc ， http等。

我觉得它的问题， 

- 第一就是当前这个版本对外开发支持不友好，
- 第二就是日志记录采用mysql，性能差





其次就是他的拓展性



#### 拓展性

> ​	在api这个地方， 感觉很鸡肋

```java
@Controller
public class JobApiController implements InitializingBean {


    @Override
    public void afterPropertiesSet() throws Exception {

    }

    @RequestMapping(AdminBiz.MAPPING)
    @PermessionLimit(limit=false)
    // 这个提供了一个其他方兼容的方式
    public void api(HttpServletRequest request, HttpServletResponse response) throws IOException, ServletException {
      // 大致就是序列化，request的请求体 ， 然后序列号成XxlRpcRequest，然后还是上面的逻辑
        XxlJobDynamicScheduler.invokeAdminService(request, response);
    }
}
```









