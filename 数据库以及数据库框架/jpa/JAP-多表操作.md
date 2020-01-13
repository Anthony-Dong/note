# JPA - 多表操作

> ​	多于关系型数据库, 大多是描述对象直接复杂的关系, 所以掌握多表之间的关系,是很重要的. 

常见的关系有 一对一, 一对多, 多对一 ,多对多 . 

对于JPA , 实现了上述的四种关系 ,分别用 `@OneToOne` , `@ManyToOne` , `@OneToMany`, `@ManyToMany`  , 一下就详细的讲解一下



## 1. 一对一

> ​	假如我们有一个用户 , 用户只能在公司办理一张工资卡, 所以 , 用户和银行卡是一对一的关系 , 

用户实体类

```java
@Setter
@Getter
@Entity
@Table(name = "users")
@Access(AccessType.FIELD) // 这个是字段注入属性,表示
public class User {

    @Id
    @GeneratedValue
    private Long id;

    @Column(length = 20)
    private String name;

    @Column(length = 50)
    private String password;

    @OneToOne
    protected Card card;
}
```

银行卡实体类

```java
@Setter
@Getter
@Entity
@Table(name = "credit_card")
@Access(AccessType.FIELD)
public class Card {

    @Id
    @GeneratedValue
    private Long id;

    @Column(length = 50)
    private String card;

    /**
     * mappedBy 他所关联的属性名称, 指定拥有方, 所以非拥有方需要指定 mappedBy
     */
    @OneToOne(mappedBy = "card",fetch = FetchType.EAGER)
    private User user;
}
```



然后看一下自动生成的建表语句

```sql
CREATE TABLE `users` (
  `id` bigint(20) NOT NULL,
  `name` varchar(20) DEFAULT NULL,
  `password` varchar(50) DEFAULT NULL,
  `card_id` bigint(20) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `FKtfe9cabsdp6g8exmp5g5qboax` (`card_id`),
  CONSTRAINT `FKtfe9cabsdp6g8exmp5g5qboax` FOREIGN KEY (`card_id`) REFERENCES `credit_card` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
```



```sql
CREATE TABLE `credit_card` (
  `id` bigint(20) NOT NULL,
  `card` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
```



我们可以发现此时 银行卡和用户, 用户有一个`card_id`的字段关联的外键指向了`credit_card`表的`id`字段 , 所以我们基本实现了 , 如果你想说为啥不能反过来, 我感觉真的无语 , 外键要分亲谁是主 , 谁是从, 如果A需要B , 那么B就是A的外键, B可以独立于A存在 . 

下来就是 `@OneToOne` 注解的`mappedBy` 属性  , 他的意思就是我上述说的 , 他代表的含义是 银行卡跟用户确实有关联, 但是不需要外键约束, 我们依靠`Java`的字段约束 , 其中`mappedBy` 字段值等于 , `User`表中,指向`Card`表中的字段名称, 此时是`card` , 所以就是 `  @OneToOne(mappedBy = "card",fetch = FetchType.EAGER)`  , 

`fetch`的含义是指加载需求 , 懒加载和及时加载 , 因为JPA对于颗粒度的实现不是很好, 所以对于是否需要加载他并不知道, 所以默认是`javax.persistence.FetchType#EAGER`  ,就是提前拿. 但是有些时候直接拿不好, 需要查询额外的数据, 一般情况下还是关闭为好,就是 `javax.persistence.FetchType#LAZY` 类型, 比如说我查询`userRepository.findById(1);`,  那么此时会给我将Card表也查询出来, 所以对于不想查询的数据可以使用延迟加载

## 2. 多对一 , 一对多

加入, 一个公司可以有多有员工 , 一个员工只能在一个公司上班, 这就是 公司-员工 一对多的关系 , 员工-公司 多对一

```java
@Setter
@Getter
@Table
@Entity
public class Company {

    @Id
    @GeneratedValue
    private Long id;


    private String name;

    /**
     *fetch :  FetchType.LAZY 代表查询的时候不会查找 user 表
     *         FetchType.EAGER 代表查询的时候回去找到 user 表
     */
    @OneToMany(mappedBy = "company",fetch = FetchType.LAZY,cascade = CascadeType.REMOVE)
    private Collection<User> users;
}
```



```java
@Setter
@Getter
@Entity
@Table(name = "users")
@Access(AccessType.FIELD)
@EntityListeners(value = {UserListener.class})
public class User {

    @Id
    @GeneratedValue
    private Long id;

    @Column(length = 20)
    private String name;

    @Column(length = 50)
    private String password;

    @OneToOne
    protected Card card;

    @ManyToOne
    private Company company;

}
```



查看自动生成的建表语句

```java
CREATE TABLE `users` (
  `id` bigint(20) NOT NULL,
  `name` varchar(20) DEFAULT NULL,
  `password` varchar(50) DEFAULT NULL,
  `card_id` bigint(20) DEFAULT NULL,
  `company_id` bigint(20) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `FKtfe9cabsdp6g8exmp5g5qboax` (`card_id`),
  KEY `FKbwv4uspmyi7xqjwcrgxow361t` (`company_id`),
  CONSTRAINT `FKbwv4uspmyi7xqjwcrgxow361t` FOREIGN KEY (`company_id`) REFERENCES `company` (`id`),
  CONSTRAINT `FKtfe9cabsdp6g8exmp5g5qboax` FOREIGN KEY (`card_id`) REFERENCES `credit_card` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
```



```java
CREATE TABLE `company` (
  `id` bigint(20) NOT NULL,
  `name` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
```



