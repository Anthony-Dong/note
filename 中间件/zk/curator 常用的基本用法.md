# Curator 

这个框架  , 相对于Zk提供官方客户端, 功能更多一些. 也很实用. 其实就是封装了一下.  

这一章节 , 基本上就是介绍这个API的简单的使用.下一章节, 主要是介绍 它的分布式锁 , 选举机制的使用. 

其中Curator的基本使用可以看  [https://github.com/Netflix/curator/wiki/](https://github.com/Netflix/curator/wiki/) 

但是后期给 Apache 维护了, 比如demo可以看看  [https://github.com/apache/curator/tree/master/curator-examples]( https://github.com/apache/curator/tree/master/curator-examples) 

## 1. 简单使用一下API.

启动很简单.  直接这么编程就可以了. 可以加入几个启动的监听器. 

#### 创建客户端

```java
private static CuratorFramework getCuratorFramework() {
    RetryPolicy retryPolicy = new ExponentialBackoffRetry(1000, 3);
    CuratorFramework client = CuratorFrameworkFactory.newClient("192.168.58.131:2181", retryPolicy);
    client.getConnectionStateListenable().addListener((c, newState) -> System.out.println("connect : " + c));
    //启动
    client.start();
    return client;
}
```

实用builder 创建客户端. 

```java
// 指数. 增长
RetryPolicy retryPolicy = new ExponentialBackoffRetry(1000, 3);
CuratorFramework client = CuratorFrameworkFactory.builder()
        // 这个意思其实是初始化的时候创建一个父节点.记住不需要加 / ,该客户端所有的操作都基于该空间上 CRUD.
        .namespace("node")
        .connectionTimeoutMs(1000)
        .connectString("192.168.58.131:2181")
        .retryPolicy(retryPolicy)
        .build();

// 启动
client.start();
```

#### 创建一个节点. 

```java
public static void main(String[] args) throws Exception {
    // 启动zk客户端
    CuratorFramework client = Zk.getCuratorFramework();

    // 路径
    String path = "/create-data/eph_";
    // 创建一个节点. 临时自增节点. 完全开放的策略. 数据为init
    client.create().creatingParentsIfNeeded().withMode(CreateMode.EPHEMERAL_SEQUENTIAL).withACL(ZooDefs.Ids.OPEN_ACL_UNSAFE).forPath(path, "init".getBytes());
    // 获取子节点.
    List<String> list = client.getChildren().forPath("/create-data");
    list.forEach(System.out::println); //eph_0000000000
}
```

> ​	zk 3.5 后面加入了一个容器节点. 当容器类型的节点下的子节点全部被删除时，则该节点也会在一段时间内被删除。

#### 判断一个节点是否存在.

监测是节点是否存在 : 

```java
private static void checkExist(String path) throws Exception {
    CuratorFramework client = getCuratorFramework();
    // 调用 checkExists() 方法. 
    Stat stat = client.checkExists().forPath(path);
    if (stat == null) {
        System.out.println("不存在");
    } else {
        System.out.println("存在 : " + stat);
    }
    client.close();
}
```



#### 获取节点数据  : 

```java
// 将节点信息存入进去
Stat stat = new Stat();
// 获取节点的内容
byte[] content = client.getData().storingStatIn(stat).forPath("/example");
```

#### 获取子节点 

```java
List<String> list = client.getChildren().forPath("/test2");
```

#### 添加watcher 

```java
public static void main(String[] args) throws Exception {
    // 启动zk客户端
    CuratorFramework client = Zk.getCuratorFramework();
    // 路径
    String path = "/create-data-exit";

    // 添加监听器.
    client.getData().usingWatcher(new Watcher() {
        @Override
        public void process(WatchedEvent event) {
            System.out.println(event.getType());
        }
    }).forPath(path);

    client.setData().forPath(path, "change".getBytes());
}
```

#### 添加 Listener

`CuratorListener` 主要是用来接收异步的消息 ,这个是 curator创建的 , zk的客户端并没有这个api , 比如`InBackground`的.   使用与异步场景. 

> ​	Receives notifications about errors and background events

```java
public static void setDataAsync(CuratorFramework client, String path, byte[] payload) throws Exception {
    // this is one method of getting event/async notifications
    CuratorListener listener = new CuratorListener() {
        @Override
        public void eventReceived(CuratorFramework client, CuratorEvent event) throws Exception {
            // examine event for details
            System.out.println("listener : "+event);
        }
    };
    client.getCuratorListenable().addListener(listener);

    // set data for the given node asynchronously. The completion notification
    // is done via the CuratorListener.
    client.setData().inBackground().forPath(path, payload);
}
```



## 2. 重复监听

> ​	以下三种对于并发写入都是不支持 , 因为zk这种场景并不是 高写入的, 你家的配置 也不是一直进行修改的. 所以就是个这. 
>
> 同时, zk 客户端写重复监听是很麻烦的. 

我们自己写一个 重复监听. 其实就是一个递归式

```java
public class App4 {

    public static void main(String[] args) throws Exception {
        // 启动zk客户端
        CuratorFramework client = Zk.getCuratorFramework();

        // 先删除
        String path = "/test-watcher";
        client.delete().deletingChildrenIfNeeded().forPath(path);
        // 创建当前节点
        client.create().withMode(CreateMode.PERSISTENT).forPath(path, "init".getBytes());

        // 重复监听
        watch(client, path);
        
        client.setData().forPath(path, "change-1".getBytes());
        client.setData().forPath(path, "change-2".getBytes());
        System.in.read();
    }

    private static void watch(CuratorFramework client, String path) throws Exception {
        client.getData().usingWatcher(new CuratorWatcher() {
            @Override
            public void process(WatchedEvent event) throws Exception {
                System.out.println(event.getPath() + " : " + event.getType());
                // 递归重复监听.
                watch(client, path);
            }
        }).forPath(path);
    }
}
```

### `NodeCache`  单节点监听.

可以帮助我们做一些事情. 就是重复监听. 

> ​	A utility that attempts to keep the data from a node locally cached. This class will watch the node, respond to update/create/delete events, pull down the data, etc. You can register a listener that will get notified when changes occur.

```java
public class App {

    public static void main(String[] args) throws Exception {
        // 启动zk客户端
        CuratorFramework client = Zk.getCuratorFramework();

        // 先删除
        String path = "/test-watcher";
        client.delete().forPath(path);
        // 创建当前节点
        client.create().withMode(CreateMode.PERSISTENT).forPath(path, "hello world - init".getBytes());

        // 创建一个 NodeCache
        NodeCache cache = new NodeCache(client, path);

        // true : 会保存初始化信息. 原状态信息 , false不会
        cache.start(true);
        // 所以这里可以直接拿到初始化信息
        System.out.println("初始化信息 ：" + new String(cache.getCurrentData().getData()));

        // 添加listener.
        cache.getListenable().addListener(new NodeCacheListener() {
            @Override
            public void nodeChanged() throws Exception {
                System.out.println("当前节点发生改变：" + new String(cache.getCurrentData().getData()));
            }
        });

        ExecutorService pool = Executors.newFixedThreadPool(5);

        IntStream.range(0, 10).forEach(value -> pool.execute(() -> {
            try {
                // 改变值. 并发改变
                client.setData().forPath(path, String.format("change-%d", value).getBytes());
            } catch (Exception e) {
                //
            }
        }));
    }
}
```

结果是什么? 

```java
当前节点发生改变：hello world - init
当前节点发生改变：change-9
```

是不是很吃惊. 因为这个回调本身没有做同步机制. 所以对于高的写入修改可能做不到及时的监听.  也正常. 



### `PathChildrenCache `

> ​	只监听改目录下的第一级子节点. 

```java
public class App3 {

    public static void main(String[] args) throws Exception {
        // 启动zk客户端
        CuratorFramework client = Zk.getCuratorFramework();

        // 先删除
        String path = "/test-watcher";
        client.delete().deletingChildrenIfNeeded().forPath(path);
        // 创建当前节点
        client.create().withMode(CreateMode.PERSISTENT).forPath(path, "hello world - 1".getBytes());

        // 创建一个 PathChildrenCache
        PathChildrenCache childrenCache = new PathChildrenCache(client, path, true);

        /**
         * NORMAL:正常模式
         * BUILD_INTIAL_CACHE:启动什么事件都不输出
         * POST_INITAL_CACHE:启动会输出初始化事件
         */
        childrenCache.start(PathChildrenCache.StartMode.NORMAL);

        // 添加listener.
        childrenCache.getListenable().addListener(new PathChildrenCacheListener() {
            @Override
            public void childEvent(CuratorFramework client, PathChildrenCacheEvent event) throws Exception {
                System.out.println(String.format("path: %s, type: %s,value: %s.", event.getData().getPath(), event.getType(), new String(event.getData().getData())));

            }
        });
        ExecutorService pool = Executors.newFixedThreadPool(5);
        IntStream.range(0, 10).forEach(value -> pool.execute(() -> {
            try {
                String path_c = client.create().creatingParentsIfNeeded().withMode(CreateMode.EPHEMERAL).forPath(path  + path + "_child_" + value, "init".getBytes());
                client.setData().forPath(path_c, String.format("change-%d", value).getBytes());
            } catch (Exception e) {
                System.out.println(e.getMessage());
            }
        }));
    }
}
```

输出 :  我发现它不能监听子节点的创建.

```java
path: /test-watcher/test-watcher_child_4, type: CHILD_ADDED,value: init.
path: /test-watcher/test-watcher_child_3, type: CHILD_ADDED,value: init.
path: /test-watcher/test-watcher_child_2, type: CHILD_ADDED,value: init.
path: /test-watcher/test-watcher_child_1, type: CHILD_ADDED,value: init.
path: /test-watcher/test-watcher_child_0, type: CHILD_ADDED,value: init.
path: /test-watcher/test-watcher_child_1, type: CHILD_UPDATED,value: change-1.
path: /test-watcher/test-watcher_child_2, type: CHILD_UPDATED,value: change-2.
path: /test-watcher/test-watcher_child_0, type: CHILD_UPDATED,value: change-0.
path: /test-watcher/test-watcher_child_4, type: CHILD_UPDATED,value: change-4.
path: /test-watcher/test-watcher_child_3, type: CHILD_UPDATED,value: change-3.
path: /test-watcher/test-watcher_child_8, type: CHILD_ADDED,value: change-8.
path: /test-watcher/test-watcher_child_7, type: CHILD_ADDED,value: change-7.
path: /test-watcher/test-watcher_child_6, type: CHILD_ADDED,value: change-6.
path: /test-watcher/test-watcher_child_5, type: CHILD_ADDED,value: change-5.
path: /test-watcher/test-watcher_child_9, type: CHILD_ADDED,value: change-9.
```

反正我感觉监听不准确.  并且还有点乱. 无非是因为 zk 是一个 写少读多的场景. 

### `TreeCache ` 

> ​	这个很简单其实就是 , 子级目录也监听. 

```java
public static void main(String[] args) throws Exception {
    // 启动zk客户端
    CuratorFramework client = Zk.getCuratorFramework();

    // 先删除
    String path = "/test-watcher";
    client.delete().deletingChildrenIfNeeded().forPath(path);
    // 创建当前节点
    client.create().withMode(CreateMode.PERSISTENT).forPath(path, "init".getBytes());

    TreeCache cache = new TreeCache(client, path);
    cache.getListenable().addListener(new TreeCacheListener() {
        @Override
        public void childEvent(CuratorFramework client, TreeCacheEvent event) throws Exception {

            // todo
        }
    });
}
```

基本写法一样. 



curator 提供了个demo其实蛮好用的. 一个命令行工具, 可以进行 crud操作 , 有些人不懂命令行如何做的 , 其实就是下面这么做的.  代码逻辑很简单, 可以自己看看. 

```java
/**
 * An example of the PathChildrenCache. The example "harness" is a command processor
 * that allows adding/updating/removed nodes in a path. A PathChildrenCache keeps a
 * cache of these changes and outputs when updates occurs.
 */
public class PathCacheExample {
    private static final String PATH = "/example/cache";

    public static void main(String[] args) throws Exception {
        TestingServer server = new TestingServer();
        CuratorFramework client = null;
        PathChildrenCache cache = null;
        try {
            client = CuratorFrameworkFactory.newClient(server.getConnectString(), new ExponentialBackoffRetry(1000, 3));
            client.start();

            // in this example we will cache data. Notice that this is optional.
            cache = new PathChildrenCache(client, PATH, true);
            cache.start();

            processCommands(client, cache);
        } finally {
            TimeUnit.SECONDS.sleep(5);
            CloseableUtils.closeQuietly(cache);
            CloseableUtils.closeQuietly(client);
            CloseableUtils.closeQuietly(server);
        }
    }


    private static void addListener(PathChildrenCache cache) {
        // a PathChildrenCacheListener is optional. Here, it's used just to log changes
        PathChildrenCacheListener listener = new PathChildrenCacheListener() {
            @Override
            public void childEvent(CuratorFramework client, PathChildrenCacheEvent event) throws Exception {
                switch (event.getType()) {
                    case CHILD_ADDED: {
                        System.out.println("Node added: " + ZKPaths.getNodeFromPath(event.getData().getPath()));
                        break;
                    }

                    case CHILD_UPDATED: {
                        System.out.println("Node changed: " + ZKPaths.getNodeFromPath(event.getData().getPath()));
                        break;
                    }

                    case CHILD_REMOVED: {
                        System.out.println("Node removed: " + ZKPaths.getNodeFromPath(event.getData().getPath()));
                        break;
                    }
                }
            }
        };
        cache.getListenable().addListener(listener);
    }

    private static void processCommands(CuratorFramework client, PathChildrenCache cache) throws Exception {
        // More scaffolding that does a simple command line processor

        printHelp();

        List<ExampleServer> servers = Lists.newArrayList();
        try {
            addListener(cache);

            BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
            boolean done = false;
            while (!done) {
                System.out.print("> ");

                String line = in.readLine();
                if (line == null) {
                    System.out.println("===null===");
                    break;
                }

                String command = line.trim();
                String[] parts = command.split("\\s");
                if (parts.length == 0) {
                    continue;
                }
                String operation = parts[0];
                String args[] = Arrays.copyOfRange(parts, 1, parts.length);

                if (operation.equalsIgnoreCase("help") || operation.equalsIgnoreCase("?")) {
                    printHelp();
                } else if (operation.equalsIgnoreCase("q") || operation.equalsIgnoreCase("quit")) {
                    done = true;
                } else if (operation.equals("set")) {
                    setValue(client, command, args);
                } else if (operation.equals("remove")) {
                    remove(client, command, args);
                } else if (operation.equals("list")) {
                    list(cache);
                }

                Thread.sleep(1000); // just to allow the console output to catch up
            }
        } finally {
            for (ExampleServer server : servers) {
                CloseableUtils.closeQuietly(server);
            }
        }
    }

    private static void list(PathChildrenCache cache) {
        if (cache.getCurrentData().size() == 0) {
            System.out.println("* empty *");
        } else {
            for (ChildData data : cache.getCurrentData()) {
                System.out.println(data.getPath() + " = " + new String(data.getData()));
            }
        }
    }

    private static void remove(CuratorFramework client, String command, String[] args) throws Exception {
        if (args.length != 1) {
            System.err.println("syntax error (expected remove <path>): " + command);
            return;
        }

        String name = args[0];
        if (name.contains("/")) {
            System.err.println("Invalid node name" + name);
            return;
        }
        String path = ZKPaths.makePath(PATH, name);

        try {
            client.delete().forPath(path);
        } catch (KeeperException.NoNodeException e) {
            // ignore
        }
    }

    private static void setValue(CuratorFramework client, String command, String[] args) throws Exception {
        if (args.length != 2) {
            System.err.println("syntax error (expected set <path> <value>): " + command);
            return;
        }

        String name = args[0];
        if (name.contains("/")) {
            System.err.println("Invalid node name" + name);
            return;
        }
        String path = ZKPaths.makePath(PATH, name);

        byte[] bytes = args[1].getBytes();
        try {
            client.setData().forPath(path, bytes);
        } catch (KeeperException.NoNodeException e) {
            client.create().creatingParentContainersIfNeeded().forPath(path, bytes);
        }
    }

    private static void printHelp() {
        System.out.println("An example of using PathChildrenCache. This example is driven by entering commands at the prompt:\n");
        System.out.println("set <name> <value>: Adds or updates a node with the given name");
        System.out.println("remove <name>: Deletes the node with the given name");
        System.out.println("list: List the nodes/values in the cache");
        System.out.println("quit: Quit the example");
        System.out.println();
    }
}
```



## 3 .事务支持

单个事件并不构成事务. 但是当多个操作进行时, 需要事务.  

这个是官方提供的例子. 主要是 创建, 修改, 删除 , 这三个操作是原子性操作, 要么全部成功, 要么全部失败. 就是个这. 

```java
public static Collection<CuratorTransactionResult> transaction(CuratorFramework client) throws Exception {
    // this example shows how to use ZooKeeper's transactions

    CuratorOp createOp = client.transactionOp().create().forPath("/example", "some data".getBytes());
    CuratorOp setDataOp = client.transactionOp().setData().forPath("/example", "other data".getBytes());
    CuratorOp deleteOp = client.transactionOp().delete().forPath("/example");


    Collection<CuratorTransactionResult> results = client.transaction().forOperations(createOp, setDataOp, deleteOp);

    for (CuratorTransactionResult result : results) {
        System.out.println(result.getForPath() + " - " + result.getType());
    }

    return results;
}
```



基本实现很简单

```java
@Override
public List<CuratorTransactionResult> forOperations(List<CuratorOp> operations) throws Exception
{
    operations = Preconditions.checkNotNull(operations, "operations cannot be null");
    Preconditions.checkArgument(!operations.isEmpty(), "operations list cannot be empty");

// 记录. 这个继承了zk的 `Iterable<Op>` , 后面再说有啥用
    CuratorMultiTransactionRecord record = new CuratorMultiTransactionRecord();
    // 遍历一遍
    for ( CuratorOp curatorOp : operations )
    {
        Schema schema = client.getSchemaSet().getSchema(curatorOp.getTypeAndPath().getForPath());
        // 设置记录.
        record.add(curatorOp.get(), curatorOp.getTypeAndPath().getType(), curatorOp.getTypeAndPath().getForPath());
        // 下面都是做一些信息的判断. 
        if ( (curatorOp.get().getType() == ZooDefs.OpCode.create) || (curatorOp.get().getType() == ZooDefs.OpCode.createContainer) )
        {
            CreateRequest createRequest = (CreateRequest)curatorOp.get().toRequestRecord();
            CreateMode createMode;
            if ( client.isZk34CompatibilityMode() )
            {
                try
                {
                    createMode = CreateMode.fromFlag(createRequest.getFlags());
                }
                catch ( KeeperException.BadArgumentsException dummy )
                {
                    createMode = CreateMode.PERSISTENT;
                }
            }
            else
            {
                createMode = CreateMode.fromFlag(createRequest.getFlags(), CreateMode.PERSISTENT);
            }
            schema.validateCreate(createMode, createRequest.getPath(), createRequest.getData(), createRequest.getAcl());
        }
        else if ( (curatorOp.get().getType() == ZooDefs.OpCode.delete) || (curatorOp.get().getType() == ZooDefs.OpCode.deleteContainer) )
        {
            DeleteRequest deleteRequest = (DeleteRequest)curatorOp.get().toRequestRecord();
            schema.validateDelete(deleteRequest.getPath());
        }
        else if ( curatorOp.get().getType() == ZooDefs.OpCode.setData )
        {
            SetDataRequest setDataRequest = (SetDataRequest)curatorOp.get().toRequestRecord();
            schema.validateGeneral(setDataRequest.getPath(), setDataRequest.getData(), null);
        }
    }

// 是不是异步操作. 我们这里不做展开. 
    if ( backgrounding.inBackground() )
    {
        client.processBackgroundOperation(new OperationAndData<>(this, record, backgrounding.getCallback(), null, backgrounding.getContext(), null), null);
        return null;
    }
    else
    {
    // 同步的话, 执行这个方法. 
        return forOperationsInForeground(record);
    }
}

```

`forOperationsInForeground` 方法. 

```java
private List<CuratorTransactionResult> forOperationsInForeground(final CuratorMultiTransactionRecord record) throws Exception
{
    TimeTrace trace = client.getZookeeperClient().startTracer("CuratorMultiTransactionImpl-Foreground");
    // 调用callWithRetry方法进行commit. 
    List<OpResult> responseData = RetryLoop.callWithRetry
    (
        client.getZookeeperClient(),
        new Callable<List<OpResult>>()
        {
            @Override
            public List<OpResult> call() throws Exception
            {
            // 最终操作还是得使用 `getZooKeeper().multi()` 方法
            // 这里其实就指向了事务提交. 
                return client.getZooKeeper().multi(record);
            }
        }
    );
    // 这个提交其实不是事务提交的意思, 而是记录一个信息. 
    trace.commit();

    return CuratorTransactionImpl.wrapResults(client, responseData, record);
}
```



点击进去 `multi` 实现也很简单. 

最终会走到 

```java
sendThread.getClientCnxnSocket().packetAdded();
```

所以zk 的客户端的话, 其实是维护了一个队列 ,所以我们的提交基本是有序的 , 至于发送, 可以是依靠一些状态量维护的吧. 



其中zk客户端提供了 `Transaction` 这个类, 也封装了 multi 操作, 所以基本上就是能简单操作就简单操作吧. 哈哈哈.  基本实现也是上诉的.  事务无非就是将三个操作一起发送了, 类似于 redis的lua脚本一样的. 







