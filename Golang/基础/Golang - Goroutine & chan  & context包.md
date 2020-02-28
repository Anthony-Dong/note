# Golang - Goroutine & chan  & context包

> ​	`Goroutine `  和 `chan ` 是golang多线程编程的核心 ,   context是上下文, 链接着 goroutine 和 chan , 所以 context包也很重要, 使用也很重要. 

## 1. Goroutine

在Go语言中，每一个并发的执行单元叫作一个goroutine。Java中叫做Thread ,其实是一回事,大家可以认为go关键字和new thread().start()  是一目一样的效果来说.  所以他是处理并发用的, 他的生命周期由Golang底层管理.

```go
func main() {
    // 启动一个tcp端口
	listener, _ := net.Listen("tcp", "localhost:8888")
	for {
        //学过Java或者其他语言的都知道 , 一个新的连接过来入口就是一个accept
		conn, _ := listener.Accept()
        // 处理新连接
		handlers(&conn)
	}
}

func handlers(conn *net.Conn) {
    // 其实这个方法不执行, 因为就算客户端断了也会不断的写的,因为下面是个死循环..所以先不考虑这个,处理的话一般得做心跳处理,或者conn.SetDeadline().
	defer (*conn).Close()
	for {
		io.WriteString(*conn, time.Now().Format("Mon Jan 2 15:04:05 -0700 MST 2006\n\r"))
		time.Sleep(1 * time.Second)
	}
}
```

此时我们分别俩窗口开启 `telnet localhost 8888`  , 会发现只会有一个返回数据, 另外一个阻塞了 不会返回数据, 是为啥呢,  Java来说都是开启一个`Thread`来处理 handler方法, 而Golang是需要创建一个 `GoRoutine `,  其实目的都是一样的, 区别不解释了.

因此我们将代码改成 `go handlers(&conn)`   , 此时俩窗口都收到数据了 , 这就是 GoRoutine, 对于Golang来说,这就是一个编程模型, 对于webhttp服务器还是其他都是这个流程, 不一样的而是不同需求封装, 对于Java来说他可能有BIO,NIO,AIO编程模型, 可能go的强大不需要吧, 

客户端代码也很简单  , `dial`  意思就是 `拨号` . 

```go
func main() {
	conn, e := net.Dial("tcp", "localhost:8888")
	if e != nil {
		fmt.Println("error")
		os.Exit(1)
	}
	defer func() {
		conn.Close()
		fmt.Println("关闭连接")
	}()
	out := os.Stdout
	io.Copy(out, conn)
}
```

这就是一个客户端服务器 , 很简单, 上述就是 goroutine的简单使用, 对于大量不相干的任务 , 为了提供并发性(执行效率), 所以使用goroutine 是很不错的选择 , 但是有些时候 goroutine之间需要协调, 这时候就需要 context了, 下文会讲到 .  

## 2. Channels

channle是一个线程安全的阻塞队列, 根据你缓冲的大小可能存在 存阻塞和取阻塞 . 如果你学过Java的BlockingQueue很像 ,   但是Golang会检测channel是否发生死锁, 防止写法上出现问题. 

chan的操作  :  

```go
// 1. 定义 , 代表这个chan类型为int,只能存入int类型数据, 缓冲区为0 , ch是一个指针变量
ch :=make(chan int,0)
// 2. 存入数据. 用 `<-` 表示 , put操作. 
ch<-1
// 3. 读取数据.  用 `<-` 表示 , 返回俩参数, 第一个是读取的数据, 第二个是成功与否. take操作. 如果只写一个是代表读取返回的数据.
x,ok:=<-ch
```

### 无缓冲区的chan

申明 `make(chan int)` , 后面是类型, 就是最多我只能放一个. 类似于Java的SynchronizedBlockingQueue. 是一个无缓冲区的队列 . 

