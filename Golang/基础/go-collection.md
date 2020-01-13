# Golang - 数据结构

## 1. 数组

> ​	数组是一个由固定长度的特定类型元素组成的序列，一个数组可以由零个或多个元素组成。 因为数组的长度是固定的，因此在Go语言中很少直接使用数组。和数组对应的类型是 Slice（切片），它是可以增长和收缩动态序列，slice功能也更灵活，但是要理解slice工作原 理的话需要先理解数组。

数组申明的写法和Java正好相反 , 其实写Java的知道, 如果你写反了申明虽然不操作,但是Idea也会提示的.

在 Java中`int[] arr=new int[10];` 后来有了语法糖可以`int[] arr = new int[]{1, 23, 4};` ,然后又可以`int[] arr ={1, 23, 4};`, 

而Golang中 `var arr [3]int=[3]int{1,2,3}` 他必须指明长度 , 或者`[...]` 指定长度 

还有就是 Golang中 `var arr1 []int=[]int{1,2}` 和 `var arr2 [2]int=[2]int{1,2}`  这俩数组类型是不同的, 因为一个是 `[]int` (切片类型, 引用传递)一个是 `[2]int` (值传递) , 

数组的每个元素可以通过索引下标来访问，索引下标的范围是从0开始到数组长度减1的位 置。内置的len函数将返回数组中元素的个数。

> ​	默认情况下，数组的每个元素都被初始化为元素类型对应的零值，对于数字类型来说就是0。 

```go
func main() {

	// 申明类型 ,初始值为 [0,0,0]
	var arr [3]int

	// 必须大于4  , [len] 是len长度, 如果小于len的则补充为0
	var arr1 [5]int = [5] int{1, 2, 3, 4}
	
	// 简写
	var arr2= [5] int{1, 2, 3, 4}
	
	// 变量语法糖申明
	arr3:=[3]int{1,2,3}

	// 编译时确定数组长度
	arr4 :=	[...]int{1,	2,	3}

	// 赋值
	arr[1] = 1

	// 遍历1
	for index, value := range arr {
		fmt.Printf("index %d , value %d \n", index, value)
	}

	// 遍历2
	for _, value := range arr {
		fmt.Printf("value %d \n", value)
	}

	// 遍历3
	for value := range arr {
		fmt.Printf("value %d \n", value)
	}
}
```



常量

```go
const (
	// 初始化0 , 后面叠加
	USD = iota
	EUR
	RMB
)

func main() {
	fmt.Println(EUR) //1
    
	// 类型
	fmt.Println(reflect.TypeOf(USD)) // int
    
    // string数组
    symbol := [...]string{USD: "$", EUR: "€", RMB: "￥"}

	fmt.Println(symbol) // [$ € ￥]
    
    // 数组写法  {索引:value} , 由最大索引值决定大小 , 没有初始化的都为0
    arr:=[...]string{1:"1",0:"0"}
	fmt.Println(arr)  // [0 1]
}
```



数组比较

```java
func main(){
    
    arr1 := [3]int{1, 2, 3}

	arr2 := [...]int{1, 2, 3}

	arr3 := [3]int{1, 2, 4}

	fmt.Println(arr1 == arr2)
	fmt.Println(arr1 == arr3)
}
```

切记不可以这么写 , 因为他要求类型一致 , 比如 `[3]int` 和 `[4]int` 类型其实是不一致的 , 其实`[] int` 可以称之为切片	

```go
arr1 := [3]int{1, 2, 4}

arr4:=[]int{1,2,3}

fmt.Println(arr1==arr4) // Invalid operation: arr1==arr4 (mismatched types [3]int and []int)
```



## 2. Slice 切片

切记 Slice是引用类型. 所以不需要指针.

是 Java中的ArrayList , 你可以这么理解, 因为数组是定长 ,所以添加不方便, 所以需要变长, 就是 Slice , 其实怎么说就是个数组 ,只不过数组长度通过他的特定方法可以扩容

> ​	Slice（切片）代表变长的序列，序列中每个元素都有相同的类型。一个slice类型一般写作 []T，其中T代表slice中元素的类型；slice的语法和数组很像，只是没有固定长度而已。 



> ​	数组和slice之间有着紧密的联系。一个slice是一个轻量级的数据结构，提供了访问数组子序 列（或者全部）元素的功能，而且slice的底层确实引用一个数组对象。一个slice由三个部分 构成：指针、长度和容量。指针指向第一个slice元素对应的底层数组元素的地址，要注意的 是slice的第一个元素并不一定就是数组的第一个元素。长度对应slice中元素的数目；长度不 能超过容量，容量一般是从slice的开始位置到底层数据的结尾位置。内置的len和cap函数分 别返回slice的长度和容量。

slice的切片操作`s[ i : j ]`， i 和 j 可以省略一个  , 左开右闭, 从索引0开始计算

其实吧 是 i决定 capacity,其实i可以改变 数组指针位置  , i和j共同决定length . 

```go
func main() {

	// 申明
	arr := []int{0, 1, 2, 3, 4, 5}

	fmt.Printf("原数据地址 : %p \n", &arr)

	//// make 申明
	//arr1 := make([]int, 3)
	printSlice(arr)

	arr1 := arr[:1]
	printSlice(arr1)

	arr2 := arr[1:4]
	printSlice(arr2)

	arr3 := arr[1:]
	printSlice(arr3)

	arr4 := arr[2:]
	printSlice(arr4)

	reserve(arr)

	printSlice(arr)

}

func printSlice(arr []int) {
	fmt.Printf("len : %d , cap : %d , value : %v , type : %T , 地址 : %p.\n", len(arr), cap(arr), arr, arr, &arr)
}

func reserve(arr []int) {
	for x, y := 0, len(arr)-1; x < y; x, y = x+1, y-1 {
		arr[x], arr[y] = arr[y], arr[x]
	}
}
```

