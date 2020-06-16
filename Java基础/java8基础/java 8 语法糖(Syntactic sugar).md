# java 8 语法糖(Syntactic sugar)

> ​	函数式编程的思想, 近几年也别火热, 因为他的语法让人吃惊 , 可以以最少的代码量完成同样的任务 , 此时作为老牌语言Java也不会放弃 , 在JDK1.7的时候引入的 `invokedynamic` 这个jvm指令就是为了JDK1.8的`lambda`打基础了 , 
>
> ​	对于函数式编程大家其实可以以面向对象编程思想来思考, 函数是什么 ? 函数和方法的区别在哪 ? 如果你明白了前面俩问题, 其实对于Java语言这种面向对象语言的函数式编程的实现就迎刃而解了  ,  如果你想不明白继续看下面的 . 
>
> ​	函数和方法这俩概念和定义是不一样的, 因为函数是一个对象 , 而方法是对象的组成部分 ,  所以他俩关系就是函数对象包含方法 , 同时这个函数对象只能有一个方法 , 同时函数对象必须是只有一个抽象方法的接口类 , 此时就是Java的函数式编程的实现了  . 

## 1. 函数式接口(Functional Interfaces)

>- Lambda表达式是Java SE 8中一个重要的新特性。允许你通过表达式`()->{}`来代替功能接口，其几乎解决了匿名内部类(单个方法的内部类)带来的所有问题。同时他编译采用的是 `invokedynamic` 属于jvm优化的, 速度上优于匿名内部类的执行速度. 因为内部类需要运行时生成字节码, lambda不需要
>
>- Functional Interfaces 是一个只有`一个抽象方法的接口`。这里我提一下Object的方法他也有, 因为Object是所有Java类的顶层父类 . 这个不需要解释吧 . 
>- 还可以声明 Java8 带来的`default`修饰的方法 还有 常见的`静态方法` , 还有一些被`public static final`修饰的字段

1.8中 局部内部类访问的局部变量不必用final修饰 (1.8之后java隐式的将x修饰为final) ,为什么添加,主要是考虑到了 变量的生命周期问题.如果我的x是非 final的,当我去调用时,发现他已经被GC了,此时就会输出0 ,  可以看出来是自动加上的(借助IDE工具可以查看编译后的class文件)

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-20/ca51b967-e026-4bdd-9b4b-cf915d5c262e.png?x-oss-process=style/template01)

## 2. 方法引用(method reference)  

> ​	方法引用是对lambda的进步一简写, 为什么这么说的 , 举一个场景的例子

```java
@Test
public void test() {
    List<String> list = Arrays.asList("a", "b", "c");
    list.forEach(e -> System.out.println(e));
}
```

这里e变量的作用是啥, 前面参数是e,后面方法需要只有e,那么是不是可以省略不写,因为前后参数都是e,可以让编译器来做补充,  所以此时就变成了

```java
@Test
public void test() {
    List<String> list = Arrays.asList("a", "b", "c");
    list.forEach(System.out::println);
}
```

这就是方法 引用,  如果你写过scala的模式匹配, 你会发现跟这个如出一辙 , 所以他只是lambda的简写形式, 为了语法美观 . 

主要分为3类 , 

- 静态方法引用    `静态类::方法名称`
- 构造函数引用  `类::new`
- 实例化方法引用  `实例化对象::方法`

所以基本写法就是 `引用对象::方法` 这个了 ;  所以叫方法引用了, 其实不如叫引用方法 , 哈哈哈 , 开玩笑

### 这里注意几点大家遇到的问题

下面这个语句 , 他会执行吗 ? , 会执行输出 `hello method reference` 吗 , 你可以先试着想一下, 我上面说的解释回想一下, 

```java
public class TestMethodReference implements Runnable {

    public static void main(String[] args) {
        new Thread(TestMethodReference::new).start();
    }

    @Override
    public void run() {
        System.out.println("hello method reference");
    }
}
```

很显然运行一下 ,啥也不会输出 , 那是为什么 . 

因为 runnable接口, 改写成lambda表达式是 `()->{....}` , 此时如果改成方法引用,只要是任何空参方法都可以填写进去 , 比如我给大家试试

```java
public class TestMethodReference implements Runnable {

    public static void main(String[] args) {
        new Thread(TestMethodReference::test).start();
    }

    @Override
    public void run() {
        System.out.println("hello method reference");
    }

    public static void test() {
        System.out.println("test");
    }
}
```

此时就会输出 test , 那是为什么 , 我们要技术 , 好记性不如烂笔头, 我们把这两个输出都改写成 ,lambda ,会是什么样子 ? 

```java
new Thread(() -> TestMethodReference.test()).start();
new Thread(() -> new TestMethodReference()).start();
```

会是这个样子 ,如果你还是不明白 , 我们继续改写

```java
new Thread(new Runnable() {
    @Override
    public void run() {
        TestMethodReference.test();
    }
}).start();

new Thread(new Runnable() {
    @Override
    public void run() {
        new TestMethodReference();
    }
}).start();
```

这下子改明白为什么了吧 . 刚刚那个 `TestMethodReference::new` 不执行 ,是为什么了吧 . 此时他run方法中执行的是实例化`TestMethodReference`对象, 而不是调用他的run方法 . 



我们简单证明一下

```java
public class TestMethodReference implements Runnable {
    public TestMethodReference() {
        System.out.println("实例化成功");
    }

    public static void main(String[] args) {
        new Thread(TestMethodReference::new).start();
    }
    
    @Override
    public void run() {
        System.out.println("hello method reference");
    }
}
```

此时会输出

```java
实例化成功
```



还有一种写法也需要注意

```java
public class TestMethodReference implements Runnable {

    public static void main(String[] args) {
        TestMethodReference reference = new TestMethodReference();
        new Thread(reference::runnable).start(); // 输出为空
    }

    private Runnable runnable() {
        return () -> {
            System.out.println("run 方法执行成功");
        };
    }

    @Override
    public void run() {
        System.out.println("hello method reference");
    }
}
```

也是上面同样的问题

​	

## 3. 总结

总结一句就是 , 对于方法引用我们对于不了解的时候, 尽可能的使用lambda写法, 这样子可以避免发生错误 , 同时对于方法引用的问题 , 也可以通过转换成 lambda来思考





如果你还对Java中其他语法糖感兴趣 , 可以去[这个链接](https://www.cnblogs.com/qingshanli/p/9375040.html) : https://www.cnblogs.com/qingshanli/p/9375040.html . 





> https://blog.csdn.net/HuakePeng/article/details/90081622)