# Dubbo-SPI拓展

> ​	SPI 全称为 Service Provider Interface，是一种服务发现机制。SPI 的本质是将接口实现类的全限定名配置在文件中，并由服务加载器读取配置文件，加载实现类。这样可以在运行时，动态为接口替换实现类。正因此特性，我们可以很容易的通过 SPI 机制为我们的程序提供拓展功能。SPI 机制在第三方框架中也有所应用，比如 Dubbo 就是通过 SPI 机制加载所有的组件。不过，Dubbo 并未使用 Java 原生的 SPI 机制，而是对其进行了增强，使其能够更好的满足需求。在 Dubbo 中，SPI 是一个非常重要的模块。基于 SPI，我们可以很容易的对 Dubbo 进行拓展。
>
> ​	[链接 :](http://dubbo.apache.org/zh-cn/docs/source_code_guide/dubbo-spi.html)   http://dubbo.apache.org/zh-cn/docs/source_code_guide/dubbo-spi.html



## 1. 如何自定义 , 如何添加进去

### 1. 我们就定义一个实现最简单的

```java
public class MyFilter implements Filter {

    @Override
    public Result invoke(Invoker<?> invoker, Invocation invocation) throws RpcException {
        System.out.println("MyFilter 被调用");
        return invoker.invoke(invocation);
    }
}
```

### 2. 添加进去

1) 在 `META-INF/dubbo/com.alibaba.dubbo.rpc.Filter` 中添加一句

```java
myFilter = com.example.dubboprovider.spi.MyFilter
```

2) 在配置中申明

```properties
# pro配置 , 输入你的 filter的 name , 也就是id
dubbo.provider.filter=myFilter

#  con配置
dubbo.consumer.filter= myFilter
```

3) 测试 - 还是我们 Dubbo入门的测试用例

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-12/c8bb955a-ebb2-4366-b0ef-38b88e99da16.jpg?x-oss-process=style/template01)

我们发现确实被调用了 



## 2. 使用SPI

比如我们负载均衡 , 当消费者调用服务提供者时 ,使用loadbalance , 我们使用的随机算法 ,在消费端配置就行了

`dubbo.consumer.loadbalance=random` , 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-12/27290674-e3d6-4b91-b6c5-bfa6cf8c1e7e.jpg?x-oss-process=style/template01)



## 3. 总结 

Dubbo对外全部采用的他的SPI机制, 但是 Dubbo 并没有让用户自己开发SPI 接口, 这个就比较坑, 只能用他内部的SPI 对外实现,  不过足够了 , 希望哪天Dubbo 可以把SPI 分离出来, 其实他是一个很好的IOC容器 ,

