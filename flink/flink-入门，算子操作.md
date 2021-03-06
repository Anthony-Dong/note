# Flink 入门 学习

入门教程：wordCount

```java
private StreamExecutionEnvironment env;

@Before
public void init() {
    env = StreamExecutionEnvironment.getExecutionEnvironment();
}
@Test
public void testWordCount1() throws Exception {
    DataStreamSource<String> source = env.socketTextStream("localhost", 9999);
    source.flatMap(new RichFlatMapFunction<String, Tuple2<String, Integer>>() {
        @Override
        public void flatMap(String value, Collector<Tuple2<String, Integer>> out) throws Exception {
            String[] split = value.split(" ");
            for (String s : split) {
                out.collect(Tuple2.of(s, 1));
            }
        }
    }).keyBy(0).sum(1).print();
    env.execute("testWordCount1");
}
```

执行脚本:

```shell
nc -l 9999
```



## Source

### 1、有界流和无界流

flink 里面认为数据源，也就是stream，分为有界流和无界流

> 无界流定义流的开始，但没有定义流的结束。它们会无休止地产生数据。**无界流的数据必须持续处理**，即数据被摄取后需要立刻处理。我们不能等到所有数据都到达再处理，因为输入是无限的，在任何时候输入都不会完成。处理无界数据通常要求以特定顺序摄取事件，例如事件发生的顺序，以便能够推断结果的完整性。

> 有界流定义流的开始，也有定义流的结束。有界流可以在摄取所有数据后再进行计算。有界流所有数据可以被排序，所以并不需要有序摄取。**有界流处理通常被称为批处理**



在最一开始的时候，Spark只存在RDD这么一说，之后Flink的那套出来后Spark就借鉴过去了，Flink是靠实时起家的，因为这玩意一开始搞离线也搞不过Spark。所以它们其实也挺相爱相杀的.



这个基本都是无界流， -> 返回都是`DataStreamSource`

![image-20200614084047603](/Users/sgcx015/Library/Application Support/typora-user-images/image-20200614084047603.png)

这个 source func就两个方法

![](/Users/sgcx015/Library/Application Support/typora-user-images/image-20200614084150701.png)



其次就是有界流，定义了很多

![image-20200614084344772](/Users/sgcx015/Library/Application Support/typora-user-images/image-20200614084344772.png)

![image-20200614084359731](/Users/sgcx015/Library/Application Support/typora-user-images/image-20200614084359731.png)

这些基本上可以理解为是有界流，file里面可以有监听机制，可以实现无界流。

### 2、并行流、非并行流

#### 1、`SourceFunction`

source 另外一个维度分，可以分为两类，并行流，和非并行流。并行流其实就是多个线程去拉去，非并行只有有单个线程

`SourceFunction` 为 非并行流

```java
private StreamExecutionEnvironment env;
@Before
public void before() {
    env = StreamExecutionEnvironment.getExecutionEnvironment();
}

@Test
public void testSourceFunction() throws Exception {
  // 
    env.addSource(new SourceFunction<String>() {
        int count = 0;
        @Override
        public void run(SourceContext<String> ctx) throws Exception {
            while (true) {
                ctx.collect(String.format("[%s] : %d", Thread.currentThread().getName(), count++));
                TimeUnit.MILLISECONDS.sleep(100);
            }
        }

        @Override
        public void cancel() {
        }
    }).addSink(new SinkFunction<String>() {
        @Override
        public void invoke(String value, Context context) throws Exception {
            System.out.println(String.format("[sink-thread %s]{data : %s}", Thread.currentThread().getName(), value));
        }
    }).setParallelism(1);
    env.execute("testSourceFunction");
}
```

输出 ： 

