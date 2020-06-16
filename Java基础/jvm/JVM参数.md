# JVM -参数

> ​		首先声明一下我的 JVM版本 ： `Java HotSpot(TM) 64-Bit Server VM版本 25.221-b11`   ，不同的版本可能结果不用，有很大的差异 。 我的JDK版本是 1.8 。

## 1. 垃圾回收 - GC 

​		Java的一大特色就是支持自动的垃圾回收(Garbage Collection) 简称GC . 但是有的时候呢 , 如果垃圾回收频繁的出现, 会出现占用太长的CPU时间, 就不得不引我们的重视了 , 就需要一些 跟踪参数进行查看GC 回收的效率和效果 .

​		对于GC的一些算法以及一些配置参数,下一篇文章讲解 , [地址](https://anthony-dong.github.io/post/jvm-gc-hui-shou-suan-fa) : https://anthony-dong.github.io/post/jvm-gc-hui-shou-suan-fa

​		常用的GC 参数有 `-XX:+PrintGC`   或者是 `-XX:+PrintGCDetails`  , 更加详细 `-XX:+PrintHeapAtGC`  , 或者打印 GC回收的时间偏移量   `-XX:+PrintGCTimeStamps`, 每次GC消耗时间 `-XX:+PrintGCApplicationConcurrentTime` ,GC的日志文件位置 :  `-Xloggc:log/gc.log`

​		下面这段代码我们尝试运行一下 . 简单体验一下 ： 

```java
public class TestAllocateStack {

    static class User{
        String name;
    }
    
    private static User user;

    static void allocUser1(){
        user = new User();
        user.name = "name";
    }
    public static void main(String[] args) {
        long b = System.currentTimeMillis();
        // 2000W次实例化
        for(int i = 0;i< 20000000;i++){
            allocUser1();
        }
        long e = System.currentTimeMillis();
        System.out.println("main进程 结束 ："+ (e-b));
        System.out.println("初始化堆内存最大值 : "+Runtime.getRuntime().maxMemory() / 1000 / 1000+"M");
    }
}

运行时请输入一下参数 ：  
-XX:+PrintGCDetails -XX:+PrintGCTimeStamps-XX:+PrintGCApplicationConcurrentTime -XX:+PrintCommandLineFlags
```

​		日志结果 :  

```java
// GC消耗了 0.0826751 seconds , 从0.286开始GC
0.286: Application time: 0.0826751 seconds  
// GC 回收了 81920K - > 952K的大小
0.287: [GC (Allocation Failure) [PSYoungGen: 81920K->952K(95232K)] 81920K->960K(312320K), 0.0012166 secs] [Times: user=0.00 sys=0.00, real=0.00 secs] 
0.311: Application time: 0.0230909 seconds
0.311: [GC (Allocation Failure) [PSYoungGen: 82872K->808K(95232K)] 82880K->816K(312320K), 0.0014887 secs] [Times: user=0.00 sys=0.00, real=0.00 secs] 
0.334: Application time: 0.0211095 seconds
0.334: [GC (Allocation Failure) [PSYoungGen: 82728K->760K(95232K)] 82736K->768K(312320K), 0.0012838 secs] [Times: user=0.00 sys=0.00, real=0.00 secs] 
main进程 结束 ：148
初始化堆内存最大值 : 4725M
Heap
// 新生代情况
 PSYoungGen      total 95232K, used 75780K [0x0000000756600000, 0x0000000762000000, 0x00000007c0000000)
 // Eden 区域 , 默认和survivor区域的比例是8:1 的关系(也不一定,反正挺高的),可以通过其他参数配置比例
  eden space 81920K, 91% used [0x0000000756600000,0x000000075af43340,0x000000075b600000)
 // Survivor  区域     from                    
  from space 13312K, 5% used [0x000000075b600000,0x000000075b6be030,0x000000075c300000)
 // Survivor  区域      to                           
  to   space 13312K, 0% used [0x0000000761300000,0x0000000761300000,0x0000000762000000)
// 老年代情况                              
 ParOldGen       total 217088K, used 8K [0x0000000683200000, 0x0000000690600000, 0x0000000756600000)
  object space 217088K, 0% used [0x0000000683200000,0x0000000683202000,0x0000000690600000)
//  元数据                              
 Metaspace       used 3274K, capacity 4500K, committed 4864K, reserved 1056768K
  class space    used 350K, capacity 388K, committed 512K, reserved 1048576K
0.357: Application time: 0.0222731 seconds
```

​		我们发现 , 系统经历了 3次GC回收 ,  其中每次回收了对应的内存 . 

​		对于`[0x0000000756600000,0x000000075af43340,0x000000075b600000) `这是16进制 , 代表着地址值 , 我们知道堆内存划分位置会对应着一个个地址值 , 这几个数字就代表着地址值 ,  其中 : 

第一个参数 : low_boundary ,  可以称之为 low , 这是commited space的最低地址边界  

第二个参数 : high, 这是 commited space的最高地址边界

第三个参数 : high_boundary, 这是 reserved space的最高地址边界

## 2. 类的加载和卸载

​		JAVA程序离不开类的加载与卸载 , 为了更好的了解程序的执行 , 有的时候知道系统加载了哪些类. 基于动态代理和AOP等技术的普遍使用 , 系统可能运行时生成某些类 ,这些类比较隐蔽 ,无法用文件系统查找到, 因此需要类的追踪 .

​		可以使用 参数 `-XX:+TraceClassLoading ` 跟踪类的加载 和  `-XX:+TraceClassUnloading`跟踪类的卸载 

```java
public class TraceClassLoading {
    public static void main(String[] args) throws ClassNotFoundException {
        Class.forName("com.jvm.trace_class_load.C");
    }
}
interface InterP{}
interface InterC extends InterP{}

class P implements  InterC{}

class C extends P{}
```

打印结果  ..... 

```java
[Loaded java.lang.Object from C:\Program Files\Java\jdk1.8.0_221\jre\lib\rt.jar]
[Loaded java.io.Serializable from C:\Program Files\Java\jdk1.8.0_221\jre\lib\rt.jar]
.....
.......
[Loaded com.jvm.trace_class_load.TraceClassLoading from file:/D:/%e4%bb%a3%e7%a0%81%e5%ba%93/javase%e5%ad%a6%e4%b9%a0/DevelopmentJAVA/java-jvm-optimize/target/classes/]
[Loaded sun.launcher.LauncherHelper$FXHelper from C:\Program Files\Java\jdk1.8.0_221\jre\lib\rt.jar]
[Loaded java.lang.Class$MethodArray from C:\Program Files\Java\jdk1.8.0_221\jre\lib\rt.jar]
[Loaded java.lang.Void from C:\Program Files\Java\jdk1.8.0_221\jre\lib\rt.jar]
[Loaded com.jvm.trace_class_load.InterP from file:/D:/%e4%bb%a3%e7%a0%81%e5%ba%93/javase%e5%ad%a6%e4%b9%a0/DevelopmentJAVA/java-jvm-optimize/target/classes/]
[Loaded com.jvm.trace_class_load.InterC from file:/D:/%e4%bb%a3%e7%a0%81%e5%ba%93/javase%e5%ad%a6%e4%b9%a0/DevelopmentJAVA/java-jvm-optimize/target/classes/]
[Loaded com.jvm.trace_class_load.P from file:/D:/%e4%bb%a3%e7%a0%81%e5%ba%93/javase%e5%ad%a6%e4%b9%a0/DevelopmentJAVA/java-jvm-optimize/target/classes/]
[Loaded com.jvm.trace_class_load.C from file:/D:/%e4%bb%a3%e7%a0%81%e5%ba%93/javase%e5%ad%a6%e4%b9%a0/DevelopmentJAVA/java-jvm-optimize/target/classes/]    
```

我们发现首先加载的类是 Object 类 , 最后会加载 我们实现的类



## 3. 堆内存

### 1. 最大堆 & 初始化堆

> ​	 	当JVM 启动时, 虚拟机就会分配一块初始堆的空间 , 可以使用参数`-Xms10m` 指定这块空间的大小(此时设置为10m) , 一般来说 , 虚拟机会尽可能的维持在初始堆的空间范围内运行 , 但是当初始堆消耗完,会将堆空间进一步的拓展,其拓展上限为 最大堆空间 ,可以通过 `-Xmx20m` 进行设置 (此时设置的最大堆空间为20m) . 

```java
public class HeapAlloc {
    public static void main(String[] args) {
        System.out.println("======================================");

        System.out.println("初始化");
        System.out.println("最大堆空间 : "+Runtime.getRuntime().maxMemory()/1024/1024+"M");
        System.out.println("空闲堆空间 : "+Runtime.getRuntime().freeMemory()/1024/1024+"M");
        System.out.println("初始化堆空间 : "+Runtime.getRuntime().totalMemory()/1024/1024+"M");

        System.out.println("======================================");

        // byte  是 一个字节 , 所以是 10M
        byte[] b = new byte[2 * 1024 * 1024];
        System.out.println("分配2M的内存空间");
        System.out.println("最大堆空间 : "+Runtime.getRuntime().maxMemory()/1024/1024+"M");
        System.out.println("空闲堆空间 : "+Runtime.getRuntime().freeMemory()/1024/1024+"M");
        System.out.println("初始化堆空间 : "+Runtime.getRuntime().totalMemory()/1024/1024+"M");

        System.out.println("======================================");

        // byte  是 一个字节 , 所以是 10M
        byte[] b2 = new byte[5 * 1024 * 1024];
        System.out.println("分配5M的内存空间");
        System.out.println("最大堆空间 : "+Runtime.getRuntime().maxMemory()/1024/1024+"M");
        System.out.println("空闲堆空间 : "+Runtime.getRuntime().freeMemory()/1024/1024+"M");
        System.out.println("初始化堆空间 : "+Runtime.getRuntime().totalMemory()/1024/1024+"M");

        System.out.println("======================================");
    }
}

输入一下指令运行 : 
-Xmx20m -Xms5m -XX:+PrintCommandLineFlags -XX:+PrintGCDetails
```

运行结果 ： 

```java
-XX:InitialHeapSize=5242880 -XX:MaxHeapSize=20971520 -XX:+PrintCommandLineFlags -XX:+PrintGCDetails -XX:+UseCompressedClassPointers -XX:+UseCompressedOops -XX:-UseLargePagesIndividualAllocation -XX:+UseParallelGC 
[GC (Allocation Failure) [PSYoungGen: 1024K->504K(1536K)] 1024K->632K(5632K), 0.0009089 secs] [Times: user=0.00 sys=0.00, real=0.00 secs] 
[GC (Allocation Failure) [PSYoungGen: 1518K->512K(1536K)] 1646K->927K(5632K), 0.0010350 secs] [Times: user=0.00 sys=0.00, real=0.00 secs] 
======================================
初始化
// 最大堆空间由于JVM还要给其他空间分配内存,所以可能会占用一部分
最大堆空间 : 18M
空闲堆空间 : 4M
初始化堆空间 : 5M
======================================
分配2M的内存空间
最大堆空间 : 18M
空闲堆空间 : 2M
初始化堆空间 : 5M
======================================
分配5M的内存空间
最大堆空间 : 18M
空闲堆空间 : 2M
// 由于初始堆空间不够 , 会拓展堆空间大小
初始化堆空间 : 11M
======================================
Heap
 PSYoungGen      total 1536K, used 814K [0x00000000ff980000, 0x00000000ffc80000, 0x0000000100000000)
  eden space 1024K, 29% used [0x00000000ff980000,0x00000000ff9cba50,0x00000000ffa80000)
  from space 512K, 100% used [0x00000000ffb00000,0x00000000ffb80000,0x00000000ffb80000)
  to   space 512K, 0% used [0x00000000ffa80000,0x00000000ffa80000,0x00000000ffb00000)
 ParOldGen       total 9728K, used 7583K [0x00000000fec00000, 0x00000000ff580000, 0x00000000ff980000)
  object space 9728K, 77% used [0x00000000fec00000,0x00000000ff367ef8,0x00000000ff580000)
 Metaspace       used 3258K, capacity 4500K, committed 4864K, reserved 1056768K
  class space    used 350K, capacity 388K, committed 512K, reserved 1048576K

```

#### 注意 ： 

 		在实际工作中可以将 -Xms 和 -Xmx 的值设置为相同 , 这样可以减少程序运行时进行的垃圾回收次数 , 从而提高程序性能 .  所以当我们将俩个参数值设置为相同时 ,上诉代码就不会进行GC回收了 ,大家可以试一试.



### 2. 新生代 & 老年代 

> ​		我们可以通过 `-Xmn20m` 设置新生代的大小 , 设置一个较大的新生代会减少老年带的大小, 这个参数对于系统的性能以及GC行为会有很大的影响 . 所以我们需要一个比例 , 一般新生代的大小设置为整个堆大小的 `[1/3,1/4]` 左右 . 
>
> ​		可以通过参数 `-XX:SurvivorRatio=<N>`  用来设置新生代中 eden空间和survivor空间的比例关系 . 他们的比例关系一般是 :  `-XX:SurvivorRatio=eden/from=eden/to  `   , 当我们了解了 GC 机制,就知道 如何配置这个比例了 . 
>
> ​		可以通过 `-XX:NewRatio=<N>` 设置新生代与老年代的比例 . 下面是一个默认比例。

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-11-03/a10d0600-ebb7-46ed-9030-fa03e5887186.png?x-oss-process=style/template01)