```go
func main() {
	ch :=make(chan int,0)

	go func() {
		ch<-1
	}()

	x,ok:=<-ch
	fmt.Println(x,ok)
}
```

输出 : 

```go
1 true
```

其中对于主线程 , golang会检测chan使用是否合理, 不然会抛出 , `fatal error: all goroutines are asleep - deadlock!` ,比如可能存在下面的代码. 

```go
func main() {
	ch := make(chan int, 0)
	<-ch
}
```

这个会直接抛出上诉异常, 因为不合法. 简称 `deadlock` , 为死锁的意思 , 什么是死锁 , 就是线程卡着不动了 , 没有办法拯救了 ,使用chan 千万要注意死锁问题 , 就算close()掉chan, 抛出异常,也不能发生死锁(子线程发生死锁可能性很大的) .  不难发现这种检测是有必要的, 在程序启动前做检测 , 是一种很好的编译习惯. 

### 具有缓冲区的chan

> ​	有缓冲区的chan , 类似于Java的 ArrayBlockingQueue.  需要指定一个队列大小, 

这个代码, 让大家体会一下 chan的缓冲区的概念. 这里会循环6次 , 向里面添加数据, 如果缓冲区大小还有, 就会存入成功. 失败就会自动走default . 

```go
func main() {
	ch := make(chan int, 5)

	for x := 1; x <= 6; x++ {
		select {
		case ch <- x:
			fmt.Println("save success ", x)
		default:
			fmt.Println("save error ",x)
		}
	}
}
```

输出 :  确实是缓冲区大小只有5 . 

```go
save success  1
save success  2
save success  3
save success  4
save success  5
save error  6
```

### 只读 , 只写chan 

> ​	这个颗粒度更加高了, 只读只写chan.  一般用来作为参数进行传递,  主要是看一些源码会写一些 chan<- 之类的, 不好理解. 这里解释一下. 

实例化一个只读chan . 只读chan,如果用做写, 会直接程序错误,无法编译的 . 其实开发工具会提示的. 

```go
r:=make(<-chan int)
```

实例化一个只写chan . , 只写chan ,如果用做读, 会直接程序错误,无法编译的 

```go
w:=make(chan<- int)
```

其实这俩 r , w 都是 `make(chan int)` 的子类, 所以不必要担心, 出现问题.  

我们简单的使用一下. 

```go
func main() {
    // 1.创建一个ch
	ch := make(chan interface{}, 10)
	// 2.开启一个goroutine执行receive
	go func() {
		receive(ch)
	}()
    // 3. 发送
	send(ch)
	time.Sleep(time.Second)
}
func receive(read <-chan interface{}) {
	for {
		r := <-read
		fmt.Println("receive : ", r)
	}
}

func send(write chan<- interface{}) {
	for x := 1; x < 10; x++ {
		write <- x
	}
}
```

输出  : 

```go
receive :  1
...
receive :  9
```



### 关闭 chan

有些情况下需要关闭 chan  , 那就涉及到 chan 的状态了 , 这里是几个状态量.  `所以一般在写入(也就是sender中)的goroutine中执行 close 操作. `  , 关闭chan , 只是为了GC的更好的回收. 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-10-11/20fe4b2d-6a33-449a-be68-8e43406f6147.jpg?x-oss-process=style/template01)

其实查看close的方法. 也是推荐 只写chan

```go
func close(c chan<- Type)
```

如何关闭chan .

```go
func main() {
	ch := make(chan interface{})

	close(ch)
}
```

注意点 

> close没有make的chan会引起panic , 也就是进程直接退出 . 在你没有抓取异常的情况下. 
>
> close以后不能再写入，写入会出现panic
>
> **close之后可以读取数据, 如果没有数据 ，则返回 nil,false**
>
> 重复close会引起panic , 看情况要不要抓取.
>
> 只读chan不能close
>
> 不手动	close chan也是可以的，当没有被引用时系统会自动垃圾回收。



### select ... case  语法

