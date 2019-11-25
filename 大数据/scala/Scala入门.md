# Scala入门

> ​	入门Scala 对于学习过Java的人很简单 , 我对他的理解就是一个Java的框架 (类似于前端的那种css和less的关系), 所以这个Java框架(Scala)需要一个编译器 , 编译成class文件 ,因为都是jvm语言吗 , 所以他们的连接点就是这个class文件 ,所以我们学习就主要是看 class文件 , 带你理解和学习 Scala .



## 1. 介绍

[官网地址](https://www.scala-lang.org/) : https://www.scala-lang.org , 如果想学习可以直接到这里学习 , 没必要去找视频之类的 , [文档链接](https://docs.scala-lang.org/zh-cn/tour/tour-of-scala.html) : https://docs.scala-lang.org/zh-cn/tour/tour-of-scala.html ,这个足够你理解scala了 , 学完

>  Scala combines object-oriented and functional programming in one concise, high-level language. Scala's static types help avoid bugs in complex applications, and its JVM and JavaScript runtimes let you build high-performance systems with easy access to huge ecosystems of libraries.
>
> Scala将面向对象和函数式编程结合在一起 , 所以他就是java版的 py或者go .....很相似 

## 2. 理解HelloWorld

首先这是一个`main函数` 是一个程序的入口 ,scala规定 `main函数`必须在一个 object修饰的类里 

```scala
object Main {

  def main(args: Array[String]): Unit = {
    print("hello world")
  }
}
```

但是我们不理解哇 .............. 所以开始编译吧

```cmd
 D:\代码库\scala\scala-test\out\production\scala-test\com\test2 的目录

2019/11/23  20:21    <DIR>          .
2019/11/23  20:21    <DIR>          ..
2019/11/23  20:21               681 Main$.class
2019/11/23  20:21               645 Main.class
               2 个文件          1,326 字节
               2 个目录 89,857,732,608 可用字节
```

我们发现文件只有两个 编译结果 , 一个是 `Main$.class` 和 `Main.class` , 此时我们通过 `javap -v`来编译一下,记得配置一下java环境变量哇 ....

```java
Classfile /D:/代码库/scala/scala-test/out/production/scala-test/com/test2/Main$.class
  Last modified 2019-11-23; size 681 bytes
  MD5 checksum 157596decb025fd3481d55a5a3f16da0
  Compiled from "Main.scala"
// 1. 申明 类信息 : public final class Main$ {......}
public final class com.test2.Main$
  minor version: 0
  major version: 52
  flags: ACC_PUBLIC, ACC_FINAL, ACC_SUPER
{
// 2. 申明 MODULE$ 成员变量 ,属性 public static final 
  public static final com.test2.Main$ MODULE$;
    descriptor: Lcom/test2/Main$;
    flags: ACC_PUBLIC, ACC_STATIC, ACC_FINAL

// 3. 静态代码块 , 初始化成员变量 public static final MODULE$=new Main$();
  public static {};
    descriptor: ()V
    flags: ACC_PUBLIC, ACC_STATIC
    Code:
      stack=2, locals=0, args_size=0
         0: new           #2                  // class com/test2/Main$
         3: dup
         4: invokespecial #12                 // Method "<init>":()V  调用构造方法
         7: putstatic     #14                 // Field MODULE$:Lcom/test2/Main$;
        10: return
      LineNumberTable:
        line 3: 0

// 4 . 申明了一个方法 , public void main(String[] arg) 的方法 , 主要是调用了 `Predef$.print` 方法
  public void main(java.lang.String[]);
    descriptor: ([Ljava/lang/String;)V
    flags: ACC_PUBLIC
    Code:
      stack=2, locals=2, args_size=2
         0: getstatic     #22                 // Field scala/Predef$.MODULE$:Lscala/Predef$;
         3: ldc           #24                 // String hello world
         5: invokevirtual #28                 // Method scala/Predef$.print (Ljava/lang/Object;)V    调用Predef$.print 
         8: return
      LineNumberTable:
        line 6: 0
      LocalVariableTable:
        Start  Length  Slot  Name   Signature
            0       9     0  this   Lcom/test2/Main$;
            0       9     1  args   [Ljava/lang/String;
    MethodParameters:
      Name                           Flags
      args                           final
}
```

再看看 `main.class`

```java
Classfile /D:/代码库/scala/scala-test/out/production/scala-test/com/test2/Main.class
  Last modified 2019-11-23; size 645 bytes
  MD5 checksum 40dacb48aad23df369da74e1d1f036ef
  Compiled from "Main.scala"
public final class com.test2.Main
  minor version: 0
  major version: 52
  flags: ACC_PUBLIC, ACC_FINAL, ACC_SUPER
Constant pool:
   #1 = Utf8               com/test2/Main
	.... 省略 
  #24 = Utf8               ScalaSig
{
// 所以这个才是主方法 ,这里调用了 Main$.MODULE$ 的 一个 main() 方法....
  public static void main(java.lang.String[]);
    descriptor: ([Ljava/lang/String;)V
    flags: ACC_PUBLIC, ACC_STATIC
    Code:
      stack=2, locals=1, args_size=1
         0: getstatic     #17                 // Field com/test2/Main$.MODULE$:Lcom/test2/Main$;
         3: aload_0
         4: invokevirtual #19                 // Method com/test2/Main$.main:([Ljava/lang/String;)V
         7: return
    MethodParameters:
      Name                           Flags
      args                           final
}
```

总结一下以上信息 , 我们大致可以写一下 `Main$` 和 `Main`的 java写法 

```java
public final class Main$ {
    // final是线程安全对象
    public static final Main$ MODULE$ = new Main$();
    
    public void mian(String[] args) {

        // 这里不属于讲解范畴 ,我觉得大家都懂 就是一个静态方法调用
        Predef$.print(args);
    }
}
```

```java
public final class Main {
    public static void main(String[] args) {
        Main$.MODULE$.mian(args);
    }
}
```

是不是很简单呢 , 所以学习别想着这么复杂 , 第一个入门就OK了 , 还有一句就是 object 修饰的类都是 final 的 , 所以他不能继承 ...........这点你注意了么 ,