```java
public class SeriallyAllocateHeap {
    public static void main(String[] args) {
        byte[] menory = null;

        System.out.println("============= 初始化内存空间 =========================");
        System.out.println("最大堆空间 : "+Runtime.getRuntime().maxMemory()/1024/1024+"M");
        System.out.println("空闲堆空间 : "+Runtime.getRuntime().freeMemory()/1024/1024+"M");
        System.out.println("初始化堆空间 : "+Runtime.getRuntime().totalMemory()/1024/1024+"M");

        for (int i = 0; i < 5; i++) {
            // 创建一块 1M 大小的 空间
            menory = new byte[2 * 1024 * 1024];
            System.out.println("=============="+"分配第"+(i+1)+"次"+"========================");
            System.out.println("最大堆空间 : "+Runtime.getRuntime().maxMemory()/1024/1024+"M");
            System.out.println("空闲堆空间 : "+Runtime.getRuntime().freeMemory()/1024/1024+"M");
            System.out.println("初始化堆空间 : "+Runtime.getRuntime().totalMemory()/1024/1024+"M");

        }
        System.out.println("============= 结束 =========================");
    }
}

运行时请输入以下参数 : 
-Xmx20m -Xms20m -XX:+PrintGCDetails -XX:+PrintCommandLineFlags -Xmn2m -XX:SurvivorRatio=2
```

