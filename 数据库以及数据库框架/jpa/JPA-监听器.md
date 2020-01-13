# JPA - EntityListeners 监听器

> `@EntityListeners` 是JPA中的实体监听器, 会在进行CRUD操作之后进行回调,一共有一下几个监听器
>
> ​	Specifies the callback listener classes to be used for an entity or mapped superclass. This annotation may be applied to an entity class or mapped superclass.



### 常见回调

> ​	下面这几个注解就是回调方法,需要使用注解到方法上`@Target({METHOD})` 

```java
@PrePersist

@PostPersist

@PreRemove

@PostRemove

@PreUpdate

@PostUpdate

@PostLoad
```

基本使用就是

```java
@PrePersist
public void prePersist(User user){
    System.out.println("PrePersist : " + user);
}
```



### 快速使用



实体类 ,`@EntityListeners`注解标记实体类

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
    private Card card;

    @ManyToOne
    private Company company;


    @ManyToMany
    private Collection<Book> books;

}
```



监听器

```java
public class UserListener {

    @PrePersist
    public void prePersist(User user){
        System.out.println("PrePersist : " + user);
    }


    @PostPersist
    public void postPersist(User user){
        System.out.println("postPersist : " + user);
    }
}
```



​	

