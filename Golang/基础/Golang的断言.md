# Golang 断言 ? 类型转换

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