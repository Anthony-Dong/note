# Hibernate - 其他细节

## 1. @Where 注解

> ​	 他可以添加在你的 entity对象中 ,对于每次查询会添加他 , 通常用于删除的软连接上 , 比如订单用户可能执行删除并不是真正执行了删除, 所以我们设置一个标识符

比如 `@Where(clause = "isDeleted = 0")`

以后**每次查询语句都会带上他** , 所以不是特殊要求别带这个 , 这个所以不是特殊需求别加



## 2. @SQLDelete 和 @SQLDeleteAll

他们在执行 delete 或者  deleteall 执行这个,  他和上面的也一样 , 在软删除中常用

```java
@SQLDelete(sql = "UPDATE jpa_user set user_name='deleted' WHERE user_id= ? ")
```

我们在执行

```java
userRepository.deleteById(11);
```

他就会执行我们的 delete 语句

```java
Hibernate: select next_val as id_val from hibernate_sequence for update
```

然后数据库

```java
mysql> select * from jpa_user where user_id=11;
+---------+-------------+-----------+-------------+
| user_id | user_gender | user_name | user_salary |
+---------+-------------+-----------+-------------+
|      11 |           1 | deleted   |     1011.00 |
+---------+-------------+-----------+-------------+
1 row in set (0.00 sec)
```



## 3. SQLUpdate 和 SQLInsert 

这俩都是覆盖 , 基本上看自己需求使用了