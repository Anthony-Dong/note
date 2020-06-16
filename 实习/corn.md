# corn

首先看corn的结构体

```go
type Cron struct {
  // 存放job
	entries  []*Entry
	stop     chan struct{}
  // 运行期间添加的job
	add      chan *Entry
	snapshot chan []*Entry
  // 是否正在运行中
	running  bool
	ErrorLog *log.Logger
	location *time.Location
}
```





entry

```go
type Entry struct {
	// The schedule on which this job should be run.
	Schedule Schedule

	// The next time the job will run. This is the zero time if Cron has not been
	// started or this entry's schedule is unsatisfiable
	Next time.Time

	// The last time this job was run. This is the zero time if the job has never
	// been run.
	Prev time.Time

	// The Job to run.
	Job Job
}
```



初始化的时候

```go
// New returns a new Cron job runner, in the Local time zone.
func New() *Cron {
	return NewWithLocation(time.Now().Location())
}

// NewWithLocation returns a new Cron job runner.
func NewWithLocation(location *time.Location) *Cron {
	return &Cron{
		entries:  nil,
		add:      make(chan *Entry),
		stop:     make(chan struct{}),
		snapshot: make(chan []*Entry),
		running:  false,
		ErrorLog: nil,
		location: location,
	}
}
```



添加job

```go
// AddFunc adds a func to the Cron to be run on the given schedule.
func (c *Cron) AddFunc(spec string, cmd func()) error {
	return c.AddJob(spec, FuncJob(cmd))
}

// AddJob adds a Job to the Cron to be run on the given schedule.
func (c *Cron) AddJob(spec string, cmd Job) error {
  // 解析过程，corn 表达式解析出来，
	schedule, err := Parse(spec)
	if err != nil {
		return err
	}
  // 添加job
	c.Schedule(schedule, cmd)
	return nil
}
```



```go
func (c *Cron) Schedule(schedule Schedule, cmd Job) {
  // 封装节点
	entry := &Entry{
		Schedule: schedule,
		Job:      cmd,
	}
  // 添加进去，前提是没运行
	if !c.running {
		c.entries = append(c.entries, entry)
		return
	}

  // 运行的时候，就阻塞了，需要等待去处理，看run的流程
	c.add <- entry
}
```



start

```go
func (c *Cron) Start() {
  // 检测
	if c.running {
		return
	}
	c.running = true
  //run
	go c.run()
}
```



```go
func (c *Cron) run() {
	// Figure out the next activation times for each entry.
  // 1. 获取当前时间
	now := c.now()
  // 
	for _, entry := range c.entries {
    // 返回下次调用的时间，放到entry里	
		entry.Next = entry.Schedule.Next(now)
	}

  // 核心流程
	for {
    // 排序 ， 根据就近原则
		sort.Sort(byTime(c.entries))

    // 如果啥也没有直接无脑睡觉，后面还有补充
		var timer *time.Timer
		if len(c.entries) == 0 || c.entries[0].Next.IsZero() {
			// If there are no entries yet, just sleep - it still handles new entries
			// and stop requests.
			timer = time.NewTimer(100000 * time.Hour)
		} else {
      // 添加一个定时器，是最近最先触发的
			timer = time.NewTimer(c.entries[0].Next.Sub(now))
		}

    // 遍历，利用chan
		for {
			select {
        // 触发计时器
			case now = <-timer.C:
				now = now.In(c.location)
				// Run every entry whose next time was less than now
				for _, e := range c.entries {
					if e.Next.After(now) || e.Next.IsZero() {
						break
					}
          // 开了个goroutine去运行
					go c.runWithRecovery(e.Job)
          // 修改上一次的时间为当前运行的这次时间
					e.Prev = e.Next
          // 计算下一次运行的时间
					e.Next = e.Schedule.Next(now)
				}
//监听 add ， 如果添加进来（不懂为啥要停止计时器）
			case newEntry := <-c.add:
				timer.Stop()
				now = c.now()
				newEntry.Next = newEntry.Schedule.Next(now)
				c.entries = append(c.entries, newEntry)

			case <-c.snapshot:
				c.snapshot <- c.entrySnapshot()
// c.stop ？ 
			case <-c.stop:
				timer.Stop()
				return
			}

			break
		}
	}
}
```

