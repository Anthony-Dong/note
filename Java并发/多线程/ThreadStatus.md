# Thread

## 1. 构造方法

### 1. 无参 

```java
public Thread() {
        init(null, null, "Thread-" + nextThreadNum(), 0);
}
```

### 2. 最常用的

> ​	target – the object whose run method is invoked when this thread is started. If null, this classes run method does nothing.
>
> **target 这个对象的run方法当线程开始是被运行 , 如果没有 , 这个类运行方法什么也不做 ,**
>
> **这种也是我们最常用的一种了**

```java
public Thread(Runnable target) {
    init(null, target, "Thread-" + nextThreadNum(), 0);
}
```

### 3. 最全的

```java
public Thread(ThreadGroup group, Runnable target, String name,
                  long stackSize) {
        init(group, target, name, stackSize);
}
```

> group :  the thread group. If null and there is a security manager, the group is determined by SecurityManager.getThreadGroup(). If there is not a security manager or SecurityManager.getThreadGroup() returns null, the group is set to the current thread's thread group.
>
> name – the name of the new thread
>
> stackSize – the desired stack size for the new thread, or zero to indicate that this parameter is to be ignored.
>
> group - 线程组 , 如果没有的话,这里有一个security manager , 则由SecurityManager的`getThreadGroup()`方法确定组 , 如果没有security manager 等 ,则将该组设置为当前线程的线程组。
>
> target -  Runnable对象的run方法当线程开始是被运行 , 如果没有 , 这个类运行方法什么也不做 ,
>
> name  - 当前线程的名字
>
> stackSize - 新线程所需的堆栈大小，或者0表示忽略该参数。这里设置0可以让JVM自动管理 ,如果我们手动设置 ,会造成 ,栈溢出`StackOverflowError`或者栈太小了,内存溢出了 `OutOfMemoryError `或者 `内部 error`
>

## 2. 守护线程

`final void setDaemon(boolean on)`

> Marks this thread as either a daemon thread or a user thread. The Java Virtual Machine exits when the only threads running are all daemon threads.
> This method must be invoked before the thread is started.	
>
> **描述的是当JVM中的运行的线程就省下守护线程了, JVM就退出了  , 守护线程这个方法 必须在线程开始前调用 .就是start() 方法前调用**

```java
public final void setDaemon(boolean on) {
    //  the current thread cannot modify this thread
    checkAccess();
    // strat 运行后 ,线程状态会改成 isAlive ,此时会抛出异常
    if (isAlive()) {
        throw new IllegalThreadStateException();
    }
    // 默认是非守护线程
    daemon = on;
}
```

所谓守护线程,是指在程序运行的时候在后台提供一种通用服务的线程，比如垃圾回收线程就是一个很称职的守护者，并且这种线程并不属于程序中不可或缺的部分。

## 3. 线程状态	

java 的 线程一共有 六种状态 

### 种类 :

NEW  : Thread state for a thread which has not yet started.

RUNNABLE  : Thread state for a runnable thread. 

BLOCKED  :  Thread state for a thread blocked waiting for a monitor lock.

WAITING :  Thread state for a waiting thread.

TIMED_WAITING :  Thread state for a waiting thread with a specified waiting time.

TERMINATED :  Thread state for a terminated thread. The thread has completed execution.



## 4. 如何关闭线程

### 1. interrupt() 方法

```java
public static void main(String[] args) {

    new Thread(() -> {
        while (true) {
            System.out.println(Thread.currentThread().getName() + " : " + "执行了一次");

            // 1. 给当前线程一个 打断标志 ,需要用户去选择
            Thread.currentThread().interrupt();

            // 2. 判断当前线程是被打断 , 此时线程还处于运行中
            if (Thread.currentThread().isInterrupted()) {
                System.out.println(Thread.currentThread().getName() + " 的线程状态 : " + Thread.currentThread().getState());
                // 我们break 结束线程
                break;
            }
        }
    }).start();
}
```

输出 : 

```java
Thread-0 : 执行了一次
Thread-0 的线程状态 : RUNNABLE
```



### 2. throw new Exception()

```java
public static void main(String[] args) {
    new Thread(() -> {
        int x = 1;
        while (true) {
            System.out.println(Thread.currentThread().getName() + " : " + x++);
            if (x == 5) {
                // 抛出异常
                throw new RuntimeException("exception");
            }
        }
    }).start();
}
```

输出

```java
Thread-0 : 1
Thread-0 : 2
Thread-0 : 3
Thread-0 : 4
Exception in thread "Thread-0" java.lang.RuntimeException: exception
	at com.threadstatus.StopThreadByException.lambda$main$0(StopThreadByException.java:20)
	at java.lang.Thread.run(Thread.java:748)
```



### 3. 设置标识符

```java
public class StopThreadWithFlag {
    private static volatile boolean flag = true;
    public static void main(String[] args) throws InterruptedException {
        Thread thread = new Thread(() -> {
            int x = 1;
            while (flag) {

                try {
                    TimeUnit.MILLISECONDS.sleep(500);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                System.out.println(Thread.currentThread().getName() + " : " + x++);
            }
        });
        thread.start();
        thread.join();

        flag = false;
    }
}
```

结果呢 ? 不会停止 , 因为主线程根本不执行 , 此时thread.join ,其实是一种阻塞状态 , 根本不会让主线程去执行 ,怎么办 .所以这种情况下 ,我们需要互换角色 , 此时主线程执行,子线程也会执行.

```java
public class StopThreadWithFlag {
    private static volatile boolean flag = true;

    public static void main(String[] args) throws InterruptedException {
        int x = 1;
        Thread thread = new Thread(() -> {
            flag = false;
        });
        thread.start();

        while (flag) {
            System.out.println(Thread.currentThread().getName() + " : " + x++);
        }
    }
}
```

