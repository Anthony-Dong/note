# java 8 语法糖(Syntactic sugar)

## 1. 函数式接口(Functional Interfaces)

>- Lambda表达式是Java SE 8中一个重要的新特性。允许你通过表达式来代替功能接口，其几乎解决了匿名内部类带来的所有问题。
>
>- Functional Interfaces 是一个只有单个抽象方法的接口。（但是可以有其他方法，例如**有其他继承Object而来的方法**,例如 tostring 方法之类的）。
>- **Object中的clone()是protected的**，而接口中的方法只能是public，所以在接口中声明Object clone();会被**当做一个新的方法** .
>- 还可以声明 default修饰的方法和static修饰的方法
>- lambda表达式允许你通过表达式来代替上面的Functonal接口。

**1.8中 局部内部类访问的局部变量不必用final修饰 (1.8之后java隐式的将x修饰为final) ,为什么添加,主要是考虑到了 变量的生命周期问题.如果我的x是非 final的,当我去调用时,发现他已经被GC了,此时就会输出0** 可以看出来是自动加上的(编译后的class文件)

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-20/ca51b967-e026-4bdd-9b4b-cf915d5c262e.png?x-oss-process=style/template01)

## 2. 方法引用(method reference)  

> ​	方法引用跟 Lambda 表达式很像，他们都需要一个目标类型，但是不同的是方法引用不提供方法的实现，他们引用一个**已经存在的类或者对象**的方法。 
>
> ​	::符号叫做双冒号操作符(double colon operator)，用来表示对某个类或对象的某个方法的引用
>
> ​	方法引用不可以控制传递参数。

- 使用方式
  - 静态方法 : 类名::方法名 
  - 实例方法 : 对象::方法名

### **存在的问题 -----  求大佬解答**

```java
public class Task implements Runnable {

    @Override
    public void run() {
        System.out.println(Thread.currentThread().getName());
    }

    public static void main(String[] args) {
        // 执行
        new Thread(new Task(),"线程一").start();
        // 不执行
        new Thread(Task::new,"线程二").start();
    }
}
```



```java
    public static void main(String[] args) throws InterruptedException {
        Executor executors = Executors.newFixedThreadPool(10);
        // 不执行
        executors.execute(Task::new);
         // 执行
        executors.execute(new Task());
        executors.execute(new Task());
    }

线程池也是 这种情况
```







## 3. 构造函数引用 

```java
    private static class  Task implements  Runnable{

        @Override
        public void run() {
            System.out.println(Thread.currentThread().getName());
        }
    }

	
    public static void main(String[] args) {
        Thread thread = new Thread(new Task());
        Thread thread1 = new Thread(Task::new);       
    }
```







> ​	详细的用法可以看看这个[链接](https://blog.csdn.net/HuakePeng/article/details/90081622)