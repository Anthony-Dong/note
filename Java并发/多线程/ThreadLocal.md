## ThreadLocal

> ​	`ThreadLocal`是一个线程变量工具类。主要用于将私有线程和该线程存放的副本对象做一个映射，各个线程之间的变量互不干扰，例如A线程和B线程都想使用一个变量,此时就存在强资源的问题,而这个变量他可以有多份,此时就可以用`ThreadLocal`作为变量的管理者 ,不存在多线程隐患. 同时他隐式的可以作为一个线程内部的传递信息的一个工具 .
>
> ​		**`ThreadLocal`从另一个角度来解决线程的并发访问**。**`ThreadLocal`会为每一个线程提供一个独立的变量副本，从而隔离了多个线程对数据的访问冲突**。例如在多线程中都去使用一个对象,但是又希望互不干涉,此时就需要用到`ThreadLocal`.
>
> 　　概括起来说，对于多线程资源共享的问题，同步机制采用了“以时间换空间”的方式，而`ThreadLocal`采用了“以空间换时间”的方式。前者仅提供一份变量，让不同的线程排队访问，而后者为每一个线程都提供了一份变量，因此可以同时访问而互不影响。

## 1. 全局观

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-28/1d86e756-b877-4020-8f81-6b1f1bf9c59a.jpg?x-oss-process=style/template01)



我们可以发现 每一个 线程 都有一个 map对象 , map存放的是本地线程对象和副本变量 .然后这个map对象由一个 threadlocal对象 维护 ,他负责去添加和维护

所以对于不同的线程，每次获取副本值时，别的线程并不能获取到当前线程的副本值，形成了副本的隔离，互不干扰。

ThreadLocal values与这个线程有关,这个map被这个thread所维护

```java
public class Thread implements Runnable {
    /* ThreadLocal values pertaining(属于) to this thread. This map is maintained(被..维护)
     * by the ThreadLocal class. */
    ThreadLocal.ThreadLocalMap threadLocals = null;
}
```





## 2. 常用的方法

1 . public T **get**()   获取当前线程的副本变量值。

2 . public void **set**(T value)    保存当前线程的副本变量值。

3 .  public void **remove**()   移除当前前程的副本变量值。

4 . public static  ThreadLocal  **withInitial**(Supplier supplier) 初始化变量

### 1. get方法

> ​	Returns the value in the current thread's copy of this thread-local variable. If the variable has no value for the current thread, it is first initialized to the value returned by an invocation of the initialValue method.
>
> ​	**返回当前线程的线程局部变量副本中的值。如果变量没有当前线程的值，则首先将其初始化为调用initialValue方法返回的值。**



### 2. set方法

> ​	Sets the current thread's copy of this thread-local variable to the specified value. Most subclasses will have no need to override this method, relying solely on the initialValue method to set the values of thread-locals.
>
> ​	**将此线程局部变量的当前线程副本设置为指定的值。大多数子类不需要重写这个方法，只需要依赖initialValue方法来设置线程局部变量的值。**



### 3. remove方法

> ​	Removes the current thread's value for this thread-local variable. If this thread-local variable is subsequently read by the current thread, its value will be reinitialized by invoking its initialValue method, unless its value is set by the current thread in the interim. This may result in multiple invocations of the initialValue method in the current thread.
>
>  **1 . 删除此线程中的局部变量。如果这个线程局部变量随后又被调用,那么它的值将通过调用其initialValue方法重新初始化，除非它的值是由当前线程在过渡期间设置的。这可能导致在当前线程中多次调用initialValue方法。**
>
> **2 . 还要注意一点就是 不remove 会发生内存泄漏,溢出,看你线程消耗吧,固有线程随意了,**

### 4. withInitial 方法

> ​	Creates a thread local variable. The initial value of the variable is determined by invoking the get method on the Supplier.
>
> **创造一个线程的局部变量 , 通过调用get方法来确定变量的初始值。**



## 3. 使用场景

### 1. 线程池的使用

