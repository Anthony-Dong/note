# Go - 项目

### 配置文件 

> ​	 go get  github.com/unknwon/goconfig 这个是我们的配置目录，项目中封装了一下默认是在`/config/env.ini`

简单实用

```ini
[mysql]
username = root
password = 12345
```

使用

```go
func main() {
	c, _ := goconfig.LoadConfigFile("config/env.ini")
	strings, _ := c.GetSection("mysql")
	fmt.Println(strings["password"])
}
```





### endless / gin 解决http

> ​	如何做到平滑启动，比如我们如何平滑的上线服务， 比如现在服务在服务器中运行着，如果我们打算更新一下服务，那么做法是什么？  第一关闭这个进程，然后把新服务重启，但是考虑过这个整个时间耗时吗，自动化系统怎样的流程呢大致，所以难以保障我们服务一直对外提供服务。

对就是这个 endless框架， 很好的解决了这个问题， 他可以帮助我们平滑启动服务，[https://github.com/fvbock/endless/](https://github.com/fvbock/endless/) 。

大致用法就是 ： 

```go
func main() {
	r := gin.Default()
	gin.SetMode(gin.DebugMode)
	r.GET("/ping", func(c *gin.Context) {
		c.JSON(200, gin.H{
			"message": "pong-3",
		})
	})
	log.Fatal(endless.ListenAndServe(":8080", r))
}
```

我们直接编译，然后启动，如果我们想要去修改服务器的其他功能，

第一步我们将新文件build一下 ， 然后找到进程PID , 然后 `kill -1 pid` , 此时自然就给我们重新加载我们的编译文件。

```go
2020/04/24 16:54:38 Actual pid is 4196
2020/04/24 16:54:38 4169 Received SIGTERM.
2020/04/24 16:54:38 4169 Waiting for connections to finish...
2020/04/24 16:54:38 4169 Serve() returning...
2020/04/24 16:54:38 accept tcp 127.0.0.1:4242: use of closed network connection
2020/04/24 16:54:38 4169 127.0.0.1:4242 Listener closed.
```

这就是一个流程， 但是我其实是不理解，看了看大概的意思就是父进程fork了一个新的子进程，然后复用父进程监听的socket。  

我们看看kill的命令

```go
 1) SIGHUP	 2) SIGINT	 3) SIGQUIT	 4) SIGILL	 5) SIGTRAP
 6) SIGABRT	 7) SIGBUS	 8) SIGFPE	 9) SIGKILL	10) SIGUSR1
11) SIGSEGV	12) SIGUSR2	13) SIGPIPE	14) SIGALRM	15) SIGTERM
16) SIGSTKFLT	17) SIGCHLD	18) SIGCONT	19) SIGSTOP	20) SIGTSTP
21) SIGTTIN	22) SIGTTOU	23) SIGURG	24) SIGXCPU	25) SIGXFSZ
26) SIGVTALRM	27) SIGPROF	28) SIGWINCH	29) SIGIO	30) SIGPWR
31) SIGSYS	34) SIGRTMIN	35) SIGRTMIN+1	36) SIGRTMIN+2	37) SIGRTMIN+3
38) SIGRTMIN+4	39) SIGRTMIN+5	40) SIGRTMIN+6	41) SIGRTMIN+7	42) SIGRTMIN+8
43) SIGRTMIN+9	44) SIGRTMIN+10	45) SIGRTMIN+11	46) SIGRTMIN+12	47) SIGRTMIN+13
48) SIGRTMIN+14	49) SIGRTMIN+15	50) SIGRTMAX-14	51) SIGRTMAX-13	52) SIGRTMAX-12
53) SIGRTMAX-11	54) SIGRTMAX-10	55) SIGRTMAX-9	56) SIGRTMAX-8	57) SIGRTMAX-7
58) SIGRTMAX-6	59) SIGRTMAX-5	60) SIGRTMAX-4	61) SIGRTMAX-3	62) SIGRTMAX-2
63) SIGRTMAX-1	64) SIGRTMAX
```

为啥要选择1呢， 是因为SIGHUP信号为终止收到该信号的进程。若程序中没有捕捉该信号，当收到该信号时，进程就会退出（常用于 重启、重新加载进程）

**流程就是 ：  感觉和Java的agent很相似，如果用JVM类比操作系统的话，JVM中核心的是字节码，替换运行时字节码的操作类似于这个，**

1. 替换可执行文件或修改配置文件
2. 发送信号量 SIGHUP
3. 拒绝新连接请求旧进程，但要保证已有连接正常
4. 启动新的子进程
5. 新的子进程开始 Accet
6. 系统将新的请求转交新的子进程
7. 旧进程处理完所有旧连接后正常结束





### 日志 ， 自定义日志框架

>  	这个样式很nice ， 这个样式会有颜色的，我也不懂为啥，悄咪咪的复制一下，

```go
func main() {
	str:="\033[36m[DEBUG]\033[0m %s"
	log.Printf(str,"echo")
}
```

项目的日志地址是写死的 ， src/api/logger/my_logger.go:90  ， src/api/logger/my_logger.go:90





## cron   定时脚本

> ​	cron 表达式，得看一看， 秒，分钟，小时，日期 ， 月份 ，周，年，一共7⃣️位。其中可以省略年份，项目中一般都是 6 位，

具体使用可以看看  [https://www.jianshu.com/p/e9ce1a7e1ed1](https://www.jianshu.com/p/e9ce1a7e1ed1)

项目中使用的corn是 `github.com/robfig/cron` 这个项目， 其中使用还是很简单，注意一下项目中使用的cron版本，比较低，如果下载直接拉取的版本，需要多加一个参数，就是创建的时候多加一个参数withsecond，

```go
func main() {
	group := sync.WaitGroup{}
	group.Add(1)
  // 1. 创建，项目中版本低，不需要传入参数
	c := cron.New(cron.WithSeconds())
  //2. 添加任务
	c.AddFunc("*/2 * * * * *", func() {
		fmt.Println("hello corn")
	})
  // 3. 启动
	c.Start()
	group.Wait()
}

```

#### 解决两台机器同时执行的问题

所以它是一个单机版的调度中心，分布式的任务调度中心，Java的quartz是分布式的，将原数据抽离了出来，而我们项目中是依靠分布式锁，实现的单独处理，利用的是redis

```go
// 这里使用的是redis的分布式锁，也就是 set nx 一个key只能在它不存在的时候设置
res := credis.RedisInstance().LockAcquire(redisKey, "1", 3600000)
if !res {
  return
}
```

```go
func (p *ConnPool) LockAcquire(name string, value string, expiry int) bool {
	conn := p.redisPool.Get()
	defer conn.Close()
  // 使用的是 set k v nx px 360000 
	reply, err := redis.String(conn.Do("SET", name, value, "NX", "PX", expiry))
	logger.Infof("Redis LockAcquire name:%s,value:%s,reply:%s", name, value, reply)
	return err == nil && reply == "OK"
}
```



## 编程范式

### HTTP 

首先注册controller

```go
unc RegisterRouters(r *gin.Engine) {
	// 分析代码
	f, err := os.Create("trace.out")
	if err != nil {
		panic(err)
	}
	defer f.Close()

	err = trace.Start(f)
	if err != nil {
		panic(err)
	}
	defer trace.Stop()

	//注册路由 , 这里会注册路由
	new(controller.ServerController).RegisterRouter(r)
	new(controller.WageController).RegisterRouter(r)
}
```



其次就是我们的Controller层了

所有的controller ， 都必须是一个结构体，同时还需要实现`RegisterRouter` 方法, 这里为何不采用接口规范呢 ？，比如 这个 

```go
type Controller interface {
	RegisterRouter(r *gin.Engine)
} 
```

然后每一个方法都会被注册进行

```go
func (this *ApproveController) RegisterRouter(r *gin.Engine) {
  // group 约定一下
	safeNewGroup := r.Group("/app/v1/approve/")
	//审批首页(demo)
	safeNewGroup.POST("approve_home_page", this.approveHomePage)
}
```



然后 controller调用service 层 ， service层的话，是每一个都会有一个对象， 约定的前缀是New

```go
NewMonthSalaryService = new(MonthSalaryService)
```

每一个service会去实例化，去绑定一个方法，面向对象的好处就是在这里，上面是实现了一个单利对象

```go
type MonthSalaryService struct {
}
```

service没有做 抽象接口， dao做了

其中service回去初始化dao层， 

```go
var (
	opWorkerDao                    = dao.NewOpWorkerDao()
	opBasciSalaryDao               = urban.NewOpBasicSalaryDao()
)
```

**这里我就不理解了， service是单利，这里为啥不行呢，非要实例化一个新的。**



```go
// 接口约定
type OpWorkerDao interface {
	//获取当前用户信息
	GetCurrentUserInfo(session *xorm.Session, userId uint64) (*model.OpWorker, error)
}
// 内部实现接口
type opWorkerDao struct{}
// 唯一暴漏的实例化地方
func NewOpWorkerDao() OpWorkerDao {
	return &opWorkerDao{}
}
```



model层，dao层， go文件全部采用下划线命名法，controller、service层 go文件全部采用了Java的命名法（首字母小写），



**总结一句， controller层实现register方法， service层外部暴漏一个实例（该实例实现了全部的方法） ，其次就是dao层，全部采用接口约定， 然后具体实现，对外暴漏一个方法。**



