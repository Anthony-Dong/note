# Unsafe

> ​	Java比较核心的类



如何获取

```java
private static Unsafe getUnsafe() throws NoSuchFieldException, IllegalAccessException {

    // 获取类申明的字段 , Declared ,可以获取到private修饰的字段
    Field unsafeField = Unsafe.class.getDeclaredField("theUnsafe");

    // private修饰的字段需要设置 可见性为 true 
    unsafeField.setAccessible(true);

    // 直接返回就行了
    return (Unsafe) unsafeField.get(null);
}
```



## 1. park , unpark

### park 方法 : 

```java
/**
 * 
 * @param var1  如果为 true , 那么 var2 需要传入等待到什么时间 , 以 System.currentTimeMillis()为点的话, 等待1000ms , 就是 System.currentTimeMillis()+1000
 *              写法就是 unsafe.park(true, System.currentTimeMillis() + 1000);
 *              如果为 false , 此时第二个参数传入等待多少纳秒 , 1s=10^9ns  ,此时 unsafe.park(false, 1000000000L); 就是睡眠1S
 *              
 * @param var2  (参数1 为 false 时) 相对时间 , 单位为ns,此时如果为0表示无限等待, 
 *              (参数1 为 true 时) 绝对时间 ,单位为ms,
 *           
 */
public native void park(boolean var1, long var2);
```

### unpark 方法 : 

```java
/**
 * 
 * @param var1 你要打断正在等待中的线程
 */
public native void unpark(Object var1);
```

### 简单demo

```java
public static void main(String[] args) throws Exception {

    Unsafe unsafe = getUnsafe();

    Thread thread = new Thread(() -> {
        System.out.println(Thread.currentThread().getName() + " : 启动成功.");
        long start = System.currentTimeMillis();
        /**
         * 第二个参数是 无脑睡眠
         */
        unsafe.park(false, 0);

        System.out.println(Thread.currentThread().getName() + " : 睡眠结束." + (System.currentTimeMillis() - start));
    });

    // 启动线程
    thread.start();
    System.out.println("main : 启动线程");
    
    TimeUnit.MILLISECONDS.sleep(1000);
    
    System.out.println("main : 解除锁");
    unsafe.unpark(thread);
}
```

输出 : 

```java
main : 启动线程
Thread-0 : 启动成功.
main : 解除锁
Thread-0 : 睡眠结束.1000
```



### LockSupport

> ​	他提供了基本的使用方法 , 类似于Unsafe的直接操作 , 他可以间接操作 . 