```java
[sink-thread Legacy Source Thread - Source: Custom Source -> Sink: Unnamed (1/1)]{data : [source-thread Legacy Source Thread - Source: Custom Source -> Sink: Unnamed (1/1)] : 0}
[sink-thread Legacy Source Thread - Source: Custom Source -> Sink: Unnamed (1/1)]{data : [source-thread Legacy Source Thread - Source: Custom Source -> Sink: Unnamed (1/1)] : 1}
[sink-thread Legacy Source Thread - Source: Custom Source -> Sink: Unnamed (1/1)]{data : [source-thread Legacy Source Thread - Source: Custom Source -> Sink: Unnamed (1/1)] : 2}
[sink-thread Legacy Source Thread - Source: Custom Source -> Sink: Unnamed (1/1)]{data : [source-thread Legacy Source Thread - Source: Custom Source -> Sink: Unnamed (1/1)] : 3}
[sink-thread Legacy Source Thread - Source: Custom Source -> Sink: Unnamed (1/1)]{data : [source-thread Legacy Source Thread - Source: Custom Source -> Sink: Unnamed (1/1)] : 4}
[sink-thread Legacy Source Thread - Source: Custom Source -> Sink: Unnamed (1/1)]{data : [source-thread Legacy Source Thread - Source: Custom Source -> Sink: Unnamed (1/1)] : 5}
[sink-thread Legacy Source Thread - Source: Custom Source -> Sink: Unnamed (1/1)]{data : [source-thread Legacy Source Thread - Source: Custom Source -> Sink: Unnamed (1/1)] : 6}

```

我们发现俩线程都是单个执行，sink-thread 和 source-thread 都是一样的单个线程，对于这种写法，每一个方法其实执行的对象是前一个对象，所以`setParallelism(1)`其实是指的sink的并行度，那么我们可以理解为，如果我们这么写.

那么比如我们现在改成这样子

```java
@Test
public void testSourceFunction2() throws Exception {
    env.addSource(new SourceFunction<String>() {
        int count = 0;
        @Override
        public void run(SourceContext<String> ctx) throws Exception {
            while (true) {
                ctx.collect(String.format("[source-thread %s] : %d", Thread.currentThread().getName(), count++));
                TimeUnit.MILLISECONDS.sleep(100);
            }
        }
        @Override
        public void cancel() {
        }
    }).addSink(new SinkFunction<String>() {
        @Override
        public void invoke(String value, Context context) throws Exception {
            System.out.println(String.format("[sink-thread %s]{data : %s}", Thread.currentThread().getName(), value));
        }
    });
    env.execute("testSourceFunction");
}
```

此时输出

```java
[sink-thread Sink: Unnamed (5/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (1/1)] : 0}
[sink-thread Sink: Unnamed (6/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (1/1)] : 1}
[sink-thread Sink: Unnamed (7/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (1/1)] : 2}
[sink-thread Sink: Unnamed (8/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (1/1)] : 3}
[sink-thread Sink: Unnamed (1/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (1/1)] : 4}
[sink-thread Sink: Unnamed (2/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (1/1)] : 5}
[sink-thread Sink: Unnamed (3/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (1/1)] : 6}
[sink-thread Sink: Unnamed (4/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (1/1)] : 7}
[sink-thread Sink: Unnamed (5/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (1/1)] : 8}
[sink-thread Sink: Unnamed (6/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (1/1)] : 9}
```

此时 sink-thread 的线程是8个，也就是说默认是8个线程操作。

那么我们说了 `SourceFunction`是非并行流，所以此时修改一下。

```java
@Test
public void testSourceFunction3() throws Exception {
    env.addSource(new SourceFunction<String>() {
        int count = 0;
        @Override
        public void run(SourceContext<String> ctx) throws Exception {
            while (true) {
                ctx.collect(String.format("[source-thread %s] : %d", Thread.currentThread().getName(), count++));
                TimeUnit.MILLISECONDS.sleep(100);
            }
        }

        @Override
        public void cancel() {
        }
    }).setParallelism(2).addSink(new SinkFunction<String>() {
        @Override
        public void invoke(String value, Context context) throws Exception {
            System.out.println(String.format("[sink-thread %s]{data : %s}", Thread.currentThread().getName(), value));
        }
    });
    env.execute("testSourceFunction");
}
```

此时输出

```go
java.lang.IllegalArgumentException: The maximum parallelism of non parallel operator must be 1.
```

> ​	抛出异常 最大的并行度为1



#### 2、`ParallelSourceFunction`

> ​	看名字也知道是并行流

```java
@Test
public void testParallelSourceFunction() throws Exception {
    env.addSource(new ParallelSourceFunction<String>() {
        int count = 0;

        @Override
        public void run(SourceContext<String> ctx) throws Exception {
            while (true) {
                ctx.collect(String.format("[source-thread %s] : %d", Thread.currentThread().getName(), count++));
                TimeUnit.MILLISECONDS.sleep(100);
            }
        }

        @Override
        public void cancel() {

        }
    }).setParallelism(2).addSink(new SinkFunction<String>() {
        @Override
        public void invoke(String value, Context context) throws Exception {
            System.out.println(String.format("[sink-thread %s]{data : %s}", Thread.currentThread().getName(), value));
        }
    });
    env.execute("testSourceFunction");
}
```

