# Golang - 反射 和 Unsafe

对于Value和Type ， 只要记住， 任何一个对象， 他都是  k-v ， 也就是  type-value（类型-具体值）， 理解了这个就知道 Type 和 Value的含义了

反射就是一个IOC的操作，换个方式去操作对象的字段和方法，而不是主动的去调用

unsafe一般涉及到内存，也就是对于数据的内存地址进行操作

## Type

> ​	Type 也就是元信息，类似于Java的Class ， 可以获取field等 

### 方法介绍

```go
type Type interface {
	// Methods applicable to all types.

	// Align returns the alignment in bytes of a value of
	// this type when allocated in memory.
	Align() int

	FieldAlign() int

	// It panics if i is not in the range [0, NumMethod()).
  // 很重要
	Method(int) Method
	MethodByName(string) (Method, bool)
	NumMethod() int

	// Name returns the type's name within its package for a defined type.
	// For other (non-defined) types it returns the empty string.
	Name() string
	PkgPath() string
	
	// Size returns the number of bytes needed to store ,跟 align一样
	Size() uintptr

	// String returns a string representation of the type.类似于Java的ToString
	String() string

	// Kind returns the specific kind of this type.
  // 很重要，返回该对象类型，比如指针，切片，结构体。。。。
	Kind() Kind

	// Implements reports whether the type implements the interface type u.
	Implements(u Type) bool

	// AssignableTo reports whether a value of the type is assignable to type u.
	AssignableTo(u Type) bool

	// ConvertibleTo reports whether a value of the type is convertible to type u.
	ConvertibleTo(u Type) bool

	// Comparable reports whether values of this type are comparable.
	Comparable() bool

	// It panics if the type's Kind is not one of the
	// sized or unsized Int, Uint, Float, or Complex kinds.
  // 其实就是基本类型，除了bool,也就是size*8 ， 比特数量
	Bits() int

	// ChanDir returns a channel type's direction.
	// It panics if the type's Kind is not Chan.
	ChanDir() ChanDir

	// 首先得是一个fun，判断是不是可变参数
	// IsVariadic panics if the type's Kind is not Func.
	IsVariadic() bool

	// Elem returns a type's element type.
	// It panics if the type's Kind is not Array, Chan, Map, Ptr, or Slice.
	Elem() Type

	// 返回字段信息
	// It panics if the type's Kind is not Struct.
	// It panics if i is not in the range [0, NumField()).
	Field(i int) StructField

  // 嵌套，比如field（1）为结构体，进入这个结构体，就需要这个[1,1]，就是这个结构体的字段一
	FieldByIndex(index []int) StructField

	FieldByName(name string) (StructField, bool)

  // 回掉，filter
	FieldByNameFunc(match func(string) bool) (StructField, bool)

  // 函数的参数类型
  // It panics if the type's Kind is not Func.
	// It panics if i is not in the range [0, NumIn()).
	In(i int) Type

	// 返回map对象的key类型
	// It panics if the type's Kind is not Map.
	Key() Type

	// 返回数组长度
	// It panics if the type's Kind is not Array.
	Len() int

	// 返回结构体的字段数
	// It panics if the type's Kind is not Struct.
	NumField() int

	// 函数的参数个数
	// It panics if the type's Kind is not Func.
	NumIn() int

	// 函数的返回值个数
	// It panics if the type's Kind is not Func.
	NumOut() int

	// 函数的输出类型
	// It panics if the type's Kind is not Func.
	// It panics if i is not in the range [0, NumOut()).
	Out(i int) Type

	common() *rtype
	uncommon() *uncommonType
}
```

```go
	student := model.Student{Name: "tom", Age: 1}
	types := reflect.TypeOf(&student)
	fmt.Println(types) // *model.Student
	fmt.Println(types.Kind()) //ptr  （就是数据的类型，比如struct，string，ptr指针。。。）
	fmt.Println(types.Name()) //
	fmt.Println(types.String()) //*model.Student
	fmt.Println(types.Elem()) //model.Student , 其中必须是Array，Chan，Map，Ptr，Slice,不然panic
	fmt.Println(types.NumMethod()) //1  
```



### Align

```go
x := int16(4)
fmt.Println(reflect.TypeOf(x).Align()) // 2 ，输出该变量占的字节数，int16为2个字节，对象类型和指针类型、字符串为8个字节
```



### Method

```go
x := model.Student{}
method := reflect.TypeOf(x).Method(0) //  获取方法 Method对象
fmt.Println(method.Type.IsVariadic())  // kind 为 function类型
```

其次就是配合 in 和 out 方法

```go
x := model.Student{}
fmt.Println(reflect.TypeOf(x).Method(0).Type.In(0)) // model.Student
```

### IsVariadic

