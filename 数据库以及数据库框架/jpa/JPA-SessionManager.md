# JPA - EntityManager

> Interface used to interact with the persistence context.
>
> An EntityManager instance is associated with a persistence context. A persistence context is a set of entity instances in which for any persistent entity identity there is a unique entity instance. Within the persistence context, the entity instances and their lifecycle are managed. The EntityManager API is used to create and remove persistent entity instances, to find entities by their primary key, and to query over entities.
> The set of entities that can be managed by a given EntityManager instance is defined by a persistence unit. A persistence unit defines the set of all classes that are related or grouped by the application, and which must be colocated in their mapping to a single database.

就是持久化层的上下文接口类   ,  他是核心



如何引用  

```java
@PersistenceContext
private EntityManager manager;
```



使用需要注意需要实现 事务  , 不然会报错 , 所以对于这个基本需要实现事务 , 以下是解决方案. 

```java
javax.persistence.TransactionRequiredException: No EntityManager with actual transaction available for current thread - cannot reliably process 'persist' call
```



对于 springboot项目只需要 

```java
@Service
public class UserService {

    @PersistenceContext
    private EntityManager manager;


    @Transactional
    public void save(User user){
        manager.persist(user);
    }

}
```



然后开启 `@EnableTransactionManagement(proxyTargetClass = true)` 基本OK 了