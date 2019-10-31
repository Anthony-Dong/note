# java类加载机制的理解

## Java程序初始化顺序

遵循三个原则(优先级 逐级递减) :

- 静态对象优于非静态对象
- 父类优于子类
- 成员变量定义顺序

父类静态变量 - 父类静态代码块 - 子类静态变量 -子类静态代码块 - 父类非静态变量 - 父类非静态代码块 - 子类非静态变量 -  子类非静态代码块 - 子类构造函数

```java
public class Demo02 {
    public static void main(String[] args) {
        new Son();
    }
}

class Person{
    static {
        System.out.println("父类 静态代码块");
    }

    {
        System.out.println("父类 非静态代码块");
    }

    public Person() {
        System.out.println("父类 构造器");
    }
}
class Son extends Person{
    static {
        System.out.println("子类 静态代码块");
    }

    {
        System.out.println("子类 非静态代码块");
    }

    public Son() {
        System.out.println("子类 构造器");
    }
}
```



## 示例一 

试着想一下 这段代码的输出情况

```java
public class JavaExecuteOrder {
    static JavaExecuteOrder order = new JavaExecuteOrder();
    public JavaExecuteOrder() {
        System.out.println("构造方法");
        System.out.println("x : "+x++);
        System.out.println("y : "+y++);
    }

    static {
        System.out.println("静态代码块");
    }

    {
        System.out.println("非静态代码块");
    }

    static int x = 1;
    int y = 2;
    public static void main(String[] args) {
        System.out.println("main 方法");
    }
}
```

**结果**

```java
非静态代码块
构造方法
x : 0
y : 2
静态代码块
main 方法
```

所以 我们可以执行流程 , 就是静态代码块先执行

1. 执行第一段静态代码`static JavaExecuteOrder order = new JavaExecuteOrder();` 这段代码实例化了一个对象 ,那么我们要知道什么是属于是对象,什么是属于类的. 所以 非静态的东西全部属于实例化对象的,那么就执行 非静态代码块,然后输出`非静态代码块` ,其次是构造方法,所以输出`构造方法` ,那么为什么 `"x : "+x++` 输出是 `x : 0`这个呢,因为就是我们说的谁属于谁,此时只执行了第一句静态块,那么 x 还没有实例化了, 因为int对于null值的默认值是0, 那么 x=0,内存中, 又因为`x++` (这个不解释了和`++x`的区别)此时输出结果就是 `0`  ,然后就顺气自然了.
2. 执行第二段静态代码 就是输出`静态代码块` 
3. 执行 main 方法的输出 `main 方法`



那么我将上面内容稍微改改 

```java
public class JavaExecuteOrder {

    static JavaExecuteOrder order = new JavaExecuteOrder();


    public JavaExecuteOrder() {
        System.out.println("构造方法");
        System.out.println("x : "+x++);
        System.out.println("y : "+y++);
    }

    static {
        System.out.println("静态代码块");
    }

    {
        System.out.println("非静态代码块1");
    }
    {
        System.out.println("非静态代码块2");
    }

    static int x = 1;

    int y = 2;
    
    public static void main(String[] args) {
        System.out.println("main 方法");
        new JavaExecuteOrder();
    }

}

```



**这个结果大家应该很清楚了吧**

```java
非静态代码块1
非静态代码块2
构造方法
x : 0
y : 2
静态代码块
main 方法
非静态代码块1
非静态代码块2
构造方法
x : 1
y : 2
```

所以 还是我上面说的那么句话 `谁属于谁`  就是类加载顺序的关键 . 这类问题就自然而然解答了







## 示例二

首先我们有两个类 : 

```java
public class TestStatic {

    public static  int x = 1;

    static {
        System.out.println("TestStatic : 执行静态代码块");
    }
}


public class TestStaticFinal {

    public static final int x = 1;

    static {
        System.out.println("TestStaticFinal : 执行静态代码块");
    }
}

```

我们要执行 : 

```java
public class JavaStaticVsFinal {
    
    public static void main(String[] args) {
        System.out.println("TestStatic.x = " + TestStatic.x);
        
        System.out.println("===========================");
        
        System.out.println("TestStatic.x = " + TestStaticFinal.x);
    }
}

```

执行结果如下 ; 

```java
TestStatic : 执行静态代码块
TestStatic.x = 1
===========================
TestStatic.x = 1
```



​		我们发现 `TestStaticFinal `并没有执行 静态代码块 . 原因是什么? 

我们用`javap `看一下  `TestStatic ` 的class反编译文件 

```java
  public static int x;
    descriptor: I
    flags: ACC_PUBLIC, ACC_STATIC
    ....
  
  static {};
    descriptor: ()V
    flags: ACC_STATIC
    Code:
      stack=2, locals=0, args_size=0
         0: iconst_1
         1: putstatic     #2                  // Field x:I  
         ....
```

`TestStaticFinal` 反编译文件 

```java
{
  public static final int x;
    descriptor: I
    flags: ACC_PUBLIC, ACC_STATIC, ACC_FINAL
    ConstantValue: int 1
    .....    
```



我们发现 final  在类初始化的时候就已经执行了 初始化,所以根本不会调用静态代码块, 所以类的加载机制里面会将final在初始化的时候就会赋值 .

而没有被final修饰的则会在static静态块里初始化



## 示例三

为什么 只有一块静态代码块呢 ?  

