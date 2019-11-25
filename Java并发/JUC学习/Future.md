# Future - 异步返回结果

## 1. Future 

> 	A {@code Future} represents the result of an asynchronous  computation. 它是代表一个异步计算的返回结果,他的其实就是实现了一个带有返回值的`call() `方法 ,毫无任何意义的 . 咱们自己也可实现一个. 
>
> 主要方法 : 
>
> - **boolean cancel(boolean mayInterruptIfRunning);boolean isCancelled();**  是否取消该任务
> - **boolean isCancelled();**   是否已经被取消
> - **boolean isDone();**   是否任务执行完
> - **V get()**  :  获取返回结果
> - **V get(long timeout, TimeUnit unit)**  :  获取返回结果 同时设置超时时间



我们 查看他的源码其实就告诉我们如何去使用了 

```java
  ExecutorService executor = Executors.newFixedThreadPool(10);

        Future<String> future = executor.submit(new Callable<String>() {
            @Override
            public String call() throws Exception {
                Thread.sleep(1000);
                return Thread.currentThread().getName() + "   hello world";
            }
        });

        System.out.println(Thread.currentThread().getName() + "   hello world");

        try {
            String s = future.get(2, TimeUnit.SECONDS);
            System.out.println(s);
            // 判断是否任务执行完
            System.out.println("isDone : "+future.isDone());
        } catch (InterruptedException | ExecutionException | TimeoutException e) {
            // 超时关闭
            executor.shutdown();
        }
        // 正确关闭
        executor.shutdown();
```



## 2. FutureTask 

> 	The {@link FutureTask} class is an implementation of {@code Future} that  implements {@code Runnable}, and so may be executed by an {@code Executor}. 
>
> 	 `FutureTask `类 是 一个实现了Runnable, Future<V> 的类  , 其实呢 `Future  `对象 , 真正执行的时候也是被转换为`FutureTask` 对象,然后执行` execute() `方法 ,才执行的  . 一回事,`submi()`方法实际上调用的就是` execute()`  方法.   
>	
> 	[可以看看我写的另一篇文章](	https://anthony-dong.github.io/post/executor/)

```java
       ExecutorService executor = Executors.newSingleThreadExecutor();
        FutureTask<String> task = new FutureTask<>(new Callable<String>() {
            @Override
            public String call() throws Exception {
                TimeUnit.SECONDS.sleep(1);

                return Thread.currentThread().getName() + " : hello world";
            }
        });
        //  有效的解耦
        executor.execute(task);
        // 主线程
        System.out.println(Thread.currentThread().getName() + "   hello world");
        // 取消任务
        task.cancel(true);
        // 获取任务是否中断
        System.out.println(task.isCancelled());
        
        try {
            if (!task.isCancelled()) {
                String s = task.get(2, TimeUnit.SECONDS);
                System.out.println(s);
            }
        } catch (InterruptedException | ExecutionException | TimeoutException e) {
            // 超时关闭
            executor.shutdown();
        }
        executor.shutdown();
```

## 3.自定义实现一个 FutureTask

代码注释还是很不错的可以看看   .. .. .. 应该你可以理解的 ...... 

```java
public class MyFutureTask<T> implements Runnable {

    // 关键之处 
    private  volatile int state;
    
    // 状态 ,没有考虑太多状态 
    private  static final int NEW          = 0;
    private  static final int COMPLETING   = 1;

    // Callable 对象 , 其call()方法其实就是有一个返回值的方法罢了
    private Callable<T> callable;


    public MyFutureTask(Callable<T> callable) {
        this.callable = callable;
        this.state = NEW;
    }

    
    // 结果
    public T t;


    // 获取结果
    public T get() {

        // 关键代码 , 死循环 啥也不做 , 就是让线程阻塞 , 利用了volatile 的特性 ,
        // 让执行get的线程不断去拉去state状态
        while (state < COMPLETING) {
            
        }
        return this.t;
    }

    /**
     * 重写 run 方法 , 就是实现了 runnable接口 , 
     *  执行 调用 call 方法 , 将返回值返回 . 
     *  所以 Callable 其实就是一个 event源,只是有返回值而已 
     */
    @Override
    public void run() {
        try {
            // 调用 call 方法 
            Object call = callable.call();
            // 给结果赋值
            this.t = (T)call;
            // 然后改变状态
            this.state = COMPLETING;
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
```



使用 ...  .. . 

```java
public class TestMyFutureTaskDemo {


    public static void main(String[] args) throws InterruptedException, ExecutionException {

        ExecutorService executor = Executors.newFixedThreadPool(3);

        FutureTask<String> task = new FutureTask<>(new Callable<String>() {
            @Override
            public String call() throws Exception {
                return "JDK 实现的 FutureTask";
            }
        });

        MyFutureTask<Object> myTask = new MyFutureTask<>(new Callable<Object>() {
            @Override
            public Object call() throws Exception {
                return "我自己 实现的 FutureTask";
            }
        });

        executor.execute(task);
        executor.execute(myTask);


        System.out.println("task = " + task.get());
        System.out.println("myTask.get() = " + myTask.get());
        
        executor.shutdown();
    }
}
```

输出结果 -- 一模一样 

```java
task = JDK 实现的 FutureTask
myTask.get() = 我自己 实现的 FutureTask
```

