# Go - unsafe包

官方文档 ： https://golang.org/pkg/unsafe/



## 简单介绍

unsafe很简单就是不安全，Java中也有(我原来写的一篇文章：https://anthony-dong.gitee.io/post/java-unsafe-lei/)，而且使用Unsafe类可以做很多操作，比如字段注入，cas等，核心其实Java也是有一个offset，每个对象都每个字段都会有对应的偏移量。

关于偏移量，不同编译器的算法不同，随意不能单纯的简单数学计算。

比如：

```go
type Foo struct {
	Age1 int8
	Age2 int8
	Age3 int32
	Age4 int64
}
// 这个结构体大小是16个字节，为什么不是14个字节呢，因为补齐了。
```





## Pointer  和 uintptr

> ​	这俩必须放到一起，因为他俩是紧密相连的，互相作用，互相转换，基本unsafe就是玩这俩，下面是官方的定义

Pointer represents a pointer to an arbitrary type. There are four special operations available for type Pointer that are not available for other types:

```
- A pointer value of any type can be converted to a Pointer.
- A Pointer can be converted to a pointer value of any type.
- A uintptr can be converted to a Pointer.
- A Pointer can be converted to a uintptr.
```

官方给的定义，指针可以指向任意类型的指针，指针类型有四种特殊操作，而其他类型没有的，

```
-任何类型的指针值都可以转换为Pointer。
-Pointer可以转换为任何类型的指针值。
-uintptr可以转换为Pointer。
-Pointer可以转换为uintptr。
```



- 前两句大概可以用如下实现，下面很简单是将一个

```java
func convert(f uint64) uint32 {
  // 任何指针类型可以转换为 Poniter
	pointer := unsafe.Pointer(&f)
  // Pointer可以转换为任何类型的指针值
	return *(*uint32)(pointer)
}
```



- 其次就是后面两句，互相转换

```go
type Foo struct {
	Name string
}
func setName(f *Foo) {
	_type := reflect.TypeOf(*f)
	field, _ := _type.FieldByName("Name")
  // uintptr类型
	u := field.Offset

  // Pointer类型
	pointer := unsafe.Pointer(f)

  // uintptr(pointer) Pointer类型可以转换为uintptr， 同时unsafe.Pointer(uintptr(pointer) + u)可以发现uintptr可以转换为Pointer
	name := (*string)(unsafe.Pointer(uintptr(pointer) + u))
	*name = "小李"
	fmt.Printf("%+v\n", *f)
}

func main() {
	setName(&Foo{})
}
```



以上就是两者的介绍，大致可以发现 Poniter的参数必须是指针(不能是空指针nil)，同时Pointer可以转换为任意类型的其他指针，

uintptr主要适用于地址计算和偏移量等，类似于Java的Unsafe设计，所以可以随意进行设置值（最主要是可以反射非公有类型）



- 注意reflect包中的Pointer方法返回的是uintptr

```go
u := reflect.ValueOf(new(int)).Pointer()
p := (*int)(unsafe.Pointer(u))
*p = 1000
fmt.Println(*p)
```





## unsafe.Offsetof(structValue.field)

> ​	这个主要是服务于结构体的，计算算结构体字段的偏移量 。  Go语言的规范并没有要求一个字段的声明顺序和内存中的顺序是一致的，所以理论上一个编译器可以随意地重新排列每个字段的内存位置。也就是你挑换结构体字段的顺序也是会发生变化的

```go
func main() {
	foo := new(model.Foo)
	// 这个参数，是一个字段类型，必须是，不然panic
	filedOffset := unsafe.Offsetof(foo.Name)
	pointer := (*string)(unsafe.Pointer(uintptr(unsafe.Pointer(foo)) + 0))
	*pointer = "Tom"
	fmt.Printf("value: %+v, offset: %d.", *foo, filedOffset)
}

// value: {Name:Tom}, offset: 0.
```

**这里可以看到偏移量确实是对齐的， name是第一个字段，而第一个字段的偏移量始终是0.**

此时我们加入第二个字段

```go
func main() {
	foo := new(model.Foo)
	// 这个参数，是一个字段类型，必须是，不然panic
	filedOffset := unsafe.Offsetof(foo.Age)
	pointer := (*int8)(unsafe.Pointer(uintptr(unsafe.Pointer(foo)) + filedOffset))
	*pointer = 10
	fmt.Printf("value: %+v, offset: %d.", *foo, filedOffset)
}
// value: {Name: Age:1111 Clazz:}, offset: 16.
```

大致是因为 string的长度是16（字符串对应结构体中的指针和字符串长度组成，所以偏移量+8就是字符串长度）。



探究一下字符串问题，哈哈哈哈，确实如此

```go
func main() {
	foo := model.Foo{}
	foo.Name = "hello world"
	len := (*int64)(unsafe.Pointer(uintptr(unsafe.Pointer(&foo)) + 8))
	fmt.Printf("字符串长度：%d\n", *len)
}
// 字符串长度：11
```







> 那么这个是正向思想，如何ioc呢，不需要对象.字段，这就是下一篇的反射了。





## unsafe.Sizeof()

> ​	计算一个对象的大小，指针、结构体、基本数据类型 , 单位是字节 。参数随意了，指针、直接引用都可以

结构体 ：

```go
type Foo struct {
	Age1 string
	Age2  string
	Age3 string
	Age4 string
}
func main() {
	foo := new(model.Foo)
	fmt.Printf("Foo size: %d",unsafe.Sizeof(*foo))
}
// Foo size: 64
```

切片（引用）：

```go
func main() {
	foo := make([]int64, 10)
	fmt.Println("slice size:",unsafe.Sizeof(foo))
}
// slice size:24
```



数组：

```go
func main() {
	foo := [6]int64{1, 2, 3, 4, 5, 6}
	fmt.Println("数组 size:", unsafe.Sizeof(foo))
}
// 数组 size: 48
```



还有很多，最后做一个练习，使用这个遍历数组

```go
func main() {
	foo := [5]int64{1, 2, 3, 4, 5}
	pointer := (*int64)(unsafe.Pointer(uintptr(unsafe.Pointer(&foo)) + 1*unsafe.Sizeof(new(int64))))
	fmt.Println(*pointer)
}
// 输出 ： 2
```

**可以发现，go的数组，确实继承了c语言的特性，也就是数组指针指向的是数组第一个元素所在的位置，切片是什么呢？后续深究，估计是一种封装类型，类似于c++的vector**



> 考虑到可移植性，引用类型或包含引用类型的大小在32位平台上是4个字节，在64位平台上是8个字节。

| 类型                            | 大小                            |
| ------------------------------- | ------------------------------- |
| `bool`                          | 1个字节                         |
| `intN, uintN, floatN, complexN` | N/8个字节(例如float64是8个字节) |
| `int, uint, uintptr`            | 1个机器字                       |
| `*T`                            | 1个机器字                       |
| `string`                        | 2个机器字(data,len)             |
| `[]T`                           | 3个机器字(data,len,cap)         |
| `map`                           | 1个机器字                       |
| `func`                          | 1个机器字                       |
| `chan`                          | 1个机器字                       |
| `interface`                     | 2个机器字(type,value)           |







## unsafe.Alignof()

返回对应参数的类型需要对齐的倍数. 和 Sizeof 类似, Alignof 也是返回一个常量表达式, 对应一个常量. 通常情况下布尔和数字类型需要对齐到它们本身的大小(最多8个字节), 其它的类型对齐到机器字大小.

可以理解为他是  sizeof/8 ， 













## 参考

https://books.studygolang.com/gopl-zh/ch13/ch13-01.html

https://golang.google.cn/pkg/unsafe/#Alignof