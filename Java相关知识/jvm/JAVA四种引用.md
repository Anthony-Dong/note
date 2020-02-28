# Java - 四种引用对象和 finalize方法

> ​	在JDK 1.2版之后，Java对引用的概念进行了扩充，将引用分为强引用（Strongly Re-ference）、软引用（Soft Reference）、弱引用（Weak Reference）和虚引用（PhantomReference）4种，这4种引用强度依次逐渐减弱。
>
> ​	还有一个finalize方法 . 



> ​	参数 `-Xmx10m -Xms10m` 可以调整堆内存最大内存和初始内存. 以下都是分配10M的空间进行计算的.

### 强引用

> ​	强引用就是Java的new 关键字, 所有new关键字都是强引用

```java
public class TestGC {

    public static void main(String[] args) {
        printFreeMemory("初始化");
        byte[] R1 = new byte[1024 * 1024 * 6];
        printFreeMemory("消耗6M内存");
        byte[] R2 = new byte[1024 * 1024 * 6];
        printFreeMemory("消耗6M内存");
        System.out.printf("R1 :%s , R2:%s\n", R1, R2);
    }

    // 打印剩余内存
    private static void printFreeMemory(String str) {
        System.out.println(String.format("%s : 剩余%dm.", str, Runtime.getRuntime().freeMemory() / (1<<20)));
    }
}
```

输出 : 

```java
初始化 : 剩余7m.
消耗6M内存 : 剩余1m.
Exception in thread "main" java.lang.OutOfMemoryError: Java heap space
	at com.jvm.reference.TestGC.main(TestGC.java:19)
```

我们发现内存无法回收,抛出OOM异常. 如果我们再次修改. 

```java
public static void main(String[] args) {
        printFreeMemory("初始化");
        byte[] R1 = new byte[1024 * 1024 * 6];
        printFreeMemory("消耗6M内存");
        // 删除引用关系, 就会被GC回收掉
        R1 = null;
        byte[] R2 = new byte[1024 * 1024 * 6];
        printFreeMemory("消耗6M内存");
        System.out.printf("R1 :%s , R2:%s\n", R1, R2);
}
```

输出 : 

```java
初始化 : 剩余8m.
消耗6M内存 : 剩余1m.
消耗6M内存 : 剩余2m.
R1 :null , R2:[B@63961c42
```

**发现结果是没有异常, 成功回收了内存 , 说明对于Java来说, new 属于强引用, 不管在JVM运行到何时 , 只要存在引用关系就不会被回收,  除非手动解除引用关系 , 或者脱离引用关系 , 比如跳出栈帧(也就执行完一个方法发现没有对象引用),如果这个对象无用, 这个对象就会被回收了.**  

```jav
public static void main(String[] args) {
	// 调用test方法.
    test();
    // 删除引用关系, 就会被GC回收掉
    byte[] R2 = new byte[1024 * 1024 * 6];
    printFreeMemory("消耗6M内存");
    System.out.printf("R2:%s\n", R2);
}

private static void test() {
    printFreeMemory("初始化");
    byte[] R1 = new byte[1024 * 1024 * 6];
    printFreeMemory("消耗6M内存");
}
```

输出 : 

```jaa
初始化 : 剩余8m.
消耗6M内存 : 剩余1m.
消耗6M内存 : 剩余2m.
R2:[B@63961c42
```

> ​	发现跳出栈帧也是会接触引用关系的. 

### 软引用

> ​	软引用是用来描述一些还有用，但非必须的对象。只被软引用关联着的对象，在系统将要发生内存溢出异常前，会把这些对象列进回收范围之中进行回收，如果这次回收还没有足够的内存，才会抛出内存溢出异常。在JDK 1.2版之后提供了`SoftReference`类来实现软引用。

```java
public static void main(String[] args) {
    printFreeMemory("初始化");
    SoftReference<byte[]> R1 = new SoftReference<>(new byte[1024 * 1024 * 6]);
    printFreeMemory("消耗6M内存");
    byte[] R2 = new byte[1024 * 1024 * 6];
    printFreeMemory("消耗6M内存");
    System.out.printf("R1 :%s,R2:%s\n", R1.get(), R2);
}
```

我们输出 :  我们发现R1这对象被回收掉了 , 是因为内存不足. 

```java
初始化 : 剩余8m.
消耗6M内存 : 剩余1m.
消耗6M内存 : 剩余2m.
R1 :null,R2:[B@63961c42
```

我们调整一下JVM参数 `-Xmx20m -Xms10m`  , 初始化的时候是10M , 最大值是20M . 再次运行

输出 :  发现并没有被回收掉.  因为并没有达到最大内存, 

