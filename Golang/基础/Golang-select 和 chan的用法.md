# Golang-select 和 chan的用法

> ​	鄙人学了几天后发现, 基础玩意基本都一样 , golang高级点的部分在于 goroutine , 其实还有个难点就是 chan , 后面context会很难理解.  因为开一个goroutine很简单, 因为任务没有关系, 但是如果涉及到任务关系会特别复杂

## chan

管道 , 就是一个用来在不同goroutine之间传递的(主线程也是一个goroutine) , 和Java不一样 , Golang中所有的Goroutine都是守护线程(Java的说法) , 也就是说主线程结束后进程就会推出, 不会等Goroutine执行. Java中就是这个概念(进程中主线程结束后除了主线程之外如果全部都是守护线程,那么进程就会推出)

线程的概念和gorouting很相似, 所以我们就不做讲解. 开启就只需要一个关键字 go  . 后面跟一个立即执行函数就行了. 

**发送和接收类型记得一致.**  chan的返回值是两个, 默认写一个是接收到的数据  `value,bool<-ch`  , 当close掉会返回false

缓冲区的意思就是 : 

A给B发送消息了 , 哎没有邮箱哇, 那咋办我只能一直给他拨号 , 知道他接听了 , 我把消息告诉他. 

 而有缓冲区呢, 比如有一个邮箱,但是最多只能存入一封信

A给B发送消息了 , 管他B在不在 我先发给他邮箱. 但是我再给他发信息, 哇邮箱满了,我只能等B接我电话了.(体验上for 和 for+select+default效果是不一样的. )

### 无缓冲区的chan

> ​	无缓冲区的chan ,要求发送方发送数据,只有等到有接收方接收才会停止等待(阻塞.) 
>
> ​	接收方只有收到数据才会停止等待.

```go
func main() {

	c1 := make(chan interface{})

	start := time.Now().UnixNano() / 1e6

	go func() {
		time.Sleep(1000 * time.Millisecond)
		c1 <- "OK"
		fmt.Printf("发送数据耗时 : %d ms\n", time.Now().UnixNano()/1e6-start)
	}()

	go func() {
		x := <-c1
		time.Sleep(1000 * time.Millisecond)
		fmt.Printf("耗时 : %d ms, 接收到数据 %s !\n", time.Now().UnixNano()/1e6-start, x)
	}()

	time.Sleep(time.Second * 5)
}
```

输出

```go
发送数据耗时 : 1001 ms
耗时 : 2001 ms, 接收到数据 OK !
```

所以基本就是个这. 

### 有缓冲区的chan

就是可以缓冲多个数据

```go
func main() {
	ch := make(chan int, 1)
	ch <- 1
	select {
	case ch <- 2:
		fmt.Println("read in channel !")
	default:
		fmt.Println("channel is full !")
	}
}
```

输出

```go
channel is full !
```

但是当我改成2

```go
func main() {
	ch := make(chan int, 2)
	ch <- 1
	select {
	case ch <- 2:
		fmt.Println("read from channel !")
	default:
		fmt.Println("channel is full !")
	}
}
```

输出

```go
read from channel !
```



### 只读 只写 chan

> ​	其实对于我们有些时候见到源码中 发现好多好多参数是  <-chan string 或者 返回值是  <-chan string , 无语

其实就是只读 只写channel . 

可以以下这么定义 ,但是没有意义, 因为怎么都会报错. 

```go
read_only := make (<-chan int)
write_only := make (chan<- int)
```

比如说 

```go
func main() {
	reader := make(<-chan string)

	go func() {
		str:=<-reader
		fmt.Println(str)
	}()
	time.Sleep(time.Second*5)
}
```

这样写, 有意义吗 . 显然是没有 . 因为没法输入 . 



**其实普通的chan(可读写chan) 是 只读只写chan的(父类). 所以可以这么使用 .**

```go
var ShutDown = make(chan interface{})
func main() {
	c := make(chan int)
	go send(c)
	go recv(c)

	// wait ...
	<-ShutDown
}

//只能向chan里写数据
func send(c chan<- int) {
	for i := 0; i < 10; i++ {
		c <- i
	}
	ShutDown <- ""
}

//只能取channel中的数据
func recv(c <-chan int) {
	for i := range c {
		fmt.Println("receive : ", i)
	}
}
```

输出 

```go
receive :  0
receive :  1
receive :  2
receive :  3
receive :  4
receive :  5
receive :  6
receive :  7
receive :  8
receive :  9
```



### 关闭 chan

有些情况下需要关闭 chan  , 那就涉及到 chan 的状态了

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-10-11/20fe4b2d-6a33-449a-be68-8e43406f6147.jpg?x-oss-process=style/template01)

所以一般在写入(也就是sender中)的goroutine中执行 close 操作. 

`func close(c chan<- Type)`  , 显然是接收一个 写入的channel

