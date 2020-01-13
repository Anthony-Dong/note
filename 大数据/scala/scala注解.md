# Scala注解

[参考文章](https://www.jianshu.com/p/2c3ad12a8d5e) : https://www.jianshu.com/p/2c3ad12a8d5e



## 1. @BeanProperty

> ​	 Java好多框架 对于 Bean的封装都需要get set 方法 ,所以需要我们定义很麻烦 , 所以是scala提供了 注解

```scala
class Person {

  @BeanProperty var name: String = _

  @BeanProperty var age: Int = _

}
```

此时编译以后的代码是 : 

> 会生成 符合java的 get set 方法
>
> 同时也会生成 scala的set,set方法

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-26/fef22e56-bc44-4ac9-af28-96ec28d61094.png?x-oss-process=style/template01)



**注意 :** 

您可以将` private `关键字添加到` val `或` var `字段，以防止getter和setter方法生成。此时也无法使用`@BeanProperty` 注解 

### 如何写一个构造器

```scala
class Person() {

  @BeanProperty var name: String = _

  @BeanProperty var age: Int = _

  def this(name: String, age: Int) {
    this()
    this.name = name
    this.age = age
  }

  override def toString = s"Person($name, $age)"
}

此时基本完成了 java开发的需求
```



## 2. @Test - Junit

> ​	 Junit 测试使用 

```scala
class Demo {

  @Test def test(): Unit ={
    println("hello world")
  }
}
```



## 3. @volatile  

> ​	可见性

```scala
class Demo {
  @volatile var done = false
}
```

编译后 

```java
private volatile boolean done = false;
```



### 其他修饰符

```java
@volatile var done = false  // JVM中将成为volatile的字段
@transient var recentLookups = new HashMap[String, String]  // 在JVM中将成为transient字段，该字段不会被序列化。
@strictfp def calculate(x: Double) = ...
@native def win32RegKeys(root: Int, path: String): Array[String]
```



## 4. @throws

scala 申明

```scala
@throws(classOf[IOException])
@varargs def process(args: String*): Unit ={
  // ...
}
```

java调用 

```java
public static void main(String[] args) throws IOException {

    Person person = new Person();

    person.process("1", "2");

}
```





## 5. @varargs

```scala
// 加上 @varargs
@varargs def process(args: String*)
// 编译器将生成如下java方法

void process(String... args)
```



## 6. @SerialVersionUID

```scala
@SerialVersionUID(6157032470129070425L)
class Employee extends Person with Serializable
```



## 7. 枚举

> ​	scala 的枚举 需要 继承 scala的 `Enumeration`

```scala
object MyEnum extends Enumeration {
  type MyEnum = Value
  val Red = Value("1")
  val Green = Value("1")
  val Blue = Value("1")


}

object Main2 {

  def main(args: Array[String]): Unit = {
    MyEnum.values.foreach(e => println(e))

    println(MyEnum.values)
  }

}
```



## 8. type

```scala
trait A {
  type T
    
  def foo(i: T) = print(i)
}

class B extends A {
  // 类似于泛型 java的  
  override type T = Int
}

object C {
  def main(args: Array[String]): Unit = {
    val b = new B
    b.foo(1)  // 1
  }
}
```

