# Golang-Goroutine

什么时候需要并发 ?  ,需要开启一个 goroutine , 比如我现在有一个任务很耗时,需要很久才能执行完, 那么我主线程其他行为就阻塞了 , 此时就需要一个goroutine 



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

这就是一个客户端服务器 , 很简单,



## 2. Channels

如果说goroutine是Go语音程序的并发体的话，那么channels它们之间的通信机制。一个 channels是一个通信机制，它可以让一个goroutine通过它给另一个goroutine发送值信息。每 个channel都有一个特殊的类型，也就是channels可发送数据的类型。一个可以发送int类型数 据的channel一般写为`chan	int` . 

其实和Java的`Exchanger` 很相似, 可以在两个线程之间交换数据,  其实`Channels`这个类似, 

### 无缓冲区的chan

申明 `make(chan int)` , 后面是类型, 就是最多我只能放一个.

```go
func main() {
    // 创建一个channel
	ch := make(chan int)

    go func() {
		// 发送数据 , 格式 : ch <- 数据 , 但是对于无缓冲区的chan这一步阻塞 , 称为 happens before
		ch <- 1
		// 直到另外一个人收到 , 才会执行这一步
		fmt.Println("发送", 1)
	}()
	
    x := <-ch
	fmt.Println("收到", x)
	time.Sleep(time.Second)
}
```

输出 : 

```go
收到 1
发送 1
```

还有一个问题 我们根据上述的改一下

```go
func main() {
	ch := make(chan int)

	go func() {
        // 睡觉2S
		time.Sleep(time.Second*2)
		ch <- 1
		fmt.Println("发送", 1)
	}()

	x:=<-ch
	fmt.Println("收到", x)
	time.Sleep(time.Second)
}
```

输出

```go
收到 1
发送 1
```

但是我们把go去掉, 或者直接把func去掉 , 会发生什么, 会抛出 `fatal error: all goroutines are asleep - deadlock!` 这个异常 , 他们称之为死锁 . 主要原因还是在于主线程的问题, 主线程是不会无脑的阻塞的. 所以就是这个问题 . 

```go
func main() {
	ch := make(chan int)
	x := <-ch
	fmt.Println("收到", x)
	time.Sleep(time.Second*5)
}
```

第一步, 我们分析一下逻辑,  当程序执行到这里`x:=<-ch`  , 此时主线程将不会继续执行, 但是你前面有没有开启goroutine, 此时他会自动认为你被死锁了, 所以直接抛出异常. 这就是原因 . 

还有一种情况

```go
func main() {
	// 创建一个channel
	ch := make(chan int)

	go func() {
		time.Sleep(time.Second * 2)
		fmt.Println("发送")
	}()

	x := <-ch
	fmt.Println("收到", x)
}
```

此时执行到 `x := <-ch` 主线程阻塞了, 此时子线程继续执行, 但是当子线程死亡, 就是执行完毕, golang会自动判断他没必要继续阻塞了, 他认为这个是死锁,所以此时就立马抛出异常 , 



### 具有缓冲区的chan

申明  `make(chan int, 10)`  , buf大小=10 , 类型int

```go
func main() {
	cint := make(chan int, 10)

	go func() {
		for x:=1;x<10;x++ {
			cint<-x
		}
	}()

	for x:=1;x<10;x++ {
		fmt.Printf("%v\t",<-cint)
	}
}
```

输出 

```go
1	2	3	4	5	6	7	8	9	
```

但是此时如果我们将子线程中的, x<10 改成 <9 . 此时子线程执行完毕会发现, 主线程还在在等待一个 ,但是此时他发现没有人会生产了 , 没必要死锁所以就抛出异常 了 . 