输出

```go
原数据地址 : 0xc00004a420 
len : 6 , cap : 6 , value : [0 1 2 3 4 5] , type : []int , 地址 : 0xc00004a460.
len : 1 , cap : 6 , value : [0] , type : []int , 地址 : 0xc00004a4c0.
len : 3 , cap : 5 , value : [1 2 3] , type : []int , 地址 : 0xc00004a520.
len : 5 , cap : 5 , value : [1 2 3 4 5] , type : []int , 地址 : 0xc00004a580.
len : 4 , cap : 4 , value : [2 3 4 5] , type : []int , 地址 : 0xc00004a5e0.
len : 6 , cap : 6 , value : [5 4 3 2 1 0] , type : []int , 地址 : 0xc00004a640.
```

其实对于 切片来说 , 他走的就是引用传递 ,所以其实你写 `reserve(arr []int)` ,底层其实就是 `reserve(arr *[]int) `  , 目的是为了什么呢 ,减少内存使用量. 所以不安全



#### append

```go
func main() {

	arr := []int{1, 2, 3, 4}

	ints := append(arr, 5)

	fmt.Printf("len:%d\tcap:%d\tvalue:%v\n", len(arr), cap(arr), arr)
	fmt.Printf("len:%d\tcap:%d\tvalue:%v\n", len(ints), cap(ints), ints)
}
```

输出 : 

```java
len:4	cap:4	value:[1 2 3 4]
len:5	cap:8	value:[1 2 3 4 5]
```

我们发现哇他每次扩容一倍的数据量. Java是原数组的1/2长度, Golang呢是原数组的1倍长度



#### copy

`func copy(dst, src []Type) int`

```go

arr := []int{1, 2, 3, 4, 5, 6}
arr1 := []int{5, 6}
copy(arr1, arr)


fmt.Printf("len:%d\tcap:%d\tvalue:%v\n", len(arr1), cap(arr1), arr1)
fmt.Printf("len:%d\tcap:%d\tvalue:%v\n", len(arr), cap(arr), arr)
```

输出

```go
len:2	cap:2	value:[1 2]
len:6	cap:6	value:[1 2 3 4 5 6]
```







## 3.Map

哈希表是一种巧妙并且实用的数据结构。它是一个无序的key/value对的集合，其中所有的key 都是不同的，然后通过给定的key可以在常数时间复杂度内检索、更新或删除对应的value。  跟Java的HashMap 一样



#### 实例化

构造方法

```go
func main() {

	// 重写构造方法
	smap := map[string]int{
		"a": 1,
		"b": 2,
	}

    // 普通方法
	smap["A"]=3
    
    // delete方法
    delete(smap,"a")
	fmt.Println(smap)  //输出 map[A:3  b:2]   , hashmap是无序的
}
```

make 方法

```go
makem := make(map[string]int,1)
fmt.Println(makem)
makem["A"] = 3
makem["B"] = 3
makem["A"] = 3
fmt.Println(makem)
// 输出
map[]
map[A:3 B:3]
```



禁止对map元素取址的原因是map可能随着元素数量的增长而重新分配更大的内存空间，从而 可能导致之前的地址无效。



#### 高级语法

其实也不是高级 , 就是他的的默认行为 , 取出空值,根据基本类型会有默认值的, 

```java
smap := map[string]int{
    "a": 1,
    "b": 2,
}

fmt.Println(smap["c"]) // 0
smap["c"]=smap["c"]+1
smap["c"]++
fmt.Println(smap["c"]) // 2
```



#### 遍历

```java
for key, value := range smap {
	fmt.Printf("key %s : value %d\n", key, value)
}
```



#### 空值

```map
var ages  map[string]int

ages["tom"]=1 // 运行错误

fmt.Println(ages)
```

map上的大部分操作，包括查找、删除、len和range循环都可以安全工作在nil值的map上，它 们的行为和一个空的map类似。但是向一个nil值的map存入元素将导致一个panic异常



#### 技巧

```go
var ages map[string]int = map[string]int{}

age, ok := ages["tony"]

fmt.Println(age, ok) // 0 false
```



## 4. 结构体



```go
type Point struct{ X, Y int }

//	"false" fmt.Println(p	==	q)																			//	"false"
func main() {
	p := Point{2, 1}
	q := Point{2, 1}
	fmt.Println(p == q)  //true
}
```



```go
type Singer struct {
	name string
}

func (s *Singer) setName(name string) {
	s.name = name
}

func (s Singer) setName1(name string) {
	s.name = name
}
```

写这俩方法的目的就是指针的作用

```go
singer := Singer{name: "张靓颖"}

fmt.Println(singer) // {张靓颖}

singer.setName("A")

fmt.Println(singer) // {A}

singer.setName1("B")

fmt.Println(singer) // {A}
```



对于类型 , 他会自动转换. 





## 5. Json



```go
type Child struct {
	Name  string   `json:"username"`
	Hobby []string `json:"hobby"`
}

func main() {

	arr := Child{Name: "tony", Hobby: []string{"basketball", "football"}}

	arr.Hobby = append(arr.Hobby, "swing")

	data, _ := json.Marshal(arr)

	fmt.Printf("%s\n", data)

	var c = Child{}

	json.Unmarshal(data, &c)

	fmt.Println(c)
}
```

```go
{"username":"tony","hobby":["basketball","football","swing"]}
{tony [basketball football swing]}
```



