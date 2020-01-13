# Scala - 集合

> [官网链接](https://docs.scala-lang.org/overviews/core/architecture-of-scala-213-collections.html) : 	https://docs.scala-lang.org/overviews/core/architecture-of-scala-213-collections.html

> ​	集合就是一门语言的数据结构 , 深度了解集合是入门一个语言必备的 ,虽然 `Scala`可以调用`Java`的集合类 , 但是 Scala语言带来的集合类 , 给Java带来了很多的补充 . 我会逐步更新 .. .. .. 可能写的有些不到位

## 开篇提示

`scala.collection`包下有两个`collection.mutable`  `collection.immutable` 这俩包 , 其中`mutable`的意思是可变的意思, 其实就是一个不可变的集合类, 和一个可变的集合类 , 默认情况下使用的是**不可变**的集合类

##  1. Option - 选项

Scala Option(选项)类型用来表示一个值是可选的（有值或无值)。如果你了解过Java的Option,我觉得你对这个并不陌生,他只是很好地处理和空值.

Option[T] 是一个类型为 T 的可选值的容器： 如果值存在， Option[T] 就是一个 Some[T] ，如果不存在， Option[T] 就是对象 None 。

## 2. Scala数组 - Array (len不可变)

> ​	数组是定义好长度以后就不可以修改了长度了 , 

### 1. 定义一维数组

```scala
// 定义数组
val array: Array[Int] = new Array[Int](3)
//或者
val array: Array[Int] = Array(1, 2, 3, 4)
//或者
val array = Array.apply(1, 2, 3, 4, 5)


// 数组下标为4 , 用圆括号
array(4) = 1
```

### 2. 遍历数组

```scala
1. 直接遍历
// 直接遍历数组
for (e <- array) {
      println(e)
}

2. 索引遍历
// until 左闭右开  ,不用加括号 Unnecessary parentheses(括号)
for (index <- 0 until array.length) {
  println(s"$index - "+array(index))
}


// to 左闭右闭
for (index <- 0 to array.length-1) {
  println(s"$index - "+array(index))
}

// array 自带的 Array.indices 返回一个 range() 数组,获取数组下标
for (index <- array.indices) {
  println(array(index))
}

4. foreach 遍历输出
array.foreach(println)
```

### 3. 多维数组  Array.ofDim(...)

```scala
var arrays = Array.ofDim[Int](3, 3)

arrays(1)(1) = 1
```

### 4. 连接数组 Array.concat(arr1,arr2)

```scala
var myList1 = Array(1.9, 2.9, 3.4, 3.5)
var myList2 = Array(8.9, 7.9, 0.4, 1.5)

var myList3 = Array.concat(myList1, myList2)

for (x <- myList3) {
  println(x)
}
```

### 5. 创建数组  Array.range(start, len)

```scala
1. 左闭右开 , 第一个参数是start , 第二个参数是 end
val ints = Array.range(0, 10)

for (i <- ints) {
  printf("%d\t" , i) // 0	1	2	3	4	5	6	7	8	9	
}

2. 带有第三个参数 step 
val ints = Array.range(0, 10,2)
for (i <- ints) {
  printf("%d\t",i)  //0	2	4	6	8	
}
```

### 6. 数组拷贝   arr.copyToArray(dest,start,len)

```scala
val ints1 = Array.apply(1, 2, 3, 4, 5)


val ints2 = new Array[Int](ints1.length)

// dest  , start , len 
ints1.copyToArray(ints2, 0, ints1.length)
```

### 7. 数组填充 Array.fill(len)(ele)

``` scala
val ints = Array.fill(5)(2)

for (elem <- ints) {
  println(elem)  // 2 2 2 2 2
} 
```

### 8. 数组过滤 arr.filter(e:T=>Boolean)

```scala
val fruit = Array("apple", "banana", "cherry")

1. 简单写法
val strings = fruit.filter(e => {
  // x 指的是数组中的元素
  e.contains("apple")
})

2. 由于e前后都一样, 所以可以使用下划线 , 返回一个数组
val strings = fruit.filter(_.contains("apple"))

for (elem <- strings) {
  println(elem)  // out :  apple
}
```

### 9. Map  - arr.map(f:T=>B)

```scala
val b = Array(1, 2, 3, 4, 5)

1. 
val value = b.map(x => x * 2)

val value = b.map(_ * 2)

println(value)
```



### 10. Reduce  arr.reduce(op:(B,B)=>B)

```scala
val ints: Array[Int] = Array(1, 2, 3, 4, 5)

// 1. 根据函数去写
val i1 = ints.reduce((x1, x2) => x1 * x2)
// 2. 可以省略成 _ 
val i2 = ints.reduce(_ * _)
println(i2)

// 或者 scala提供的内置函数计算累乘
val i2 = ints.product

// 累加
val i3 = ints.sum
```



## 2. Scala数组 - ArrayBuffer (len可变)

> ​	这个数组长度随着数组元素的个数的变化而变化

```scala
val b = ArrayBuffer('a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l')

b -= 'a' //删除1个指定元素;  ArrayBuffer(b, c, d, e, f, g, h, i, j, k, l) 
b -= ('b', 'c') //删除多个指定元素  ArrayBuffer(d, e, f, g, h, i, j, k, l)
b --= Seq('d', 'e') //通过1个集合来删除元素(可以用Array,Set) ArrayBuffer(f, g, h, i, j, k, l)
b.remove(0) //通过index来删除元素  ArrayBuffer(g, h, i, j, k, l)
b.remove(1, 2) //通过index来删除多个元素  ArrayBuffer(g, j, k, l)
b.clear() //清除所有元素  ArrayBuffer()
```