运行结果 : 

```java
-XX:InitialHeapSize=20971520 -XX:MaxHeapSize=20971520 -XX:MaxNewSize=2097152 -XX:NewSize=2097152 -XX:+PrintCommandLineFlags -XX:+PrintGCDetails -XX:SurvivorRatio=2 -XX:+UseCompressedClassPointers -XX:+UseCompressedOops -XX:-UseLargePagesIndividualAllocation -XX:+UseParallelGC 
[GC (Allocation Failure) [PSYoungGen: 1024K->504K(1536K)] 1024K->616K(19968K), 0.0008850 secs] [Times: user=0.00 sys=0.00, real=0.00 secs] 
[GC (Allocation Failure) [PSYoungGen: 1528K->504K(1536K)] 1640K->827K(19968K), 0.0008248 secs] [Times: user=0.00 sys=0.00, real=0.00 secs] 
============= 初始化内存空间 =========================
最大堆空间 : 19M
空闲堆空间 : 18M
初始化堆空间 : 19M
==============分配第1次========================
最大堆空间 : 19M
空闲堆空间 : 16M
初始化堆空间 : 19M
==============分配第2次========================
最大堆空间 : 19M
空闲堆空间 : 14M
初始化堆空间 : 19M
==============分配第3次========================
最大堆空间 : 19M
空闲堆空间 : 12M
初始化堆空间 : 19M
==============分配第4次========================
最大堆空间 : 19M
空闲堆空间 : 10M
初始化堆空间 : 19M
==============分配第5次========================
最大堆空间 : 19M
空闲堆空间 : 8M
初始化堆空间 : 19M
============= 结束 =========================
Heap
// 这里一共是 1536K ,其中eden 为1024K , from 为 512K ,那么 1024K/512k = 2 ,1024K+512k=1536K
 PSYoungGen      total 1536K, used 785K [0x00000000ffe00000, 0x0000000100000000, 0x0000000100000000)
  eden space 1024K, 27% used [0x00000000ffe00000,0x00000000ffe466d0,0x00000000fff00000)
  from space 512K, 98% used [0x00000000fff80000,0x00000000ffffe010,0x0000000100000000)
  to   space 512K, 0% used [0x00000000fff00000,0x00000000fff00000,0x00000000fff80000)
 ParOldGen       total 18432K, used 10563K [0x00000000fec00000, 0x00000000ffe00000, 0x00000000ffe00000)
  object space 18432K, 57% used [0x00000000fec00000,0x00000000ff650f38,0x00000000ffe00000)
 Metaspace       used 3244K, capacity 4500K, committed 4864K, reserved 1056768K
  class space    used 350K, capacity 388K, committed 512K, reserved 1048576K
```