简单使用 : 

```go
func main() {
	ch1 := make(chan int64)
	ch2 := make(chan int64)
	start := time.Now().UnixNano() / 1e6
	go func() {
		time.Sleep(time.Millisecond * 500)
		ch1 <- time.Now().Unix()
	}()
	go func() {
		time.Sleep(time.Millisecond * 300)
		ch2 <- time.Now().Unix()
	}()

	select {
	case x := <-ch1:
		fmt.Printf("wait %dms , receive : %d.", time.Now().UnixNano()/1e6-start, x)
	case x := <-ch2:
		fmt.Printf("wait %dms , receive : %d.", time.Now().UnixNano()/1e6-start, x)
	}
}
```

输出 :  

```go
wait 301ms , receive : 1581084448.
Process finished with exit code 0
```



对于 : 下面这个语句 , **他会不断的执行下面的语句一 和 语句二(可能是个方法返回的chan, 也可能是个chan) , 直到满足put成功或者tack成功 , 不然不断的轮询**. 当都成功可以他会随机选择一个执行. 

```go
select {
	case 语句一:
    	..
    case 语句二:
    	..
}
```

然后就是他 还有一个 default关键字.  这个关键字呢 , 他会执行前俩个 , 如果俩都失败, 那么直接执行default退出select语句. 

```go
select {
	case 语句一:
    	..
    case 语句二:
    	..
    default:
		...
}
```

简单的实现一个wait功能. 

```go

```



### 简单的使用chan

> ​	如果我们学了chan , 不学以致用, 那么就很扯淡 , 学了不用学了又有啥用 
>
> ​	这个demo . 是我写的一个简单的例子, 比如一个请求, 需要将用户信息存入到数据库中, 同时还要反馈回用户一些信息 , 比如活动信之类的, 所以这俩不冲突, 可以异步执行. 

```go
// 1.查询服务
func Query(str string) []string {
	time.Sleep(time.Second * 2)
	return strings.Split(str, ".")
}
// 2.保存的数据库
func SaveUser() {
	time.Sleep(time.Second * 2)
}
// 2. User服务
func UserServer(str string) []string {
    // 1.创建一个chan
	ch := make(chan []string)
    // 2. 最后关闭, 释放资源
    defer close(ch)
    // 3. 异步查询
	go func() {
		ch <- Query(str)
	}()
    // 然后我们的处理逻辑, 
    SaveUser()
    // 处理完返回.
	return <-ch
}

func main() {
	start := time.Now().UnixNano() / 1e6
	res := UserServer("hello.world.!")
	fmt.Printf("耗时 %dms , 查询结果 : %v.\n",time.Now().UnixNano()/1e6-start, res)
}
```

输出 : 

```go
耗时 2001ms , 查询结果 : [hello world !].
```

​	我们发现上诉的问题在哪, 如果我查询时间特别长, 比如 10m , 20m , 都没有返回 . 用户体验好吗.  显然不可控的东西是程序禁止的 ,这时候就需要伟大的` context包`了. 下面我们先介绍一下这个包. 



## 3. context包

> ​	`context.Context`这个是一个上下文对象, 他是一个链表形式, 有root节点, 根节点一般是让golang给我们控制的, 我们只需要子节点.  对于快速失败, 也是根节点到子节点不断失败.
>
> ​	context包 提供了对`context.Context`的操作. 

#### `context.Background()`

```go
// Background returns a non-nil, empty Context. It is never canceled, has no
// values, and has no deadline. It is typically used by the main function,
// initialization, and tests, and as the top-level Context for incoming
// requests.
func Background() Context {
	return background
}
```

他是一个根节点, 其实就是一个 `as the top-level Context` , 懂了吧 .  不推荐自己传入一个top-level context.

#### `context.WithTimeout()` 

是创建一个 超时的ctx , 当超时他的 `<-ctx.Done()` 会返回一个空数据.  

