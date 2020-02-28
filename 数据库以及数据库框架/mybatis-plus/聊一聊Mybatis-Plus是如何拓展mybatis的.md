# 聊一聊MyBatis-Plus是如何拓展mybatis的

[MyBatis-Plus 官网 :](https://mp.baomidou.com/)   [https://mp.baomidou.com/](https://mp.baomidou.com/)

[Github 地址 :](https://github.com/baomidou/mybatis-plus)  [https://github.com/baomidou/mybatis-plus]( https://github.com/baomidou/mybatis-plus)

> ​	首先我们只解释为什么他可以做到不写SQL . 我们回答这个问题. 其实拓展Mybatis是个很麻烦的事 , page-helper依靠的事Mybatis自带的拦截器. 会代理executor执行. 可以看看 `org.apache.ibatis.plugin.Plugin` 和 `org.apache.ibatis.plugin.InterceptorChain`  我一直不懂pluginall 是做啥了, 一个循环迭代, 那么为啥要循环, 但是代理层并没有迭代哇. 我不懂,有小伙伴懂可以点我联系方式. 



> ​	说完Mybatis我们聊聊 Plush咋做的.  其实由了解过TKmapper也和他做法相似. 



简单开始 : 

```java
public interface OrderMapper extends BaseMapper<Orders> {

}
```

为啥要实现这个接口呢.  接口一般是代理 , 所以其实他修改了原来Mybatis的代理逻辑.  在 `com.baomidou.mybatisplus.core.override.PageMapperProxy` 这个类里.  其实他基本重写了Mybatis注册逻辑. 但是流程不变, 比如 

他封装了一个 `MybatisConfiguration` 继承了 `Configuration`  , 

其次就是`MybatisMapperRegistry` 和 `MapperRegistry`   ,  

`PageMapperProxy` 和 `MapperProxy`  , 

  `PageMapperMethod` 和 `MapperMethod`  , 

`MybatisSqlSessionTemplate`  和 `SqlSessionTemplate`代理关系.  (这里是Spring的)



流程不说了, 就是Configuration就是元信息. 给了SqlSession. 其次我们拿Mapper依靠的是 MapperRegister拿到一个代理对象. 剩余的流程就是Executor不断的委托代理执行, 其实也类似于装饰着模式. 就是a只做缓存, 查询的交给下一级 . 可以说包装了一层缓存. 





其实说到这里也很简单 , 获取一个SQL 就行了, 所以有一个接口  `com.baomidou.mybatisplus.core.conditions.ISqlSegment`  来获取SQL . 下层有 

他的Wrapper 类了, 就负责包装SQL .   `com.baomidou.mybatisplus.core.conditions.Wrapper`  来实现的 . (注意版本不一样, 可能也不一样 . 我是3.0) . 



这里负责构建SQL .

```java
@Test
public void testSelect() {
    LambdaQueryWrapper<Orders> eq1 = new LambdaQueryWrapper<Orders>().eq(Orders::getName, "小李");
    System.out.println(eq1.getSqlSegment());
}
```

会输出 : 

```java
name = #{ew.paramNameValuePairs.MPGENVAL1}
```

其实如果有人Debug不知道ew是什么? 其实就是个占位符, ew的意思是 `EntityWrapper` , 是2.X版本的玩意, 但是迁移把他删了. 

就是依靠这个玩意来构建SQL . 最后把他给 SqlStatement. 也就能获取最后的SQL了. 





反正我就记录一下.  也就是 上层全部有MybatisPlush进行封装了. 但是核心部分还是Mybatis .  MybatisPlus也就是基于原来进行继续来覆盖核心方法 . 或者增强. 