### 3. Eden & From & To 是什么 

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-11-02/8226297e-1fe0-4580-811f-bbbc95cc8ed9.jpg)

​				       	**这是内存划分图(JDK 1.8 将 permanent 区域移到了堆外内存 ,改名为 Metaspace )**



​	 	 1、为什么要分代 ？ 

​	  	其实不分代完全可以，分代的唯一理由就是优化GC性能 . 我们可以想想，如果没有分代，那我们所有的对象都在一块，GC的时候我们要找到哪些对象没用，这样就会对堆的所有区域进行扫描 , 此时会消耗很长的时间去查找 。而我们的很多对象都是会死亡的 ,而且伴随着阶段性，如果分代的话，我们把新创建的对象放到某一地方，当GC的时候先把这块存“会死亡的”对象的区域进行回收，这样就会腾出很大的空间出来。

​		2、其次就是  `Eden`  , `From` , `To`   这三个到底是什么 ?  

​		`Eden`在英文中是 伊甸园的意思,代表着新生 , 而  `From` , `To`  空间 其实就是 `Survivor` 区域, 在用`Jconsole工具` 时 ,并没有说明这个  `From` , `To`   空间 .所以我们不能依靠那个工具去看 .  一般情况下，新创建的对象都会被分配到Eden区(一些大对象特殊处理) , 这些对象经过第一次Minor GC后，如果仍然存活，将会被移到Survivor区。对象在Survivor区中每熬过一次Minor GC，年龄就会增加1岁，当它的年龄增加到一定程度时，就会被移动到年老代中。