```go
func main() {
	// ctx是一个指针类型的实现了context.Context接口的变量
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*2)
	defer cancel()

	select {
	case <-ctx.Done():
		fmt.Println("waiting 2 seconds")
	}
}
// 2s后输出 : 
// waiting 2 seconds
```

#### `context.WithDeadline()`

其实上面那个 `context.WithTimeout()` 其实就是 ` time.Now().Add(timeout)` , 当前时间加上一个timeout , 所以这个也很好理解. 

```go
func WithTimeout(parent Context, timeout time.Duration) (Context, CancelFunc) {
	return WithDeadline(parent, time.Now().Add(timeout))
}
```

#### `context.WithCancel()`

这个就是根据你的进程退出与否 ,决定的, 其实就是你调用了 `cancel()` 方法 , 会回退. 并不是上面那种根据时间来控制 , 这个是根据你执行cancel方法来控制, 我们简单的使用一下. 大家体会一下. 

```go
import (
	"context"
	"fmt"
	"time"
)
func main() {
	gen := func(ctx context.Context) <-chan int {
		dst := make(chan int)
		n := 1
		go func() {
			for {
				select {
				case <-ctx.Done():
					fmt.Printf("down")
					return // returning not to leak the goroutine
				case dst <- n:
					n++
				}
			}
		}()
		return dst
	}

	ctx, cancel := context.WithCancel(context.Background())
	defer func() {
		fmt.Println("cancel")
		cancel() // cancel when we are finished consuming integers
		time.Sleep(time.Second)
	}()

	for n := range gen(ctx) {
		fmt.Println(n)
		if n == 5 {
			break
		}
	}
}
```

比如这个代码, 我们看看输出 : 

```go
1
2
3
4
5
cancel // 由于for执行完毕. 所以主程序执行了cancel方法
down // 然后我们延迟了1s种, 等待子线程执行.
```

#### `context.WithValue()`

这个就是在context上下文中传递对象数据的.  然后返回一个新的上下文对象.  所以就是一个存储数据的东西. 他可以存储多个kv数据  . 并不能只存一个. 

```go
import (
	"context"
	"fmt"
	"time"
)

func main() {
	// 这个写法比较好 . 不断的赋值这个指针变量
	var ctx context.Context
	var cancel context.CancelFunc

	ctx, cancel = context.WithTimeout(context.Background(), time.Second*2)
	defer cancel()

	ctx = context.WithValue(ctx, "k1", "v1")
	ctx = context.WithValue(ctx, "k2", "v2")
	
	fmt.Printf("k1 : %s\n", ctx.Value("k1").(string))
	fmt.Printf("k1 : %s\n", ctx.Value("k2").(string))

	select {
	case <-ctx.Done():
		fmt.Println("waiting 2 seconds")
	}
}
```

输出   : 

```go
k1 : v1
k1 : v2
waiting 2 seconds
```



#### `cancel()` 方法

> ​	他会从上到下调用 .  我们可以这么看看

```go
func main() {
	var ctx context.Context
	var cancel context.CancelFunc

	ctx, cancel = context.WithTimeout(context.Background(), time.Second*2)
	defer func() {
		cancel()
		time.Sleep(time.Second)
	}()
	go func() {
		select {
		case <-ctx.Done():
			fmt.Println("ctx 1 Done")
		}
	}()

	ctx = context.WithValue(ctx, "k1", "v1")
	go func() {
		select {
		case <-ctx.Done():
			fmt.Println("ctx 2 Done")
		}
	}()

	ctx = context.WithValue(ctx, "k2", "v2")
	go func() {
		select {
		case <-ctx.Done():
			fmt.Println("ctx 3 Done")
		}
	}()
}
```

输出 :  所以就是从低到上 cancel.  (从下到上)

```go
ctx 3 Done
ctx 2 Done
ctx 1 Done
```



对于以上基本就是 golang的核心了 . 