符合我们的需求 , 

此时查询

```java
Company save = companyRepository.save(company);
user1.setCompany(save);
userRepository.save(user1);
user2.setCompany(save);
userRepository.save(user2);
Optional<Company> id = companyRepository.findById(save.getId());
```

查询的SQL是 

```java
select company0_.id as id1_1_0_, company0_.name as name2_1_0_, users1_.company_id as company_5_3_1_, users1_.id as id1_3_1_, users1_.id as id1_3_2_, users1_.card_id as card_id4_3_2_, users1_.company_id as company_5_3_2_, users1_.name as name2_3_2_, users1_.password as password3_3_2_, card2_.id as id1_2_3_, card2_.card as card2_2_3_ from company company0_ left outer join users users1_ on company0_.id=users1_.company_id left outer join credit_card card2_ on users1_.card_id=card2_.id where company0_.id=?
```

基本啥都查询 了 , credit_card表 , company表哇 .  user表哇, 所以懒加载是很必要的 . 



## 3. 多对多

比如公司开设了图书馆, 图书馆中的书 , 一本书可能多个人借阅, 同时一个人可能借阅多本, 这就是多对多的关系 ,这时候就需要有一个表来作为中间表进行维护数据了 , 

书实体类

```java
@Setter
@Getter
@Entity
@Table(name = "books")
@Access(AccessType.FIELD)
public class Book {
    @Id
    @GeneratedValue
    private Long id;

    @Column(length = 50)
    private String name;

    private Double price;

    private Date publishDate;

    @ManyToMany(mappedBy = "books")
    private Collection<User> users;
}
```



```java
@Setter
@Getter
@Entity
@Table(name = "users")
@Access(AccessType.FIELD)
@EntityListeners(value = {UserListener.class})
public class User {

    @Id
    @GeneratedValue
    private Long id;

    @Column(length = 20)
    private String name;

    @Column(length = 50)
    private String password;

    @OneToOne
    protected Card card;

    @ManyToOne
    private Company company;
//
    @ManyToMany
    private Collection<Book> books;

}
```



运行生成表

用户表

```java
CREATE TABLE `users` (
  `id` bigint(20) NOT NULL,
  `name` varchar(20) DEFAULT NULL,
  `password` varchar(50) DEFAULT NULL,
  `card_id` bigint(20) DEFAULT NULL,
  `company_id` bigint(20) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `FKtfe9cabsdp6g8exmp5g5qboax` (`card_id`),
  KEY `FKbwv4uspmyi7xqjwcrgxow361t` (`company_id`),
  CONSTRAINT `FKbwv4uspmyi7xqjwcrgxow361t` FOREIGN KEY (`company_id`) REFERENCES `company` (`id`),
  CONSTRAINT `FKtfe9cabsdp6g8exmp5g5qboax` FOREIGN KEY (`card_id`) REFERENCES `credit_card` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
```

图书表

```java
CREATE TABLE `books` (
  `id` bigint(20) NOT NULL,
  `name` varchar(50) DEFAULT NULL,
  `price` double DEFAULT NULL,
  `publish_date` datetime DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
```

中间表

```java
CREATE TABLE `users_books` (
  `users_id` bigint(20) NOT NULL,
  `books_id` bigint(20) NOT NULL,
  KEY `FKjck6upwhlc41ktqa7sg09tge0` (`books_id`),
  KEY `FKikjr8gdxtts5ubg8nvd0avjjm` (`users_id`),
  CONSTRAINT `FKikjr8gdxtts5ubg8nvd0avjjm` FOREIGN KEY (`users_id`) REFERENCES `users` (`id`),
  CONSTRAINT `FKjck6upwhlc41ktqa7sg09tge0` FOREIGN KEY (`books_id`) REFERENCES `books` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
```

因此基本可以完成 , 需要注意的还是弄清楚 主次关系



## 4. @JoinColumn

他就是外键锁关联的主表的键

`referencedColumnName` 指的是外键引用的表的字段名 , `name`指的是表中字段的名称 , 其他的就么啥了 和 column一样 , `javax.persistence.ForeignKey 就是你外键约束的一堆定义, 你不定义默认会生成的

```java
@Setter
@Getter
@Entity
@Table(name = "users")
@Access(AccessType.FIELD)
@EntityListeners(value = {UserListener.class})
public class User {

    @Id
    @GeneratedValue
    private Long id;

    @Column(length = 20)
    private String name;

    @Column(length = 50)
    private String password;

    @OneToOne
    protected Card card;

    @ManyToOne(targetEntity = Company.class)
    @JoinColumn(referencedColumnName = "id", name = "user_company_id",foreignKey = @ForeignKey(name = "fk_user_company_id"))
    private Company company;

    @ManyToMany
    private Collection<Book> books;

}
```



```java
CREATE TABLE `users` (
  `id` bigint(20) NOT NULL,
  `name` varchar(20) DEFAULT NULL,
  `password` varchar(50) DEFAULT NULL,
  `card_id` bigint(20) DEFAULT NULL,
  `user_company_id` bigint(20) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `FKtfe9cabsdp6g8exmp5g5qboax` (`card_id`),
  KEY `fk_user_company_id` (`user_company_id`),
  CONSTRAINT `FKtfe9cabsdp6g8exmp5g5qboax` FOREIGN KEY (`card_id`) REFERENCES `credit_card` (`id`),
  CONSTRAINT `fk_user_company_id` FOREIGN KEY (`user_company_id`) REFERENCES `company` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
```



基本就是这个用法, 其他么了 , 对比上面的表生成语句 . ......

