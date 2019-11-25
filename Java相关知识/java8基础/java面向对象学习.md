# java面向对象学习

## 1. 继承

### constructor

- **Java会为每一个对象创建一个默认的构造器,但是当重写时,他就不会给你去创建了** , 例如 你写了一个有参的构造器,此时就 不会自动给你生成一个默认的构造器 .

- 还有就是 **子类会默认调用父类的构造器**,只是我们不去写而已 ,我用eclipse的一张图 说明一下 

  ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-23/03c235a6-a231-4120-94d9-b336e9408cfd.png?x-oss-process=style/template01)

它这里说的很明白就是 省略 调用默认的父类构造器 ,所以当我们不去写 这个时,其实Java 就会 隐式的给我们加上这个. 但是当你父类有其他构造器时,可以覆盖这个默认构造器: 例如

```java
public Person(Integer unm) {
        this.unm = unm;
        System.out.println("我是父类带参构造器");
 }
public Son() {
    // 这里可以覆盖掉
      super(null);
      System.out.println("我是子类的默认构造器");
 }
```

**而且注意: 当我们使用单例模式的时候,会把构造方法私有化,所以此时就无法让子类继承,因为继承会自动引用父类构造器,可以用protected修饰**

所以我们看看下面这段代码的执行结果

```java
public class Demo {

    public static void main(String[] args) {
        Son son = new Son();
    }
}

class Person{
    public Person() {
        super();
        System.out.println("我是父类默认构造器");
    }
}

class Son extends  Person{
    public Son() {
        super();
        System.out.println("我是子类默认构造器");
    }
}
```

执行结果 

```java
我是父类默认构造器
我是子类默认构造器
```

所以发现实例化一个对象时,会自动调用父类的构造方法, 不过我们可以显示的重写 . 顶层对象都是 Object 



当我们知道结果 尝试去改变时,会发现  他说 调用父类的构造器必须在开始位置声明在子类构造器中.所以我们不可能改变 父类优于子类构造器调用的关系

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-23/94cbc6c7-164a-4687-a89f-d6c6f089a0b6.jpg?x-oss-process=style/template01)



### 向上-向下转型

- 向上转型 时,例如:  `Person person = new Son();` ,不会影响子类重写父类方法的调用,但是此时不能调用子类自己的方法 ,需要向下转型.`((Son) person).m2();` . 
- 向下转型 时 , 例如 `Son son = (Son) new Person();` ,此时当 子类调用父类的方法时候会抛出异常 `java.lang.ClassCastException: com.javase.Java_Object_Oriented.extend.Person cannot be cast to com.javase.Java_Object_Oriented.extend.Son`  所以向下转型时会发生子类对象无法调用父类方法.

### 协变返回 - Covariant Return 

> ​	Java5.0 放宽了这一限制，只要子类方法与超类方法具有相同的方法签名，或者子类方法的返回值是超类方法的子类型，就可以覆盖。

```java
public class covariantReturn {
 
    public static void main(String[] args) {
        MilK milK = new MilK();
        System.out.println("milK.process() = " + milK.process());

        WheatMilK wheatMilK = new WheatMilK();
        System.out.println("wheatMilK.process() = " + wheatMilK.process());

    }

}

class Grandparent{
    public String toString(){
        return "Grandparent";
    }
}
class Parent extends Grandparent{
    public String toString(){
        return "Parent";
    }
}


class MilK{
    Grandparent process(){
        return new Grandparent();
    }
}

class WheatMilK extends MilK{

    /**
     * 只要子类方法与超类方法具有相同的方法签名
     * 子类方法的返回值是超类方法的子类型，就可以覆盖。
     * @return
     */
    @Override
    Parent process(){
        return new Parent();
    }
}
```

输出 : 

```java
milK.process() = Grandparent
wheatMilK.process() = Parent
```

很显然是两个不同的结果,重写时可以改变返回类型的 , 只要是返回类型属于一种继承关系



## 2. private & 空 & protected & public 

​	  当我们不写作用域修饰词时,此时他默认是friendly,只可以被当前类和同一个package类访问  ,就算是子类(不是同一个包下的) 也无法访问(此时需要修改为protected,可以让子类访问) ,

> ​	并不是只有 private和public,我们看好多源码都是 protected
>
> ​	private 私有权限,只有自己能用。 friendly 包权限,同一个包下的可用。 protected 继承权限,（是包权限的扩展，子女类也可使用）。 public 谁都可以用。
>
> ​	其实 就是 一种范围逐级扩大 ,当你私有时,只属于你自己 ; 当不写时,属于同包下 ;当用protected时,此时属于可继承状态,
>
> 子类可以享受 ;  当public时属于全部.



## 3. transient

1）一旦变量被transient修饰，变量将不再是对象持久化的一部分，该变量内容在序列化后无法获得访问。

2）transient关键字只能修饰变量，而不能修饰方法和类。注意，本地变量是不能被transient关键字修饰的。变量如果是用户自定义类变量，则该类需要实现Serializable接口。

3）被transient关键字修饰的变量不再能被序列化，一个静态变量不管是否被transient修饰，均不能被序列化。