### 解决我们的问题 . 

> ​	这里其实还有一个问题, 无法中断query执行.  或者就是无法终端goroutine执行. 

```go
// 1.查询服务
func Query(str string, ch chan<- []string) {
	time.Sleep(time.Second * 3)
	ch <- strings.Split(str, ".")
}

// 2.保存的数据库
func SaveUser() {
	time.Sleep(time.Second * 2)
}

// 2. User服务
func UserServer(str string) ([]string, error) {
	// 1.创建一个chan , 有一个缓冲区的目的是为了防止查询服务发生死锁, 比如说我们超时了,那么Query成功死锁
	ch := make(chan []string,1)
	// 2. 我们知道我们保存的时间是 2s, 所以我们允许等待最长时间为2s
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*2)
	defer cancel()
	go func() {
		// 执行query 是无法中断的, 所以这里只能等待执行完毕
		Query(str, ch)
	}()
	// 保存
	SaveUser()
	select {
	case <-ctx.Done():
		return nil, ctx.Err()
	case rest := <-ch:
		return rest, nil
	}
}
func main() {
	start := time.Now().UnixNano() / 1e6
	res, e := UserServer("hello.world.!")
	if e != nil {
		fmt.Println(e, time.Now().UnixNano()/1e6-start, "ms")
	} else {
		fmt.Printf("耗时 %dms , 查询结果 : %v.\n", time.Now().UnixNano()/1e6-start, res)
	}
}
```









### Google官文实例代码. . 

`com.test/context_example/main.go ` 文件代码

```go
import (
	"com.test/context_example/google"
	"com.test/context_example/userip"
	"context"
	"html/template"
	"log"
	"net/http"
	"time"
)

func main() {
	http.HandleFunc("/search", handleSearch)
	log.Fatal(http.ListenAndServe(":8080", nil))
}

// handleSearch handles URLs like /search?q=golang&timeout=1s by forwarding the
// query to google.Search. If the query param includes timeout, the search is
// canceled after that duration elapses.
func handleSearch(w http.ResponseWriter, req *http.Request) {
	// ctx is the Context for this handler. Calling cancel closes the
	// ctx.Done channel, which is the cancellation signal for requests
	// started by this handler.
	var (
		ctx    context.Context
		cancel context.CancelFunc
	)
	timeout, err := time.ParseDuration(req.FormValue("timeout"))
	if err == nil {
		// The request has a timeout, so create a context that is
		// canceled automatically when the timeout expires.
		ctx, cancel = context.WithTimeout(context.Background(), timeout)
	} else {
		ctx, cancel = context.WithCancel(context.Background())
	}
	defer cancel() // Cancel ctx as soon as handleSearch returns.

	// Check the search query.
	query := req.FormValue("q")
	if query == "" {
		http.Error(w, "no query", http.StatusBadRequest)
		return
	}

	// Store the user IP in ctx for use by code in other packages.
	userIP, err := userip.FromRequest(req)
	if err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}
	ctx = userip.NewContext(ctx, userIP)

	// Run the Google search and print the results.
	start := time.Now()
	results, err := google.Search(ctx, query)
	elapsed := time.Since(start)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	if err := resultsTemplate.Execute(w, struct {
		Results          google.Results
		Timeout, Elapsed time.Duration
	}{
		Results: results,
		Timeout: timeout,
		Elapsed: elapsed,
	}); err != nil {
		log.Print(err)
		return
	}
}

var resultsTemplate = template.Must(template.New("results").Parse(`
<html>
<head/>
<body>
  <ol>
  {{range .Results}}
    <li>{{.Title}} - <a href="{{.URL}}">{{.URL}}</a></li>
  {{end}}
  </ol>
  <p>{{len .Results}} results in {{.Elapsed}}; timeout {{.Timeout}}</p>
</body>
</html>
`))
```

`com.test/context_example/userip/query.go`文件