```java
初始化 : 剩余7m.
消耗6M内存 : 剩余1m.
消耗6M内存 : 剩余1m.
R1 :[B@63961c42,R2:[B@65b54208
```

### 弱引用

> ​	弱引用也是用来描述那些非必须对象，但是它的强度比软引用更弱一些，**被弱引用关联的对象只能生存到下一次垃圾收集发生为止。当垃圾收集器开始工作，无论当前内存是否足够，都会回收掉只被弱引用关联的对象。**在JDK 1.2版之后提供了WeakReference类来实现弱引用。

我们调整一下JVM参数 `-Xmx10m -Xms10m  -XX:+PrintGCDetails`  , 初始化的时候是10M , 最大值是20M . 

```java
public static void main(String[] args) {
    printFreeMemory("初始化");
    // 弱引用
    WeakReference<byte[]> R1 = new WeakReference<>(new byte[1024 * 1024 * 6]);
    printFreeMemory("消耗6M内存");
    byte[] R2 = new byte[1024 * 1024 * 6];
    printFreeMemory("消耗6M内存");
    System.out.printf("R1 :%s,R2:%s\n", R1.get(), R2);
}
```

输出 : 

```java
[GC (Allocation Failure) [PSYoungGen: 2048K->504K(2560K)] 2048K->788K(9728K), 0.0010764 secs] [Times: user=0.00 sys=0.00, real=0.00 secs] 
初始化 : 剩余7m.
消耗6M内存 : 剩余1m.
// 这里执行了一次GC , 由于数据对象很大会被直接分配到老年代.所以`ParOldGen: 6712K->905K(7168K)` 减少了大致6M空间 , 代表此时回收了我们实例化的R1对象. 
[GC (Allocation Failure) [PSYoungGen: 1978K->488K(2560K)] 8407K->7200K(9728K), 0.0011733 secs] [Times: user=0.00 sys=0.00, real=0.00 secs] 
[Full GC (Ergonomics) [PSYoungGen: 488K->0K(2560K)] [ParOldGen: 6712K->905K(7168K)] 7200K->905K(9728K), [Metaspace: 3834K->3834K(1056768K)], 0.0056012 secs] [Times: user=0.00 sys=0.00, real=0.00 secs] 
消耗6M内存 : 剩余2m.
R1 :null,R2:[B@63961c42
```



> ​	关于JVM堆内存回收和分配 可以自行查阅资料, JVM的核心并发一个GC , 其实还有很多细节知识 . 希望你做个有心人.



## 虚引用

> ​	虚引用也称为“幽灵引用”或者“幻影引用”，它是最弱的一种引用关系。一个对象是否有虚引用的存在，完全不会对其生存时间构成影响，也无法通过虚引用来取得一个对象实例。为一个对象设置虚引用关联的唯一目的只是为了能在这个对象被收集器回收时收到一个系统通知。在JDK 1.2版之后提供了PhantomReference类来实现虚引用。 同时必须用`引用队列关联` , 如果不用传入null就行了. 

这个`PhantomReference` 的 get方法返回null. 代表你的队列拿绝对拿不出来, 因为他返回就是null . 我不懂为啥这么设计 . 一直不懂虚引用可以做啥.  尽管他们说了 `为一个对象设置虚引用关联的唯一目的只是为了能在这个对象被收集器回收时收到一个系统通知` , 但是这句话如何实现. 



虚引用有一个很重要的用途就是用来做堆外内存的释放，DirectByteBuffer就是通过虚引用来实现堆外内存的释放的。可以看看他那个 Cleaner实现. 我也不懂做啥了. 找不到入口. 

这里我做了简单的Demo , 让大家看看. 如何知道一个引用对象是否被回收

```java
// 这个堆
private static final List<byte[]> heap = new LinkedList<>();
private static final ReferenceQueue<Object> QUEUE = new ReferenceQueue<>();

public static void main(String[] args) {
    PhantomReference<Object> phantomReference = new PhantomReference<>(new Object(), QUEUE);
    System.out.println("创建虚引用对象 : " + phantomReference);

    // 该线程不断读取这个虚引用，并不断往列表里插入数据，以促使系统早点进行GC. 这里也一定会发生OOM
    Thread thread = new Thread(() -> {
        while (true) {
            heap.add(new byte[1024 * 512]);
            // 随眠500ms
            sleep(500);
        }
    }, "A");
    thread.start();

    // 这个线程不断读取引用队列，当弱引用指向的对象呗回收时，该引用就会被加入到引用队列中,这里只是展示实际开发千万别这么做. 浪费CPU性能,严重空转
    new Thread(() -> {
        // 当A线程存活的时候不断读取
        while (thread.isAlive()) {
            Reference<?> poll = QUEUE.poll();
            if (poll != null) {
                System.out.println("虚引用对象被jvm回收了:" + poll);
            }
        }
    }).start();
}

```

