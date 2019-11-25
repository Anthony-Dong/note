# lombox

## 介绍

 [官网地址 ](https://projectlombok.org/)  :  https://projectlombok.org/

> 	    ​	Project Lombok is a java library that automatically plugs into your editor and build tools, spicing up your java. Never write another getter or equals method again, with one annotation your class has a fully featured builder, Automate your logging variables, and much more.





## @Data



```java
@Data(staticConstructor = "of")
public class Point {
    final int x, y;
}

这个参数是一个静态方法,会生成

public class Point {
    final int x;
    final int y;

    private Point(final int x, final int y) {
        this.x = x;
        this.y = y;
    }

    public static Point of(final int x, final int y) {
        return new Point(x, y);
    }

    private Point() {
        this.x = 0;
        this.y = 0;
    }

    public int getX() {
        return this.x;
    }

    public int getY() {
        return this.y;
    }

    public boolean equals(final Object o) {
        if (o == this) {
            return true;
        } else if (!(o instanceof Point)) {
            return false;
        } else {
            Point other = (Point)o;
            if (!other.canEqual(this)) {
                return false;
            } else if (this.getX() != other.getX()) {
                return false;
            } else {
                return this.getY() == other.getY();
            }
        }
    }

    protected boolean canEqual(final Object other) {
        return other instanceof Point;
    }

    public int hashCode() {
        int PRIME = true;
        int result = 1;
        int result = result * 59 + this.getX();
        result = result * 59 + this.getY();
        return result;
    }

    public String toString() {
        return "Point(x=" + this.getX() + ", y=" + this.getY() + ")";
    }
}
```





## @Accessors 

@Accessors(存取器)用于配置getter和setter方法的生成结果，下面介绍三个属性 

### fluent

fluent的中文含义是流畅的，设置为true，则getter和setter方法的方法名都是基础属性名，且setter方法返回当前对象。如下

```java
@Data
@Accessors(fluent = true)
public class User {
    private Long id;
    private String name;
    
    // 生成的getter和setter方法如下，方法体略
    public Long id() {}
    public User id(Long id) {}
    public String name() {}
    public User name(String name) {}
}
```

### chain

chain的中文含义是链式的，设置为true，则setter方法返回当前对象。如下

```java
@Data
@Accessors(chain = true)
public class User {
    private Long id;
    private String name;
    
    // 生成的setter方法如下，方法体略
    public User setId(Long id) {}
    public User setName(String name) {}
}
```

### prefix

prefix的中文含义是前缀，用于生成getter和setter方法的字段名会忽视指定前缀（遵守驼峰命名）。如下

```java
@Data
@Accessors(prefix = "p")
class User {
	private Long pId;
	private String pName;
	
	// 生成的getter和setter方法如下，方法体略
	public Long getId() {}
	public void setId(Long id) {}
	public String getName() {}
	public void setName(String name) {}
}
```



## @Builder

```java
@Builder
//@Accessors(chain = true)
@Data
public class Orders {

    @TableId(value = "id", type = IdType.AUTO)
    private Integer id;

    private  String name;

    private Integer count;

    @Version
    private Integer version;

    public static void main(String[] args) {
        Orders build = Orders.builder().count(1).name("1").id(1).build();
    }
}
```



```java
public class Example<T> {
        private T foo;
        private final String bar;

        private Example(T foo, String bar) {
                this.foo = foo;
                this.bar = bar;
        }

        public static <T> ExampleBuilder<T> builder() {
                return new ExampleBuilder<T>();
        }

        public static class ExampleBuilder<T> {
                private T foo;
                private String bar;

                private ExampleBuilder() {}

                public ExampleBuilder foo(T foo) {
                        this.foo = foo;
                        return this;
                }

                public ExampleBuilder bar(String bar) {
                        this.bar = bar;
                        return this;
                }

                @java.lang.Override 
                public String toString() {
                        return "ExampleBuilder(foo = " + foo + ", bar = " + bar + ")";
                }

                public Example build() {
                        return new Example(foo, bar);
                }
        }
 }
```



## @NonNull

> ​	If put on a parameter, lombok will insert a null-check at the start of the method / constructor's body, throwing a NullPointerException with the parameter's name as message. If put on a field, any generated method assigning a value to this field will also produce these nullchecks.
>
> ​	如果在一个参数字段上加,lombox会在这个方法或者构造函数的开头插入一个null-chec , 抛出一个 带有改参数字段名字的NullPointerException 消息.  如果在一个字段上加(属性上) ,任何为该字段赋值的生成方法也将产生这些nullcheck。
>
> ​	**这里需要注意的一点是  如果用在字段上 , 必须使用 lombox 提供的 get,set方法,**



```java
public class ValidatedService {

    public void test(@NonNull TestBean testBean) {

    }

    public static void main(String[] args) {
        ValidatedService service = new ValidatedService();
        service.test(null);
    }
}
```

结果 : 

```java
Exception in thread "main" java.lang.NullPointerException: testBean
	at com.spring.springvalidator.service.ValidatedService.test(ValidatedService.java:20)
	at com.spring.springvalidator.service.ValidatedService.main(ValidatedService.java:30)
```

