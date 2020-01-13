# GoLang的败笔-异常机制

> Go语言追求简洁优雅，所以，Go语言不支持传统的 try…catch…finally 这种异常，因为Go语言的设计者们认为，将异常与控制结构混在一起会很容易使得代码变得混乱。因为开发者很容易滥用异常，甚至一个小小的错误都抛出一个异常。在Go语言中，使用多值返回来返回错误。不要用异常代替错误，更不要用来控制流程。在极个别的情况下，也就是说，遇到真正的异常的情况下（比如除数为0了）。才使用Go中引入的Exception处理：defer, panic, recover

`panic`  英文是 恐慌  , 其实就是制造异常的意思



## 1. error

就是一个接口而已, 需要实现他的Error方法而已,返回一个字符串

```go
type error interface {
	Error() string
}
```

所以对于golang来说, 异常就是这 , 需要直接返回, 不反回代表没有异常 , 

```go
func main() {
	i, e := external()
	fmt.Println(i, e)
}

func external() (int, error) {
	return 1, errors.New("external 异常")
}
```



## 2. panic recover defer

简单使用

```go
func main() {
    // 必须用defer申明一个全局的抓取panic机制, 需要一个recover
	defer func() {
		if err := recover(); err != nil {
			fmt.Println("异常：", err) // 这里的err其实就是panic传入的内容
		}
	}()

	fun1()
	fun2()
	fun3()
}

func fun1() {
	fmt.Println("fun1 执行")
}
func fun2() {
	panic("fun2 抛出一个panic")
	fmt.Println("fun2 执行")
}
func fun3() {
	fmt.Println("fun3 执行")
}
```

输出 

```go
fun1 执行
异常： fun2 抛出一个panic
```



如果不用defer声明

```go
func main() {

	fun1()
	fun2()
	fun3()
}

func fun1() {
	fmt.Println("fun1 执行")
}
func fun2() {
	panic("fun2 抛出一个panic")
	fmt.Println("fun2 执行")
}
func fun3() {
	fmt.Println("fun3 执行")
}
```

输出  : 程序直接停止了. 

```go
fun1 执行
panic: fun2 抛出一个panic

goroutine 1 [running]:
main.fun2(...)
	D:/代码库/golang/src/github.anthonydong.io/web/main/test_fun.go:24
main.main()
	D:/代码库/golang/src/github.anthonydong.io/web/main/test_fun.go:16 +0x9f
```

这里还有就是 如果你开启了 goroutine 他也是如此

```go
func main() {
	fun1()
	go fun2()
	fun3()

	// 等待 goroutine执行
	time.Sleep(time.Second * 5)
}

func fun1() {
	fmt.Println("fun1 执行")
}
func fun2() {
	panic("fun2 抛出一个panic")
	fmt.Println("fun2 执行")
}
func fun3() {
	fmt.Println("fun3 执行")
}
```

输出

```go
fun1 执行
fun3 执行
panic: fun2 抛出一个panic

goroutine 19 [running]:
main.fun2()
	D:/代码库/golang/src/github.anthonydong.io/web/main/test_fun.go:26 +0x40
created by main.main
	D:/代码库/golang/src/github.anthonydong.io/web/main/test_fun.go:16 +0x9a
```



所以很坑  ,就算是我们goroutine外不抓取 了



```go
func main() {

	defer func() {
		if e := recover(); e != nil {
			fmt.Println("抓取异常 : ", e)
		}
	}()

	fun1()
	go fun2()
	fun3()

	// 等待 goroutine执行
	time.Sleep(time.Second * 5)
}

func fun1() {
	fmt.Println("fun1 执行")
}
func fun2() {
	panic("fun2 抛出一个panic")
	fmt.Println("fun2 执行")
}
func fun3() {
	fmt.Println("fun3 执行")
}
```

输出也是如此

```go
fun1 执行
fun3 执行
panic: fun2 抛出一个panic

goroutine 19 [running]:
main.fun2()
	D:/代码库/golang/src/github.anthonydong.io/web/main/test_fun.go:34 +0x40
created by main.main
	D:/代码库/golang/src/github.anthonydong.io/web/main/test_fun.go:23 +0xba
```



所以必须在他的函数内部申明 , 也就是 fun2

```go
func fun2() {
	defer func() {
		if e := recover(); e != nil {
			fmt.Println("抓取异常 : ", e)
		}
	}()
	panic("fun2 抛出一个panic")
	fmt.Println("fun2 执行")
}
```

输出

```go
fun1 执行
fun3 执行
抓取异常 :  fun2 抛出一个panic
```