```java
public class TestStaticFinal {

      public int x = 1;

    public static int x1 = 1;


    {
        System.out.println("TestStatic : 执行非静态代码块");
    }

    static {
        System.out.println("TestStatic : 执行静态代码块1");
    }


    public static int x2 = 1;

    static {
        System.out.println("TestStatic : 执行静态代码块2");
    }

    public static final int x3 = 1;


    public static void staticMethod(){
        System.out.println("staticMethod");
    }


    public  void ordinaryMethod(){
        System.out.println("ordinaryMethod");
    }
}
```

反编译的源码 `javap -v`

```java
{
     // 变量赋值  :  变量名 , 限定词 , 是否静态 , 是否 final  
  public int x;
    descriptor: I
    flags: ACC_PUBLIC

  public static int x1;
    descriptor: I
    flags: ACC_PUBLIC, ACC_STATIC

  public static int x2;
    descriptor: I
    flags: ACC_PUBLIC, ACC_STATIC

  public static final int x3;
    descriptor: I
    flags: ACC_PUBLIC, ACC_STATIC, ACC_FINAL
    ConstantValue: int 1
	
        
  // 构造方法 1 
  public com.javase.the_order_of_java_execute.TestStatic();
    descriptor: ()V
    flags: ACC_PUBLIC
    Code:
      stack=2, locals=1, args_size=1
         0: aload_0
         1: invokespecial #1                  // Method java/lang/Object."<init>":()V
         4: aload_0
         5: iconst_1
         6: putfield      #2                  // Field x:I
         9: getstatic     #3                  // Field java/lang/System.out:Ljava/io/PrintStream;
        12: ldc           #4                  // String TestStatic : 执行非静态代码块
        14: invokevirtual #5                  // Method java/io/PrintStream.println:(Ljava/lang/String;)V
        17: return
      LineNumberTable:
        line 25: 0
        line 12: 4
        line 18: 9
        line 26: 17
      LocalVariableTable:
        Start  Length  Slot  Name   Signature
            0      18     0  this   Lcom/javase/the_order_of_java_execute/TestStatic;

    
   // 构造方法 2
  public com.javase.the_order_of_java_execute.TestStatic(int);
    descriptor: (I)V
    flags: ACC_PUBLIC
    Code:
      stack=2, locals=2, args_size=2
         0: aload_0
         1: invokespecial #1                  // Method java/lang/Object."<init>":()V
         4: aload_0
         5: iconst_1
         6: putfield      #2                  // Field x:I
         9: getstatic     #3                  // Field java/lang/System.out:Ljava/io/PrintStream;
        12: ldc           #4                  // String TestStatic : 执行非静态代码块
        14: invokevirtual #5                  // Method java/io/PrintStream.println:(Ljava/lang/String;)V
        17: aload_0
        18: iload_1
        19: putfield      #2                  // Field x:I
        22: return
      LineNumberTable:
        line 28: 0
        line 12: 4
        line 18: 9
        line 29: 17
        line 30: 22
      LocalVariableTable:
        Start  Length  Slot  Name   Signature
            0      23     0  this   Lcom/javase/the_order_of_java_execute/TestStatic;
            0      23     1     x   I
    MethodParameters:
      Name                           Flags
      x

          
  // 普通静态方法
  public static void staticMethod();
    descriptor: ()V
    flags: ACC_PUBLIC, ACC_STATIC
    Code:
      stack=2, locals=0, args_size=0
         0: getstatic     #3                  // Field java/lang/System.out:Ljava/io/PrintStream;
         3: ldc           #6                  // String staticMethod
         5: invokevirtual #5                  // Method java/io/PrintStream.println:(Ljava/lang/String;)V
         8: return
      LineNumberTable:
        line 42: 0
        line 43: 8

   // 普通方法         
  public void ordinaryMethod();
    descriptor: ()V
    flags: ACC_PUBLIC
    Code:
      stack=2, locals=1, args_size=1
         0: getstatic     #3                  // Field java/lang/System.out:Ljava/io/PrintStream;
         3: ldc           #7                  // String ordinaryMethod
         5: invokevirtual #5                  // Method java/io/PrintStream.println:(Ljava/lang/String;)V
         8: return
      LineNumberTable:
        line 47: 0
        line 48: 8
      LocalVariableTable:
        Start  Length  Slot  Name   Signature
            0       9     0  this   Lcom/javase/the_order_of_java_execute/TestStatic;

    // 静态代码块
  static {};
    descriptor: ()V
    flags: ACC_STATIC
    Code:
      stack=2, locals=0, args_size=0
         0: iconst_1
         1: putstatic     #8                  // Field x1:I
         4: getstatic     #3                  // Field java/lang/System.out:Ljava/io/PrintStream;
         7: ldc           #9                  // String TestStatic : 执行静态代码块1
         9: invokevirtual #5                  // Method java/io/PrintStream.println:(Ljava/lang/String;)V
        12: iconst_1
        13: putstatic     #10                 // Field x2:I
        16: getstatic     #3                  // Field java/lang/System.out:Ljava/io/PrintStream;
        19: ldc           #11                 // String TestStatic : 执行静态代码块2
        21: invokevirtual #5                  // Method java/io/PrintStream.println:(Ljava/lang/String;)V
        24: return
      LineNumberTable:
        line 14: 0
        line 22: 4
        line 32: 12
        line 35: 16
        line 36: 24
}
```



我们发现在上面  确实只有一个静态代码块 ,在里面 ,所有被static修饰的 全部包括在里面  而且非静态代码块也是包含在对象里 . 当你看到我反编译的源码的时候你瞬间理解了上面的为什么了`谁属于谁 `



##  idea 如何查看 class字节码

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-29/66c4b58f-c36a-4d4b-8b87-c84fb293f95d.jpg)



然后使用

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-29/99f5bced-afe9-43f6-a785-0757162506c9.png)