此时输出：

发现source-thread 会有两个Custom Source (1/2)和Custom Source (2/2)，同时每个人的数据是`count`是不一样的，所以也就是每个线程都回去实例化一个 source。

```java
[sink-thread Sink: Unnamed (1/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (1/2)] : 0}
[sink-thread Sink: Unnamed (7/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (2/2)] : 0}
[sink-thread Sink: Unnamed (2/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (1/2)] : 1}
[sink-thread Sink: Unnamed (8/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (2/2)] : 1}
[sink-thread Sink: Unnamed (3/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (1/2)] : 2}
[sink-thread Sink: Unnamed (1/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (2/2)] : 2}
[sink-thread Sink: Unnamed (2/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (2/2)] : 3}
[sink-thread Sink: Unnamed (4/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (1/2)] : 3}
[sink-thread Sink: Unnamed (5/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (1/2)] : 4}
[sink-thread Sink: Unnamed (3/8)]{data : [source-thread Legacy Source Thread - Source: Custom Source (2/2)] : 4}
```

#### 3、`RichParallelSourceFunction`

可以get到他是丰富了 `ParallelSourceFunction`的功能，所以，我们看看有啥功能

`getRuntimeContext` 获取当前的ctx ,关于这个context 后面会讲到，可以理解为是一个全局的上下文可以拿到一些全局的配置信息之类的。

`open()`  开始的时候会调用

```java
@Test
public void testRichParallelSourceFunction() throws Exception {

    env.addSource(new RichParallelSourceFunction<String>() {
        int count = 0;

        @Override
        public void open(Configu ration parameters) throws Exception {
            RuntimeContext context = getRuntimeContext();
            System.out.println("open context: " + context);
        }

        @Override
        public void run(SourceContext<String> ctx) throws Exception {
            System.out.println("run context : " + ctx);
            ctx.collect(String.format("[source-thread %s] : %d", Thread.currentThread().getName(), count++));
            TimeUnit.MILLISECONDS.sleep(100);
        }

        @Override
        public void cancel() {

        }
    }).setParallelism(2).addSink(new SinkFunction<String>() {
        @Override
        public void invoke(String value, Context context) throws Exception {
            System.out.println(String.format("[sink-thread %s]{data : %s}", Thread.currentThread().getName(), value));
        }
    }).setParallelism(1);
    env.execute("testRichParallelSourceFunction");
}
```

输出：

```java
open context: org.apache.flink.streaming.api.operators.StreamingRuntimeContext@56d05a06
open context: org.apache.flink.streaming.api.operators.StreamingRuntimeContext@6ee7263b
run context : org.apache.flink.streaming.api.operators.StreamSourceContexts$NonTimestampContext@491e50f9
run context : org.apache.flink.streaming.api.operators.StreamSourceContexts$NonTimestampContext@595e4aa9
[sink-thread Sink: Unnamed (1/1)]{data : [source-thread Legacy Source Thread - Source: Custom Source (1/2)] : 0}
[sink-thread Sink: Unnamed (1/1)]{data : [source-thread Legacy Source Thread - Source: Custom Source (2/2)] : 0}
```



## 





### 3、合并多个流

#### 1、简单合并

> ​	并行处理多个流，可以使用简单合并。

```java
public static void test() throws Exception {
    StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();
    // stream 1
    DataStreamSource<Integer> intStream = env.fromCollection(Arrays.asList(1, 2, 3));
    // stream 2
    DataStreamSource<String> source = env.fromCollection(Arrays.asList("a", "b", "c"));
    // 连接多个流
    ConnectedStreams<String, Integer> connect = source.connect(intStream);

    connect.process(new CoProcessFunction<String, Integer, Object>() {
        @Override
        public void processElement1(String value, Context ctx, Collector<Object> out) throws Exception {
            out.collect(value);
        }

        @Override
        public void processElement2(Integer value, Context ctx, Collector<Object> out) throws Exception {
            out.collect(value);
        }
    }).print();
    env.execute("demo");
}
```



#### 2、广播合并

> ​	多利用boardcase的思想，去分发信息，多用于配置场景。