## 3. Scala列表  -  List

定义 : `val list:List[String] = List("1","2")`

定义 : `val list2: List[String] = "aa" :: ("aaaa" :: Nil)` , 其中 `::`用法为x::list ,表示像集合头部添加x, `Nil`代表空单位

定义 : `val list3: List[List[String]] = List(List("a", "b"),List("c", "d"))`

定义 : `val list4: List[String] = Nil`  这就是一个控集合

**list 相互 连接 :** 

```scala
val fruit1 = "apples"::("oranges"::("pears"::Nil))
val fruit2 = "mangoes"::("banana"::Nil)

// use two or more lists with ::: operator
var fruit = fruit1 ::: fruit2
println( "fruit1 ::: fruit2 : " + fruit )

// use two lists with Set.:::() method
fruit = fruit1.:::(fruit2)
println( "fruit1.:::(fruit2) : " + fruit )

// pass two or more lists as arguments
fruit = List.concat(fruit1, fruit2)
println( "List.concat(fruit1, fruit2) : " + fruit  )


输出 : 
fruit1 ::: fruit2 : List(apples, oranges, pears, mangoes, banana)
fruit1.:::(fruit2) : List(mangoes, banana, apples, oranges, pears)
List.concat(fruit1, fruit2) : List(apples, oranges, pears, mangoes, banana)
```

创建重复的元素集合 : **List.fill(填充多少次)(元素)**

```scala
val fruit: List[String] = List.fill(3)("apples") // Repeats apples three times.
println("fruit : " + fruit)

val num: List[Int] = List.fill(10)(2) // Repeats 2, 10 times.
println("num : " + num) //

fruit : List(apples, apples, apples)
num : List(2, 2, 2, 2, 2, 2, 2, 2, 2, 2)
```



翻转集合 : `List.reverse `

## 4. Scala集 - Set

> ​	set 就是可以去重 , 和java一样

## 5. Scala映射 - Map

> ​	scala中 有两种map , 默认的map是 `scala` , hashmap是 `import scala.collection.mutable.Map` , 写法 `(K->V,K->v)`

### 1. 定义map0

```scala
val map: Map[String, Int] = Map()

val map1: Map[String, Int] = Map("a" -> 1, "a" -> 1)
```



### 2. 添加元素

```scala
var map1: Map[String, Int] = Map("a" -> 1, "a" -> 1)

map1 = map1.+("c" -> 1)

println(map1)  // Map(a -> 1, c -> 1)
```



### 3. 连接map

```scala
val colors1 = Map("red" -> "#FF0000", "azure" -> "#F0FFFF", "peru" -> "#CD853F")
val colors2 = Map("blue" -> "#0033FF", "yellow" -> "#FFFF00", "red" -> "#FF0000")

// 或者 colors1.concat(colors2)
var colors = colors1 ++ colors2
println(  colors )  // HashMap(blue -> #0033FF, azure -> #F0FFFF, peru -> #CD853F, yellow -> #FFFF00, red -> #FF0000)
```



### 4. 遍历

```scala
val colors3 = Map("red" -> "#FF0000", "azure" -> "#F0FFFF", "peru" -> "#CD853F")

colors.foreach({
  case (k, v) => println(s"$k - $v")
})

//输出
red - #FF0000
azure - #F0FFFF
peru - #CD853F
```

### 5. 查找

```scala
val colors3 = Map("red" -> "#FF0000", "azure" -> "#F0FFFF", "peru" -> "#CD853F")
println(colors3.contains("red"))  // true
```

## 6. Scala -Seq

> ​	我们可以看一下架构图 , 一般List和array 属于 Seq 的子类 , Seq是一个接口 .,是 `trait`

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-08/b7d83470-5a0e-4ec9-a817-d04b40836958.png?x-oss-process=style/template01)

```scala
1. 申明使用
val unit = Seq[Int](1,2,3) 
val unit = Seq(1,2,3)  // List(1, 2, 3)

2. 获取值
unit(1)
```

## 7. Scala元组

> ​	` Tuple`这个类似于Python , Python里面就有元组的概念, 是一个不可变的数组 , 不能修改数组中的元素 , 写法 `(A,B,...)`

```scala
1.最全写法
val unit = Tuple3[Int, Int, Int](1, 2, 3)

2. 默认写法,不指定类型
val unit = Tuple3(1,2,3)

3.不加任何类型申明的全部是元组
val unit = (1,2,3)

4.获取值 
unit._1  // 1
unit._2  // 2
unit._3  // 3
```

## 8. Scala迭代器

> ​	和java的 `Iterator<E>` 接口很相似  , 是一个迭代器 , 一个类想要有迭代器 ,需要实现 `Iterable<T>`接口 , 他是所有集合的父接口(应该是 , Scala 设计上好像是)

### 定义 

```scala
val it = Iterator("a", "b", "c", "d")

while (it.hasNext) {
  println(it.next())
}
// 
```

### 2. 常用方法

```scala
val it: Iterator[Int] = Iterator(1, 2, 3, 4)

it.max(ord = Ordering[Int]) // 4
```



## 9. Scala - Range

to 是闭区间 , until 是左闭右开区间

```java
val inclusive: Range.Inclusive = 1 to 10
val list: List[Int] = inclusive.toList

val size: Int = list.size   // 10

val range: Range = 1 until  10

val size2: Int = range.size  // 9
```



还有 range 转换为 Seq

```java
val range = Range(1, 10)
println(range.size)  // 9 

val ints: Seq[Int] = Seq(range: _*)
println(ints)  //List(1, 2, 3, 4, 5, 6, 7, 8, 9)

    
```

