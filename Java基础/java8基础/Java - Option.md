# Java - 处理空指针的三种方式

> ​	对于程序经常遇见NnullPointException , 其实对于其他语言也是 , 比如Java , C++ ,都会存在空指针的问题 . 那么怎么处理更优雅, 
>
> ​	首先有两种方式. 第一种书写代码. 程序员自己将代码写好 , 第二种 AOP ,可以动态代理, 也可以借助插件生成代码. 

## 1. Lombox

这个工具类, 相信大家很熟悉 , 那么就看看他如何处理空指针的.

简单的Do类 , 根据阿里开发规范说过, 对于do类, 基本类型全部用包装类型. 

```java
@Data
public class PeopleDo {
    private String name;
    private Integer age;
}
```

我们的处理过程 .

```java
public class Demo {
    public static void handlerPeople(@NonNull PeopleDo people) {
        System.out.println(String.format("That people name is %s.", people.getName()));
    }

    public static void main(String[] args) {
        handlerPeople(null);
    }
}
```

然后输出

```java
Exception in thread "main" java.lang.NullPointerException: peopleDo is marked non-null but is null
```

我们看看他生成的代码, 其实借助于IDEA , 可以直接看反编译的  . 

```java
public class Demo {
    public Demo() {
    }

    public static void handlerPeople(@NonNull PeopleDo peopleDo) {
        // 这里他自动插入了判断. 并且抛出异常. 
        if (peopleDo == null) {
            throw new NullPointerException("peopleDo is marked non-null but is null");
        } else {
            System.out.println(peopleDo.getName());
        }
    }

    public static void main(String[] args) {
        handlerPeople((PeopleDo)null);
    }
}
```

这个是在JVM启动前插件给你动态生成的判断 , 所以这种效率最高. 但是灵活性差点. 

其实就是如何切面处理 , 利用AOP技术.   

## 2. Java的动态代理

> ​	一下例子我简单的写了写. 大家可以参考

我们的标记注解. 作用就是告诉程序 ,这个方法的参数需要进行空检测. 

```java
@Target({ElementType.TYPE_USE})
@Retention(RetentionPolicy.RUNTIME)
public @interface NotNull {
}
```

我们的代理类. 

```java
public class NullProxy<T> {
    
    // 代理bean
    private final T proxyBean;

    public NullProxy(T proxy) {
        this.proxyBean = proxy;
    }

    public T getProxy() throws Exception {
        // 判断前置逻辑
        if (proxyBean == null) {
            throw new RuntimeException("代理对象不能为 NULL");
        }
        if (proxyBean.getClass().getInterfaces() == null) {
            throw new RuntimeException("代理对象必须是接口实现类");
        }
        // 通过底层获取一个代理的字节码
        Class<?> proxyClass = Proxy.getProxyClass(proxyBean.getClass().getClassLoader(), proxyBean.getClass().getInterfaces());
        // 构造器,需要传入一个InvocationHandler,最后调用用户实现就可以了.
        return (T) proxyClass.getConstructor(InvocationHandler.class).newInstance((InvocationHandler) (proxy, method, args) -> {
            // 超级简单的判断逻辑
            Parameter[] parameters = method.getParameters();
            if (parameters != null) {
                for (int i = 0; i < parameters.length; i++) {
                    Parameter parameter = parameters[i];
                    Annotation annotation = parameter.getAnnotatedType().getAnnotation(NotNull.class);
                    if (annotation != null) {
                        if (args[i] == null) {
                            String name = parameter.getName();
                            throw new NullPointerException(name + " 不能为空");
                        }
                    }
                }
            }
            return method.invoke(NullProxy.this.proxyBean, args);
        });
    }
}
```

我们的业务逻辑 .

```java
public interface PeopleService {

    void handlerPeople(@NotNull PeopleDo people);
}
```

我们的实现类.

```java
public class PeopleServiceImpl implements PeopleService {

    @Override
    public void handlerPeople(@NotNull PeopleDo people) {
        System.out.println(String.format("That people name is %s.", people.getName()));
    }
}
```

测试类 :

