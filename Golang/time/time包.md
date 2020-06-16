# time 包

## Timer  和 Ticker

> ​	这俩都是定时器，那么区别在哪

```go
func main() {
	wg := sync.WaitGroup{}
	wg.Add(1)
	go func() {
		//  ticket 滴答
		ticket := time.NewTicker(time.Second)
		for {
			<-ticket.C
			fmt.Println("ticker trigger", time.Now().Format("2006-01-02 15:04:05"))
		}
	}()

	go func() {
		// timer 定时器
		timer := time.NewTimer(time.Second)
		for {
			<-timer.C
			fmt.Println("timer trigger", time.Now().Format("2006-01-02 15:04:05"))
		}
	}()

	runtime.Gosched()
	wg.Wait()
}
```

输出：

```go
timer trigger 2020-05-25 23:44:39
ticker trigger 2020-05-25 23:44:40
ticker trigger 2020-05-25 23:44:42
ticker trigger 2020-05-25 23:44:44
```

可以看到，timer只会触发一次，而ticket会一直触发



简单的差别就是上面讲的， 那么ticket触发，那么如果我加入任务时间呢

```go
go func() {
  //  ticket 滴答
  ticket := time.NewTicker(time.Second)
  for {
    cur := <-ticket.C
    // 休息2s
    time.Sleep(2 * time.Second)
    fmt.Printf("ticker trigger %s receive:%s\n", time.Now().Format("2006-01-02 15:04:05"),cur.Format("2006-01-02 15:04:05"))
  }
}()
```

输出：

```go
ticker trigger 2020-05-25 23:50:46 receive:2020-05-25 23:50:44
ticker trigger 2020-05-25 23:50:48 receive:2020-05-25 23:50:45
ticker trigger 2020-05-25 23:50:50 receive:2020-05-25 23:50:47
ticker trigger 2020-05-25 23:50:52 receive:2020-05-25 23:50:49
ticker trigger 2020-05-25 23:50:54 receive:2020-05-25 23:50:51
ticker trigger 2020-05-25 23:50:56 receive:2020-05-25 23:50:53
```

我们发现触发是2s触发一次，那么为啥收到的触发时间是相差3s，其实是因为，当我们sleep的时候，channel也会灌入数据，buffer是1，所以会多出一个。

1、一开始比如是收到 44s ，此时等待2s， 46s触发，等待过程中channel里继续装入数据45s。

2、我们等待结束收到45s，此时时间点是46s，然后我们继续sleep 2s，然后channel下一次触发结果是47s，然后我们只完毕是48s。

3、所以下一次就是50s,47s.  就是因为channel  buffer是1的缘故。



## 源码分析

```go
// The Timer type represents a single event.（单事件）
// When the Timer expires, the current time will be sent on C,
// unless the Timer was created by AfterFunc.
// A Timer must be created with NewTimer or AfterFunc.
type Timer struct {
	C <-chan Time
	r runtimeTimer
}
```



```go
// A Ticker holds a channel that delivers `ticks' of a clock
// at intervals.（每隔一段时间发送滴答）
type Ticker struct {
	C <-chan Time // The channel on which the ticks are delivered.
	r runtimeTimer
}
```



```go
type runtimeTimer struct {
	tb uintptr     // 不懂
	i  int        // do not know
	when   int64   // 执行时间,下一次触发的时间
	period int64  // 循环周期（这就是ticket为啥可以重复执行的问题）
	f      func(interface{}, uintptr) // NOTE: must not be closure// 这个是定时器触发执行的任务。
	arg    interface{} // 参数，f的参数，根据需求指定。
	seq    uintptr
}
```

关于初始化ticket  和 timer的区别就是在于period参数。

### reset 方法

那么还有几个函数

```go
func (t *Timer) Reset(d Duration) bool {
	if t.r.f == nil {
		panic("time: Reset called on uninitialized Timer")
	}
  // 设置下一次触发的时间
	w := when(d)
  // 尝试去停止，如果一个timer还没有触发，那么无法停止的
	active := stopTimer(&t.r)
  // 设置触发时间
	t.r.when = w
  // 启动
	startTimer(&t.r)
	return active
}
```

这个有个注意点就是: 1、**设置在一个已经过期了的timer上，或者已经关闭的timer上**，**或者channel已经被消费过才可以设置成功**，因此正确的做法是：

```go
go func() {
  timer := time.NewTimer(time.Second)
  for {
    cur := <-timer.C
    fmt.Printf("timer trigger %s receive:%s\n", time.Now().Format("2006-01-02 15:04:05"), cur.Format("2006-01-02 15:04:05"))
    fmt.Println(timer.Reset(time.Second)) // false
  }
}()
```

### stop 方法

第二个就是我们的 stop 方法 ，**成功关闭一个正在运行中的timer 返回true，如果这个timer已经关闭了返回false**

如果关闭一个正在运行的timer，那么调用stop方法，那么chan会被关闭，但是关闭并不会触发，timer.C收到数据。

```go
func (t *Timer) Stop() bool {
	if t.r.f == nil {
		panic("time: Stop called on uninitialized Timer")
	}
	return stopTimer(&t.r)
}
```



### startTimer & stopTimer

> ​	内部实现，但是stop绝对不是close channel，如果是的话，我们会监听到消息, 而是一种不发送消息，因此我们的channel 处于一种阻塞的状态一直。可以通过reset恢复



### time.After()  和 time.AfterFunc()

> ​	源码很简单，基本不用看

```go
// 最为简单
func After(d Duration) <-chan Time {
	return NewTimer(d).C
}
```



```go
func AfterFunc(d Duration, f func()) *Timer {
	t := &Timer{
		r: runtimeTimer{
			when: when(d),
			f:    goFunc, //trigger 的时候调用GoFunc，参数是f
			arg:  f,
		},
	}
	startTimer(&t.r)
	return t
}
// 启动goroutine,执行传入的任务
func goFunc(arg interface{}, seq uintptr) {
	go arg.(func())()
}
```



