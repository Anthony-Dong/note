# lombox

## @Accessors

Accessor的中文含义是存取器，@Accessors用于配置getter和setter方法的生成结果，下面介绍三个属性

###### fluent

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

###### chain

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

###### prefix

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