```go
func main() {

	ch := make(chan string)

	go func() {
		for x := 1; x < 5; x++ {
			ch <- "ok"
		}
		close(ch)
	}()

	go func() {
		for {
			select {
			case x, ok := <-ch:
				if ok {
					fmt.Println(x, ok)
				} else {
					fmt.Println(x == "", ok)
					os.Exit(-1)
				}
			}
		}
	}()

	time.Sleep(time.Second * 10)
}
```



注意点 

> close没有make的chan会引起panic , 也就是进程直接退出 . 在你没有抓取异常的情况下. 
>
> close以后不能再写入，写入会出现panic
>
> close之后可以读取，无缓冲chan读取返回0值和false，有缓冲chan可以继续读取，返回的都是chan中数据和true，直到读取完所有队列中的数据。
>
> 重复close会引起panic
>
> 只读chan不能close
>
> 不close chan也是可以的，当没有被引用时系统会自动垃圾回收。



## 2. select ... case 

>select 先遍历所有case, 所有channel表达式都会被求值、所有被发送的表达式都会被求值。求值顺序：自上而下、从左到右.
>
>当case没有阻塞则**随机**执行一个没有阻塞的case就退出select
>
>当所有case阻塞时, 则一直阻塞直到某个case解除阻塞, 但是如果有default则直接执行default
>
>也就是一个select最多**只执行一次case**里的代码
>
>要一直检测case则必须外层使用for循环包起来



他不同于switch...case , 他只负责IO操作 , case收到的是IO操作

```go
func main() {
	c1 := make(chan interface{})
	c2 := make(chan interface{})

	go func() {
		time.Sleep(time.Millisecond * 500)
		c1 <- 1
	}()
	go func() {
		time.Sleep(time.Millisecond * 100)
		<-c2
	}()

	select {
	case <-c1:
		fmt.Println("read from c1")
	case c2 <- 1:
		fmt.Println("send to c2")
	}
}
```

会输出 

```go
send to c2
```

上面没有godefault修饰的会一直循环等待到, 有一个IO操作完成,才会结束  , 下面这个会直接退出

```go
select {
case <-c1:
    fmt.Println("read from c1")
case c2 <- 1:
    fmt.Println("send to c2")
default:
    os.Exit(-1)
}
```



## chan 有啥用? 

有些时候 我们在学一些东西, 一直不知道他到底有啥用 ? , 所以带给我一些疑惑 .  对于Java语言来说, 他也有, 我举例子 . 

```java
public class TestExchange {


    public static void main(String[] args) {
        Exchanger<String> exchanger = new Exchanger<>();

        new Thread(() -> {
            try {
                String exchange = exchanger.exchange(String.format("HELLO , MY NAME IS %s .", Thread.currentThread().getName()));
                System.out.printf("Thread : %s  receive \" %s \"\n", Thread.currentThread().getName(), exchange);
            } catch (InterruptedException e) {
                //
            }
        }).start();

        new Thread(() -> {
            try {
                String exchange = exchanger.exchange(String.format("HELLO , MY NAME IS %s .", Thread.currentThread().getName()));
                System.out.printf("Thread : %s  receive \" %s \"\n", Thread.currentThread().getName(), exchange);
            } catch (InterruptedException e) {
                //
            }
        }).start();
    }
}
```

输出 : 

```java
Thread : Thread-0  receive " HELLO , MY NAME IS Thread-1 . "
Thread : Thread-1  receive " HELLO , MY NAME IS Thread-0 . "
```

我们发现我们也成功交换信息了哎 . 是吧. 但是用处难道真的是这个吗 . 其实还有个蛮好的用处 . 

假设我们想一下 . 我们现在有一个需求就是 , 需要异步处理一个任务. 我们会在任务结束的时候拿到这个任务结果 , 那么这个是不是就有用处了 呢 . 对于Java的显然是不好实现的, 他必须交换信息. 但是Chan是单向, 



我们现在有个任务就是处理一下任务 ,  比如查询 , 写入任务 . 我们查询可以来一个goroutine来处理 .

```go
func query(str string) []string {
	time.Sleep(time.Second * 2)
	return strings.Split(str, ".")
}

func server(str string) []string {
	ok := make(chan []string)

	go func() {
		ok <- query(str)
	}()
	time.Sleep(time.Second * 2)
	return <-ok
}

func main() {
	start := time.Now().UnixNano() / 1e6
	res := server("hello.world.!")
	fmt.Printf("耗时 %dms , 查询结果 : %v.\n",time.Now().UnixNano()/1e6-start, res)
}
```

输出: 

```go
耗时 2000ms , 查询结果 : [hello world !].
```

这样很好的处理了等待结果 . 如果我们顺序执行的话. 那么会耗时 4000ms . 所以很好的使用 chan 会带来很棒的效果.  异步拿去结果 . 其实跟Java的Future 很像 . 



但是还有一个问题就是,  如果我们超时了咋办 . 如何处理呢 .  万一我们查询时间需要耗时 10m 呢 .  我们怎么处理呢 ?  





