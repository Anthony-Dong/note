# Scala - Future

文章链接 https://colobu.com/2015/06/11/Scala-Future-and-Promise/

Futures 和 Promises是Scala的语言的功能加强

Future提供了一个漂亮的方式提供并行执行代码的能力，高效且非阻塞。Future可以并发地执行，可以提供更快，异步，非阻塞的并发代码。
通常，future和promise都是非阻塞的执行，可以通过回调函数来获得结果。但是，你也可以通过阻塞的方式串行的执行Future。

## 1. Future

```java
def getName: List[String] = {
  Thread.sleep(1000)
  println("执行线程 : " + Thread.currentThread().getName)
  List("1", "2", "3")
}

val future: Future[List[String]] = Future {
    // 异步执行
  getName
}

// 1. 总方式
future.onComplete {
  case Success(list) => for (name <- list) println(name)
  case Failure(error) => println(s"异常信息 : ${error.getMessage} .")
}

// 1. 成功
future.onSuccess {
   case list: List[String] => for (name <- list) println(name)
}

//2. 失败
future.onFailure{
      case error: Error=>println(error)
	}

System.in.read()
```

