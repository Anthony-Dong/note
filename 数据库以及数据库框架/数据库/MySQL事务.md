# MySQL - 事务隔离界级别

## 1. 事务四大属性

分别是原子性、一致性、隔离性、持久性。

#### 1、原子性（Atomicity）

原子性是指事务包含的所有操作要么全部成功，要么全部失败回滚，因此事务的操作如果成功就必须要完全应用到数据库，如果操作失败则不能对数据库有任何影响。

#### 2、一致性（Consistency）

一致性是指事务必须使数据库从一个一致性状态变换到另一个一致性状态，也就是说一个事务执行之前和执行之后都必须处于一致性状态。举例来说，假设用户A和用户B两者的钱加起来一共是1000，那么不管A和B之间如何转账、转几次账，事务结束后两个用户的钱相加起来应该还得是1000，这就是事务的一致性。

#### 3、隔离性（Isolation）

隔离性是当多个用户并发访问数据库时，比如同时操作同一张表时，数据库为每一个用户开启的事务，不能被其他事务的操作所干扰，多个并发事务之间要相互隔离。关于事务的隔离性数据库提供了多种隔离级别，稍后会介绍到。

#### 4、持久性（Durability）

持久性是指一个事务一旦被提交了，那么对数据库中的数据的改变就是永久性的，即便是在数据库系统遇到故障的情况下也不会丢失提交事务的操作。例如我们在使用JDBC操作数据库时，在提交事务方法后，提示用户事务操作完成，当我们程序执行完成直到看到提示后，就可以认定事务已经正确提交，即使这时候数据库出现了问题，也必须要将我们的事务完全执行完成。否则的话就会造成我们虽然看到提示事务处理完毕，但是数据库因为故障而没有执行事务的重大错误。这是不允许的。



## 2. 事务的隔离级别

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

### 1. READ-UNCOMMITTED (未授权读取、读未提交)

​	如果A事务已经开始写数据，则另外B事务则不允许同时进行写操作，但允许B事务读此行数据。该隔离级别可以通过“排他写锁”实现。这样就避免了更新丢失，却可能出现脏读。也就是说事务B读取到了事务A未提交的数据。所以容易出现脏读的问题. 

### 2.  READ-COMMITTED （授权读取、读已提交）

​	读取数据的事务允许其他事务继续访问该行数据，但是未提交的写事务将会禁止其他事务访问该行。该隔离级别避免了脏读，但是却可能出现不可重复读。事务A事先读取了数据，事务B紧接了更新了数据，并提交了事务，而事务A再次读取该数据时，数据已经发生了改变(rallback也是)。

### 3. REPEATABLE-READ 可重复读取 (MySQL默认)

​		可重复读是指在一个事务内，多次读同一数据。比如AB开启事务, AB开始都读取到数据为11, 此时B把10修改成10了, 同时提交了. 但是此时A再次读取的时候还是10 , 所以是可重复读.  

​		读取数据的事务将会禁止写事务（但允许读事务），写事务则禁止任何其他事务。这样避免了不可重复读取和脏读，但是有时可能出现幻象读。（读取数据的事务）这可以通过“共享读锁”和“排他写锁”实现。

​		但是就算可重复读. 当我们给A执行写入的时候, ` update t1 set count=12 where id=1 and count=11;`  此时执行是失败的. 所以他还是能保证数据的一致性的.  因为这里事务只是保证了读可重复, 但是更新的时候就不只是读了. 所以更新失败可以立马知道原来的数据.  

### 4. SERIALIZABLE  序列化

不允许两个事物同时执行 , 就是A事物操作开始, B事物全部在等待 ,只有当A事物commit;B事物才能执行.

提供严格的事务隔离。它要求事务序列化执行，事务只能一个接着一个地执行，但不能并发执行。如果仅仅通过“行级锁”是无法实现事务序列化的，必须通过其他机制保证新插入的数据不会被刚执行查询操作的事务访问到。序列化是最高的事务隔离级别，同时代价也花费最高，性能很低，一般很少使用，在该级别下，事务顺序执行，不仅可以避免脏读、不可重复读，还避免了幻像读。 

## 3. 总结

我真的觉得 第二种和第三种级别么区别 .... 无非一个是A提交事务, B可以读到A提交后的数据 , 一个读的是A提交前的数据 ..... 各有所长 , 就是这俩的区别 ....

对于事务来说 , 合理的解决方案优于执行

一下语句在执行的时候会执行update, 所以会等待另外更新的事务执行完毕才要查询完毕.

```sql
select * from orders for update;
```



## 4.开发中如何使用

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



