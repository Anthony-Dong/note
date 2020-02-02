# Golang 断言 ? 类型转换

> ​	断言是相对于interface{}提供的功能. 由于Golang存在大量的 type定义类型. 所以需要类型转换, 而interface{} 是所有对象的父类  . 

## 类型转换

我们知道 golang中一切对象都是 `interface{}` , 所以我们可以拿它当返回值

```go
func reserve(msg string) interface{} {
	return msg
}

func main() {
    // 返回一个interface{}
	msg := reserve("message")
    
    // 再次调用? 异常
    // cannot use msg (type interface {}) as type string in argument to reserve: need type assertion
	reserve(msg)
}
```

这里就提示我们了需要使用 `type assertion`  ,那么问题来了 类型断言是什么 ? 

其实很简单 .... 格式就是    `c,ok : = a.(Type)  `  返回一个转换后的类型对象和一个boolean值

所以这下我们试试 , 此时发现很简单, 直接转换了, 

```go
func reserve(msg string) interface{} {
	return msg
}

func main() {
	msg := reserve("message")

	s,ok := msg.(string)
	if ok {
		reserve(s)
	}
}
```

断言基本都是这么用了 , 默认如果你不写 ok, 那么返回的参数是转换后的类型. 



##  switch case 判断类型

还有就是 switch case 使用断言判断类型

```go
import "fmt"

type A int
type B int
type C int

func main() {
	b := B(1)
	getType(b)
}

func getType(val interface{}) {

	switch val.(type) {
	case A:
		fmt.Println("A 类型")
	case B:
		fmt.Println("B 类型")
	case C:
		fmt.Println("C 类型")
	default:
		fmt.Println("未知类型")
	}
}
```

输出

```go
B 类型
```

