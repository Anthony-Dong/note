# Alibaba - Sentinel

> ​	A lightweight powerful flow control component enabling reliability and monitoring for microservices. (轻量级的流量控制、熔断降级 Java 库) .` sentinel `如果你学过redis会知道这个啥意思.他有一个模式就是sentinel模式, 叫做哨兵模式.

阿里巴巴写的框架, 中文支持比较好  , [链接](https://github.com/alibaba/Sentinel/wiki/%E4%BB%8B%E7%BB%8D) : [https://github.com/alibaba/Sentinel/wiki/%E4%BB%8B%E7%BB%8D](https://github.com/alibaba/Sentinel/wiki/介绍)

​    Sentinel主要就是轻量级 , 如果使用过 Hystrix的话, 大家的感受是啥, 我觉得就是配置项, 确实不符合程序的编写风格 , 虽然以构造方法传入, 但是大量的配置让人难以下手.  他对于颗粒度的分离并没有sentinel做的比较好. 还有功能方法 , Hystrix异步采用的是`ReactiveX` 框架, 是一个Java异步编程的框架. 喜欢的可以去[Github上看看](https://github.com/ReactiveX/RxJava) : https://github.com/ReactiveX/RxJava . 

对于Hystrix主要有两种模型  , 一种是线程,一种是信号量来控制. 对于性能消耗比较大. 而且我并没有发现他的限流体现在哪 , 更多的是熔断保护.

​	**Sentinel 主要是提供了5个核心规则(规则就相当于配置, 相对与hystrix更加解耦) , 都需要在程序启动前注入进去, 并且支持运行时修改等等操作.** **流量控制规则**、**熔断降级规则**、**系统保护规则**、**来源访问控制规则** 和 **热点参数规则**。 对于多种方式的使用, [请看这个链接](https://github.com/alibaba/Sentinel/wiki/%E5%A6%82%E4%BD%95%E4%BD%BF%E7%94%A8) : https://github.com/alibaba/Sentinel/wiki/%E5%A6%82%E4%BD%95%E4%BD%BF%E7%94%A8 .  文章写得很详细, 所以我就不展示如何使用了. 这篇基本可以满足你的日常需求了.. 再配上官方提供Demo. 

如果你想要学习官方提供了很多的Demo. [链接](https://github.com/alibaba/Sentinel/tree/master/sentinel-demo) : https://github.com/alibaba/Sentinel/tree/master/sentinel-demo

### Slot 核心

> ​	版本全部是 1.7.1 是官网最新的, 

[sentinel的核心工作流程](https://github.com/alibaba/Sentinel/wiki/Sentinel%E5%B7%A5%E4%BD%9C%E4%B8%BB%E6%B5%81%E7%A8%8B)  : [https://github.com/alibaba/Sentinel/wiki/Sentinel工作主流程](https://github.com/alibaba/Sentinel/wiki/Sentinel工作主流程)

他的这么多功能其实依靠于他的 `slot chain` , 插槽 . 每个插槽都有不同的功能. 他的核心内容全部依赖与Slot工作. 通过SPI 拓展 `SlotChainBuilder` , 程序启动时会将其初始化完成.  用户可以通过在`chan`中添加执行链进行控制.

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/5fd2efed-5a47-49dc-840c-3c3481696f25.png)

我们查看他的 builder, 默认的builder .  在`com/alibaba/csp/sentinel-core/1.7.1/sentinel-core-1.7.1.jar!/META-INF/services/com.alibaba.csp.sentinel.slotchain.SlotChainBuilder` 文件中.  以SPI的方式注入的. 

`com.alibaba.csp.sentinel.slotchain.SlotChainProvider#newSlotChain  `  这个静态方法加载`slotChainBuilder`  , 构建ProcessorSlotChain. 

```java
public static ProcessorSlotChain newSlotChain() {
    if (slotChainBuilder != null) {
        return slotChainBuilder.build();
    }

    // Resolve the slot chain builder SPI. 这里是一个SPI加载. 默认是DefaultSlotChainBuilder,可以通过`com.alibaba.csp.sentinel.spi.SpiOrder` 注解控制优先级
    slotChainBuilder = SpiLoader.loadFirstInstanceOrDefault(SlotChainBuilder.class, DefaultSlotChainBuilder.class);

    if (slotChainBuilder == null) {
        // Should not go through here.
        RecordLog.warn("[SlotChainProvider] Wrong state when resolving slot chain builder, using default");
        slotChainBuilder = new DefaultSlotChainBuilder();
    } else {
        RecordLog.info("[SlotChainProvider] Global slot chain builder resolved: "
            + slotChainBuilder.getClass().getCanonicalName());
    }
    // 最后调用返回.
    return slotChainBuilder.build();
}
```

这里是 `com.alibaba.csp.sentinel.slots.DefaultSlotChainBuilder` 实现. 

```java
public class DefaultSlotChainBuilder implements SlotChainBuilder {

    @Override
    public ProcessorSlotChain build() {
        ProcessorSlotChain chain = new DefaultProcessorSlotChain();
        // node
        chain.addLast(new NodeSelectorSlot());
        // 集群
        chain.addLast(new ClusterBuilderSlot());
        // 日志
        chain.addLast(new LogSlot());
        chain.addLast(new StatisticSlot());
        // 权限
        chain.addLast(new AuthoritySlot());
        // 系统
        chain.addLast(new SystemSlot());
        // 限流
        chain.addLast(new FlowSlot());
        // 降级
        chain.addLast(new DegradeSlot());

        return chain;
    }
}
```



**假如我们服务只需要一个限流服务, 我们不需要其他服务, 怎么办呢 . 很简单.** 

```java
public class DemoSlotChainBuilder implements SlotChainBuilder {

    @Override
    public ProcessorSlotChain build() {
        ProcessorSlotChain chain = new DefaultProcessorSlotChain();
        // 这三个必须加的. 因为要保存一些必要的信息.后面讲解
        chain.addLast(new NodeSelectorSlot());
        chain.addLast(new ClusterBuilderSlot());
        chain.addLast(new StatisticSlot());
        // 只加入限流服务
        chain.addLast(new FlowSlot());
        return chain;
    }
}
```

然后在 `META-INF/services/com.alibaba.csp.sentinel.slotchain.SlotChainBuilder` 文件中加入

```java
com.alibaba.csp.sentinel.demo.slot.DemoSlotChainBuilder
```

就可以了.  简单的限流项目

```java
public static void main(String[] args) {

    FlowRule rule = new FlowRule();
    // 限制APP.如果不写是不限制.这个意思是只能是app服务可以
    rule.setLimitApp("app");
    // 资源名称.
    rule.setResource("test");
    rule.setGrade(RuleConstant.FLOW_GRADE_QPS);
    rule.setCount(10);
    FlowRuleManager.loadRules(Collections.singletonList(rule));


    while (true) {
        // 第二个参数是 限制服务的auth. 第一个就是context名称无所谓
        Entry entry = null;
        try {
            ContextUtil.enter("test2", "app");
            // 资源名称. 需要去rule中找对应的服务名.没有则不执行.(一般都是以方法名为resource名的)
            entry = SphU.entry("test");
            System.out.println("I have pass the limiter.");
        } catch (BlockException ex) {
            System.err.println(String.format("block exception :%s", ex.getMessage()));
        } finally {
            if (entry != null) {
                entry.exit();
            }
            ContextUtil.exit();
        }
    }
}
```



 

> ​	一下这些都在[这篇文章中](https://github.com/alibaba/Sentinel/wiki/Sentinel%E5%B7%A5%E4%BD%9C%E4%B8%BB%E6%B5%81%E7%A8%8B) : [https://github.com/alibaba/Sentinel/wiki/Sentinel工作主流程](https://github.com/alibaba/Sentinel/wiki/Sentinel%E5%B7%A5%E4%BD%9C%E4%B8%BB%E6%B5%81%E7%A8%8B)

#### NodeSelectorSlot

这个 slot 主要负责收集资源的路径，并将这些资源的调用路径以树状结构存储起来，用于根据调用路径进行流量控制。 主要是就是生成 node 



#### ClusterBuilderSlot

此插槽用于构建资源的 `ClusterNode` 以及调用来源节点。`ClusterNode` 保持某个资源运行统计信息（响应时间、QPS、block 数目、线程数、异常数等）以及调用来源统计信息列表。调用来源的名称由 `ContextUtil.enter(contextName，origin)` 中的 `origin` 标记。可通过如下命令查看某个资源不同调用者的访问情况：`curl http://localhost:8719/origin?id=caller`：

```java
id: nodeA
idx origin  threadNum passedQps blockedQps totalQps aRt   1m-passed 1m-blocked 1m-total 
1   caller1 0         0         0          0        0     0         0          0        
2   caller2 0         0         0          0        0     0         0          0        
```



#### StatisticSlot

`StatisticSlot` 是 Sentinel 的核心功能插槽之一，用于统计实时的调用数据。

- `clusterNode`：资源唯一标识的 ClusterNode 的 runtime 统计
- `origin`：根据来自不同调用者的统计信息
- `defaultnode`: 根据上下文条目名称和资源 ID 的 runtime 统计
- 入口流量的统计

Sentinel 底层采用高性能的滑动窗口数据结构 `LeapArray` 来统计实时的秒级指标数据，可以很好地支撑写多于读的高并发场景。 对于限流算法[这一篇文章](https://anthony-dong.gitee.io/post/xian-liu-chang-jian-de-suan-fa-shi-xian/)我写过.,大家可以看看 https://anthony-dong.gitee.io/post/xian-liu-chang-jian-de-suan-fa-shi-xian/ . 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/09d30eda-b298-4465-b000-6e0b93e587de.png?x-oss-process=style/template01)

 





### Sentinel-Dashboard

这里我使用的是1.7.1的版本 , 你使用官方的源码编译可能出现一堆问题 , 自己编译的话可以改成了SpringBoot版本 .  你也可以使用官方提供的release版本. https://github.com/alibaba/Sentinel/releases

这个控制工具使用起来很简单, 这个Jar包直接运行启动就行了 . 

```java
java -Dserver.port=8080 -Dcsp.sentinel.dashboard.server=localhost:8080 -Dproject.name=sentinel-dashboard -jar sentinel-dashboard.jar
```

我们的普通服务只需要加入依赖 , 并且加入JVM参数 `-Dcsp.sentinel.dashboard.server=localhost:8080`

```xml
<dependency>
    <groupId>com.alibaba.csp</groupId>
    <artifactId>sentinel-transport-simple-http</artifactId>
    <version>1.7.1</version>
</dependency>
```

就可以了 , 监控中心就可以查看了,  

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/8ab75725-0058-45bd-bc91-2ee0363f3124.png?x-oss-process=style/template01)

功能很多, 也很nice . 提供热点参数配置之类的. 很简单 . 使用起来.  所以 sentinel用处非常棒. 

