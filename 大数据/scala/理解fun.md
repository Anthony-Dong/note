# Scala函数 和 方法

> ​	`def functionName ([list of parameters]) : [return type]`

1. 最简单的函数

```java
object Main extends App {

  // 函数名 (参数) : 返回类型 = { 逻辑 } 
  def echo(msg: Any): Any = {
    msg
  }

  println(echo("aaaa"))
}      
```

2. 高级函数'

```scala
 
// 函数名 urlBuilder , 参数 ssl ,domainName , 返回类型 是一个 匿名函数 , 匿名函数的返回类型时一个string
def urlBuilder(ssl: Boolean, domainName: String): (String, String) => String = {
    val schema = if (ssl) "https://" else "http://"
    (endpoint: String, query: String) => s"$schema$domainName/$endpoint?$query"
  }

 println(urlBuilder(true, "www.baidu.com")("user", "id=1"))
```



闭包是一个函数，它返回值取决于在此函数之外声明的一个或多个变量的值。

```scala
val z = (e: String) => "aaa"

println(z("1"))
```



可变参数

```scala
 def calcSumWithQueue(numbers: Int*):Int = {
    var sum = 0
    for (element <- numbers){
      sum+= element
    }
    sum
  }

```

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

```scala
  def main(args: Array[String]): Unit = {
    //可变参数的实参为Range的时候，需要添加:_*来进行数据的转换
    println("sum = " + calcSumWithQueue(1 to 10: _*))
    //可变参数的实参为Array时，需要添加:_*来进行数据的转换
    val arr = Array("Hadoop", "Spark", "Storm")
    printArr(arr: _*)
  }

```

