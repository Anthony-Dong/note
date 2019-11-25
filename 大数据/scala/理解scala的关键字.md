# Scala的关键字

## 1. object

## 2. class

## 3. case

### 1. 基本使用

```scala
case class Point(x: Int, y: Int)

object Main{
  def main(args: Array[String]): Unit = {
    val point1 = Point.apply(1,3)
    val point = Point(1, 2)
    val maybeTuple = Point.unapply(point)
    println(maybeTuple.get._1)  
  }
}
```

输出

```scala
1
```



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

> 

