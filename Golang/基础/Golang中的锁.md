# Golang - 并发与锁

## 1. Mutex

第一种就是 `Mutex`

我们下面有一个场景 , 将变量x累加50000次 我们开启5个线程去执行

```go
func main() {
	var x = 0
	for num := 0; num < 4; num++ {
		go func() {
			for i := 0; i < 10000; i++ {
				increase(&x)
			}
		}()
	}

	// 等待子线程退出
	time.Sleep(time.Millisecond * 2000)
	fmt.Println(x)
}

func increase(x *int) {
	*x = *x + 1
}
```

如果输出结果是 50000那就对了, 我们看看结果

```java
23283
```

为什么呢, 因为多线程同时操作一个变量时就会出现这种问题 , 出现竞争问题  , 如何处理呢

我们申明一个 锁

```go
var (
	ILock sync.Mutex
)

func main() {
	var x = 0
	for num := 0; num < 5; num++ {
		go func() {
			for i := 0; i < 10000; i++ {
				increase(&x)
			}
		}()
	}

	// 等待子线程退出
	time.Sleep(time.Millisecond * 2000)
	fmt.Println(x)
}

func increase(x *int) {
	ILock.Lock()
	*x = *x + 1
	ILock.Unlock()
}
```

此时执行 发现结果

```go
50000
```

输出正确. 

他的实现是基于CAS操作的, 类似于自旋锁赋值失败一直锁着. 



## 2. Channel 中依靠信号量来实现

```go
var (
	lock = make(chan int, 1)
)

func Lock() {
	lock <- 1
}
func Unlock() {
	<-lock
}
```

此时这就是一个最简单的锁.

## 3.RWMutex 读写锁



```go
func main() {
	// 读写锁
	mutex := sync.RWMutex{}

	go func() {
		// 读锁
		mutex.RLocker().Lock()
		fmt.Println("读")
		mutex.RLocker().Unlock()
	}()
	go func() {
		mutex.RLocker().Lock()
		fmt.Println("读")
		time.Sleep(time.Millisecond * 5000)
		mutex.RLocker().Unlock()
	}()

	time.Sleep(time.Millisecond * 1000)

	go func() {
		// 写锁
		fmt.Println("拿到了")
		mutex.Lock()
		fmt.Println("写")
		mutex.Unlock()
	}()
	time.Sleep(time.Millisecond * 6000)
}
```

输出

```go
读
读
拿到了 // 耗时1ms
写 // 对比前一步耗时 5ms
```



跟Java的一模一样 , 我就不多说了, 读写互斥, 可多读, 但读写不能同时存在, 写互斥



## Once  一次操作

这个玩意可以让 `once.Do()` 方法只执行一次.  其实类似于一个flag,一开始为true.  每次执行判断是否为true , 当执行了一次以后改成false. 其实他就是这个原理 , 不过他使用了cas , 保证了线程安全性, 

```go
func main() {
	once := sync.Once{}
	one(&once)
	one(&once)
	one(&once)
}

func one(once *sync.Once)  {
	fmt.Println("执行函数")
	once.Do(func() {
		fmt.Println("执行了一次")
	})
}
```

输出

```go
执行函数
执行了一次
执行函数
执行函数
```

所以他可以只执行一次  , 适合做初始化操作, 或者其他一次性的操作, 不需要多次



## 线程安全的 Map

提供的线程安全的map

```go
maps := sync.Map{}

maps.Store("","")

maps.Delete("")

maps.Load("")

maps.LoadOrStore("","")
```





