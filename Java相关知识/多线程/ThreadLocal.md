## ThreadLocal

> ​	ThreadLocal是一个本地线程副本变量工具类。主要用于将私有线程和该线程存放的副本对象做一个映射，各个线程之间的变量互不干扰，在高并发场景下，可以实现无状态的调用，特别适用于各个线程依赖不通的变量值完成操作的场景
>
> ​		而ThreadLocal则从**另一个角度来解决多线程的并发访问**。**ThreadLocal会为每一个线程提供一个独立的变量副本，从而隔离了多个线程对数据的访问冲突**。因为每一个线程都拥有自己的变量副本，从而也就**没有必要对该变量进行同步了**。ThreadLocal提供了线程安全的共享对象，在编写多线程代码时，可以把不安全的变量封装进ThreadLocal。
>
> 　　概括起来说，对于多线程资源共享的问题，同步机制采用了“以时间换空间”的方式，而ThreadLocal采用了“以空间换时间”的方式。前者仅提供一份变量，让不同的线程排队访问，而后者为每一个线程都提供了一份变量，因此可以同时访问而互不影响。





## 全局观

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





## 常用的方法

> public T get()   获取变量
>
> public void set(T value)    存储变量
>
> public void remove()   移除变量
>
> - get()方法用于获取当前线程的副本变量值。
> - set()方法用于保存当前线程的副本变量值。
> - initialValue()为当前线程初始副本变量值。
> - remove()方法移除当前前程的副本变量值。



### get方法

```java
public T get() {
    1. 获取当前线程
    Thread t = Thread.currentThread();
    2. 获取当前线程的内部的ThreadLocalMap(每一个thread内部都有一个map,还有一个可继承的map)
    ThreadLocalMap map = getMap(t);
    
    3. 不为空 直接取
    if (map != null) {
        ThreadLocalMap.Entry e = map.getEntry(this);
        if (e != null)
            return (T)e.value;
    }
    4. 为空的话进行初始化 (就是给复制一个空值)
    return setInitialValue();
}

ThreadLocalMap getMap(Thread t) {
    return t.threadLocals;
}

private T setInitialValue() {
    T value = initialValue();
    Thread t = Thread.currentThread();
    ThreadLocalMap map = getMap(t);
    if (map != null)
        map.set(this, value);
    else
        createMap(t, value);
    return value;
}

protected T initialValue() {
    return null;
}
```

> 步骤：
> 1.获取当前线程的ThreadLocalMap对象threadLocals
> 2.从map中获取线程存储的K-V Entry节点。
> 3.从Entry节点获取存储的Value副本值返回。
> 4.map为空的话返回初始值null，即线程变量副本为null，在使用时需要注意判断NullPointerException。



### set方法

```java
public void set(T value) {
    1. 获取当前线程
    Thread t = Thread.currentThread();
    2.获取map
    ThreadLocalMap map = getMap(t);
    3.复制
    if (map != null)
        map.set(this, value);
    else
        createMap(t, value);
}

ThreadLocalMap getMap(Thread t) {
    return t.threadLocals;
}

void createMap(Thread t, T firstValue) {
    t.threadLocals = new ThreadLocalMap(this, firstValue);
}
```

> 1.获取当前线程的成员变量map
>
> 2.map非空，则重新将ThreadLocal和新的value副本放入到map中。
>
> 3.map空，则对线程的成员变量ThreadLocalMap进行初始化创建，并将ThreadLocal和value副本放入map中。



### remove方法

```java
public void remove() {
 ThreadLocalMap m = getMap(Thread.currentThread());
 if (m != null)
     m.remove(this);
}
```



## 如何使用

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
        @Override
        public String toString() {
            return "Connection{" +
                    "name='" + name + '\'' +
                    '}';
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







## 问题: ThreadLocalMap - entry 

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





## 使用注意的问题

#### ThreadLocalMap的问题

由于ThreadLocalMap的key是弱引用，而Value是强引用。这就导致了一个问题，ThreadLocal在没有外部对象强引用时，发生GC时弱引用Key会被回收，而Value不会回收，如果创建ThreadLocal的线程一直持续运行，那么这个Entry对象中的value就有可能一直得不到回收，发生内存泄露。

**如何避免泄漏**
既然Key是弱引用，那么我们要做的事，就是在调用ThreadLocal的get()、set()方法时完成后再调用remove方法，将Entry节点和Map的引用关系移除，这样整个Entry对象在GC Roots分析后就变成不可达了，下次GC的时候就可以被回收。

如果使用ThreadLocal的set方法之后，没有显示的调用remove方法，就有可能发生内存泄露，所以养成良好的编程习惯十分重要，使用完ThreadLocal之后，记得调用remove方法。