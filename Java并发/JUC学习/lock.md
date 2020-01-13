# Java - 锁与同步操作

## 1. 认识 synchronized关键字 与 Lock锁

### 两者的区别是什么 ?

- `synchronized`为 **java中的内置锁**：,特点是使用简单，一切交给JVM去处理,不需要显示释放(显式与隐式: 显式就是需要自己手动去调用 , 隐式就是不需要自己去执行,jvm内部给你执行) . 分为方法同步和代码块同步, 如果是同步代码块的话在字节码中会有两个操作指令 ,  `monitorenter` 和 `monitorexit` ,   同时对于方法同步来说. static方法话,同步对象是Class对象, 对于非静态方法来说,同步对象是this对象.

- JDK 1.5 中加入了 `Lock`   , 其实需要 **手动加锁和释放锁** , 分为 两大类 主要是 `Lock`  和 `ReadWriteLock`

- 本节我们主要讲的是 `ReentrantReadWriteLock`  和 `ReentrantLock`

- 相比于`synchronized`，`ReentrantLock`在功能上更加丰富，它具有**可重入、可中断、可限时、公平锁**等特点。

- 在jdk1.5里面，`ReentrantLock`的性能是明显优于`synchronized`的，但是在jdk1.6里面，`synchronized`做了优化，他们之间的性能差别已经不明显了。可能在多核上表现的略显得较为明显,

  ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-22/2cc3bcf5-f8b0-4c65-ba63-2fb80b8b315e.jpg?x-oss-process=style/template01)





## 2. ReentrantLock 可重入锁

锁的用处 : 当一个资源被多个线程共用时,如果执行修改操作, 那么就存在线程安全的问题. 有可能ab线程同时读取到的都是1,那么修改只能+1,那么两次计算结果就会出现少计算一次. 有兴趣的可以看看线程读取变量到写入变量的各种操作, 其实分为很多步.

```java
public class Reentrant {

    public static void main(String[] args) throws InterruptedException {
        ExecutorService pool = Executors.newFixedThreadPool(10);
        IntStream.range(0,10).forEach(e->{
            pool.execute(()->{
                for (int x = 0; x < 10000; x++) {
                    increase();
                }
            });
        });
        // 等到全部线程执行完毕输出
        pool.shutdown();
        pool.awaitTermination(Integer.MAX_VALUE, TimeUnit.DAYS);
        System.out.println("输出结果 : "+x);
    }

    private static int x = 0;

    private static void increase() {
        x += 1;
    }
}
```

每次执行结果,可能有些偏差,但是绝对会小于 100000的

```java
输出结果 : 67398
```

那如何处理呢, 这时候就需要加一把锁了 .  可以使用sync修饰 `increase` 方法

```java
private synchronized static void increase() {
    x += 1;
}
```

此时输出结果为 

```java
输出结果 : 100000
```

也可以使用我们说到的Lock , 我们这里使用的是 ReentrantLock , 是一个可重入锁. 

```java
private static final Lock LOCK = new ReentrantLock();

private  static void increase() {
    try {
        LOCK.lock();
        x += 1;
    } finally {
        LOCK.unlock();
    }
}
```

这样子输出结果也是对的. `LOCK.lock();` 会使得只有一个线程通过去, 其他的都在等待他前面的线程执行unlock操作. 

### LOCK.lock()

做这个就是一个加锁, 一个解锁的操作, 很简单的 . 所有加锁的对象在没有获取通行证的情况下都会被阻塞.

### LOCK.tryLock()

> ​	意思就是尝试获取一把锁, 获取失败就返回false. 成功返回true , 同时他还可以加获取锁的最长等待时间, 并不是直接返回

```java
public class ReentrantTry {

    public static void main(String[] args) throws InterruptedException {
        ExecutorService pool = Executors.newFixedThreadPool(10);
        IntStream.range(0, 10).forEach(e -> {
            pool.execute(ReentrantTry::increase);
        });
        // 等待所有线程执行完毕.关闭线程池
        pool.shutdown();
    }

    private static final Lock LOCK = new ReentrantLock();
    private static void increase() {
        // 1. 尝试获取
        if (LOCK.tryLock()) {
            try {
                // 2. 执行逻辑
                System.out.println(Thread.currentThread().getName() + "获取锁成功");
            } finally {
                // 3. 记住拿到锁,一定要释放锁
                LOCK.unlock();
            }
        } else {
            System.out.println(Thread.currentThread().getName() + "获取锁失败");
        }
    }
}
```

输出  : 部分失败

```java
pool-1-thread-1 : 获取锁成功
pool-1-thread-2 : 获取锁成功
pool-1-thread-3 : 获取锁失败
pool-1-thread-4 : 获取锁成功
pool-1-thread-5 : 获取锁成功
pool-1-thread-6 : 获取锁失败
pool-1-thread-7 : 获取锁成功
pool-1-thread-8 : 获取锁失败
pool-1-thread-9 : 获取锁成功
pool-1-thread-10 : 获取锁失败
```

如果我们设置一个超时时间 

```java
public class ReentrantTry {

    public static void main(String[] args) throws InterruptedException {
        ExecutorService pool = Executors.newFixedThreadPool(10);
        IntStream.range(0, 10).forEach(e -> {
            pool.execute(()->{
                try {
                    increase();
                } catch (InterruptedException e1) {
                    e1.printStackTrace();
                }
            });
        });
        // 等到全部线程执行完毕输出
        pool.shutdown();
//        pool.awaitTermination(Integer.MAX_VALUE, TimeUnit.DAYS);
    }

    private static final Lock LOCK = new ReentrantLock();
    private static void increase() throws InterruptedException {
        // 500ms获取锁
        if (LOCK.tryLock(500,TimeUnit.MILLISECONDS)) {
            try {
                System.out.println(Thread.currentThread().getName() + " : 获取锁成功");
            } finally {
                LOCK.unlock();
            }
        } else {
            System.out.println(Thread.currentThread().getName() + " : 获取锁失败");
        }
    }
}
```