```go
package userip

import (
	"context"
	"fmt"
	"net"
	"net/http"
)

// FromRequest extracts the user IP address from req, if present.
func FromRequest(req *http.Request) (net.IP, error) {
	ip, _, err := net.SplitHostPort(req.RemoteAddr)
	if err != nil {
		return nil, fmt.Errorf("userip: %q is not IP:port", req.RemoteAddr)
	}

	userIP := net.ParseIP(ip)
	if userIP == nil {
		return nil, fmt.Errorf("userip: %q is not IP:port", req.RemoteAddr)
	}
	return userIP, nil
}

// The key type is unexported to prevent collisions with context keys defined in
// other packages.
type key int

// userIPkey is the context key for the user IP address.  Its value of zero is
// arbitrary.  If this package defined other context keys, they would have
// different integer values.
const userIPKey key = 0

// NewContext returns a new Context carrying userIP.
func NewContext(ctx context.Context, userIP net.IP) context.Context {
	return context.WithValue(ctx, userIPKey, userIP)
}

// FromContext extracts the user IP address from ctx, if present.
func FromContext(ctx context.Context) (net.IP, bool) {
	// ctx.Value returns nil if ctx has no value for the key;
	// the net.IP type assertion returns ok=false for nil.
	userIP, ok := ctx.Value(userIPKey).(net.IP)
	return userIP, ok
}
```

`com.test/context_example/google/search.go`  文件代码

```go
package google

import (
	"com.test/context_example/userip"
	"context"
	"encoding/json"
	"net/http"
)

// Results is an ordered list of search results.
type Results []Result

// A Result contains the title and URL of a search result.
type Result struct {
	Title, URL string
}

// Search sends query to Google search and returns the results.
func Search(ctx context.Context, query string) (Results, error) {
	// Prepare the Google Search API request.
	req, err := http.NewRequest("GET", "https://ajax.googleapis.com/ajax/services/search/web?v=1.0", nil)
	if err != nil {
		return nil, err
	}
	q := req.URL.Query()
	q.Set("q", query)

	// If ctx is carrying the user IP address, forward it to the server.
	// Google APIs use the user IP to distinguish server-initiated requests
	// from end-user requests.
	if userIP, ok := userip.FromContext(ctx); ok {
		q.Set("userip", userIP.String())
	}
	req.URL.RawQuery = q.Encode()

	// Issue the HTTP request and handle the response. The httpDo function
	// cancels the request if ctx.Done is closed.
	var results Results
	err = httpDo(ctx, req, func(resp *http.Response, err error) error {
		if err != nil {
			return err
		}
		defer resp.Body.Close()

		// Parse the JSON search result.
		// https://developers.google.com/web-search/docs/#fonje
		var data struct {
			ResponseData struct {
				Results []struct {
					TitleNoFormatting string
					URL               string
				}
			}
		}
		if err := json.NewDecoder(resp.Body).Decode(&data); err != nil {
			return err
		}
		for _, res := range data.ResponseData.Results {
			results = append(results, Result{Title: res.TitleNoFormatting, URL: res.URL})
		}
		return nil
	})
	// httpDo waits for the closure we provided to return, so it's safe to
	// read results here.
	return results, err
}

// httpDo issues the HTTP request and calls f with the response. If ctx.Done is
// closed while the request or f is running, httpDo cancels the request, waits
// for f to exit, and returns ctx.Err. Otherwise, httpDo returns f's error.
func httpDo(ctx context.Context, req *http.Request, f func(*http.Response, error) error) error {
	// Run the HTTP request in a goroutine and pass the response to f.
	c := make(chan error, 1)
	req = req.WithContext(ctx)

	go func() {
		c <- f(http.DefaultClient.Do(req))
		}()
	select {
	case <-ctx.Done():
		<-c // Wait for f to return.
		return ctx.Err()
	case err := <-c:
		return err
	}
}
```

