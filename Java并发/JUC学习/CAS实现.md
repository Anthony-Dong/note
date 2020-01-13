# CAS实现

- CAS 的全称 Compare-And-Swap，它是一条 CPU 并发语句。
- 它的功能是判断内存某一个位置的值是否为预期，如果是则更改这个值，这个过程就是原子的。



## Java实现

基本就是 `Atomic` 前缀开头的基本都是原子类	`java.util.concurrent.atomic` 这个包下面

主要就是 int , long , reference

##  基本原理

其实他间接使用了 Java的 `sun.misc.Unsafe` 类的 `sun.misc.Unsafe#compareAndSwapInt` 方法实现的, 注意不是直接, 还有其他修饰的方法 , 举个demo吧

```java
public class CASTest {

    private final int num;

    public CASTest(int num) {
        this.num = num;
    }

    public static void main(String[] args) throws Exception {
        Unsafe unsafe = getUnsafe();
        // 类
        CASTest target = new CASTest(10);

        // 类字段
        Field field = CASTest.class.getDeclaredField("num");

        // 传入字段
        long offset = unsafe.objectFieldOffset(field);

        // 设置
        unsafe.compareAndSwapInt(target, offset, 10, 100);

        System.out.println(target.num);  // 输出 100 
    }
}
```

这就是 `compareAndSwapInt` 称之为CAS算法 , 他是一个原子操作, 建立在CPU指令基础上的



像 `java.util.concurrent.atomic.AtomicInteger`  中使用到的是 

```java
 public final int incrementAndGet() {
        return unsafe.getAndAddInt(this, valueOffset, 1) + 1;
}
```

大概解释一下 

```java
public static void main(String[] args) throws Exception {

    Unsafe unsafe = getUnsafe();
    // 类
    CASTest target = new CASTest(10);

    // 类字段
    Field field = CASTest.class.getDeclaredField("num");

    // 传入字段
    long offset = unsafe.objectFieldOffset(field);

    // 设置 , 第三个参数是增量值, 所以将num的值改成了11
    unsafe.getAndAddInt(target, offset, 1);

    System.out.println(target.num);  // 输出11
}
```



## 基本方法

> ​	 基本上就只有 long 和int 类型, 所以CAS 基本只支持这俩类型

- `objectFieldOffset`   获取当前类对象的 当前字段的偏移量

```java
public native long objectFieldOffset(Field var1);
```

参数就是 字段

返回值 就是偏移量

- `compareAndSwapInt` 

```java
public final native boolean compareAndSwapInt(Object var1, long var2, int var4, int var5);
```

参数一 : 实例化对象

参数二 : 偏移量

参数三 : 原来的值 (不等于原来的值,是不会修改成功的)

参数四 : 修改后的值

返回值 : 是否修改成功

- `getAndAddInt`

```java
public final int getAndAddInt(Object var1, long var2, int var4)
```

参数一 : 实例化对象

参数二 : 偏移量

参数三: 增量值

返回值 : 修改前的值

- `getIntVolatile`

```java
public native int getIntVolatile(Object var1, long var2);
```

参数一 : 实例化对象

参数二 : 偏移量

返回值 : 当前值

- `getAndSetInt`

```java
public final int getAndSetInt(Object var1, long var2, int var4)
```

参数一 : 实例化对象

参数二 : 偏移量

参数三 : 修改后的值

返回值 : 修改前的值

## 存在的问题

- 循环时间长开销很大
-  如果 CAS 失败，会一直尝试，如果 CAS 长时间一直不成功，可能会给 CPU 带来很大的开销（比如线程数很多，每次比较都是失败，就会一直循环），所以希望是线程数比较小的场景。
- 只能保证一个共享变量的原子操作 , 对于多个共享变量操作时，循环 CAS 就无法保证操作的原子性。
- 引出 ABA 问题



#### ABA 问题

CAS 判断的条件是`期望值跟当前值`相等时，就交换
期望值 : 加入AB线程都拿到了相同的期望值 , 此时 A线程将他设置成A,然后A又设置成期望值, 此时B线程也会执行成功, 