```java
@Test
public void testBroadcast() throws Exception {
    DataStreamSource<String> source1 = environment.addSource(new RichParallelSourceFunction<String>() {
        @Override
        public void run(SourceContext<String> ctx) throws Exception {
            while (true) {
                ctx.collect("=========");
                TimeUnit.MILLISECONDS.sleep(100);
            }
        }

        @Override
        public void cancel() {

        }
    }).setParallelism(8);

    DataStream<String> source2 = environment.addSource(new RichParallelSourceFunction<String>() {
        @Override
        public void run(SourceContext<String> ctx) throws Exception {
            while (true) {
                ctx.collect(new Random().nextInt(100) + "");
                TimeUnit.SECONDS.sleep(1);
                System.out.println("refresh");
            }
        }

        @Override
        public void cancel() {

        }
    }).setParallelism(1).broadcast();


    source1.connect(source2).flatMap(new CoFlatMapFunction<String, String, String>() {
        String config;

        @Override
        public void flatMap1(String value, Collector<String> out) throws Exception {
            System.out.println(String.format("process [%s] config %s", Thread.currentThread().getName(), config));
        }

        @Override
        public void flatMap2(String value, Collector<String> out) throws Exception {
            this.config = value;
        }
    }).addSink(new SinkFunction<String>() {
        @Override
        public void invoke(String value, Context context) throws Exception {

        }
    });
    environment.execute("");
}
```

我们可以get到，只需要broadcast一个方法，就可以实现配置的分发。

```go
process [Co-Flat Map -> Sink: Unnamed (1/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (2/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (3/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (6/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (4/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (5/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (7/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (8/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (6/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (3/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (2/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (1/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (4/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (5/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (7/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (8/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (3/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (6/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (2/8)] config 18
process [Co-Flat Map -> Sink: Unnamed (1/8)] config 18
```





## 算子操作（Transformation）

### 1、map

a->b  就是map

```java
SingleOutputStreamOperator<Tuple2<String, Integer>> map = env.socketTextStream("localhost", 8888).map(new MapFunction<String, Tuple2<String, Integer>>() {
    @Override
    public Tuple2<String, Integer> map(String value) throws Exception {
        return new Tuple2<>(value, value.split(" ").length);
    }
});
```

### 2、flatMap

```java
SingleOutputStreamOperator<Object> flatMap = env.socketTextStream("", 8888).flatMap(new FlatMapFunction<String, Object>() {
    @Override
    public void flatMap(String value, Collector<Object> out) throws Exception {

    }
});
```



显然和map输出是一样的，但是有个不同点就是，map是直接 a-b， flatMap是 a->collector->b， 也就是 a 可以产生多个b ， 而 map只能a产生一个b。



### 3、split

```java
@Test
public void testSplit() throws Exception {
    DataStreamSource<Integer> source = environment.addSource(new SourceFunction<Integer>() {
        Random random = new Random();
        @Override
        public void run(SourceContext<Integer> ctx) throws Exception {
            while (true) {
                ctx.collect(random.nextInt(1000));
                TimeUnit.MILLISECONDS.sleep(100);
            }
        }
        @Override
        public void cancel() {
        }
    });
    // 这个被不推荐使用了
    SplitStream<Integer> split = source.split((OutputSelector<Integer>) value -> Collections.singletonList(value % 2 == 0 ? "even" : "uneven"));
    DataStream<Integer> even = split.select("even");
    even.print();
    environment.execute("testSplit");
}
```

感觉和 filter很相似，但是人家可以切割多个，好处，但是也不推荐使用



### 3、key-by

> ​	类似于group by

一种分区策略，比如现在我们需要把，偶数相加，奇数相加，每4个累加一次。