```java
public class TestThreadLocal2 {

    private static ThreadLocal<Connection> threadLocal = new ThreadLocal<>();

    public static Connection getConnection(){
        Connection connection = threadLocal.get();
        if (null == connection) {
            connection = Datasource.get();
            threadLocal.set(connection);
        }
        return connection;
    }

    
    public static void remove(){
        threadLocal.remove();
    }



    public static void main(String[] args) throws InterruptedException {

        Executor executors = Executors.newFixedThreadPool(10);
        executors.execute(()->{
            System.out.println(getConnection());
        });
        executors.execute(()->{
            System.out.println(getConnection());
        });

        System.out.println(getConnection());
    }

    private static class Connection{
        String name;
        public Connection(String name) {
            this.name = name;
        }
    }

    private static class Datasource{
        static ArrayBlockingQueue<Connection> connections = new ArrayBlockingQueue<Connection>(5);

        static {
            connections.offer(new Connection("1"));
            connections.offer(new Connection("2"));
            connections.offer(new Connection("3"));
            connections.offer(new Connection("4"));
            connections.offer(new Connection("5"));
        }

        static Connection get(){
            Connection connection =null;
            try {
                // 移除并返回头部元素 阻塞操作
                connection = connections.take();
                // 把移除的元素再添加回去 比如 一开始是 1 2 3 4 5 现在 take一个变成了 2 3 4 5 _ 然后put变成了 2 3 4  5 1 .
                connections.put(connection);
            } catch (InterruptedException e) {
                System.out.println("线程阻塞");
            }
            return connection;
        }
    }
}

```

### 2. 内部变量的使用 

```java
public class MainTest {

    public static void main(String[] args) {
        ExecutorService pool = Executors.newFixedThreadPool(1000);

        // 使用 ThreadLocal
        Bank bank = new Bank();
        IntStream.range(0, 3).forEach(e->{
            pool.execute(()->{
                bank.deposit(200,"新员工 : "+e);
            });
        });

        pool.shutdown();
    }
}

class Bank {
    // 初始化账户余额为 1000
    ThreadLocal<Integer> account = ThreadLocal.withInitial(() -> 1000);

    public void deposit(int money,String name) {
        System.out.println(name + "--当前账户余额为：" + account.get());
        account.set(account.get() + money);
        System.out.println(name + "--存入 " + money + " 后账户余额为：" + account.get());
        account.remove();
    }
}
```

输出结果  : 

```java
新员工 : 0--当前账户余额为：1000
新员工 : 0--存入 200 后账户余额为：1200
新员工 : 2--当前账户余额为：1000
新员工 : 2--存入 200 后账户余额为：1200
新员工 : 1--当前账户余额为：1000
新员工 : 1--存入 200 后账户余额为：1200
```



### 3.  日期格式化并发问题

```java
public class DateUtil {

    private static ThreadLocal<SimpleDateFormat> sdf = ThreadLocal
            .withInitial(() -> new SimpleDateFormat("yyyy-MM-dd HH:mm:ss"));

    // 
    public static String format(Date date) {
        String msg = null;
        try {
            // 调用get方法 -> withInitial初始化 
            msg = sdf.get().format(date);
        } finally {
            // 用完 remove 
            sdf.remove();
        }
        return msg;
    }


    public static String formatJava8(Date date){
        return LocalDateTime.now().format(DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss"));
    }

    public static void main(String[] args) throws Exception {
        System.err.println(format(new Date()));

        System.err.println(formatJava8(new Date()));
    }

}
```





## 4 . 问题: ThreadLocalMap - entry 

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-02/85530434-974c-42c8-908a-e4723265379c.png?x-oss-process=style/template01)

```java
static class Entry extends WeakReference<ThreadLocal> {
    /** The value associated with this ThreadLocal. */
    Object value;

    Entry(ThreadLocal k, Object v) {
        // 只有key是弱类型引用,value并不是 , 所以容易出现内存溢出的现象,如果不去手动remove的话
        super(k);
        value = v;
    }
    
    .....
}
```





## 5. 使用注意的问题

#### ThreadLocalMap的问题

由于ThreadLocalMap的key是弱引用，而Value是强引用。这就导致了一个问题，ThreadLocal在没有外部对象强引用时，发生GC时弱引用Key会被回收，而Value不会回收，如果创建ThreadLocal的线程一直持续运行，那么这个Entry对象中的value就有可能一直得不到回收，发生内存泄露。

**如何避免泄漏**
既然Key是弱引用，那么我们要做的事，就是在调用ThreadLocal的get()、set()方法时完成后再调用remove方法，将Entry节点和Map的引用关系移除，这样整个Entry对象在GC Roots分析后就变成不可达了，下次GC的时候就可以被回收。

如果使用ThreadLocal的set方法之后，没有显示的调用remove方法，就有可能发生内存泄露，所以养成良好的编程习惯十分重要，使用完ThreadLocal之后，记得调用remove方法。