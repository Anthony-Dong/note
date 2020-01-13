# GoLang - FunctionType

> ​	function type 可以理解为一组拥有相同参数类型和结果类型的方法的集合。我看也有人管他叫接口型函数。



> A function type denotes the set of all functions with the same parameter and result types. The value of an uninitialized variable of function type is nil.
>
> Two function types are identical if they have the same number of parameters and result values, corresponding parameter and result types are identical, and either both functions are variadic or neither is. Parameter and result names are not required to match.



### 定义

我们定义了一个处理字符串的函数, 要直接修改原字符串 , 所以需要指针引用 , 不需要返回值 , 所以定义了一下接口

```go
type Handler func(*string)
```

定义一个处理器

```java
func process(handler Handler, source *string) {
	handler(source)
}
```

使用

```go
func main() {
	msg := "hello world"

	process(func(source *string) {
		*source = *source + " !"  // 就是加一个 !结尾
	}, &msg)

    fmt.Println(msg) // 输出 : hello world !
}
```

所以他其实就是一个lambda表达式 , 功力尚且不知道语法糖长啥样子



还可以实现

```go
type Handler func(*string)


// 只有参数类型 和 返回值一致就可以了
func handler(source *string) {
	*source = *source + "!"
}

func main() {
	msg := "hello world"
	process(handler, &msg)
	fmt.Println(msg) // 输出 : hello world !
}
```



