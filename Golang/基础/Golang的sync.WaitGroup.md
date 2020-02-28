# Golang 的  sync.WaitGroup (=CountdownLatch?)

## 开始

这个是Google 提供的官方例子.   其实跟 Java的CountdownLatch很相似 , 因为他的`wg.Wait()`方法是等待, 计数器到了0才会停止阻塞 , 如果大于0会一直阻塞.  他的 `wg.Add(1)` 是计数器加一 . 初始化为0

```java
import (
	"fmt"
	"sync"
)

type httpPkg struct{}

func (httpPkg) Get(url string) string {
	return url
}

var http httpPkg

func main() {
	var wg sync.WaitGroup
	var urls = []string{
		"http://www.golang.org/",
		"http://www.google.com/",
		"http://www.somestupidname.com/",
	}
	for _, url := range urls {
		// Increment the WaitGroup counter.
		wg.Add(1)
		// Launch a goroutine to fetch the URL.
		go func(url string) {
			// Decrement the counter when the goroutine completes.
			defer wg.Done()
			// Fetch the URL.
			get := http.Get(url)
			fmt.Println(get)
		}(url)
	}
	// Wait for all HTTP fetches to complete.
	wg.Wait()
}
```

输出 : 

```go
http://www.somestupidname.com/
http://www.golang.org/
http://www.google.com/
```



## 注意点

但是又一点需要注意的是 , 这个对象传递需要使用指针传递.  

```go
import (
	"fmt"
	"sync"
)

type httpPkg struct{}

func (httpPkg) Get(url string) string {
	return url
}

var http httpPkg

func main() {
	var wg sync.WaitGroup
	var urls = []string{
		"http://www.golang.org/",
		"http://www.google.com/",
		"http://www.somestupidname.com/",
	}

    // 注意这里
	runner(urls, &wg)
	// Wait for all HTTP fetches to complete.
	wg.Wait()
}

// 注意这里 . 
func runner(urls []string, wg *sync.WaitGroup) {
	for _, url := range urls {
		// Increment the WaitGroup counter.
		wg.Add(1)
		// Launch a goroutine to fetch the URL.
		go func(url string) {
			// Decrement the counter when the goroutine completes.
			defer wg.Done()
			// Fetch the URL.
			get := http.Get(url)
			fmt.Println(get)
		}(url)
	}
}
```



所以对象传递的使用记得使用指针传递 . 不然无效. 就等于复制了一个对象. 



## 如何加入超时

> ​	sync.WaitGroup 是会一直等待. 所以不适合用于有超时任务的进程 . 

```go
import (
	"errors"
	"fmt"
	"log"
	"time"
)

func worker() error {
	for i := 0; i < 1000; i++ {
		select {
         // time.After返回一个只读chan , 会阻塞时间.
		case <-time.After(50 * time.Millisecond):
			fmt.Println("Doing some work ", i)
		}
	}
	return nil
}

func main() {
	fmt.Println("Hey, I'm going to do some work")

	ch := make(chan error)
	go func() {
		ch <- worker()
	}()

	select {
	case err := <-ch:
		if err != nil {
			log.Fatal("Something went wrong :", err)
		}
	case <-time.After(4 * time.Second):
		fmt.Println("等的不耐烦了，就这样吧...")
	}

	fmt.Println("Finished. I'm going home")
}
```



但是这个问题就是在于上面这个是一个进程 , 进程退出自然子线程就退出了(任务自然结束了) . 但是对于有些任务显然不能进程退出哇 .  因此我们加入了 context . 可以传递信息. 

```go
var (
	wg2 sync.WaitGroup
)

func workWithTimeout(ctx context.Context) error {
	defer func() {
		wg2.Done()
	}()

	for i := 0; i < 1000; i++ {
		select {
		case <-time.After(50 * time.Millisecond):
			fmt.Println("Doing some work ", i)
			// we received the signal of cancelation in this channel
		case <-ctx.Done():
			fmt.Println("Cancel the context ", i)
			return ctx.Err()
		}
	}
	return nil
}

func main() {
	ctx, cancel := context.WithTimeout(context.Background(), 2*time.Second)
	defer cancel()

	fmt.Println("Hey, I'm going to do some work")

	wg2.Add(1)
	go workWithTimeout(ctx)
	wg2.Wait()

	fmt.Println("Finished. I'm going home")
}
```



