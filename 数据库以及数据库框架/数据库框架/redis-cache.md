# redis - cache

@EnableCaching: 程序入口开启使用缓存@EnableCaching 等价于 cache:annotation-driven/ 。能够在服务类方法上标注@Cacheable

@CacheConfig : **这个注解在类上使用，用来描述该类中所有方法使用的缓存名称，当然也可以不使用该注解，直接在具体的缓存注解上配置名称**

**@Cachable**: 将方法的运行结果进行缓存;以后再要相同的数据,直接从缓存中获取,不用调用方法;默认情况下，缓存的key就是方法的参数，缓存的value就是方法的返回值。 有结果就不调用了

> ​	参数 :  cacheNames="books"
>
> ​				key="#map['bookid'].toString()"
>
> ​				sync="true"   (在多线程环境下，某些操作可能使用相同参数同步调用。默认情况下，缓存不锁定任何资源，可能导致多次计算，而违反了缓存的目的。对于这些特定的情况，属性 sync 可以指示底层将缓存锁住，使只有一个线程可以进入计算，而其他线程堵塞，直到返回结果更新到缓存中 )
>
> ​				keyGenerator 
>
> ​				condition =
>
> ​				@Cacheable(value=”testcache”,condition=”#userName.length()>2”)



**@CachePut**: @CachePut 的作用 主要针对方法配置，能够根据方法的请求参数对其结果进行缓存，和 @Cacheable 不同的是，它每次都会触发真实方法的调用

**@CacheEvict**: 这个注解一般加在删除方法上，当数据库中的数据删除后，相关的缓存数据也要自动清除，该注解在使用的时候也可以配置按照某种条件删除（condition属性）或者或者配置清除所有缓存（allEntries属性）



```java

@Transactional
@CacheConfig(cacheNames = "order")
@Service
public class IOrderService implements OrderService{


    @Autowired
    private OrderMapper orderMapper;


    @CachePut(key = "#result.id")
    @Override
    public Orders updateOrder(Orders orders) {
        int i = orderMapper.updateById(orders);
        if (i != 1) {
            throw new RuntimeException("error result ");
        }
        return orders;
    }


    @CacheEvict(key = "#p0", allEntries = true)
    @Override
    public void deleteOrdersById(Integer id) {
        int i = orderMapper.deleteById(id);
        if (i != 1) {
            throw new RuntimeException("error result ");
        }

    }


    @Cacheable(key = "#id")
    @Override
    public Orders queryOrdersByid(Integer id) {
        Orders orders = orderMapper.selectById(id);

        if (ObjectUtils.isEmpty(orders)) {
            throw new RuntimeException(" error result is  empty");
        }

        return orderMapper.selectById(id);
    }


    /**
     * 插入的时候 是不知道 主键 id 的  --------------- ,所以 没必要刷新缓存 ,除非你指定id
     * @param orders
     * @return
     */
//    @Cacheable(key = "#orders.getId()")
    @Override
    public Orders insertOrdersByid(Orders orders) {
        int insert = orderMapper.insert(orders);

        

        if (insert != 1) {
            throw new RuntimeException("error result ");
        }

        return orders;
    }

}

```

|               |            |                                                              |                            |
| ------------- | ---------- | ------------------------------------------------------------ | -------------------------- |
| methodName    | root对象   | 当前被调用的方法名                                           | `#root.methodname`         |
| method        | root对象   | 当前被调用的方法                                             | `#root.method.name`        |
| target        | root对象   | 当前被调用的目标对象实例                                     | `#root.target`             |
| targetClass   | root对象   | 当前被调用的目标对象的类                                     | `#root.targetClass`        |
| args          | root对象   | 当前被调用的方法的参数列表                                   | `#root.args[0]`  或者`#p0` |
| caches        | root对象   | 当前方法调用使用的缓存列表                                   | `#root.caches[0].name`     |
| Argument Name | 执行上下文 | 当前被调用的方法的参数，如findArtisan(Artisan artisan),可以通过#artsian.id获得参数 | `#artsian.id`              |
| result        | 执行上下文 | 方法执行后的返回值（仅当方法执行后的判断有效，如 unless cacheEvict的beforeInvocation=false） | `#result`                  |