```go
@Test
public void testKeyBy() throws Exception {
    DataStreamSource<Integer> source = environment.addSource(new SourceFunction<Integer>() {
        @Override
        public void run(SourceContext<Integer> ctx) throws Exception {
            List<Integer> list = Arrays.asList(1, 2, 1, 2, 1, 2, 1, 2);
            list.forEach(integer -> {
                ctx.collect(integer);
                try {
                    TimeUnit.MILLISECONDS.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            });
        }

        @Override
        public void cancel() {

        }
    });

    // 这里根据key，进行分区
    KeyedStream<Integer, String> integerObjectKeyedStream = source.keyBy(new KeySelector<Integer, String>() {
        @Override
        public String getKey(Integer value) throws Exception {
            return value % 2 == 0 ? "even" : "uneven";
        }
    });
    integerObjectKeyedStream.flatMap(new RichFlatMapFunction<Integer, Integer>() {
        ValueState<Integer> state = null;
        int trigger = 0;

        @Override
        public void open(Configuration parameters) throws Exception {
            // state 是一个十分核心的东西，后面再讲解，其实可以理解为类似于context，属于一种runtime状态
            ValueStateDescriptor<Integer> descriptor = new ValueStateDescriptor<>("", TypeInformation.of(Integer.class));
            this.state = getRuntimeContext().getState(descriptor);
        }

        @Override
        public void flatMap(Integer value, Collector<Integer> out) throws Exception {
            Integer sum = state.value();
            if (sum == null) {
                sum = 0;
            }
            sum += value;
            state.update(sum);
            // 没四次刷新一次和
            if (++trigger % 4 == 0) {
                out.collect(sum);
                state.clear();
            }
        }
    }).print();
    environment.execute("testKeyBy");
}
```

以上就是简单的demo；





## watermark 水印 和 time

```java
env.setStreamTimeCharacteristic(TimeCharacteristic.EventTime);
```

event介绍 https://cloud.tencent.com/developer/article/1373594	

waterMark介绍 https://www.jianshu.com/p/9db56f81fa2a  ，https://developer.aliyun.com/article/682873

主要是讲的，处理时间的概念，flink会根据不同维度的时间去计算。 当设置eventTime，用户必须去设置一个拿到time的一个方法，但是有个问题就是用户的数据，可能比如a的时间搓1s,b是2s，我们的统计维度是2s统计一次。但是由于网路延迟，b拿到的时候已经是3s了，这时候就需要一个watermark，只有当 watermark>windows_end_time，才回去聚合。其实也就是一个容错的机制。 具体容错事件根据真实情况设置。

写法如下：

```java
  StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();
  // 设置自定义的时间搓属性
  env.setStreamTimeCharacteristic(TimeCharacteristic.EventTime);
  // 添加kafka  source
  FlinkKafkaConsumer011<EventLog> consumer = new FlinkKafkaConsumer011<>(KafkaProducer.topic, new EventLogSer(), getKafkaSourceProperties());
  DataStreamSource<EventLog> streamSource = env.addSource(consumer);
  // water mark,容错是100s
  streamSource.assignTimestampsAndWatermarks(new BoundedOutOfOrdernessTimestampExtractor<EventLog>(Time.of(100, TimeUnit.MILLISECONDS)) {
      @Override
      public long extractTimestamp(EventLog element) {
          // 以日志的上面记录的时间为窗口计算维度
          return element.time;
      }
      // 聚合数量需要做转换
  }).map(new MapFunction<EventLog, Tuple1<Integer>>() {
      @Override
      public Tuple1<Integer> map(EventLog value) throws Exception {
          return new Tuple1<>(1);
      }
  }).timeWindowAll(Time.seconds(1)).sum(0).print();
```

