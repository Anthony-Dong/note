# Golang - 对象 & 接口

## 1. 对象 Struct

面向对象的三大属性`封装、继承和多态`  , 继承是类和接口都拥有的. 多态体现在方法的重载和实现 . 封装体现在技巧. 

> ​	Struct其实是面向过程的产物. 所以C有struct , 但是C++却有class. Java也是class. 其实都一样, 对于Golang来说struct拥有两者共有的特点. 

### 1. 简单 Struct

这里定义了一个user对象 , 他呢大写代表public. 成员变量首字母大写代表代表public. 他没有protected,只有 public和private . 

```go
type User struct {
	Name  string
	Age   int
	Skill func(interface{}) interface{}//对于Java可以理解是一个接口函数,这个写法
}
```

实例化 

```go
func main() {
	user := &User{Name: "xiaoli", Age: 100, Skill: func(i interface{}) interface{} {
		s, ok := i.(string)
		if ok {
			return "skill " + s
		}
		return i
	}}
	fmt.Printf("%#v\n", *user)

	res := user.Skill("football")//main.User{Name:"xiaoli", Age:100, Skill:(func(interface {}) interface {})(0x492eb0)}

	fmt.Println(res)// skill football
}
```



**其中有几个初始化的方法 , 基本就这么几个.**  

> ​	跟C++很像的. 不和Java一样. 只有new 关键字或者反射比较高级

```go
var user User  // 初始化一个空对象, 成员变量为初始值(基本类型为默认值,指针类型为nil). 对象不能和nil比较的,要知道. 

user:=User{} // 初始化一个空对象, 成员变量为初始值(基本类型为默认值,指针类型为nil).

var user *User  // 初始化一个指向User对象的指针 , 等于nil.

user := new(User) // 初始化一个指向User对象的指针. 不等于nil.

user := &User{}  //  初始化一个指向User对象的指针. 不等于nil.
```



### 2. 继承

首先父类 : 

```go
type People struct {
	Gender string
}
```

子类, 还是上面的User. 

```go
type User struct {
	People
	Name  string
	Age   int
	Skill func(interface{}) interface{}
}
```

这个就是继承 .  直接将父类对象写进去 , 记住不要申明变量

测试

```go
func main() {

	user := &User{}

	user.Gender="female"

	fmt.Printf("%#v\n",*user)
    // main.User{People:main.People{Gender:"female"}, Name:"", Age:0, Skill:(func(interface {}) interface {})(nil)}
}
```

那么如果父类有方法, 子类能调用吗. (向上转型)

```go
func (p *People) echo(s string) {
	fmt.Println("people say : ", s)
}
```

测试 

```go
func main() {

	user := &User{}

	user.echo("my name is anthony")
    // people say :  my name is anthony
}
```

所以也满足.  基本对象就这点了 . 



### 类型定义

和上面一样.  我们继续使用User. 

```go
type FemaleUser User

func (f *FemaleUser) operation() {
	f.Gender = "female"
	fmt.Printf("By an operation that change the people became a %s.\n", f.Gender)
}
```

测试 

```go
func main() {
    // 也可以使用类似于构造器,有些场景需要,比如基本数据类型.
    u := User{}
	user := FemaleUser(u)
    // 可以直接申明.
	user := &FemaleUser{}

	user.operation()
    // By an operation that change the people became a female.
}
```



## 2. 接口 

其实函数式接口就是 function type .(一个方法) , 但是有多个方法的时候, 就需要接口 , 实现接口必须实现接口全部的方法 . 

### 1. 实现

我们定义了一个基础能力`BaseCapacity`的接口. 

```go
type BaseCapacity interface {
	Run(int)
	Say(string)
}
```

然后User(还是上面定义的User类)去实现接口 .

```go
func (*User) Run(len int) {
	fmt.Printf("你可以跑%d公里.\n", len)
}

func (*User) Say(msg string) {
	fmt.Printf("你刚刚说了 : %s.\n", msg)
}
```

使用. 

```go
func main() {
	user := &User{}
	user.Run(1111)
	user.Say("my name is dong")
}
```

输出 

```go
你可以跑1111公里.
你刚刚说了 : my name is dong.
```

### 接口继承 

> ​	跟类继承一样 , 只要申明了就可以. 所以他可以继承多个接口. 我就写了一个 . 

我们的能力可能不光 , 基础能力, 所以还有学习能力. 所以我们定义了一个能力接口 

```go
type Capcity interface {
	BaseCapacity
	Study(string)
}
```

然后User. 实现接口 ,只需要再实现一个Study方法即刻

```go
func (*User) Study(msg string) {
	fmt.Printf("我正在学习%s.\n", msg)
}
```

测试 : 

```go
func main() {

	user := &User{}

	user.Run(1111)

	user.Say("my name is dong")

	user.Study("English")
}
```

输出

```go
你可以跑1111公里.
你刚刚说了 : my name is dong.
我正在学习English.
```

