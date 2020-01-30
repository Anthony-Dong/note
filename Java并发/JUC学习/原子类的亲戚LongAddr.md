# 原子类的亲戚LongAdder

> ​	这个是JDK1.8出的. 

> ​	One or more variables that together maintain an initially zero long sum. When updates (method add) are contended across threads, the set of variables may grow dynamically to reduce contention. 

一个或多个变量共同维护一个初始化为0的sum集, 当线程竞争更新时 , 变量集可以动态的增加以减少竞争 , 

> ​	Method sum (or, equivalently, longValue) returns the current total combined across the variables maintaining the sum.

sum方法呢返回当前当前集合的和 ,就是一系列变量的和. (其实就是cell数组+base)

> ​	This class is usually preferable to AtomicLong when multiple threads update a common sum that is used for purposes such as collecting statistics, not for fine-grained synchronization control. 

这个类性能一般会比AtomicLong 好 , 当多个线程更新一个共同的sum,



首先`java.util.concurrent.atomic.LongAdder` 继承自 `java.util.concurrent.atomic.Striped64 `  , 所以基本上就是基于这个类写的 



简单使用

```java
public class TestLangAdder {


    public static void main(String[] args) throws InterruptedException {
        LongAdder adder = new LongAdder();

        ExecutorService service = Executors.newFixedThreadPool(50);


        long start = System.currentTimeMillis();
        IntStream.range(0, 50).forEach(e -> service.execute(() -> {
            IntStream.range(0, 100000).forEach(z -> {
                adder.add(1L);
            });
        }));

        service.shutdown();

        service.awaitTermination(Integer.MAX_VALUE, TimeUnit.DAYS);

        System.out.printf("消耗时间 : %d , sum = %d\n", (System.currentTimeMillis() - start), adder.sum());
    }
}
```

输出 : 

```java
消耗时间 : 109 , sum = 5000000
```

,对比一下 atomic , 其实这个longadder性能比较好 . 





## 1. LongAdder.add(long x)

```java
public void add(long x) {
    Cell[] as; long b, v; int m; Cell a;
    // cells!=null , cas交换base是否成功  , 如果第一个false , 第二个为true , 其实就是atomic.
    if ((as = cells) != null || !casBase(b = base, b + x)) {
        // 当cas交换失败时 , 就需要标记为竞争失败
        boolean uncontended = true;
        // cells如果为空,反正一堆.判断
        if (as == null || (m = as.length - 1) < 0 ||
            (a = as[getProbe() & m]) == null ||
            !(uncontended = a.cas(v = a.value, v + x)))
            // 创建一个新的cell,然后操作他.累加
            longAccumulate(x, null, uncontended);
    }
}
```

其实就是多个线程都去执行 add操作, 第一步, 只有一个线程成功,那么他就操作base  ,如果赋值失败呢 , 他会继续操作一次base, 还是失败就会创建一个cell,进去累加. 



多以可以有效的避免cas自旋 , 因为自旋对于CPU消费很高 , 多线程竞争不友好. 





## 2.  LongAdder.sum()

```java
public long sum() {
    Cell[] as = cells; Cell a;
    // base + cells集合中全部的数字和 . 很简单.
    long sum = base;
    if (as != null) {
        for (int i = 0; i < as.length; ++i) {
            if ((a = as[i]) != null)
                sum += a.value;
        }
    }
    return sum;
}
```

这个就是累加 base和cell . 