# JUC- Fork/Join 

> ​	首先提到的是 `Future` 对象 ,我们知道 `Fork/Join` 框架给我们提供的是一种 切分 ,合并 的思想 ,类似与 `map/reduce`思想 , 那么他就需要一个异步的结果 , 所以需要 `future` 对象来处理这个异步线程,然后把这些线程结果合并起来. 写起来代码很简单.就是拆分合并
>
> ​	`Fork/Join` 框架 来自于 jdk1.7



简单代码逻辑 

```java
public class TestForkJoin {

	
    public static void main(String[] args) throws ExecutionException, InterruptedException {
		// 线程池
        ForkJoinPool pool = new ForkJoinPool();
		// task
        CounterRecursiveTask task = new CounterRecursiveTask(0, 100000);
		// submit 任务
        ForkJoinTask<Integer> submit = pool.submit(task);

		// get 获取结果
        System.out.println("submit.get() = " + submit.get());

		// 正常流程
        System.out.println("IntStream.rangeClosed(0,100000).sum() = " + IntStream.rangeClosed(0, 100000).sum());

    }


    static class CounterRecursiveTask extends RecursiveTask<Integer>{
        // 起始
        private int start;

        // 结束
        private int end;

        // fork 点
        private static final int MAX_SPLICE_NUMBER = 10000;

        // 传入参数,构造方法
        public CounterRecursiveTask(int start, int end) {
            this.start = start;
            this.end = end;
        }

        /**
         * The main computation performed by this task.
         *
         * @return the result of the computation
         */
        @Override
        protected Integer compute() {

            // 临界点
            if (end - start < MAX_SPLICE_NUMBER) {
                // 真正的计算逻辑在这里 , 将累加结果返回 ,给下面合并结果使用
                return IntStream.rangeClosed(start, end).sum();
            } else {
                // 这是向下取整
                int middle = (start + end) / 2;

                // 拆分子任务
                CounterRecursiveTask leftTasK = new CounterRecursiveTask(start, middle);

                CounterRecursiveTask rightTask = new CounterRecursiveTask(middle + 1, end);

                // 异步执行计算
                leftTasK.fork();
                rightTask.fork();

                System.out.printf("start : %s\tend : %s\tmiddle : %s\tinterval : %s\n\r", start, end, middle,(end-start));

                // 合并结果
                return leftTasK.join() + rightTask.join();
            }
        }
    }

}
```

