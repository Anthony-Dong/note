# Golang - 方法与函数

>  其实就是一字之差 , 函数是一个指针变量, 方法是一个对象的(面向对象的产物,比如` a.run(100)`) , 懂了吧. 

## 1. 函数

函数声明包括函数名、形式参数列表、返回值列表（可省略）以及函数体。

```go
func name(parameter-list)(result-list){
    body 
}
```

**形式参数**列表描述了函数的参数名以及参数类型。这些参数作为局部变量，其值由参数调用者提供。**返回值列表**描述了函数返回值的变量名以及类型。如果函数返回一个无名变量或者没有返回值返回值列表的括号是可以省略的。如果一个函数声明不包括返回值列表，那么 函数体执行完毕后，不会返回任何值。	

例如一个翻转切片的函数 , 你可以直接调用

```go
func reserve(arr []int) {
	for x, y := 0, len(arr)-1; x < y; x, y = x+1, y-1 {
		arr[x], arr[y] = arr[y], arr[x]
	}
}
```

main函数调用 : 

```go
arr := []int{0, 1, 2, 3, 4, 5}

reserve(arr)
 
fmt.Println(arr)  //[5 4 3 2 1 0]
```

这就是一个函数 . 

### 匿名函数

什么是匿名函数 , 就死没有名字呗 ,  或者直接说Lambda也可以 , 就是函数没有名字 , 就是函数没有名字

```go
var f1 = func(x, y int) (int) {
    fmt.Println(x + y)
    return x + y
}

sum := f1(1, 2)

fmt.Println(sum)
```

### 立即执行函数

其实就是只允许一次, 不能重复使用. 

他呢很简单, 三部分组成 , 第一部分填充参数, 第二部分就是函数体, 第三部分就是传参

```go
返回值:=func(args)(返回类型){
    //body
}(args)
```

举个例子

```go
s := func(s string) string {
    fmt.Println("调用我了 : ",s)
    return s
}("a")

fmt.Println(s)
```

输出

```go
调用我了 :  a
a
```

### 函数对象

每一个函数都是一个对象, 他可以互相传递, 所以可以当做参数, 当做返回值. 都可以 

记住func变量他就是一个指针, 懂了吧, 怎么区分一个变量是不是指针, 你用过编辑器的拿到func对象输入 `func==nil` , 看他提不提示异常, 提示了就说明是指针. 

#### 作为参数

```go
/**
传入一个函数 来处理我们传入的参数 , 打印一下他的地址
 */
func fun1(f func(string) string, s string) string {
	fmt.Printf("%p\n", f)
	return f(s)
}
```

实现 . 

```go
func main() {
	// 创建一个函数对象指针
	fun := func(s string) string {
		return strings.Join(strings.Split(s, " "), "-")
	}

	// 传递
	res := fun1(fun, "hello world")

	// 打印
	fmt.Printf("%p\n", fun)

    // 结果.
	fmt.Printf("result : %s\n", res)
}
```

输出 : 说明啥.... 

```go
0x494950
0x494950
result : hello-world
```

#### 作为返回值 ,这个也很方便.

```go
func fun2(name string) func(string) string {
	return func(s string) string {
		// 这里的name就是一种闭包行为
		return name + " run " + s
	}
}

func main() {
    // 创建一个函数
	fun := fun2("fun2")
    // 使用
	res := fun("hello")
	fmt.Println(res)
}
```

输出

```go
fun2 run hello
```

### 闭包

就是参数 从内到外找 , 么了 .  所以就近原则. 

## 2. 方法

### 方法申明

在函数声明时，在其名字之前放上一个变量，即是一个方法。这个附加的参数会将该函数附加到这种类型上，即相当于为这种类型定义了一个独占的方法。

```go
type Singer struct {
	name string
}

func (s Singer) getName() string {
	return s.name
}

func main() {
	singer := Singer{name: "张靓颖"}
    // get方法
	fmt.Println(singer.getName())
}
```

上面的代码里那个附加的参数s，可以认为是Java中的this关键字 , 这个在go里面叫做接收器 , 我们的写法他相当于拷贝了一个Singer对象给getName方法.

### 指针对象的方法

当调用一个函数时，会对其每一个参数值进行拷贝，如果一个函数需要更新一个变量，或者函数的其中一个参数实在太大我们希望能够避免进行这种默认的拷贝，这种情况下我们就需要用到指针了。

```go
type Singer struct {
	name string
}

func (s *Singer) setName(name string) {
	s.name = name
}

func main() {
	singer := Singer{name: "张靓颖"}

	fmt.Println(singer.name) //张靓颖

    (&singer).setName("邓紫棋") // 指针调用

	fmt.Println(singer.name) // 邓紫棋
}
```

其实有心的人会发现不需要指针变量也可以对其修改的, 所以改成了下面的 ,也是可以编译成功的 ,原因如下

```go
func main() {
    singer := Singer{name: "张靓颖"}

    fmt.Println(singer.name)

    singer.setName("邓紫棋")

    fmt.Println(singer.name)
}
```

> ​	go语言本身在这种地方会帮到我们,如果接收器s是 一个Singer类型的变量，并且其方法需要一个Singer指针作为接收器，编译器会隐式地帮我们用&s去调用setName这个方法 , 这种简写方法只适用于“变量".

## 3. Function Type

定义一个function-type , 他对比接口的区别就是他只有一个方法而已, 其他都一样的 . 他需要传入一个匿名函数(跟Java特别像) , 但是它有着匿名函数没有的功能, 他同时也是一个对象 . 

```go
type HandlerString func(string, []string) string
```

也可以作为对象. 

```go
func (fun HandlerString) Merger(w string, r []string) {
    // 验证
    fmt.Printf("%p\n",fun)
    // 调用
	fun(w, r)
}
```

对于`Function Type` , 他是一个指针对象. 

怎么说呢. 我们可以 

```go
func main() {

	fun := HandlerString(func(s string, arr []string) string {
		return "ok"
	})

	fmt.Printf("%p\n",fun)

	fun.test("",nil)
}
```

输出 : 

```go
0x492be0
0x492be0
```

