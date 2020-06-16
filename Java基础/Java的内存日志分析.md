# JVM内存的日志分析

启动日志分析. 就是下面这个命令.  一般选择第二个就行了.  或者将

> 1. -XX:+PrintGC 输出GC日志
> 2. -XX:+PrintGCDetails 输出GC的详细日志
> 3. -XX:+PrintGCTimeStamps 输出GC的时间戳（以基准时间的形式）
> 4. -XX:+PrintGCDateStamps 输出GC的时间戳（以日期的形式，如 2013-05-04T21:53:59.234+0800）
> 5. -XX:+PrintHeapAtGC 在进行GC的前后打印出堆的信息
> 6. -Xloggc:../logs/gc.log 日志文件的输出路径



## 通用的GC算法

测试用例 . 默认是JDK1.8 ,我用的是, 所以默认的GC回收器是 parallel sevenage 

```java
public static void main(String[] args) {
    for (int x = 0; x < 100; x++) {
        // 为啥R1会销毁,因为for循环,出了域,可达性分析后,可以GC. 如果我们不是for循环,写了实例化,直接OOM
        byte[] R1 = new byte[1024 * 1024 * 5];
        sleep();
    }
}
```

VM参数  `-Xms10m -Xmx10m -XX:+PrintGCDetails`  

我就那一条记录来说 . 

```java
// 普通的GC,就是新生代没有内存空间了,就是发生GC.
//`[PSYoungGen: 40K->0K(2560K)]` : PS(GC算法) , 40K-0K(2560) .指的是 新生代原先40K ,然后到了0K, 一共2560K. 所以2560K绝对不够 1024*5K哇. 所以发生了GC . 
// `6333K->6292K(9728K), 0.0007983 secs` 意思就是堆内存整个空间从 6333K变成了6292K , 一共是9728K .我们知道JDK1.8后, 将常量池也放进来堆内存了.
[GC (Allocation Failure) [PSYoungGen: 40K->0K(2560K)] 6333K->6292K(9728K), 0.0007983 secs] [Times: user=0.00 sys=0.00, real=0.00 secs] 

// FULL GC.是因为普通GC后, 还是没有内存了, 就会发生完全GC. 清理
// `[PSYoungGen: 0K->0K(2560K)]`  新生代  0K(GC前)->0K(GC后) , 新生代一共2560K 
// `[ParOldGen: 6292K->1091K(7168K)]`  老年代 6292K(GC前)->1091K(GC后) , 老年代一共7168K.
// `6292K->1091K(9728K)` 指的是对内存整个空间的变化.6292K->1091K ,一共9728K (9728=7168+2560).
// `[Metaspace: 3797K->3797K(1056768K)]` 指的是元数据区.
[Full GC (Ergonomics) [PSYoungGen: 0K->0K(2560K)] [ParOldGen: 6292K->1091K(7168K)] 6292K->1091K(9728K), [Metaspace: 3797K->3797K(1056768K)], 0.0164515 secs] [Times: user=0.06 sys=0.00, real=0.02 secs] 
```



## CMS 的使用

使用方式 :  `-XX:+UseConcMarkSweepGC `

