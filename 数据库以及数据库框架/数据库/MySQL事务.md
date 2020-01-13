# MySQL - 事务隔离界级别

> ​	JTA，即Java Transaction API，JTA允许应用程序执行分布式事务处理——在两个或多个网络计算机资源上访问并且更新数据。[JDBC](https://baike.baidu.com/item/JDBC)[驱动程序](https://baike.baidu.com/item/驱动程序)的JTA支持极大地增强了数据访问能力。

`isolation` 隔离的意思

> ​	隔离级别主要分为四种 : 读未提交, 读已提交 , 可重复读, 序列化 , 越来越强

可以通过下面这个查询当前事务等级

```sql
mysql> SELECT @@tx_isolation;
+-----------------+
| @@tx_isolation  |
+-----------------+
| REPEATABLE-READ |
+-----------------+
1 row in set, 1 warning (0.00 sec)
```

设置隔离级别

```sql
//设置read uncommitted级别：
set session transaction isolation level read uncommitted;

//设置read committed级别：
set session transaction isolation level read committed;

//设置repeatable read级别：
set session transaction isolation level repeatable read;

//设置serializable级别：
set session transaction isolation level serializable;
```

这个事务级别是相对于客户端的, 比如A客户端设置了,B客户端并没有改变

相关操作

```sql
 // 开启事务
 start transaction;
 
 // 回滚
 rollback;
 
 // 提交
 commit;
```

## 1. READ-UNCOMMITTED 读未提交

比如A,B都开启了事务, 此时A更新了数据 ,B此时读到了A更新的数据 ,但是此时A并未提交事务 . 所以出现了脏数据 , 

当A执行rollback 时 ,  B也会读到他回滚前的数据  , 

注意 : 就算B不开启事务, 也是这种情况

## 2.  READ-COMMITTED  读已提交

AB同时开启了事务 , A此时更新了数据 ,此时没有提交 ,B此时读到的数据是A没有更新之前的数据 , 只有当A提交了数据后 , B才能读到A修改后的数据 , 

注意 : B如果没有开启事务也是上述情况

## 3. REPEATABLE-READ 可重复读 (默认)

AB 同时开启事务  , A执行写操作, 此时A更新后的数据,B是可以读不到的 , 此时A将事务提交了, 此时B读到的数据还是A未修改的数据 , 但是当我们执行B的更新操作时 `update orders set count=6 where id=1 and count>8;`  此时去更新是失败的,是因为执行这个操作会执行一个查询操作 , 此时再去查询数据就会发现数据更改了 ..... 

## 4. SERIALIZABLE  序列化

不允许两个事物同时执行 , 就是A事物操作 开始, B事物全部在等待 ,只有当A事物commit;B事物才能执行



## 5. 总结

我真的觉得 第二种和第三种级别么区别 .... 无非一个是A提交事务, B可以读到A提交后的数据 , 一个读的是A提交前的数据 ..... 各有所长 , 就是这俩的区别 ....

对于事务来说 , 合理的解决方案优于执行

一下语句在执行的时候会执行update, 所以会等待另外更新的事务执行完毕才要查询完毕.

```sql
select * from orders for update;
```



## 6.开发中如何开启

springboot 一般这么开启

```java
// 开启事务支持
@EnableTransactionManagement
// 开启可重复读事务级别
@Transactional(isolation = Isolation.REPEATABLE_READ)
@Component
public class TestServiceImpl implements TestService {

    @Override
    public void echo(String msg) {
        // 模拟
    }
}
```



其中 `org.springframework.transaction.annotation.Transactional` 注解中会发现

还有一些属性 : 

- `readOnly `表示是否只读, 比如查询语句是只读

```java
boolean readOnly() default false;
```

- `Propagation` 是事务传播类型 , 默认是REQUIRED类型, 

```java
Propagation propagation() default Propagation.REQUIRED;
```

- `transactionManager` 属性是指 `PlatformTransactionManager`的bean的id , 比如 `@Bean`的name

```java
@Bean("platform1")
public PlatformTransactionManager transactionManager1() {
    return new PlatformTransactionManager() {
        @Override
        public TransactionStatus getTransaction(TransactionDefinition definition) throws TransactionException {
            System.out.println("get transaction ...");
            return new SimpleTransactionStatus();
        }

        @Override
        public void commit(TransactionStatus status) throws TransactionException {
            System.out.println("commit transaction ..." + status.isCompleted());
        }

        @Override
        public void rollback(TransactionStatus status) throws TransactionException {
            System.out.println("rollback transaction ...");
        }
    };
}
```

- `rollbackFor` , `noRollbackFor` 以及`noRollbackForClassName` 和 `rollbackForClassName` 事务回滚异常的类的类型 , 很显然 能try-catch的基本都是Exception的子类 ,使用 : 

```java
noRollbackFor = {RuntimeException.class}
```



