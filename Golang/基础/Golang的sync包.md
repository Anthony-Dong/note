# Golang - sync包

> ​	Golang的 sync包提供了大量的 线程同步操作的类. 所以满足我们日常使用. 

## 1. sync.Mutex

第一种就是 `Mutex` 为互斥锁 , 实现跟Java的reentrantlock很像. 基本都是自旋锁,

我们下面有一个场景 , 将变量x累加50000次 我们开启5个goroutine去执行

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
// 这个操作是Java做不了的.只有C,C++和Golang可以
func increase(x *int) {
	*x = *x + 1
}
```

如果输出结果是 50000那就对了, 我们看看结果

```java
23283
```

为什么呢, 因为多线程同时操作一个变量时就会出现这种问题 , 出现读写不一致的问题 , 如何处理呢

我们申明一个锁,让他同步执行 , 这么申明就可以了

```go
var ILock = sync.Mutex{}

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

func increase(x *int) 
	// 1. 先加锁
	ILock.Lock()
	// 执行完方法释放锁. 跟Java的很像,但是不是reentrant
    defer ILock.Unlock()
	*x = *x + 1
}
```

此时执行 发现结果

```go
50000
```

输出正确.  

其实还可以这么写. 效率更高

```go
go func() {
    ILock.Lock()
    defer ILock.Unlock()
    for i := 0; i < 10000; i++ {
        increase(&x)
    }
}()
```

### 利用Chan阻塞实现锁

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

## 2. sync.RWMutex 读写锁

> ​	跟Java的一模一样 , 我就不多说了, **读写互斥, 可多读(读锁可以同时存在多个), 但读写不能同时存在, 写互斥**

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
		// 写锁 , 就是普通的锁了
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
写 // 对比前一步耗时 ms
```



## 3. sync.WaitGroup

> ​	这个类类似于Java的`CountDownLatch` 类, 但是比Java的好点. 第一Java的需要初始化告诉他计数器是多少, 所以他只有一个countdown和wait操作. 
>
> ​	但是 `sync.WaitGroup` 却是三个操作, 第一个 down , add(int) , wait 三个操作. 所以他实现更加好. 

简单以第一个例子为例子吧, 比如说 , 我们不知道goroutine啥时候退出是吧, 但是有了这玩意就知道了. 

```go
func main() {
	var WG = sync.WaitGroup{}

	start := time.Now().UnixNano() / 1e6
	for num := 0; num < 5; num++ {
		WG.Add(1)
		go func(num int) {
			defer WG.Done()
			ran := rand.Int63n(1000)
			fmt.Printf("goroutine-%d sleep : %dms\n", num, ran)
			time.Sleep(time.Millisecond * time.Duration(ran))
		}(num)
	}
	// 等待子线程退出
	WG.Wait()
	fmt.Printf("main waitting : %dms\n", time.Now().UnixNano()/1e6-start)
}
```

输出 :  根据木桶原理, 耗时最长的是937ms, 所以主线程等待了939ms.

```go
goroutine-1 sleep : 410ms
goroutine-0 sleep : 821ms
goroutine-2 sleep : 51ms
goroutine-3 sleep : 937ms
goroutine-4 sleep : 551ms
main waitting : 939ms
```

### 注意点 : 

> ​	由于`sync.WaitGroup`也是一个对象Structs, 所以需要指针传递, 不能使用值传递, 注意一下.因为状态值复制了就无效了. 

根据封装, 我们需要传递 sync.waitgroup . 

```go
func fun(num int, wg *sync.WaitGroup) {
	defer wg.Done()
	ran := rand.Int63n(1000)
	fmt.Printf("goroutine-%d sleep : %dms\n", num, ran)
	time.Sleep(time.Millisecond * time.Duration(ran))
}
```

然后main方法

```go
func main() {
	var WG = &sync.WaitGroup{}
	start := time.Now().UnixNano() / 1e6

	for num := 0; num < 5; num++ {
		WG.Add(1)
		go fun(num, WG)
	}

	// 等待子线程退出
	WG.Wait()
	fmt.Printf("main waitting : %dms\n", time.Now().UnixNano()/1e6-start)
}
```

输出 : 

```go
goroutine-4 sleep : 410ms
goroutine-1 sleep : 551ms
goroutine-0 sleep : 821ms
goroutine-2 sleep : 51ms
goroutine-3 sleep : 937ms
main waitting : 939ms
```



## 4. sync.Once 一次操作

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
		fmt.Println("只会执行了一次")
	})
}
```

输出

```go
执行函数
只会执行了一次
执行函数
执行函数
```

所以他可以只执行一次  , 适合做初始化操作, 或者其他一次性的操作, 不需要多次

## 4. sync.Map

提供的线程安全的map , 多线程访问时, 对于crud 操作, 会加锁. 

```go
maps := sync.Map{}
// 存
maps.Store("","")
// 删
maps.Delete("")
// 取
maps.Load("")

// 有就不存,返回已经存了的对象和true, 如果没有就返回存的value和false.
maps.LoadOrStore("","")
```





## sync.Pool

> ​	顾名思义一个池子, 那么我们看看这个池子主要做啥了. 
>
> **Pool用于存储那些被分配了但是没有被使用，而未来可能会使用的值，以减小垃圾回收的压力。(适合大对象)**
>
> ​	同时他提供了一个存储的地方. 减少大量实例化过程 .  但是效率未必要比实例化快奥 . 因为维护一个对象要考虑各种问题, 这就是效率 , 但是实例化啥也不用考虑. 

操作很简单.  

```go
func main() {

	pool := &sync.Pool{
		New: func() interface{} {
			return "new"
		},
	}
	
	// 首先的放一个 , 由于put操作.
	pool.Put("init")

	go func() {
		// 拿一个
		s := pool.Get().(string)
		// 使用
		fmt.Println(s)
		// 使用完放进去, 所以特别适合大对象. 
		pool.Put(s)
	}()
}
```



