# Java - 生态界的序列化方式(RPC的核心)

> ​	我的版本是 2.7.X , dubbo用的序列化是 hessian2  .  他相对于java的序列化更加的小巧,  相对于FastJson来说更加方便, 因为JSON是字符串还需要转换成字节数组, 不适合网络传输的字节流. 
>
> ​	MessagePack需要的要求高, 注解配置不符合这种灵活性高的场景 , 对用户不友好. 
>
> ​	Hessian2 提供了更好的方式 , 实现序列化机制. 直接基于字节流, 相对于Java的序列化机制更加灵活, 但是也有bug. Java的序列化不会有BUG. 

我们先对比一下Java的序列化方式 

#### Java原生 的序列化与反序列

> ​	自带的很方便. 

```java
private static <T> byte[] serializeJava(T obj) throws IOException {
    if (obj == null) throw new NullPointerException();
    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    try (ObjectOutputStream oos = new ObjectOutputStream(bos)) {
        oos.writeObject(obj);
        return bos.toByteArray();
    }
}

@SuppressWarnings("all")
private static <T> T deserializeJava(byte[] by) throws IOException, ClassNotFoundException {
    if (by == null || by.length == 0) throw new NullPointerException();
    ByteArrayInputStream bis = new ByteArrayInputStream(by);
    try (ObjectInputStream ois = new ObjectInputStream(bis)) {
        return (T) ois.readObject();
    }
}
```



Java测试类: 

```java
@Test
public void testJava() throws IOException, ClassNotFoundException {
    TestBean testBean = new TestBean("a", Collections.singletonList("b"), Collections.singletonMap("c", "d"));
    byte[] bytes = serializeJava(testBean);
    System.out.printf("byte length : %d.", bytes.length);
    TestBean bean = deserializeJava(bytes);
    System.out.println(bean);
    bean.printlnType();
}
```

Java输出: 

```java
byte length : 302.
TestBean{name='a', list=[b], map={c=d}}
name: class java.lang.String , list : class java.util.Collections$SingletonList , map : class java.util.Collections$SingletonMap
```



#### hessian2 序列化与反序列化

```java
/**
 * 序列化
 * @throws IOException
 */
private static <T> byte[] serialize(T obj) throws IOException {
    if (obj == null) throw new NullPointerException();
    ByteArrayOutputStream os = new ByteArrayOutputStream();
    Hessian2Output ho = new Hessian2Output(os);
    try {
        ho.writeObject(obj);
        // 这里必须刷新出去.
        ho.flushBuffer();
        return os.toByteArray();
    } finally {
        ho.close();
    }
}


/**
 * 反序列化
 *
 * @throws IOException
 */
@SuppressWarnings("all")
private static <T> T deserialize(byte[] by) throws IOException {
    if (by == null || by.length == 0) throw new NullPointerException();
    ByteArrayInputStream is = new ByteArrayInputStream(by);
    Hessian2Input hi = new Hessian2Input(is);
    try {
        return (T) hi.readObject();
    } finally {
        hi.close();
    }
}
```



Hessian测试 : 

```java
@Test
public void testHessian2() throws IOException {
    TestBean testBean = new TestBean("a", Collections.singletonList("b"), Collections.singletonMap("c", "d"));
    byte[] bytes = serialize(testBean);
    System.out.printf("byte length : %d.\n", bytes.length);
    TestBean bean = deserialize(bytes);
    System.out.println(bean);
    bean.printlnType();
}
```

Hessian输出 : 

```java
byte length : 138.
TestBean{name='a', list=[b], map={c=d}}
name: class java.lang.String , list : class java.util.ArrayList , map : class java.util.HashMap
```



Java输出 : 

```java
byte length : 302.
TestBean{name='a', list=[b], map={c=d}}
name: class java.lang.String , list : class java.util.Collections$SingletonList , map : class java.util.Collections$SingletonMap
```

#### 总结: 

我们发现. Java的序列化大小是相差大小为两倍左右 ,Java序列化的体积是Hessian的两倍. 

Java序列化对于类型支持比较好. 可以将其恢复为原生类型. Hessian不可以. 

两者都需要将 序列化对象实现 `java.io.Serializable` 此接口.

效率上面, 我们模拟了10次序列化与.  100个线程模拟. 

测试处理. 记得将上诉打印方法的都去掉.

```java
public static void main(String[] args) throws InterruptedException {
    ExecutorService service = Executors.newFixedThreadPool(100);
    long start = System.currentTimeMillis();
    IntStream.range(0,100000).forEach(e->{
        service.execute(()->{
            try {
                testJava();
            } catch (IOException | ClassNotFoundException e1) {
                e1.printStackTrace();
            }
        });
    });
    service.shutdown();
    service.awaitTermination(Integer.MAX_VALUE, TimeUnit.DAYS);
    System.out.printf("cost : %dms", System.currentTimeMillis() - start);
}
```

Java需要时间为 : 

```java
cost : 1452ms
```

hessian2需要为 : 

```java
cost : 11915ms
```

时间上相差大约为`8倍左右 ` , 可以多测试几下,所以Java的序列化方式更加高效, 但是占用体积大,类型支持好 .  Hessian序列化体积小, 类型支持不好, 时间消耗过长 .  



#### 实体Bean 

```java
public class TestBean implements Serializable {

    private String name;

    private List<String> list;


    private Map<String, Object> map;

    public TestBean(String name, List<String> list, Map<String, Object> map) {
        this.name = name;
        this.list = list;
        this.map = map;
    }

    @Override
    public String toString() {
        return "TestBean{" +
                "name='" + name + '\'' +
                ", list=" + list +
                ", map=" + map +
                '}';
    }
    // 打印类型. 看看是否支持可靠
    public void printlnType(){
        System.out.printf("name: %s , list : %s , map : %s\n", name.getClass(), list.getClass(), map.getClass());
    }
}
```





#### 附加FASTJSON测试

> ​	要求必须有get,set方法, 想要暴露的字段. 构造方法先不说, 没有构造器都不能进行实例化.

```java
@Test
public void testJson() {
    TestBean testBean = new TestBean("a", Collections.singletonList("b"), Collections.singletonMap("c", "d"));
    String s = JSON.toJSONString(testBean);
    System.out.printf("byte length : %d.\n", s.getBytes().length);
    TestBean testBean1 = JSON.parseObject(s, TestBean.class);
    System.out.println(testBean1);
    testBean1.printlnType();
}
```

输出: 

```java
byte length : 41.
TestBean{name='a', list=[b], map={c=d}}
name: class java.lang.String , list : class java.util.ArrayList , map : class java.util.HashMap
```

体积小. 类型支持适中 , 和 hessian2一样. 

效率测试. 记住进行byte转换一次. 

```java
public static void testJson() {
    TestBean testBean = new TestBean("a", Collections.singletonList("b"), Collections.singletonMap("c", "d"));
    // string -> byte
    String s = JSON.toJSONString(testBean);
    byte[] bytes = s.getBytes();
    // byte-> string
    String json = new String(bytes);
    JSON.parseObject(json, TestBean.class);
}
```

还是上面的模拟. 

```java
cost : 985ms
```

时间为 985ms , 效率更高 . 所以FastJson 名副其实的快,  提交小, 类型支持适中.  





#### MessagePack测试

这里就不测试了 ,  记住一点 . 只需要实例化一个 MessagePack对象便可以了, 大量的实例化效率特别低, 差10倍. 单个对象序列化就是线程安全的. 必须要多个. 

类型支持差, 我们就不尝试了. 



#### protobuff 测试