![](https://img-blog.csdn.net/20160929172201717)



watermark有两类：完美式（perfect）watermark和启发式（heuristic）watermark。

## 窗口的概念

这里有一篇文章推荐一下：https://juejin.im/post/5ee090cd6fb9a047d564685b

看看下面两段代码

```java
env.socketTextStream("", 8888).flatMap(new FlatMapFunction<String, Tuple2<String, Integer>>() {
    @Override
    public void flatMap(String value, Collector<Tuple2<String, Integer>> out) throws Exception {
        String[] split = value.split("");
        for (String s : split) {
            out.collect(new Tuple2<>(s, 1));
        }
    }
}).keyBy(0).sum(1).print();
```

> ​	这个计算的维度是全量

```java
env.socketTextStream("", 8888).flatMap(new FlatMapFunction<String, Tuple2<String, Integer>>() {
    @Override
    public void flatMap(String value, Collector<Tuple2<String, Integer>> out) throws Exception {
        String[] split = value.split("");
        for (String s : split) {
            out.collect(new Tuple2<>(s, 1));
        }
    }
}).keyBy(0).timeWindow(Time.seconds(1)).sum(1).print();
```

> ​	这个计算的维度是时间窗





那么 时间窗是什么？



比如我们处理一件事情，往往是以周期去执行的，比如统计1分钟内日志的数量，这个就是以日志时间去聚合的。那么问题来了，这个时间是按照什么维度的呢，比如日志时间，有日志本身记录的时间，还是flink处理的时间，其实这个flink都考虑了。



下面这个例子，就很好的处理了这样的事情

```java
StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();
// 设置属性
env.setStreamTimeCharacteristic(TimeCharacteristic.EventTime);
// 添加kafka  source
FlinkKafkaConsumer011<EventLog> consumer = new FlinkKafkaConsumer011<>(KafkaProducer.topic, new EventLogSer(), getKafkaSourceProperties());
DataStreamSource<EventLog> streamSource = env.addSource(consumer);

// water mark
streamSource.assignTimestampsAndWatermarks(new BoundedOutOfOrdernessTimestampExtractor<EventLog>(Time.of(100, TimeUnit.MILLISECONDS)) {
    @Override
    public long extractTimestamp(EventLog element) {
        // 以日志的上面记录的时间为窗口计算维度
        return element.time;
    }
    // 聚合数量需要做转换
}).map(new MapFunction<EventLog, Tuple1<Integer>>() {
    @Override
    public Tuple1<Integer> map(EventLog value) throws Exception {
        return new Tuple1<>(1);
    }
}).timeWindowAll(Time.seconds(1)).sum(0).print();
```

这个就是一个 



## state

https://juejin.im/post/5e7a13915188255e245ec12a

state 的维度有两个，一个是task级别（我测试测试不通），一个是key级别。

### 1、task 级别

```java
org.apache.flink.runtime.JobException: Recovery is suppressed by NoRestartBackoffTimeStrategy
	at org.apache.flink.runtime.executiongraph.failover.flip1.ExecutionFailureHandler.handleFailure(ExecutionFailureHandler.java:110)
	at org.apache.flink.runtime.executiongraph.failover.flip1.ExecutionFailureHandler.getFailureHandlingResult(ExecutionFailureHandler.java:76)
	at org.apache.flink.runtime.scheduler.DefaultScheduler.handleTaskFailure(DefaultScheduler.java:192)
	at org.apache.flink.runtime.scheduler.DefaultScheduler.maybeHandleTaskFailure(DefaultScheduler.java:186)
	at org.apache.flink.runtime.scheduler.DefaultScheduler.updateTaskExecutionStateInternal(DefaultScheduler.java:180)
	at org.apache.flink.runtime.scheduler.SchedulerBase.updateTaskExecutionState(SchedulerBase.java:484)
	at org.apache.flink.runtime.jobmaster.JobMaster.updateTaskExecutionState(JobMaster.java:380)
	at sun.reflect.NativeMethodAccessorImpl.invoke0(Native Method)
	at sun.reflect.NativeMethodAccessorImpl.invoke(NativeMethodAccessorImpl.java:62)
	at sun.reflect.DelegatingMethodAccessorImpl.invoke(DelegatingMethodAccessorImpl.java:43)
	at java.lang.reflect.Method.invoke(Method.java:498)
	at org.apache.flink.runtime.rpc.akka.AkkaRpcActor.handleRpcInvocation(AkkaRpcActor.java:279)
	at org.apache.flink.runtime.rpc.akka.AkkaRpcActor.handleRpcMessage(AkkaRpcActor.java:194)
	at org.apache.flink.runtime.rpc.akka.FencedAkkaRpcActor.handleRpcMessage(FencedAkkaRpcActor.java:74)
	at org.apache.flink.runtime.rpc.akka.AkkaRpcActor.handleMessage(AkkaRpcActor.java:152)
	at akka.japi.pf.UnitCaseStatement.apply(CaseStatements.scala:26)
	at akka.japi.pf.UnitCaseStatement.apply(CaseStatements.scala:21)
	at scala.PartialFunction$class.applyOrElse(PartialFunction.scala:123)
	at akka.japi.pf.UnitCaseStatement.applyOrElse(CaseStatements.scala:21)
	at scala.PartialFunction$OrElse.applyOrElse(PartialFunction.scala:170)
	at scala.PartialFunction$OrElse.applyOrElse(PartialFunction.scala:171)
	at scala.PartialFunction$OrElse.applyOrElse(PartialFunction.scala:171)
	at akka.actor.Actor$class.aroundReceive(Actor.scala:517)
	at akka.actor.AbstractActor.aroundReceive(AbstractActor.scala:225)
	at akka.actor.ActorCell.receiveMessage(ActorCell.scala:592)
	at akka.actor.ActorCell.invoke(ActorCell.scala:561)
	at akka.dispatch.Mailbox.processMailbox(Mailbox.scala:258)
	at akka.dispatch.Mailbox.run(Mailbox.scala:225)
	at akka.dispatch.Mailbox.exec(Mailbox.scala:235)
	at akka.dispatch.forkjoin.ForkJoinTask.doExec(ForkJoinTask.java:260)
	at akka.dispatch.forkjoin.ForkJoinPool$WorkQueue.runTask(ForkJoinPool.java:1339)
	at akka.dispatch.forkjoin.ForkJoinPool.runWorker(ForkJoinPool.java:1979)
	at akka.dispatch.forkjoin.ForkJoinWorkerThread.run(ForkJoinWorkerThread.java:107)
Caused by: java.lang.NullPointerException: Keyed state can only be used on a 'keyed stream', i.e., after a 'keyBy()' operation. // 核心的一点就是 keyed state must after keyby op
	at org.apache.flink.util.Preconditions.checkNotNull(Preconditions.java:75)
	at org.apache.flink.streaming.api.operators.StreamingRuntimeContext.checkPreconditionsAndGetKeyedStateStore(StreamingRuntimeContext.java:185)
	at org.apache.flink.streaming.api.operators.StreamingRuntimeContext.getState(StreamingRuntimeContext.java:142)
	at state.StateDemo$1.open(StateDemo.java:39)
	at org.apache.flink.api.common.functions.util.FunctionUtils.openFunction(FunctionUtils.java:36)
	at org.apache.flink.streaming.api.operators.AbstractUdfStreamOperator.open(AbstractUdfStreamOperator.java:102)
	at org.apache.flink.streaming.runtime.tasks.StreamTask.initializeStateAndOpen(StreamTask.java:1007)
	at org.apache.flink.streaming.runtime.tasks.StreamTask.lambda$beforeInvoke$0(StreamTask.java:454)
	at org.apache.flink.streaming.runtime.tasks.StreamTaskActionExecutor$SynchronizedStreamTaskActionExecutor.runThrowing(StreamTaskActionExecutor.java:94)
	at org.apache.flink.streaming.runtime.tasks.StreamTask.beforeInvoke(StreamTask.java:449)
	at org.apache.flink.streaming.runtime.tasks.StreamTask.invoke(StreamTask.java:461)
	at org.apache.flink.runtime.taskmanager.Task.doRun(Task.java:707)
	at org.apache.flink.runtime.taskmanager.Task.run(Task.java:532)
	at java.lang.Thread.run(Thread.java:748)
```

### 2、key级别

其实state很好理解，就是存储一个上下文，以一个key为聚合，这个上下文的数据结构，就是以下这几个key的特点。keyed state 记录的是每个key的状态，Keyed state托管状态有六种类型：

1. ValueState
2. ListState
3. MapState
4. ReducingState
5. AggregatingState
6. FoldingState

我这里介绍一下，最简单的ValueState，其实就是仅仅一个状态值 ：

下面这个demo就是统计，每一个分区的处理数量，聚合维度是key ， 代码很简单。



```java
private StreamExecutionEnvironment env;

@Before
public void init() {
    env = StreamExecutionEnvironment.getExecutionEnvironment();
}

@Test
public void testKeyState() throws Exception {
    env.addSource(new ParallelSourceFunction<String>() {
        private Random random = new Random();

        @Override
        public void run(SourceContext<String> ctx) throws Exception {
            while (true) {
                // 分成了10个分区
                int count = random.nextInt(5);
                ctx.collect(count + ",content");
                TimeUnit.MILLISECONDS.sleep(100);
            }
        }

        @Override
        public void cancel() {
        }
    }).setParallelism(8).keyBy(new KeySelector<String, Integer>() {
        @Override
        public Integer getKey(String value) throws Exception {
            String[] split = value.split(",");
            // 以 0，1，2，3，4 为key 操作，也就是类似于kafka的分区策略
            return Integer.parseInt(split[0]);
        }
    }).flatMap(new RichFlatMapFunction<String, Integer>() {
        private ValueState<Integer> state;
        @Override
        public void open(Configuration parameters) throws Exception {
            super.open(parameters);
            ValueStateDescriptor<Integer> descriptor = new ValueStateDescriptor<Integer>("key", Integer.class);
            state = getRuntimeContext().getState(descriptor);
        }

        @Override
        public void flatMap(String value, Collector<Integer> out) throws Exception {
            Integer count = state.value();
            if (count == null) {
                count = 0;
            }
            System.out.printf("[%s]%s\n", Thread.currentThread().getName(), value);
            count++;
            state.update(count);
            out.collect(count);
        }
    }).print();
    env.execute("testKeyState");
}
```

主要是看输出

```java
[Flat Map -> Sink: Print to Std. Out (8/8)]3,content
8> 1
[Flat Map -> Sink: Print to Std. Out (8/8)]3,content
8> 2
[Flat Map -> Sink: Print to Std. Out (8/8)]3,content
8> 3
[Flat Map -> Sink: Print to Std. Out (8/8)]3,content
8> 4
[Flat Map -> Sink: Print to Std. Out (8/8)]3,content
8> 5
[Flat Map -> Sink: Print to Std. Out (8/8)]3,content
8> 6
[Flat Map -> Sink: Print to Std. Out (8/8)]3,content
8> 7
[Flat Map -> Sink: Print to Std. Out (8/8)]3,content // 这里是8
8> 8
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 1
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 2
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 3
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 4
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 5
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 6
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 7
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 8
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 9
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 10
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 11
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 12
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 13
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 14
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 15
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 16
[Flat Map -> Sink: Print to Std. Out (8/8)]2,content
8> 1
[Flat Map -> Sink: Print to Std. Out (8/8)]2,content
8> 2
[Flat Map -> Sink: Print to Std. Out (8/8)]2,content
8> 3
[Flat Map -> Sink: Print to Std. Out (8/8)]2,content
8> 4
[Flat Map -> Sink: Print to Std. Out (8/8)]2,content
8> 5
[Flat Map -> Sink: Print to Std. Out (8/8)]2,content
8> 6
[Flat Map -> Sink: Print to Std. Out (8/8)]2,content
8> 7
[Flat Map -> Sink: Print to Std. Out (8/8)]2,content
8> 8
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 17
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 18
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 19
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 20
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 21
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 22
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 23
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 24
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 1
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 2
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 3
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 4
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 5
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 6
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 7
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 8
[Flat Map -> Sink: Print to Std. Out (8/8)]2,content
8> 9
[Flat Map -> Sink: Print to Std. Out (8/8)]2,content
8> 10
[Flat Map -> Sink: Print to Std. Out (8/8)]2,content
8> 11
[Flat Map -> Sink: Print to Std. Out (8/8)]2,content
8> 12
[Flat Map -> Sink: Print to Std. Out (8/8)]2,content
8> 13
[Flat Map -> Sink: Print to Std. Out (8/8)]2,content
8> 14
[Flat Map -> Sink: Print to Std. Out (8/8)]2,content
8> 15
[Flat Map -> Sink: Print to Std. Out (8/8)]2,content
8> 16
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 9
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 10
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 11
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 12
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 13
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 14
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 15
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 16
[Flat Map -> Sink: Print to Std. Out (8/8)]3,content // 下一次出现的位置，这里是9，所以成功拿到状态。
8> 9
[Flat Map -> Sink: Print to Std. Out (8/8)]3,content
8> 10
[Flat Map -> Sink: Print to Std. Out (8/8)]3,content
8> 11
[Flat Map -> Sink: Print to Std. Out (8/8)]3,content
8> 12
[Flat Map -> Sink: Print to Std. Out (8/8)]3,content
8> 13
[Flat Map -> Sink: Print to Std. Out (8/8)]3,content
8> 14
[Flat Map -> Sink: Print to Std. Out (8/8)]3,content
8> 15
[Flat Map -> Sink: Print to Std. Out (8/8)]3,content
8> 16
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 25
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 26
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 27
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 28
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 29
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 30
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 31
[Flat Map -> Sink: Print to Std. Out (6/8)]0,content
6> 32
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 17
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 18
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 19
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 20
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 21
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 22
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 23
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 24
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 25
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 26
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 27
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 28
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 29
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 30
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 31
[Flat Map -> Sink: Print to Std. Out (6/8)]1,content
6> 32
```



其实测试结果，还发现一个问题就是，每一个key都是绑定一个线程的，可能是方便聚合。