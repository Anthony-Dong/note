# Scala  函数 和 方法

> ​	官方定义的函数是 : 函数是带有参数的表达式。其实函数我自我感觉就是`Function<N>`的实例,自我感觉
>
> ​	方法是 : 方法的表现和行为和函数非常类似，但是它们之间有一些关键的差别。方法由`def`关键字定义。`def`后面跟着一个名字、参数列表、返回类型和方法体。大致形式就是这样子 `def functionName ([list of parameters]) : [return type]` , 同时方法可以接收多个参数,同时也可以没有参数

> ​	

## 1. 函数

简单的函数就是下面的形式 , 它是一个常量/变量(常量准确一点) , scala其实可以告诉你什么可以是常量,什么可以使变量 , 在我们Java开发中我觉得我们很少对于一个变量加final修饰 , 其实final修饰可以帮助系统执行程序时提供很好地提示,同时可以提高程序性能, 和C++的const很相似.... 跑题了 ......

```scala
scala> val fun=(num:Int)=>num+1
fun: Int => Int = $$Lambda$751/1164934558@75d366c2

scala> fun(1)
res0: Int = 2
```



## 2. 方法

### 1.无参数函数

```scala
scala> def echo:Unit={
     | println("Hello Scala")
     | }
echo: Unit

// 此时调用的时候不需要 写成 `echo()`
scala> echo
Hello Scala
```

### 2.单一参数函数

> ​	**scala的参数对应的java中是一个被final修饰的 ,所以在函数体中是无法对参数重新赋值的,**

#### 1. 参数类型是基本类型 , 或者引用类型

```java
scala> def fun(num: Int): Int = {
     |     num + 1
     |   }
fun: (num: Int)Int

scala> fun(1)
res1: Int = 2
```

### 2. 返回类型是一个函数

```scala
// 函数名 urlBuilder , 参数 ssl ,domainName , 返回类型 是一个 匿名函数 , 匿名函数的返回类型时一个string
def urlBuilder(ssl: Boolean, domainName: String): (String, String) => String = {
    val schema = if (ssl) "https://" else "http://"
    (endpoint: String, query: String) => s"$schema$domainName/$endpoint?$query"
}
urlBuilder(true, "www.baidu.com")("user", "id=1")  //https://www.baidu.com/user?id=1

// 其实他是分了两步执行 .... 
// 第一步
urlBuilder(true, "www.baidu.com")返回一个函数 , 假设fn
// 第二步调用fn的方法
fn("user", "id=1")
```

### 3. 多参数

```scala
def add(x1: Int, x2: Int)(f: Function2[Int, Int, Int]): Int = {
  f.apply(x1, x2)
}

println(add(1, 2)(_ + _))  // 输出3
```

### 3. 可变参数

基本用法

```scala
 def calcSumWithQueue(numbers: Int*):Int = {
    var sum = 0
    for (element <- numbers){
      sum+= element
    }
    sum
}
或者我们可以加一个注
```

或者我们如果希望Java代码可以调用,此时可以添加 `@varargs` 注解

```scala
@varargs def calcSumWithQueue(numbers: Int*): Int = {
var sum = 0
for (element <- numbers) {
  sum += element
}
sum
}
```

可变参数的使用条件 ....

```scala
object MultipleArguments {
 
  //这种写法没毛病
  def foo(arg1: Int,arg2: Double*):Unit = { }
 
  //可变参数后面不能再出现其他参数
  //def foo2(arg1: Double*,arg2: Int):Unit = { }
 
  //不允许出现多个类型的可变参数
  //def foo3(arg1: Double*,arg2: Int*):Unit = { }
 
  //总结：一个函数中只能有一个可变参数，并且只能放在参数列表最后
}
```

可变参数传参时, 如果是一个 `Range` 或者是一个 `Array` 时需要进行类型转换

