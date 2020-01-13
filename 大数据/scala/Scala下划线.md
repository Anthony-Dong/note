# Scala模式匹配

## 1. 匿名函数参数占位符

> ​	当匿名函数传递给方法或其他函数时，如果该匿名函数的参数在=>的右侧只出现一次，那么就可以省略=>，并将参数用下划线代替。这对一元函数和二元函数都适用。

```scala
scala> val list = List(5,3,7,9,1)
list: List[Int] = List(5, 3, 7, 9, 1)

scala> list.map
   final override def map[B](f: Int => B): List[B]

scala> list.map(x=>x*10)
res5: List[Int] = List(50, 30, 70, 90, 10)

scala> list.map(_*10)
res6: List[Int] = List(50, 30, 70, 90, 10)
```

## 2. 无用匿名函数参数

> ​	当匿名函数的参数未被实际使用到时，可以不给它一个命名，而直接用下划线代替。

```scala
scala> list.foreach
   final override def foreach[U](f: Int => U): Unit

scala> list.foreach(x=>println("hello"))
hello
hello
hello
hello
hello

scala> list.foreach(_=>println("hello"))
hello
hello
hello
hello
hello
```

## 3. **泛型定义中的通配符**

跟java的 `List<?>` 是等效的

```scala
scala> def listPrint(list: List[_]): Unit = {
     |     list.foreach(_$1 => print(_$1))
     |   }
listPrint: (list: List[_])Unit


scala>listPrint(List(1,"2",3))
123
```

## 4. **模式匹配中的通配符/占位符**

```scala

    println("=======模式匹配 - 默认匹配====================")
    val x: String = "1"
    x match {
      case "1" => println("match 1")
      case _ => println("其他")
    }


    println("=======模式匹配 - 匹配以0开头，长度为三的列表 ====================")
    val unit = List(0, 1, 2)
    unit match {
      case List(0, _, _) => println(s"found  $List")
      case _ => print("no found")
    }


    println("=======模式匹配 - 匹配以0开头，长度任意的列表====================")
    unit match {
      case List(0, _*) => println("found it ")
      case _ => println("not found")
    }

    println("=======模式匹配 - 匹配元组元素====================")
    val tuple = ("Sugar", 25): Tuple2[String, Int]
    tuple match {
      case ("", _) => println("found")
      case _ => println("no found")
    }

    println("=======模式匹配 - 将首元素赋值给head变量====================")
    val List(head, _*) = List("a")
    println(head)

```



## **变长参数的转化**

下划线与星号连用，可以将序列转化为变长参数的序列，方便调用变长参数的方法。例：

```scala
def sum(args: Int*): Int = {
  var result = 0
  for (arg <- args) {
    result += arg
  }
  result
}


println(sum(1, 2, 3, 4))
println(sum(1 to 4: _*))
```



## **将方法转换成函数**

> 我们都知道Scala中方法与函数的不同点。其中最显著的不同就在于，函数在Scala中是一种对象实例，因此它可以赋值给变量，也可以作为参数。如果方法在赋值时直接写名称的话，编译器会认为是对方法的调用，因此会报没有参数列表的错误。在方法名称后加一个下划线，会将其转化为偏应用函数（partially applied function），就能直接赋值了。例：

```scala
scala> def twoSum(a: Int, b: Int) = a + b
twoSum: (a: Int, b: Int)Int

scala> val two = twoSum _
two: (Int, Int) => Int = $$Lambda$991/1080911253@2a25dd46

scala> list.foreach(println _)
5
3
7
9
1

scala> list foreach println _
5
3
7
9
1
```





## **定义偏应用函数**

> ​	偏应用函数就是不提供或者只提供部分参数调用时返回的函数，我们也可以用下划线来代替那些不提供的参数。

```scala
scala> def threeMult(a: Int, b: Int, c:Int) = a * b * c
threeMult: (a: Int, b: Int, c: Int)Int

scala> val parThreeMult = threeMult(8, _: Int, 42)
parThreeMult: Int => Int = $$Lambda$1007/1023818098@53e57cee

scala> parThreeMult(1)
res35: Int = 336
```



## 导入通配符

```java
// import java.util.concurrent.*
scala> import java.util.concurrent._
import java.util.concurrent._

// import java.util.*，并将ArrayList重命名为al
scala> import java.util.{ArrayList => al, _}
import java.util.{ArrayList=>al, _}

// import java.util.*，但不导入Timer类
scala> import java.util.{Timer => _, _}
import java.util.{Timer=>_, _}

// import static java.lang.Math.*
scala> import java.lang.Math._
import java.lang.Math._
```





## for

```scala
val twentySomethings = for (user <- userBase if (user.age >= 20 && user.age < 30))
  yield user.name // i.e. add this to a list

twentySomethings.foreach(println _) // prints Travis Dennis


//
def foo(n: Int, v: Int): IndexedSeq[(Int, Int)] =
  for (i <- 0 until n; j <- i until n if i + j == v)
    yield (i, j)

foo(10, 10) foreach {
  case (i, j) =>
    println(s"($i, $j) ") // prints (1, 9) (2, 8) (3, 7) (4, 6) (5, 5)
}

foo(10, 10).foreach(x => println(x._1, x._2))

```