输出 : 全部成功

```java
pool-1-thread-2 : 获取锁成功
pool-1-thread-3 : 获取锁成功
pool-1-thread-1 : 获取锁成功
pool-1-thread-4 : 获取锁成功
pool-1-thread-5 : 获取锁成功
pool-1-thread-6 : 获取锁成功
pool-1-thread-7 : 获取锁成功
pool-1-thread-8 : 获取锁成功
pool-1-thread-9 : 获取锁成功
pool-1-thread-10 : 获取锁成功
```

###  lockInterruptibly

```java
public class ReentrantTry {

    public static void main(String[] args) throws InterruptedException {
        ExecutorService pool = Executors.newFixedThreadPool(10);
        IntStream.range(0, 10).forEach(e -> {
            pool.execute(ReentrantTry::inter);
        });
        TimeUnit.MILLISECONDS.sleep(500);
        // 打断执行中的线程.
        pool.shutdownNow();
        System.out.println("全部超过 500ms的全部将会被取消执行");
    }

    private static final Lock LOCK = new ReentrantLock();

    private static void inter() {
        try {
            int sleepTime = new Random().nextInt(1000);
            TimeUnit.MILLISECONDS.sleep(sleepTime);
            // 获取锁,打断将会被抛出异常,其实正常来说lock()打断也会被抛出异常的.
            LOCK.lockInterruptibly();
            System.out.println(Thread.currentThread().getName() + " : 获取锁成功 , 耗时 : " + sleepTime + " ms.");
        } catch (Exception e) {
            System.out.println(Thread.currentThread().getName() + " : 异常");
        } finally {
            try {
                LOCK.unlock();
            } catch (Exception e) {
                //
            }
        }
    }
}
```

输出 

```java
pool-1-thread-9 : 获取锁成功 , 耗时 : 14 ms.
pool-1-thread-2 : 获取锁成功 , 耗时 : 68 ms.
pool-1-thread-3 : 获取锁成功 , 耗时 : 152 ms.
pool-1-thread-7 : 获取锁成功 , 耗时 : 175 ms.
pool-1-thread-4 : 获取锁成功 , 耗时 : 208 ms.
pool-1-thread-6 : 获取锁成功 , 耗时 : 435 ms.
pool-1-thread-5 : 异常
pool-1-thread-8 : 异常
pool-1-thread-10 : 异常
pool-1-thread-1 : 异常
全部超过 500ms的全部将会被取消执行
```



## 3. ReentrantReadWriteLock 读写锁

> ​	一个读写锁**只能同时存在一个写锁 , 但是可以存在多个读锁**，**但不能同时存在写锁和读锁**。  这就是读写锁的核心. 
>
> 对于`ReentrantReadWriteLock` 内部其实维护了俩Lock , 一个是 `ReadLock` 一个是`WriteLock` 

```java
public static void main(String[] args) throws InterruptedException {

    ReentrantReadWriteLock lock = new ReentrantReadWriteLock();
    long start = System.currentTimeMillis();

    new Thread(() -> {
        lock.readLock().lock();
        System.out.println(Thread.currentThread().getName() + " : 正在读取数据 , 耗时 : " + (System.currentTimeMillis() - start));
        sleep(1000);
        lock.readLock().unlock();
    }).start();

    new Thread(() -> {
        lock.readLock().lock();
        System.out.println(Thread.currentThread().getName() + " : 正在读取数据 , 耗时 : " + (System.currentTimeMillis() - start));
        sleep(1000);
        lock.readLock().unlock();
    }).start();


    new Thread(() -> {
        lock.writeLock().lock();
        System.out.println(Thread.currentThread().getName() + " : 正在写入数据耗时 : " + (System.currentTimeMillis() - start));
        lock.writeLock().unlock();
    }).start();

}
```

输出  : 读可以同时读, 但是读写不能同时进行. 

```java
Thread-0 : 正在读取数据 , 耗时 : 70
Thread-1 : 正在读取数据 , 耗时 : 70
Thread-2 : 正在写入数据耗时 : 1074
```

## 4. volatile 保证数据可见性

> volatile 修饰的成员变量, 可以保证多个线程每次读取的时候永远是主存中的数据,   而不是线程的私有数据, 这个实现是基于CPU的设计来实现的, 所以我们并不需要关注与CPU架构的实现, 

```java
public class Volatile {

    private static int x = 0;

    public static void main(String[] args) {
        new Thread(() -> {
            sleep(500);
            x = 1;
        }).start();

        while (true) {
            if (x == 1) {
                break;
            }
        }
    }
}
```

此时主线程会一直死循环下去,因为主线程中的数据一致是0 , 他并没有查看主存中的数据

此时只需要将 `private static int x = 0;` 改成 `private static volatile int x = 0;`  程序就不会阻塞下去了 , 这个方法很多地方使用, 比如 `java.util.concurrent.FutureTask` 就是基于这个原理实现, 但是while循环会找出CPU空转. 浪费CPU资源, 

## 5. 原子性问题

举例 :

>  Java中执行 x=x+1 操作是,  将读取变量x,然后入栈, 将1入栈, 将栈顶两个数相加, 然后结果入栈, 将结果保存在变量x中'
>
>  所以结果就是分为  读取变量x  , 将x+1 得到的结果再赋值给x  . 
>
>  这就是一个原子操作, 他不是一步, 而是几步是一个原子  , 可以说是一个事务.  ​



所以引入了 atomic类. 里面都是原子操作. 基于CAS实现的. 





