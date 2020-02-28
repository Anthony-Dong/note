#  False Sharing 伪共享

> 伪共享, 其实对于我们现在来说,不了解也正常. 主要是效率问题, 他可以有效的提高效率, 是很高的效率. 



## 概念

这篇文章不错,

有关于伪共享的概念 , 下面这篇文章讲的不错, 以及缓存行的东西

 https://www.cnblogs.com/tong-yuan/p/FalseSharing.html 

这篇文章也不错 . 

https://my.oschina.net/u/3892023/blog/2999854

还有 Java并发编程之美 这本书也不错. 都讲到了 伪共享.  其实就是对于volatile数据的问题 . 因为volition数据每次都需要从 main memory中读取到缓存中然后寄存器.  其实对于 false sharing处理, 也不光局限于volatile , 主要是对于修改多的数据使用, 写比读多的数据. 

其中Java8中如果用户开启处理伪元素共享 , 需要加入注解 `sun.misc.Contended`, 同时还需要加入JVM参数  `-XX:-RestrictContended`



## MESI协议

多核CPU都有自己的专有缓存（一般为L1，L2），以及同一个CPU插槽之间的核共享的缓存（一般为L3）。不同核心的CPU缓存中难免会加载同样的数据，那么如何保证数据的一致性呢，就是MESI协议了。 
在MESI协议中，每个Cache line有4个状态，可用2个bit表示，它们分别是： 
M(Modified)：这行数据有效，数据被修改了，和内存中的数据不一致，数据只存在于本Cache中； 
E(Exclusive)：这行数据有效，数据和内存中的数据一致，数据只存在于本Cache中； 
S(Shared)：这行数据有效，数据和内存中的数据一致，数据存在于很多Cache中； 
I(Invalid)：这行数据无效。

那么，假设有一个变量i=3（应该是包括变量i的缓存块，块大小为缓存行大小）；已经加载到多核（a,b,c）的缓存中，此时该缓存行的状态为S；此时其中的一个核a改变了变量i的值，那么在核a中的当前缓存行的状态将变为M，b,c核中的当前缓存行状态将变为I。如下图：

