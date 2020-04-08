# Alibaba - Arthas工具

## 介绍

> ​	Arthas(中文名: 阿尔萨斯) , 介绍就在 [https://github.com/alibaba/arthas/blob/master/README_CN.md](https://github.com/alibaba/arthas/blob/master/README_CN.md)   , 十分详细, 就是一个线上问题查找工具, 由于JDK的工具 , 并不适合所有人, 而且有些功能不全面. 所以基于这个. 就出现了这个Arthas. 

用之前, 问一些问题 ? 

1. 这个类从哪个 jar 包加载的？为什么会报各种类相关的 Exception？
2. 我改的代码为什么没有执行到？难道是我没 commit？分支搞错了？
3. 遇到问题无法在线上 debug，难道只能通过加日志再重新发布吗？
4. 线上遇到某个用户的数据处理有问题，但线上同样无法 debug，线下无法重现！
5. 是否有一个全局视角来查看系统的运行状况？
6. 有什么办法可以监控到JVM的实时运行状态？
7. 怎么快速定位应用的热点，生成火焰图？

 哈哈哈. 好吧, 如果没有工作确实不知道啥问题  . 



文档的详细介绍都是在 https://alibaba.github.io/arthas 这里.  我觉得比我写, 要好的多. 所以留个链接  . 基本看看这个  , 自己敲一遍命令就够了. 



> ​	其实阿尔萨斯,  实现原理也是Java提供的一些基础工具类实现的, 这些太底层的东西确实大多数人不了解 . 

这里有一篇文章 , 教你如何实现 `redefine ` 功能的实现. 所以可以感兴趣看看 . 链接 : [https://www.cnblogs.com/goodAndyxublog/p/11880314.html](https://www.cnblogs.com/goodAndyxublog/p/11880314.html)  

## 细节

比较好玩的是 redefine . 不过这个玩意 . 怎么说, 需要你拿一个已经编译好的class文件, 可以重新加载到JVM的内存中.  但是mc直接编译的话. 大多都编译不通 , 最好借助第三方工具. 



还有就是 tt 命令的遇见方法重载的问题 . 可以通过 `tt -t com.example.springbeanconfig.arthas.Demo echo  'params[0] instanceof Integer'`   最后这个 `'params[0] instanceof Integer'` 意思就是第一个参数类型是Integer , 这个还是要知道的.  tt命令会将每次调用保存起来, 就是查询的记录下索引. 可以根据索引找到具体细节. 



还有很多后续补充. 

## 原理

其实也就是基于Java提供的一些工具类去实现的. 想看源码的可以导入 依赖. 

```xml
<!-- https://mvnrepository.com/artifact/com.taobao.arthas/arthas-boot -->
<dependency>
    <groupId>com.taobao.arthas</groupId>
    <artifactId>arthas-boot</artifactId>
    <version>3.1.7</version>
</dependency>
```



Arthas的具体实现, 其实是我们忽略的细节. 其中 issues部分就表述了很多功能的实现 : [https://github.com/alibaba/arthas/issues?q=label%3Auser-case](https://github.com/alibaba/arthas/issues?q=label%3Auser-case)



## idea 插件

最后安利一波插件 ,  arthas的插件, 使用起来很方便, idea的插件, 主要是用来生成命令的  . 

地址 : [https://plugins.jetbrains.com/plugin/13581-arthas-idea](https://plugins.jetbrains.com/plugin/13581-arthas-idea)

使用介绍 : [https://www.yuque.com/docs/share/fa77c7b4-c016-4de6-9fa3-58ef25a97948?#](https://www.yuque.com/docs/share/fa77c7b4-c016-4de6-9fa3-58ef25a97948?#)

使用起来很方便.    

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-30-33/c5c3137f-3769-478e-917d-6ab2d2d27a5c.png?x-oss-process=style/template01)

