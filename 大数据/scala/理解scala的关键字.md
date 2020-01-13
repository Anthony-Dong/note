# Scala的关键字

## 1. class

> ​	最基本的类

```java
class Test(x:Int,var y:Int,val z:String,private var w:String,protected val a:Long) {
  private var b:Int=_

  protected var c:String=_

  var d:String=_

  private[this] var e: String = _

  @BeanProperty var f: String = _

}
```

上面的换成java代码 

```java
public class com.link_node.demo.Test {
  private int y;
  private final java.lang.String z;
  private java.lang.String w;
  private final long a;
  private int b;
  private java.lang.String c;
  private java.lang.String d;
  private java.lang.String e;
  private java.lang.String f;
  public int y();
  public void y_$eq(int);
  public java.lang.String z();
  private java.lang.String w();
  private void w_$eq(java.lang.String);
  public long a();
  private int b();
  private void b_$eq(int);
  public java.lang.String c();
  public void c_$eq(java.lang.String);
  public java.lang.String d();
  public void d_$eq(java.lang.String);
  public java.lang.String f();
  public void f_$eq(java.lang.String);
  public java.lang.String getF();
  public void setF(java.lang.String);
  public com.link_node.demo.Test(int, int, java.lang.String, java.lang.String, long);
}
```

当不加类型修饰`var`或者`val`的时候,`x`成员变量么啥用 ,不会生成getset方法 , 

当你用private修饰一个变量是同时`b`变量也没啥用,他的方法全是`私有`的, 

当你用`private[this]` 修饰一个变量时 ,他**不会生成getset方法** , 

当你用 `var`修饰一个变量时,此时不被任何限定词修饰时 , 他默认getset方法是公有的, 当你用`val`修饰是`final`

当你用 `@BeanProperty`修饰变量时 , 他会生成java的getset方法

## 2. object

> ​	一种派生类

```java
object Test2 {

  def main(args: Array[String]): Unit = {
    
  }
}
```

编译后

```java
D:\代码库\javase学习\DevelopmentJAVA\java-arithmetic\target\classes\com\link_node\demo>javap -p Test2$.class
Compiled from "Test2.scala"
public final class com.link_node.demo.Test2$ {
  public static final com.link_node.demo.Test2$ MODULE$;
  public static {};
  public void main(java.lang.String[]);
  private com.link_node.demo.Test2$();
}

D:\代码库\javase学习\DevelopmentJAVA\java-arithmetic\target\classes\com\link_node\demo>javap -p Test2.class
Compiled from "Test2.scala"
public final class com.link_node.demo.Test2 {
  public static void main(java.lang.String[]);
}
```

我们发现object修饰的类 , 他会被`final`修饰 , 同时会额外生成一个 `ClassName$.class`的对象 ,都被final修饰 



## 3. case

> ​	案例类,会生成两个类, 原来的类就是个普通类 , 还有一个 `ClassName.class` 的final修饰的类

### 1. 基本使用

```scala
case class Test3(var a:Int,val b: String,c:String,private var name:String) {

}
```

你什么也不修饰 `c:String` , 他默认是被`val`修饰的

你比如加了 `private` 修饰, 此时就是啥用也没有







### 字段类型

```scala
object Main3 extends App {

  case class Message(sender: String, recipient: String, body: String)

  val message1 = Message("guillaume@quebec.ca", "jorge@catalonia.es", "Ça va ?")

  println(message1.sender) // prints guillaume@quebec.ca
  
  //  message1.sender = "travis@washington.us"  // this line does not compile

  class Mes(sender: String, recipient: String)

   val mes = new Mes("", "")

}
```

案例类  默认的参数是 val修饰的

普通类  如果就写我上面的 , 此时这个类只有一个构造函数就是 俩参数的其他都没有  , 但是加了var ,修饰就会自带get和set方法 ,同时成员变量被private修饰... , 如果换做是val , 成员变量被private final修饰 , 所以无法修改 



### 对象一致

```scala
case class Per2()

object Main {

  def main(args: Array[String]): Unit = {
    val per1 = Per2
    val per2 = Per2
    val per3 = per2.apply()

    println(per1==per2)
    println(per1==per3)
  }
}
```

输出

```java
true
false
```



### match ... case

```scala
abstract class Notification

case class Email(sender: String, title: String, body: String) extends Notification

case class SMS(caller: String, message: String) extends Notification

case class VoiceRecording(contactName: String, link: String) extends Notification


case class SendSMS(caller: String, message: String) extends Notification

object Main4 extends App {
  def showNotification(notification: Notification): String = {
    notification match {
      case Email(sender, title, _) =>
        s"You got an email from $sender with title: $title"
      case SMS(number, message) =>
        s"You got an SMS from $number! Message: $message"
      case VoiceRecording(name, link) =>
        s"you received a Voice Recording from $name! Click the link to hear it: $link"
      case SendSMS(number, message) =>
        s"You got an SendSMS from $number! Message: $message"
    }
  }

  val mS = SendSMS("123", "Are you OK")
  val someSms = SMS("12345", "Are you there?")
  val someVoiceRecording = VoiceRecording("Tom", "voicerecording.org/id/123")


  println(showNotification(mS))  //You got an SendSMS from 123! Message: Are you OK

  println(showNotification(someSms)) // prints You got an SMS from 12345! Message: Are you there?

  println(showNotification(someVoiceRecording)) // you received a Voice Recording from Tom! Click the link to hear it: voicerecording.org/id/123
}
```



```scala
object Main10 extends App {
  abstract class Device
  case class Phone(model: String) extends Device {
    def screenOff = "Turning screen off"
  }
  case class Computer(model: String) extends Device {
    def screenSaverOn = "Turning screen saver on..."
  }
  def goIdle(device: Device):String = device match {
    case p: Phone => p.screenOff
    case c: Computer => c.screenSaverOn
  }
  println(goIdle(Computer("conputer")))
}
```







## 4. trait







> ​	class : public class TestClass {}
>
> ​	abstract class : public abstract class TestAba {}
>
> ​	trait :   public abstract interface TestTra {}
>
> ​	object  : public final class TestObj {}
>
> ​	class TestExt extends TestAba with TestTra :    public class TestExt extends TestAba implements TestTra {}
>
> ​	class TestExtend extends TestTra :   public class TestExtend implements TestTra
>
> ​	



## var val private 修饰的作用

```scala
// 什么也不修饰的 get set方法都是public的
class Person(var name: String, val age: Int, private var sex: String) {

  // 这个修饰的不会生成get , set方法 
  private[this] var gender: String = _

  //被 private修饰的,get,set方法是私有的
  private var birthday: Date = _
}
```



```java
public class com.spark.test2.Person {
  private java.lang.String name;
  private final int age;
  private java.lang.String sex;
  private java.lang.String gender;
  private java.util.Date birthday;
  public java.lang.String name();
  public void name_$eq(java.lang.String);
  public int age();
  private java.lang.String sex();
  private void sex_$eq(java.lang.String);
  private java.util.Date birthday();
  private void birthday_$eq(java.util.Date);
  public com.spark.test2.Person(java.lang.String, int, java.lang.String);
}    
```





