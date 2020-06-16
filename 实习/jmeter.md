# JMeter



安装：https://archive.apache.org/dist/jmeter/binaries/apache-jmeter-5.0.zip

修改中文字体：vim jmeter.properties 

```properties
#Preferred GUI language. Comment out to use the JVM default locale's language.
language=zh_CN
```



## 简单使用

![image-20200513185626705](https://tyut.oss-accelerate.aliyuncs.com/image/2020-50-55/9ed1c123-8317-41dc-bb36-e0f1562d5f6c.png?x-oss-process=style/template01)

进入右键，生成线程组，其次是http ， 结果树， 图像结果，聚合报告

### 线程组

![image-20200513191622231](https://tyut.oss-accelerate.aliyuncs.com/image/2020-50-55/7ced782a-06b6-427a-9c7e-a7ef2cf86e11.png?x-oss-process=style/template01)



核心其实是  `ramp-up`字段含义 ， 就是多少s启动完全部线程。

循环次数是 ： 每个线程的循环次数。



### Http 请求设置

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-50-55/757923b1-add7-4b1e-80de-d6863bcf4545.jpg?x-oss-process=style/template01)





更多：https://www.jianshu.com/p/77a896780528

## 聚合报告

关于聚合报告字段含义：https://www.cnblogs.com/whitewasher/p/7656824.html

![image-20200513191024304](https://tyut.oss-accelerate.aliyuncs.com/image/2020-50-55/7fd2386f-ae9d-4b67-944b-77f2b085cdb7.png?x-oss-process=style/template01)

1、Label - 请求对应的name属性值。

2、 Samples - 具有相同标号的样本数，总的发出请求数。

3、Average - 请求的平均响应时间(ms)。

4、Median - 50%的样本都没有超过这个时间(ms)。这个值是指把所有数据按由小到大将其排列，就是排列在第50%的值。

5、90% Line - 90%的样本都没有超过这个时间(ms)。这个值是指把所有数据按由小到大将其排列，就是排列在第90%的值。

6、95% Line - 95%的样本都没有超过这个时间(ms)。这个值是指把所有数据按由小到大将其排列，就是排列在第95%的值。

7、99% Line - 99%的样本都没有超过这个时间(ms)。这个值是指把所有数据按由小到大将其排列，就是排列在第99%的值。

8、Min - 最小响应时间(ms)。

9、Max - 最大响应时间(ms)。

10、Error % - 本次测试中，有错误请求的百分比。

11、Throughput - 吞吐量是以每秒/分钟/小时的请求量来度量的。这里表示每秒完成的请求数。

12、Received KB/sec - 收到的千字节每秒的吞吐量测试。

13、Sent KB/sec - 发送的千字节每秒的吞吐量测试。



## 结果树

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-50-55/2d901511-2940-46d7-99d3-cf2d94481c21.jpg?x-oss-process=style/template01)



可以看到具体的响应结果。