```scala
def main(args: Array[String]): Unit = {
//可变参数的实参为Range的时候，需要添加:_*来进行数据的转换
println("sum = " + calcSumWithQueue(1 to 10: _*))
//可变参数的实参为Array时，需要添加:_*来进行数据的转换
val arr = Array("Hadoop", "Spark", "Storm")
printArr(arr: _*)
}
```

## 3. Scala自带函数

> ​	其实 Scala约定了很多函数 , 比如 `Function0 `,`Function1` ,`Function2` ,这些都是我们可以直接使用的, 其中 0 1 2的意思是参数的个数,返回类型都是一个 , 如果无返回值,返回值为 `Unit`

### 1. Function1

以`Array.map()` 方法为例子

```scala
def map[B](f : scala.Function1[A, B])(implicit ct : scala.reflect.ClassTag[B]) : scala.Array[B] = { /* compiled code */ }
```

> ​	implicit , 隐式转换 ,他会自动检测你的上下文找到匹配的隐式值

其中它需要传入一个参数 , 此时参数类型时一个 `scala.Function1[A, B]` 最终返回类型时 `scala.Array[B] ` , 那么我们需要了解一下`scala.Function1[A, B]`是什么 

举个简单的例子, 我们以一个`wordcount`为例子 , 当我们拿到数据后, 此时需要将他转成一个map , 类似于 k-v结构

```scala
val arr = Array("hello", "hello", "hello", "hello")

val tuples = arr.map(new Function1[String, Tuple2[String, Int]] {
  override def apply(v1: String): (String, Int) = {
    Tuple2(v1, 1)
  }
})
// (hello,1)	(world,1)	(java,1)	(scala,1)	
```

这就是一个 `Function1 `函数 , 他需要两个泛型 和一个要求重写的方法`apply` , 但是由于`scala强大的模式匹配 ` ,我们上诉的 三行代码可以一行解决 ,所以这就是scala最强大的地方.

```scala
val tuples = arr.map((_, 1))
```

#### 1. compose方法

#### 2. andThen  方法

```scala
val fun1 = new Function1[Int, Int] {
  def apply(x: Int) = {
    println("第一步:" + x)
    x + 1
  }
}
val fun2 = (x: Int) => {
  println("第二步:" + x)
  x + 3
}


println(fun2.andThen(fun1).apply(5)) // fun2先执行 , fun1 后执行
println(fun2.compose(fun1).apply(5)) // fun1休闲执行, fun2 后执行
```

```scala
第二步:5
第一步:8
9
第一步:5
第二步:6
9
```



### 2.  Function2 函数

> ​	和 `Function1`类似 ,只是他只是需要传入的参数是两个, 返回类型是一个 ,这里我们以 `Array.reduce()` 方法为例子



```scala
def reduce[B >: A](op : scala.Function2[B, B, B]) : B = { /* compiled code */ }
```

简单使用...

```scala
val function = new Function2[Int, Int, Int] {
  override def apply(v1: Int, v2: Int): Int = {
    v1 + v2
  }
}

val array = Array(1, 2, 3, 4)
val i = array.reduce(function)  // i=10
```

完全符合我们的预期

但是依靠`scala的模式匹配` ,还可以优化

```scala
val array = Array(1, 2, 3, 4)
val i = array.reduce(_+_)  // i=10
```



... 后面还有很多 . 不一一介绍了 . . . 



## 4. : => 与() : =>

### 1. () : =>  表示一个函数

```scala
def fun1(fun: () => String): Unit = {
    println(fun)
    val str: String = fun()
    println(str)
}
```

```scala
def main(args: Array[String]): Unit = {
    fun1(()=>"hello world")
}

输出 : 
// com.tree.TreeNode$$$Lambda$1/1996181658@2a18f23c
// hello world
```

### 2. : =>  不严谨

```scala
def fun2(fun: => Unit): Unit = {
    println("start")
    fun
}

输出  : 
// start
// hello world
```

