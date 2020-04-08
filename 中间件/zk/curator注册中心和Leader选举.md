# curator 实现注册中心  以及 Leader选举

Demo 其实都是官方提供的. 链接 :  [http://curator.apache.org/curator-examples/index.html](http://curator.apache.org/curator-examples/index.html)

## 注册中心

依赖

```xml
<dependency>
    <groupId>org.apache.curator</groupId>
    <artifactId>curator-x-discovery</artifactId>
    <version>4.0.1</version>
</dependency>
// 这个是一个测试用的zk服务端.直接new testzk 就可以启动了.
<dependency>
    <groupId>org.apache.curator</groupId>
    <artifactId>curator-test</artifactId>
    <version>4.0.1</version>
    <!--<scope>test</scope>-->
</dependency>
```

### 简单例子

下面例子里. 使用了 FastJson做序列化. 还有就是使用Lombox .

```java
public class TestDiscovery {

    public static void main(String[] args) throws Exception {
        String path = "/discovery/example";
        // 1. 创建一个客户端
        CuratorFramework client = Zk.startClient();

        // 2. 这个是用于序列化. 每一个服务节点的.
        InstanceSerializer<Detail> serializer = new InstanceSerializer<Detail>() {
            // 序列化过程 , 都是使用的json
            @Override
            public byte[] serialize(ServiceInstance<Detail> instance) throws Exception {
                Detail detail = instance.getPayload();
                detail.id = instance.getId();
                return JSON.toJSONBytes(detail);
            }

            // 反序列化过程
            @Override
            public ServiceInstance<Detail> deserialize(byte[] bytes) throws Exception {
                Detail detail = JSON.parseObject(bytes, Detail.class);
                return ServiceInstance.<Detail>builder().payload(detail).name(detail.name).address(detail.host).port(detail.port).build();
            }
        };

        // 3. 假设这个就是我们的 服务消费方 , 需要去消费节点. (basePath , 其实在zk3.5中是创建了一个 container)
        ServiceDiscovery<Detail> discovery = ServiceDiscoveryBuilder.builder(Detail.class).client(client).basePath(path).serializer(serializer).build();


        //4. 创建三个服务提供方 , 创建节点.
        newNode(path, client, serializer, "192.168.58.131");
        newNode(path, client, serializer, "192.168.58.132");
        newNode(path, client, serializer, "192.168.58.133");


        //5. 我们的服务消费方, 需要去发现redis服务 节点. 
        Collection<ServiceInstance<Detail>> instances = discovery.queryForInstances("redis");
        instances.forEach(detailServiceInstance -> System.out.println("find redis instance : " + detailServiceInstance.getPayload()));

        // 6. 方便去zk查看信息.
        System.in.read();
    }

    // 创建redis 节点
    private static void newNode(String path, CuratorFramework client, InstanceSerializer<Detail> serializer, String host) throws Exception {
        // 1. 创建一个Detail
        Detail detail = new Detail();
        detail.port = 6379;
        detail.host = host;
        detail.name = "redis";
        // 2. 实例化一个 服务节点
        ServiceInstance<Detail> instance = ServiceInstance.<Detail>builder().name(detail.name).payload(detail).address(detail.host).port(detail.port).build();

        // 3. 注册节点 . 此时需要执行start. 其实就是注册了改节点.
        ServiceDiscovery<Detail> discovery = ServiceDiscoveryBuilder.builder(Detail.class).client(client).serializer(serializer).basePath(path).thisInstance(instance).build();
        discovery.start();
    }


    @Getter
    @Setter
    @ToString
    static class Detail {
        // id , 默认是UUID
        String id;

        // 属于哪个服务
        String name;

        // host
        String host;

        // port
        int port;
    }
}
```



输出 : 

```java
find redis instance : TestDiscovery.Detail(id=36c95464-6846-4b69-8c5b-07cf91ad0d62, name=redis, host=192.168.58.133, port=6379)
find redis instance : TestDiscovery.Detail(id=11bf74c8-8e4d-4b8d-821c-fecdf0492b6b, name=redis, host=192.168.58.132, port=6379)
find redis instance : TestDiscovery.Detail(id=ed01f5c4-2c92-484b-9194-b168958cf834, name=redis, host=192.168.58.131, port=6379)
```

所以使用起来还是很方便的. 我们这里封装的`Detail` 主要是保存一些我们每个节点的元信息 . 

此时去zk 查看

```java
[zk: localhost:2181(CONNECTED) 18] ls /discovery/example/redis
[11bf74c8-8e4d-4b8d-821c-fecdf0492b6b, 36c95464-6846-4b69-8c5b-07cf91ad0d62, ed01f5c4-2c92-484b-9194-b168958cf834]
```

确实 id默认是 uuid 生成的. 不过这些是可以指定的. 

我们看看具体长啥样子每个. 

```java
[zk: localhost:2181(CONNECTED) 19] get /discovery/example/redis/11bf74c8-8e4d-4b8d-821c-fecdf0492b6b
{"host":"192.168.58.132","id":"11bf74c8-8e4d-4b8d-821c-fecdf0492b6b","name":"redis","port":6379}
```

具体状态呢. 其实是一个临时节点. 

```java
[zk: localhost:2181(CONNECTED) 20] stat /discovery/example/redis/11bf74c8-8e4d-4b8d-821c-fecdf0492b6b
cZxid = 0x2a5
ctime = Wed Mar 25 19:17:14 CST 2020
mZxid = 0x2a5
mtime = Wed Mar 25 19:17:14 CST 2020
pZxid = 0x2a5
cversion = 0
dataVersion = 0
aclVersion = 0
ephemeralOwner = 0x1000000fdb80061
dataLength = 96
numChildren = 0
```



### 实现

以上就是一个注册中心的基本实现.  其实是用起来还是很方便的. 

主要是注意 两个对象. `ServiceDiscovery` 这个对象是代表一个服务端 . (可以是消费者, 可以是提供者) 需要start进行暴露 ,close进行删除, `ServiceInstance` 代表一个单个服务实例.    一个服务端可以注册多个服务实例 

```java
// 1. 注册服务
discovery.registerService(Instance);
// 2. 发现服务
discovery.queryForInstances(server-name);
// 3. 启动服务 , 会将注册到服务器的节点全部注册给zk服务端. 
discovery.start();
```



我们看看 `registerService方法 ` 方法

```java
public void registerService(ServiceInstance<T> service) throws Exception
{
    Entry<T> newEntry = new Entry<T>(service);
    Entry<T> oldEntry = services.putIfAbsent(service.getId(), newEntry);
    Entry<T> useEntry = (oldEntry != null) ? oldEntry : newEntry;
    synchronized(useEntry)
    {
        if ( useEntry == newEntry ) // i.e. is new
        {
            useEntry.cache = makeNodeCache(service);
        }
        // 这里就去执行去 zk服务器中注册改节点.
        internalRegisterService(service);
    }
}
```

再进去就是

```java
@VisibleForTesting
protected void internalRegisterService(ServiceInstance<T> service) throws Exception
{
    byte[] bytes = serializer.serialize(service);
    String path = pathForInstance(service.getName(), service.getId());

    final int MAX_TRIES = 2;
    boolean isDone = false;
    for ( int i = 0; !isDone && (i < MAX_TRIES); ++i )
    {
        try
        {
        // 判断节点创建的类型. 
            CreateMode mode;
            switch (service.getServiceType()) {
            case DYNAMIC:
                mode = CreateMode.EPHEMERAL;
                break;
            case DYNAMIC_SEQUENTIAL:
                mode = CreateMode.EPHEMERAL_SEQUENTIAL;
                break;
            default:
                mode = CreateMode.PERSISTENT;
                break;
            }
            client.create().creatingParentContainersIfNeeded().withMode(mode).forPath(path, bytes);
            isDone = true;
        }
        catch ( KeeperException.NodeExistsException e )
        {
            // 如果该节点存在了, 则删除该节点,  则会继续循环(因为 isDown=false),然后重新注册改节点.
            client.delete().forPath(path);  // must delete then re-create so that watchers fire
        }
    }
}
```



### 注意点

所以怎么说呢. start 和 register别一起使用吧.  比如我们可以先创建`ServiceDiscovery` 服务发现中心.   

```java
ServiceDiscovery<Detail> discovery = ServiceDiscoveryBuilder.builder(Detail.class).client(client).basePath(path).serializer(serializer).build();
```

然后再去调用他的register instance. 最后就别使用start.  基本就是这个.   还有这个根节点是一个 container . 所以zk会自动帮我们回收这个节点 . 如果子对象为空的话. 



所以API 封装的还是很棒的.  



上面我看了看 demo , 自己编写的.  其中还有一个功能是 :  `UriSpec` , 可以使用占位符. 进行设置属性. 

```java
UriSpec uriSpec = new UriSpec("{scheme}://foo.com:{port}");

thisInstance = ServiceInstance.<InstanceDetails>builder()
        .name(serviceName)
        .payload(new InstanceDetails(description))
        .port((int) (65535 * Math.random())) // in a real application, you'd use a common port
        .uriSpec(uriSpec)
        .build();
```

如果加入了 ssl 配置 , 自动成了 https 

```java
if ( serviceInstance.getSslPort() != null )
{
    localVariables.put(FIELD_SCHEME, "https");
}
```





## Leader选举

> ​	主要实现原理还是基于 分布式锁实现的. 因为排它锁默认就是一把, 其实就是leader.  leader会拿着那把锁 , 基于这种思想去做的. 可以看看他的具体原理和实现. `org.apache.curator.framework.recipes.leader.LeaderSelector#doWork` 在这里实现的. 

我们拿官方的例子来说 : 

这个是个客户端, 核心的api在 `takeLeadership` , 当你被选为leader的时候, 此时当该方法没有返回之前你一直是leader. 

```java
/**
 * An example leader selector client. Note that {@link LeaderSelectorListenerAdapter} which
 * has the recommended handling for connection state issues
 */
public class ExampleClient extends LeaderSelectorListenerAdapter implements Closeable {
    private final String name;
    private final LeaderSelector leaderSelector;
    private final AtomicInteger leaderCount = new AtomicInteger();

    public ExampleClient(CuratorFramework client, String path, String name) {
        this.name = name;

        // create a leader selector using the given path for management
        // all participants in a given leader selection must use the same path
        // ExampleClient here is also a LeaderSelectorListener but this isn't required
        leaderSelector = new LeaderSelector(client, path, this);

        // for most cases you will want your instance to requeue when it relinquishes leadership
        // 这里就是当你放弃领导的时候, 完全放弃就不设置了, 当你不完全放弃.还想参与竞选.此时就需要设置.
        leaderSelector.autoRequeue();
    }

    public void start() throws IOException {
        // the selection for this instance doesn't start until the leader selector is started
        // leader selection is done in the background so this call to leaderSelector.start() returns immediately
        leaderSelector.start();
    }

    @Override
    public void close() throws IOException {
        leaderSelector.close();
    }

    @Override
    public void takeLeadership(CuratorFramework client) throws Exception {
        // we are now the leader. This method should not return until we want to relinquish leadership

        final int waitSeconds = (int) (5 * Math.random()) + 1;

        System.out.println(name + " is now the leader. Waiting " + waitSeconds + " seconds..." );
        System.out.println(name + " has been leader " + leaderCount.getAndIncrement() + " time(s) before.");
        try {
            Thread.sleep(TimeUnit.SECONDS.toMillis(waitSeconds));
        } catch (InterruptedException e) {
            System.err.println(name + " was interrupted.");
            Thread.currentThread().interrupt();
        } finally {
            System.out.println(name + " relinquishing leadership.\n");
        }
    }
}
```





主方法 : 模拟 leader的选举. 

```java
public class LeaderSelectorExample {
    private static final int CLIENT_QTY = 10;

    private static final String PATH = "/examples/leader";

    public static void main(String[] args) throws Exception {
        // all of the useful sample code is in ExampleClient.java

        System.out.println("Create " + CLIENT_QTY + " clients, have each negotiate for leadership and then wait a random number of seconds before letting another leader election occur.");
        System.out.println("Notice that leader election is fair: all clients will become leader and will do so the same number of times.");

        List<CuratorFramework> clients = Lists.newArrayList();
        List<ExampleClient> examples = Lists.newArrayList();
        TestingServer server = new TestingServer();
        try {
            for (int i = 0; i < CLIENT_QTY; ++i) {
                CuratorFramework client = CuratorFrameworkFactory.newClient(server.getConnectString(), new ExponentialBackoffRetry(1000, 3));
                clients.add(client);

                ExampleClient example = new ExampleClient(client, PATH, "Client #" + i);
                examples.add(example);

                client.start();
                example.start();
            }

            System.out.println("Press enter/return to quit\n");
            new BufferedReader(new InputStreamReader(System.in)).readLine();
        } finally {
            System.out.println("Shutting down...");

            for (ExampleClient exampleClient : examples) {
                CloseableUtils.closeQuietly(exampleClient);
            }
            for (CuratorFramework client : clients) {
                CloseableUtils.closeQuietly(client);
            }
            CloseableUtils.closeQuietly(server);
        }
    }
}
```







我还写了一个类似的demo

```java
public class ExampleLeader2 {

    public static void main(String[] args) throws InterruptedException {

        String path = "/leader/example";
        int count = 10;
        ExecutorService pool = Executors.newFixedThreadPool(10);
        IntStream.range(0, count).forEach(new IntConsumer() {
            @Override
            public void accept(int value) {
                pool.execute(() -> {
                    String id = "c-" + value;
                    CuratorFramework client = Zk.startClient();

                    // 监视器
                    Object monitor = new Object();

                    LeaderSelector selector = new LeaderSelector(client, path, new LeaderSelectorListener() {
                        // 当你是领导的时候, 会触发这个回调事件.
                        @Override
                        public void takeLeadership(CuratorFramework client) throws Exception {
                            System.out.println(id + " :具有leader权限. : " + Thread.currentThread().getName());

                            // 比如此时具有了leader权限. 此时就是执行leader所做的具体业务.
                            // 止到leader被down掉之后. 所以设置了一个wait . 等待系统down掉.
                            synchronized (monitor) {
                                monitor.wait();
                            }
                        }

                        @Override
                        public void stateChanged(CuratorFramework client, ConnectionState newState) {
                            System.out.println(id + " change : " + newState);
                        }
                    });
                    selector.start();


                    // 后续业务吧.
                    // down掉后 , 具体逻辑这里写, 

                    synchronized (monitor) {
                        monitor.notifyAll();
                        System.out.println(id + " down机.... , 释放当前线程");
                    }
                    CloseableUtils.closeQuietly(selector);
                    CloseableUtils.closeQuietly(client);
                });
            }
        });

        pool.shutdown();
        pool.awaitTermination(1000, TimeUnit.DAYS);
    }
}
```



这种写法 对于非leader的选手不友好. 因为他并不知道它是不是leader和follower.  还有一种写法是等到自己是leader的时候继续执行

```java
String id = "c-" + value;
CuratorFramework client = Zk.startClient();
LeaderLatch latch = new LeaderLatch(client, path, id);
try {
    latch.start();
} catch (Exception e) {
    //
}
// 一直等到自己作为leader.
try {
    latch.await();
} catch (InterruptedException | EOFException e) {
    e.printStackTrace();
}
```





这里我写了一个 leader简单版 , 启动多个观察leader的变化. 

```java
public class ExampleClient3 {

    public static void main(String[] args) {
        CuratorFramework client = Zk.startClient();

        String path = "/leader/example";


        Object monitor = new Object();

        LeaderSelector selector = new LeaderSelector(client, path, new LeaderSelectorListener() {
            @Override
            public void takeLeadership(CuratorFramework client) throws Exception {
                new Timer(true).schedule(new TimerTask() {
                    @Override
                    public void run() {
                        System.out.println("leader worker");
                    }
                }, 1000, 1000);

                synchronized (monitor) {
                    monitor.wait();
                }
            }

            @Override
            public void stateChanged(CuratorFramework client, ConnectionState newState) {
                System.out.println("change : " + newState);
            }
        });


        selector.setId("192.168.58.131:6379");

        selector.start();


        try {
            TimeUnit.DAYS.sleep(Integer.MAX_VALUE);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        Runtime.getRuntime().addShutdownHook(new Thread(new Runnable() {
            @Override
            public void run() {
                synchronized (monitor) {
                    monitor.notifyAll();
                }
                CloseableUtils.closeQuietly(selector);
                CloseableUtils.closeQuietly(client);
            }
        }));
    }
}
```





我们查看节点信息 . ID其实就是 保存的数据. 

```java
[zk: localhost:2181(CONNECTED) 66] ls /leader/example/_c_537ef73c-8923-46f5-8689-dc001cae446b-lock-0000000000
[]
[zk: localhost:2181(CONNECTED) 67] get /leader/example/_c_537ef73c-8923-46f5-8689-dc001cae446b-lock-0000000000
192.168.58.131:6379
```



基本就是这些. Leader选举是适用场景并不多, 所以我这里也么办法写一个demo. 