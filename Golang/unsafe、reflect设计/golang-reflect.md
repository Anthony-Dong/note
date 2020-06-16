# Golang - Reflect

> ​	反射是高级语言的特性，主要是用于IOC。

下面内容，主要以官方文档为主，https://golang.google.cn/pkg/reflect/ ， 标题是方法/类型



## Copy(dst, src Value)

> ​	可以理解为他是Java的 system.copyarr()，我没记错的话。

```go
func main() {
	arr1 := []int{
		1, 2, 3, 4,
	}
	arr2 := make([]int, 3)
	
	//copy(arr2, arr1) 这个更加高效
	// 它会填充切片/数组（其他类型panic），直到把它填满
	i := reflect.Copy(reflect.ValueOf(arr2), reflect.ValueOf(arr1))
	fmt.Printf("copy的长度：%d , copy的内容:%v\n", i, arr2)
}
// copy的长度：3 , copy的内容:[1 2 3]
```



## IsValid

> ​	表示是否有效，根据判断，一般只有nil的时候是无效的

```go
func main() {

	fmt.Println(reflect.ValueOf(nil).IsValid()) // false
	fmt.Println(reflect.Value{}.IsValid()) //false
	fmt.Println(reflect.ValueOf(111).IsValid()) //false
}
```



## AssignableTo

是否可以类型相同赋值，A-b

```go
a := 12
b := 1
_a := reflect.ValueOf(a)
_b := reflect.ValueOf(b)

fmt.Println(_a.Type().AssignableTo(_b.Type()))
```



## Indirect

获取真实的value对象，  ptr会返回 elem。





### 切片获取类型

```go
func main() {
	arr := []int{
		1, 2, 3, 4,
	}
	sliceValue := reflect.Indirect(reflect.ValueOf(&arr))
	fmt.Println(sliceValue.Kind())               // 这个对象的类型
	fmt.Println(sliceValue.Type().Elem().Kind()) // slice的元素类型
}
```





### Map 获取类型

```go
func main() {
	strings := map[string]interface{}{
		"1": "2",
	}
	mapValue := reflect.Indirect(reflect.ValueOf(&strings))
	fmt.Println(mapValue.Kind()) // 这个对象的类型
	fmt.Println(mapValue.Type().Key().Kind()) // map 的 key类型
	fmt.Println(mapValue.Type().Elem().Kind()) // map的 value 类型
}
```





## New(Type)

> ​	这个方法很重要 ， 会生成一个新的对象，而且是指向传入类型的指针对象

```go
func main() {
	_type := reflect.TypeOf(User{})
	fmt.Println(_type.Kind())
	value := reflect.New(_type) // struct 
	fmt.Println(value.Kind()) // ptr
}
```



## 获取一个接口的Type

以下是一个接口

```go
type TableName interface {
	TableName() string
}
```

如何获取它的type呢， 难道写一个实现类，不现实。 可以通过类型转换的方式。

```go
func main() {
	inter := (*TableName)(nil)
	// 调用Elem()获取包装类型的type
	_type := reflect.TypeOf(inter).Elem()
	fmt.Println(_type)
}
```



## MakeFunc  动态代理

> ​	制造函数，因此确实名字上很特殊 ， 其实换个意思类似于动态代理，但是这个代理只能代理一个接口。

```go
// 方法
type Echo func(string)

// 代理
func Proxy(inter interface{}, invocationHander func(in []reflect.Value) []reflect.Value) {
	_value := reflect.ValueOf(inter)
	if _value.Kind() != reflect.Ptr {
		panic("参数必须是指针类型")
	}
	fn := reflect.Indirect(_value)
	v := reflect.MakeFunc(fn.Type(), invocationHander)
	fn.Set(v)
}

func main() {
	var echo Echo
	invocationHandler := func(in []reflect.Value) []reflect.Value {
		fmt.Printf("receive:%v", in)
		return nil
	}
	Proxy(&echo, invocationHandler)
	echo("hello world")
}
```



但是Golang中，类型的不统一，多重指针，可能往往造成，判断逻辑很多。



```go
func MakeFunc(typ Type, fn func(args []Value) (results []Value)) Value {
  // 类型必须是func
	if typ.Kind() != Func {
		panic("reflect: call of MakeFunc with non-Func type")
	}

	t := typ.common()
	ftyp := (*funcType)(unsafe.Pointer(t))

  // 这里也就是将方法内的code设置为空，也就是用一个空方法取代
	dummy := makeFuncStub
	code := **(**uintptr)(unsafe.Pointer(&dummy))

	// makeFuncImpl contains a stack map for use by the runtime
	_, argLen, _, stack, _ := funcLayout(ftyp, nil)

  // 最终的结构体，code，stack，arglen，指针，方法
	impl := &makeFuncImpl{code: code, stack: stack, argLen: argLen, ftyp: ftyp, fn: fn}

	return Value{t, unsafe.Pointer(impl), flag(Func)}
}

func makeFuncStub()
```





## 