​		3、关于详细过程 ，下一篇文章讲到 ， [地址](https://anthony-dong.github.io/post/jvm-gc-hui-shou-suan-fa) ： https://anthony-dong.github.io/post/jvm-gc-hui-shou-suan-fa

## 4. 非堆内存

### 1. 方法区(元数据区)

​		方法区主要是存放类的元信息 , 在JDK 1.6 , 1.7 的版本中 , 可以通过使用 `-XX:PermSize` 和 `-XX:MaxPermSize` 配置永久区的大小 , 一个是初始的,一个是最大的 

​		在JDK1.8中 , 永久区被彻底删除, 使用了新的元数据区域存放类的元数据 . 默认情况下 ,元数据只收系统可用内存的限制 , 但是依旧可以使用参数 `-XX:MaxMetaspaceSize` 指定元数据区域的最大可用值 . 

### 2. 栈 

​		栈是每个线程私有的内存空间 , 在java 虚拟机中可以使用`-Xss`参数 指定线程的栈大小 . 

### 3. 直接内存

​    	直接内存主要是在NIO中应用广泛 , 使用非常的普遍 ,直接内存可以跳过Java 堆 , 使Java 程序可以直接访问原生堆空间 , 因此一定程度上加快了内存空间的访问速度 . 

​		最大直接内存可以用 `-XX:MaxDirectMemorySize ` 设置 

#### 优势 :

​		读写能力,直接内存如果被 server 优化后 , 比堆内存提升了一个量级 , 下面是一个测试代码 : 

```java
public class TestDirectMemory {

    public void directAccess(){
        long start = System.currentTimeMillis();

        ByteBuffer buffer = ByteBuffer.allocateDirect(4000);

        for (int i = 0; i < 10000; i++) {
            for (int j = 0; j < 1000; j++) {
                buffer.put((byte) j);
            }
            buffer.flip();
            while (buffer.hasRemaining()) {
                buffer.get();
            }
            buffer.clear();
        }

        System.out.println("directAccess : "+(System.currentTimeMillis() - start));
    }


    public void bufferAccess(){
        long start = System.currentTimeMillis();
        ByteBuffer buffer = ByteBuffer.allocate(4000);
        for (int i = 0; i < 100000; i++) {
            for (int j = 0; j < 1000; j++) {
                buffer.put((byte) j);
            }
            buffer.flip();
            while (buffer.hasRemaining()) {
                buffer.get();
            }
            buffer.clear();
        }

        System.out.println("bufferAccess : "+(System.currentTimeMillis() - start));
    }

    public static void main(String[] args) {
        TestDirectMemory directMemory = new TestDirectMemory();
        directMemory.bufferAccess();
        directMemory.directAccess();

        System.out.println("=====================");

        directMemory.bufferAccess();
        directMemory.directAccess();
    }
}

启动 -server 模式
```

执行结果 : 

```java
bufferAccess : 164
directAccess : 46
=====================
bufferAccess : 350
directAccess : 10
```

#### 劣势 :

对于内存空间的申请上 , 直接内存毫无优势可言 ,所以还是要分情况的

```java
public class TestAllocDirectBuffer {

    public void allocDirect() {
        long start = System.currentTimeMillis();
        for (int i = 0; i < 300000; i++) {
            ByteBuffer.allocateDirect(1024);
        }
        System.out.println("allocDirect : "+(System.currentTimeMillis() - start));
    }


    public void allocBuffer() {
        long start = System.currentTimeMillis();
        for (int i = 0; i < 300000; i++) {
            ByteBuffer.allocate(1024);
        }
        System.out.println("allocBuffer : "+(System.currentTimeMillis() - start));
    }

    public static void main(String[] args) {
        TestAllocDirectBuffer allocDirectBuffer = new TestAllocDirectBuffer();

        allocDirectBuffer.allocBuffer();
        allocDirectBuffer.allocDirect();

        System.out.println("===============================");

        allocDirectBuffer.allocBuffer();
        allocDirectBuffer.allocDirect();
    }
}
启动 -server 模式
```

结果 : 

```java
allocBuffer : 114
allocDirect : 313
===============================
allocBuffer : 196
allocDirect : 290
```



#### 结论 : 

通过上面我们发现 直接内存适合于申请次数比较少, 访问比较频繁的场合 .



## 5. Server & Client 模式

​		目前java的虚拟机都是支持 Client  和 Server 两种运行模式的, 使用 `java -version` 就可以看出当前虚拟机模式

```java
C:\WINDOWS\system32>java -version
java version "1.8.0_221"
Java(TM) SE Runtime Environment (build 1.8.0_221-b11)
Java HotSpot(TM) 64-Bit Server VM (build 25.221-b11, mixed mode)
```

​		与 client 模式相比 ,server 模式启动比较慢 , 因为server 模式启动会收集更多的系统性能信息 , 使用更复杂的算法对程序进行优化 , 因此当程序进入稳定期以后, server模式的执行速度会远远快于client模式 . 一般64位系统的默认是server 模式 

​		关于这两种模式的区别,大家可以网上收集一下 ,下面我只提供 书上讲的 . 

​		client模式中 CompileThreshold (编译 阈值 ) 默认是 1500 ,当程序即函数被调用 1500次 后 ,会进行JIT编译 , 而server 模式下 这个阈值是 10000 . 所以server模式更有可能解释执行,但是一旦编译,server模式优化效果会远远好于client模式 . 

