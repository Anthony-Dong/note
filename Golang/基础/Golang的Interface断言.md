# Golang - interface & 断言

> ​	断言是接口特有的, 接口类型比如申明一个接口 ,比如 `type Service interface{}`, 还有接口对象,比如` interface{}` , 对于Golang来说 , 所有类型的父类都是` interface{}` , 其实也不难理解. 因为每个对象至少都含有0个方法.  
>
> 所以` func f1(x interface{}){}` , 既可以传递一个指针类型, 也可以传递一个原类型. 并需要写成 `func f1(x *interface{}){}`

## interface{} 对象类型转换

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



## interface 对象类型转换

这里是一个接口.

```go
type Service interface {
	Echo(string)
}
```

这里是一个接口实现类 

```go
type ServiceImpl struct {
	Name string
}

func (ServiceImpl) Echo(s string) {
	fmt.Println("echo : " + s)
}
```

如何类型转换呢, 有些时候是需要接口转换成原来对象的. 

```go
func f2(inter Service, str string) {
    // 类型转换
	impl := inter.(*ServiceImpl)
	impl.Name = "tom"
	inter.Echo(str)
}
```

调用 

```go
func main() {
	// 实例化
	impl := &ServiceImpl{Name: "anthony"}
	// 调用函数
	f2(impl, "hello world")
	// 打印, 查看是否修改过了
	fmt.Printf("name is %s.\n", impl.Name)
}
```

输出 : 

```go
echo : hello world
name is tom.
```



> ​	其实通过这个想告诉大家, 其实对于接口默认传递是支持指针传递的. 但是必须原传递为指针传递, 
>
> ​	比如上诉例子 `impl := &ServiceImpl{Name: "anthony"}` , impl为指针类型, 但是` f2(inter Service, str string)`  接收的却不是指针类型 , 因为接口,好比上面说的 `interface{}` . 他是爹. 他可以表示一切 , 这里也是. 既可以传递指针 , 也可以拷贝值. 
>
> ​	正如好多做类型转换的需要你显示指定 指针类型.  比如 json包下的`func Unmarshal(data []byte, v interface{}) error` , 第二个v必须是一个指针类型. 





> ​	其实很多人考虑多线程的问题,  啥时候会出现多线程的问题, 其实我们只用考虑 , 啥是线程私有的, 啥是线程共有的部分, Java中表述的比较好 , JVM的堆和方法区都是共有的, JVM栈和计数器是线程私有的  , 所以你看看对象到底是属于哪个  , 就知道多线程问题了. 