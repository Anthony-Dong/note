# 分布式ID的需求

## 需求

现实中很多业务都有**生成唯一ID**的需求，例如：

- 用户ID
- 微博ID
- 聊天消息ID
- 帖子ID
- 订单ID







## 分布式ID的要求

1.全局唯一性：不能出现重复的ID号，既然是唯一标识，这是最基本的要求。
2.趋势递增：在MySQL InnoDB引擎中使用的是聚集索引，由于多数RDBMS使用B-tree的数据结构来存储索引数据，在主键的选择上面我们应该尽量使用有序的主键保证写入性能。
3.单调递增：保证下一个ID一定大于上一个ID，例如事务版本号、IM增量消息、排序等特殊需求。
4.信息安全：如果ID是连续的，恶意用户的扒取工作就非常容易做了，直接按照顺序下载指定URL即可；如果是订单号就更危险了，竞对可以直接知道我们一天的单量。所以在一些应用场景下，会需要ID无规则、不规则。
5.分布式id里面最好包含时间戳，这样就能够在开发中快速了解这个分布式id的生成时间

> ​	上述123对应三类不同的场景，3和4需求还是互斥的，所以无法使用同一个方案满足。



## 分布式ID系统要求

1. 可用性高：就是我用户发了一个获取分布式id的请求，那么你服务器就要保证99.999%的情况下给我创建一个分布式id
2. 延迟低：就是我用户给你一个获取分布式id的请求，那么你服务器给我创建一个分布式id的速度就要快
3. 高QPS：这个就是用户一下子有10万个创建分布式id请求同时过去了，那么你服务器要顶的住，你要一下子给我成功创建10万个分布式id





![](/Users/sgcx015/Library/Application Support/typora-user-images/image-20200427135718271.png)

The Foundation's mission iS to make cloud native computing ubiquitous. The CNCF Cloud Native Definition v1.0 says:
Cloud native technologies empower organizations to build and run scalable applications in modern, dynamic environments  such as public, private, and hybrid clouds. Containers, service meshes, microservices, immutable infrastructure, and declarative APls exemplify this approach.
**These techniques enable loosely coupled systems that are resilient, manageable, and observable.Combined with robust automation, they allow engineers to make high-impact changes frequently and predictably with minimal toil.**
The Cloud Native Computing Foundation seeks to drive adoption of this paradigm by fostering and sustaining an ecosystem of open source, vendor-neutral projects. We democratize state-of-the-art patterns to make these innovations accessible for everyone.

### resilient  -- 韧性

### manageable -- 可管理

### observable  -- 可观测