```java
public class PeopleServiceImplTest {

    @Test
    public void testNull() throws Exception {
        PeopleService service = new PeopleServiceImpl();
        PeopleService proxy = new NullProxy<>(service).getProxy();

        // 测试非空
        PeopleDo people = new PeopleDo();
        proxy.handlerPeople(people);
        
        // 测试空
        proxy.handlerPeople(null);
    }
}
```

输出 : 

```java
That people name is null.

java.lang.NullPointerException: arg0 不能为空
```

所以很简单的方式去判断 , 业务逻辑, 有效的防止空指针代理的后续的一系列问题. 

但是这样做的坏处就是, 方法调用比较慢, 其次就是没有降级 , 没有fallback , 也就是空, 我可以默认指定一个参数. 其实也可以添加进去. 大家自行拓展 , 我就简单讲一下逻辑 



## 3. Java8 - Opthion

对于上诉的问题我们可以直接利用Lambda的链式处理, 写成

```java
public void echoPeople(PeopleDo people){
    String name = Optional.ofNullable(people).map(p->p.getName()).orElse("NULL");
    System.out.println(String.format("That people name is %s.", name));
}
```

我们再试试

```java
That people name is NULL.
```

所以他很好的处理空. 

### 构造方法

#### `of(V)`

这个就是一个最简单的静态方法 , 他不能做任何处理.  直接实例化

#### ` Optional.empty()`

这个返回一个空的实例对象 Option. 

#### `Optional.ofNullable(V)`

前两者的结合体, 当null , 则返回empty , 如果不为空则调用 of.

### 常用方法

一下这前几个是 类似于Builder 一样, 流式处理, 每次都返回一个新的Option. 类似于Stream. 

#### `map(func)`

最简单, 一直映射关系,  原来是 `Optional<PeopleDo>` -> 可以通过 people.getName()映射成 -> `Optional<String>` 

```java
Integer len = Optional.of("hello world").map(a -> a.length()).get();
```

#### `flatMap()`

map的升级版, 需要手动生成 映射结果. 

```java
Integer len = Optional.ofNullable("hello world").flatMap(s1 -> Optional.of(s1.length())).get();
```

这俩各有优缺点. 看自己需求 . 

#### `filter(pred)`

```java
String s = Optional.of("HELLO WORLD").filter(new Predicate<String>() {
            @Override
            public boolean test(String s) {
                return s.length() == 5;
            }
        }).orElseThrow(new Supplier<Throwable>() {
            @Override
            public Throwable get() {
                return new RuntimeException("len should equal 10");
            }
        });
```

输出 : 

```java
java.lang.RuntimeException: len should equal 10
```

简写成lambda . 

```java
Optional.of(str).filter(s1 -> s1.length() == 5).orElseThrow(() -> new RuntimeException("len should equal 10"));
```

是不是简单. 



以下几个是结尾符 . 

#### `get()`

如果value = null , 直接抛出异常

```java
public T get() {
    if (value == null) {
        throw new NoSuchElementException("No value present");
    }
    return value;
}
```

#### `orElse(v)`

如果value 为空, 则返回这个代替值

```java
public T orElse(T other) {
    return value != null ? value : other;
}
```



#### `orElseGet(fun)`

如果为空则加入回调函数. 直接回调函数. 

```java
public T orElseGet(Supplier<? extends T> other) {
    return value != null ? value : other.get();
}
```

#### `orElseThrow()`

如果为空, 则抛出`Supplier` 提供的异常, 其实他应该再多一个方法

```java
public <X extends Throwable> T orElseThrow(Supplier<? extends X> exceptionSupplier) throws X {
    if (value != null) {
        return value;
    } else {
        throw exceptionSupplier.get();
    }
}
```

 比如 :  这个也不错. 

```java
public <X extends Throwable> T orElseThrow(X exception) throws X {
    if (value == null) {
        throw exception;
    } else {
        return value;
    }
}
```



#### `isPresent()`

判断是否存在, 这种写法,  比较low , 用 or 比较好.  但是他提供了value是否存在. 



#### `ifPresent(cons)`

这个就是存在即处理. 



#### `equale(obj)`

```jav
boolean equals = Optional.ofNullable("A").equals(Optional.of("A"));
System.out.println(equals); // true
```