```go
fun:= func(str ... string) {
}
// 这个是判断是不是可变参数，首先kind得是function
fmt.Println(reflect.TypeOf(fun).IsVariadic())
```

### Field

> ​	返回值为StructField类型 ， 不是结构体直接panic

```go
x := model.Student{}
fmt.Println(reflect.TypeOf(x).Field(1))
```



其他的都有介绍有 ， 核心就是 Field ， Method 



## Value

> ​	Value可以做Type全部的事情，所以拿到value就可以拿到Type

```go
type Value struct {
	// typ holds the type of the value represented by a Value.
	typ *rtype

	// Pointer-valued data or, if flagIndir is set, pointer to data.
	// Valid when either flagIndir is set or typ.pointers() is true.
	ptr unsafe.Pointer

	// flag holds metadata about the value.
	// The lowest bits are flag bits:
	//	- flagStickyRO: obtained via unexported not embedded field, so read-only
	//	- flagEmbedRO: obtained via unexported embedded field, so read-only
	//	- flagIndir: val holds a pointer to the data
	//	- flagAddr: v.CanAddr is true (implies flagIndir)
	//	- flagMethod: v is a method value.
	// The next five bits give the Kind of the value.
	// This repeats typ.Kind() except for method values.
	// The remaining 23+ bits give a method number for method values.
	// If flag.kind() != Func, code can assume that flagMethod is unset.
	// If ifaceIndir(typ), code can assume that flagIndir is set.
	flag

	// A method value represents a curried method invocation
	// like r.Read for some receiver r. The typ+val+flag bits describe
	// the receiver r, but the flag's Kind bits say Func (methods are
	// functions), and the top bits of the flag give the method number
	// in r's type's method table.
}
```



### 原类型和Value相互转换

```go
func foo() {
	student := model.Student{}
	
	// Value
	value := reflect.ValueOf(student)
	// 转换为原类型，通过断言
	stu := value.Interface().(model.Student)
	fmt.Println(stu)
}
```



### Value 修改值

反射最大的特点就是动态修改值

```go
func foo() {
	student := model.Student{Name:"xiaoli"}
	elem := reflect.ValueOf(&student).Elem()
	elem.FieldByName("Name").SetString("change")

  // 对于set操作，可以通过value.CanAddr()进行判断，也就是判断是否可以被取址
	fmt.Println(student.Name) // change
}
```



关于为什么必须是指针引用，因为修改原类型，必须是地址， 关于为什么必须调用Elem方法， 这个好像是指向真正的引用，关于为什么可以看看圣经上的一篇文章  ： https://books.studygolang.com/gopl-zh/ch12/ch12-05.html



### 调用函数

```go
func foo() {
	student := model.Student{Name: "xiaoli"}
	
	// 首先拿到Method-Value ， 然后调用call
	results := reflect.ValueOf(&student).Method(0).Call([]reflect.Value{
		reflect.ValueOf("1"),
	})
	str := results[0].Interface().(string)
	fmt.Println(str)
}
```

> ​	类似于Java的Method.invoke（）



关于使用Value 还有很多细节，比如Tag ，



### 获取Tag

> ​	方法一

```go
func foo() {
	student := model.Student{Name: "xiaoli"}
	field, _ := reflect.ValueOf(&student).Elem().Type().FieldByName("Name")
	fmt.Println(field.Tag)
}
```

> ​	方法2

```go
	func foo() {
	student := model.Student{Name: "xiaoli"}
	field, _ := reflect.ValueOf(student).Type().FieldByName("Name")
	fmt.Println(field.Tag)
}
```

> ​	方法3

```go
func foo() {
	student := model.Student{Name: "xiaoli"}
	field, _ := reflect.TypeOf(student).FieldByName("Name")
	fmt.Println(field.Tag)
}
```

## Unsafe

> ​	这个比较难以理解，其实也很好操作，关于如何使用，可以看看官方的文档：
>
> https://golang.google.cn/pkg/unsafe/

```
func foo() {
	student := model.Student{}
	// 拿到一个指针，需要提供一个指针地址
	str := (*string)(unsafe.Pointer(uintptr(unsafe.Pointer(&student)) + unsafe.Offsetof(student.Name)))
	*str = "hello world"
	fmt.Println(student)
}

func main() {
	foo()
}
```

A pointer value of any type can be converted to a Pointer.
A Pointer can be converted to a pointer value of any type.
A uintptr can be converted to a Pointer.
A Pointer can be converted to a uintptr.

其实可以这么理解， 就是Pointer是指针类型（c中的 void*），  而unitptr是用于指针地址操作的 ， 这俩类似于Java的unsafe类，依赖于偏移量进行操作，其中go也是提供了个偏移量的玩意，其实对于结构体，其本身各个字段的偏移量是死的，唯一改变的是结构体地址。