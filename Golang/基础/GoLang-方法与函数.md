# Golang - 方法与函数

> ​	其实就是一字之差 , 函数属于任何人, 方法属于对象

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

什么是匿名函数 , 就死没有名字呗 , 类似于Java的匿名内部类 , 或者直接说Lambda也可以 , 就是函数没有名字 , 就是函数没有名字

```go
var f1 = func(x, y int) (int) {
    fmt.Println(x + y)
    return x + y
}

sum := f1(1, 2)

fmt.Println(sum)
```

### 立即执行函数

Golang中的写法就是下面的样子 , 他呢很简单, 三部分组成 , 第一部分填充参数, 第二部分就是函数体, 第三部分就是传参

```go
func(args) {
    //body
}(args)
```

举个例子

```go
for x := 0; x < 10; x++ {
    func(i int) {
        fmt.Printf("%d\t",i)
    }(x+10)
}
```

输出

```go
10	11	12	13	14	15	16	17	18	19	
```

### 函数参数

```go
// 里面执行
func f2(f1 func(int, int), x1, x2 int) {
	f1(x1, x2)
}

// 返回值函数
func f3(f1 func(int, int)(int), x1, x2 int)(int) {
	return f1(x1, x2)
}


func f4(f1 func(int, int) (int), x1, x2 int) (int) {
	return f1(x1, x2)
}


func f5(f1 func(int, int) (int), x1, x2 int) (func() (int)) {
	return func() int {
		return f1(x1, x2)
	}
}
```



### 闭包

就是参数 从内到外找 , 么了 . 

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

上面的代码里那个附加的参数s，可以认为是Java中的this关键字 , 这个在go里面叫做接收器 , 我们的写法他相当于拷贝了一个Singer对象给getName方法



### 指针对象的方法

当调用一个函数时，会对其每一个参数值进行拷贝，如果一个函数需要更新一个变量，或者 函数的其中一个参数实在太大我们希望能够避免进行这种默认的拷贝，这种情况下我们就需要用到指针了。

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
singer := Singer{name: "张靓颖"}

fmt.Println(singer.name)

singer.setName("邓紫棋")

fmt.Println(singer.name)
```

go语言本身在这种地方会帮到我们,如果接收器s是 一个Singer类型的变量，并且其方法需要一个Singer指针作为接收器，编译器会隐式地帮我们用&s去调用setName这个方法 , 这种简写方法只适用于“变量.

摘抄一段话 : 

译注：	作者这里说的比较绕，其实有两点：

1.	不管你的method的receiver是指针类型还是非指针类型，都是可以通过指针/非指针类型 进行调用的，编译器会帮你做类型转换。
2.	 在声明一个method的receiver该是指针还是非指针类型时，你需要考虑两方面的内部，第 一方面是这个对象本身是不是特别大，如果声明为非指针变量时，调用会产生一次拷贝；第二方面是如果你用指针类型作为receiver，那么你一定要注意，这种指针类型指向的始终是一块内存地址，就算你对其进行了拷贝。熟悉C或者C艹的人这里应该很快能明白。



其中这个receive可以为nil,为空指针

```go
var singers Singer
name := singers.getName()
fmt.Println("name : ",name) // 输出空,但是不报错
```





