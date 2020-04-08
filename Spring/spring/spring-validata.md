# Spring - Validation 支持

对于aip  ,参数校验可能是必须的, 比如前端的一些弱类型参数 , 需要我们必须进行检验.

Spring基于JSR303规范, 实现了一套体系 

具体的使用可以参考这篇文章 https://blog.csdn.net/lzy_168/article/details/81099699 , 我觉得写得比较全. 但是不一定有我细节. 

### 简单 校验

最简单的就是  , 需要注意在类中需要加入 @Validated 关键字开启校验. 对于Length字段校验才有效.

```java
@Validated
@RestController
@SpringBootApplication
public class SpringValidateApplication {

    public static void main(String[] args) {
        SpringApplication.run(SpringValidateApplication.class, args);
    }


    @GetMapping(path = "/echo")
    public String echo(@Length(min = 5,max = 10,message = "msg长度5-10位") @RequestParam("name") String msg) {
        return "hello "+msg;
    }
}
```



### 嵌套 校验

```java
@ToString
@Setter
@Getter
public class User {
    @NotBlank
    String id;
    String name;

    // 加上@Valid可以实现嵌套校验Auth. 不加不会去校验的.
    @Valid
    @NotNull
    Auth auth;

    @Getter
    @Setter
    static class Auth {
        @Length(min = 4, max = 4)
        String type;
    }
}
```

调用 , 因此只能使用一些 , 比如RequestBody场景可以反序列. 因为普通的 `x-www-form-urlencoded` 是无法拿到子对象的.

```java
@PostMapping(path = "/user/add")
public User userAdd(@Validated @RequestBody User name) {
    return name;
}
```



### 分组情况 校验

我们更新操作的时候要求, 我们的id字段不能为空. 

此时可以先定义一个接口.

```java
public interface Update {
}
```

然后字段分组

```java
@ToString
@Setter
@Getter
public class User {

    /**
     * 更新操作不能为空
     */
    @NotBlank(groups = {Update.class})
    String id;

    // name字段不能为空都.
    @NotBlank
    String name;
}
```

最后

```java
@PostMapping(path = "/user/update")
public User updateUser(@Validated(value = {Update.class}) @RequestBody User name) {
    return name;
}
```

#### 注意事项

比如此时的需求改了, 我们同时还要校验, 姓名长度在 6-12位之间,通用的属性都是

```java
public class User {
    /**
     * 更新操作不能为空
     */
    @NotBlank(groups = {Update.class})
    String id;

    /**
     * 所有操作都是 长度在 6-12位之间.注意必须配合NotBlank或者Null使用,不然无效.不传入不会监测
     */
    @NotBlank
    @Length(min = 6, max = 12, message = "用户名长度在6-12位之间")
    String name;
}
```

此时我们启动在申请一次 , 比如申请如下, 发现也能通过, 那是为什么 Length校验失效呢. 此时我们发现了问题

```java
{
"id":"1",
"name":"tom"    
}
```

如何解决呢, 需要修改这个, 因为默认group在 default组下 . 所以我们Update组不在里面所以不执行.

```java
public interface Update extends Default {
}
```

此时再次申请, 发现走了校验.

### 自定义校验 

```java
@Documented
@Constraint(validatedBy = PhoneNumberValidator.class)
@Target({ElementType.FIELD, ElementType.PARAMETER})
@Retention(RetentionPolicy.RUNTIME)
public @interface PhoneNumber {
    String message() default "Invalid phone number";
    // 组还是很有用的. 必要事件使用
    Class[] groups() default {};
    Class[] payload() default {};
}
```

实现接口`ConstraintValidator` 拓展具体实现.

```java
public class PhoneNumberValidator implements ConstraintValidator<PhoneNumber, String> {

    @Override
    public boolean isValid(String phoneField, ConstraintValidatorContext context) {
        if (phoneField == null) return true; // can be null
        return phoneField != null && phoneField.matches("[0-9]+")
                && (phoneField.length() > 8) && (phoneField.length() < 14);
    }
}
```