输出 :  我给大家分析一下日志信息 . 因为我加入了参数 `-Xms4m -Xmx4m -Xmn2m -XX:+PrintGCDetails` 这里-Xmn是新生代大小, 目的就是看到新生代的GC .等到fullGC 就看不到效果了.  我给大家解读一下日志

```java
// GC , 分配失败. 新生代GC, 1024-504k , 减少了500K左右, 是去了下一代了. 然后堆的大小从1024->640,堆大小为3584k,
[GC (Allocation Failure) [PSYoungGen: 1024K->504K(1536K)] 1024K->640K(3584K), 0.0015887 secs] [Times: user=0.00 sys=0.00, real=0.00 secs] 
[GC (Allocation Failure) [PSYoungGen: 1524K->504K(1536K)] 1660K->815K(3584K), 0.0011580 secs] [Times: user=0.00 sys=0.00, real=0.00 secs] 
// 这里
创建虚引用对象 : java.lang.ref.PhantomReference@a09ee92
[GC (Allocation Failure) [PSYoungGen: 1515K->488K(1536K)] 1826K->975K(3584K), 0.0024427 secs] [Times: user=0.00 sys=0.00, real=0.00 secs] 
[GC (Allocation Failure) [PSYoungGen: 1512K->504K(1536K)] 1999K->1087K(3584K), 0.0021418 secs] [Times: user=0.01 sys=0.00, real=0.00 secs] 
// 这里和上面那个是同样的对象 ,但是get不到的.只能达到通知效应.
虚引用对象被jvm回收了:java.lang.ref.PhantomReference@a09ee92
[GC (Allocation Failure) [PSYoungGen: 1525K->504K(1536K)] 3132K->2375K(3584K), 0.0010024 secs] [Times: user=0.00 sys=0.00, real=0.00 secs] 
// 完全GC , 是新生代 , 存活带, 老年带都GC. 
[Full GC (Ergonomics) [PSYoungGen: 504K->497K(1536K)] [ParOldGen: 1856K->1674K(2048K)] 2360K->2172K(3584K), [Metaspace: 4377K->4377K(1056768K)], 0.0072135 secs] [Times: user=0.00 sys=0.00, real=0.01 secs] 
/// .....没内存可分了就会发生异常. 
[Full GC (Allocation Failure) [PSYoungGen: 967K->967K(1536K)] [ParOldGen: 1792K->1758K(2048K)] 2760K->2726K(3584K), [Metaspace: 4651K->4651K(1056768K)], 0.0081205 secs] [Times: user=0.02 sys=0.00, real=0.01 secs] 
// 异常
Exception in thread "Thread - 1" java.lang.OutOfMemoryError: Java heap space

// 余下一个OOM信息
```

关于日志分析可疑看看[这个链接](https://blog.csdn.net/yxc135/article/details/12137663/) : https://blog.csdn.net/yxc135/article/details/12137663/





其实我也不怎么理解 . 可以看看这篇文章 : https://zhuanlan.zhihu.com/p/29454205



## `FinalReference` 和 `Finalizer`   和 `finalize` 三者关系



希望看看这篇文章.  其实都是连载的 : https://zhuanlan.zhihu.com/p/29522201   和  https://www.infoq.cn/article/jvm-source-code-analysis-finalreference



反正不推荐是使用这个 , 因为他的执行时间不确定. 我们拿一个例子来讲吧. 

```java
public class TestGC {
    // 6M大小
    byte[] R1 = new byte[1024 * 1024 * 6];

    @Override
    protected void finalize() throws Throwable {
        System.out.println("回收掉了 ");
    }

    public static void main(String[] args) throws InterruptedException {
        test4();
        // 申请6M
        byte[] R2 = new byte[1024 * 1024 * 6];
	   System.out.println("申请内存成功");
    }

    private static void test4() {
        TestGC testGC = new TestGC();
    }
}
```

堆内存大小是10M, 我们实现了 `finalize` 方法 , 同时调用test4()的时候, 由于已经跳出方法栈了, 此时 R2 申请了一块6M的空间. 按理来说可以释放空间的.  但是我们执行 , 发现抛出异常了 . OOM , 说明确实释放掉了. 但是释放的时候, 正在实例化R2对象. 

```java
回收掉了 Exception in thread "main" 
java.lang.OutOfMemoryError: Java heap space
	at com.jvm.reference.TestGC.main(TestGC.java:25)
```

如果我们去掉这个方法呢.  结果可以正确执行. 

```java
申请内存成功 
```

通过以上例子. 我觉得很真实的反映了这个方法带来的弊端 , 他的执行时间不确定性, 会给程序带来很多问题. 所以不建议使用. 

