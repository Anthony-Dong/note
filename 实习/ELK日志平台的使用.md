# ELK日志的使用

### 1、Discover界面

![image-20200509224458335](/Users/sgcx015/Library/Application Support/typora-user-images/image-20200509224458335.png)

基本就是这些了

![image-20200509224559367](/Users/sgcx015/Library/Application Support/typora-user-images/image-20200509224559367.png)

这个是日志详情，也就是每个字段的信息，这些都是通过 logstach聚合起来的数据，通常我们的日志体一般在message里面。

所以一般日志操作也是在message里面。

### 过滤操作学习

> ​	蹭着晚上没人，实操一波，es机器600G的JVM，太强了。

#### 1、全文搜索

这个会将整个日志作为搜索单位，也就是各个字段内容都算在内，字段本身不算过滤字段。

搜索很简单，直接输入 “warning” ， 就可以过滤包含“warning”的日志了， 比如还可以 且或非操作，  比如 “warning” AND “error” 他会自动的搜索包含“waring”和“error”的日志，当一条日志中同时满足这两个会返回，

或操作是 “OR” ，



#### 2、精确查询