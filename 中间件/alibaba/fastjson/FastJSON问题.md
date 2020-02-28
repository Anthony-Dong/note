# FastJson处理起来的问题

## 序列化问题

#### 1. 参数要么是public , 要么get方法. 

> ​	前置条件无序什么,默认构造器,什么有参的 ,都不需要, 只需要下面这两种, 可以将字段序列化.

```java
private String name;
public String getName() {
    return name;
}

// 或者
public String name;
```



## 反序列问题

#### 需要什么样的构造器

构造器优先级问题 : ` 无参构造器 > 有参构造器 `

第一种:  无参构造器, 但是没有set方法.  那么返回的将是一个空对象. 也就是字段全部未初始化, 但不是null.

第二种 :  无参构造器. 有的字段有set方法.  那么只会填写有set方法的. 

第二种 :  无参构造器私有. 有set方法也可以 . 也可以序列化成功 . 



`有参构造器` :  会选择参数最多的那个构造器,与JSON字段有几个无关. 也和set方法无关. 一般会抛出 : 

```java
com.alibaba.fastjson.JSONException: create instance error
```



## 内部类的问题.

> ​	基本上也是上述问题. 第一必须static修饰么. 首先得可能拿到.  不然会抛出 `com.alibaba.fastjson.JSONException: can't create non-static inner class instance.` 

第二 , 就是类的问题了, 上诉都有 .  fastjson处理内部类使用的是ASM技术. 有兴趣可以看看. 所以内部类问题一般都会处理. 

但是对于集合类型 另有处理方式, 所以万事不是一定的. 

还有一种异常是`java.util.Collections$SingletonMap` 这个类型无法处理 : `Exception in thread "main" com.alibaba.fastjson.JSONException: unsupport type class java.util.Collections$SingletonMap`

这个问题的出现是Fastjson的bug . 可以看看这里 `com.alibaba.fastjson.parser.deserializer.MapDeserializer#createMap`   基本就是不断的类型判断, 

```java
if (type == Properties.class) {
}
if (type == Hashtable.class) {
}
if (type == IdentityHashMap.class) {
}
if (type == SortedMap.class || type == TreeMap.class) {
}
if (type == ConcurrentMap.class || type == ConcurrentHashMap.class) {
}
if (type == Map.class) {
    return (featrues & Feature.OrderedField.mask) != 0
            ? new LinkedHashMap()
            : new HashMap();
}
if (type == HashMap.class) {
}
if (type == LinkedHashMap.class) {
}
if (type instanceof ParameterizedType) {
}
Class<?> clazz = (Class<?>) type;
// 如果为一个接口 . 抛出异常
if (clazz.isInterface()) {
    throw new JSONException("unsupport type " + type);
}
/ 如果是这个类型, 则new hashmap
if ("java.util.Collections$UnmodifiableMap".equals(clazz.getName())) {
    return new HashMap();
}
// 这里直接拿到class对象, 然后newinstance.那么私有构造器.无法实现
try {
    return (Map<Object, Object>) clazz.newInstance();
} catch (Exception e) {
    throw new JSONException("unsupport type " + type, e);
}
```



对于上诉的问题, 其实可以加一个条件判断.  对于我自己个人平时处理序列化反序列问题. 对于用json用来作为处理的, 可以看看我咋做的.  主要是对于序列化出现问题, 就走接口参数类型, 还不行就是object类型了. 

```java
/**
 * 转换 , 如果为空就抛出异常
 * 处理流程, 如果原类型不支持, 则接口类型, 如果接口类型还是不支持就OBJ了 , 我这里已经业务前面做了空处理了.所以空抛出异常
 * @throws IllegalArgumentException
 */
@SuppressWarnings("all")
public static Object[] convert(String json, Method method) throws IllegalArgumentException {
    List<Arg> args = JSON.parseObject(json, TYPE);
    if (args == null || args.size() != method.getParameters().length) {
        throw new IllegalArgumentException("传入参数和实际参数长度不一致");
    }
    Object[] arr = new Object[args.size()];
    for (int i = 0; i < args.size(); i++) {
        Arg arg = args.get(i);
        Object object = null;
        try {
            object = JSON.parseObject(arg.getValue(), arg.getClazz());
        } catch (Exception e) {
            try {
                Parameter[] parameters = method.getParameters();
                object = JSON.parseObject(arg.getValue(), parameters[i].getType());
            } catch (Exception e1) {
                object = JSON.parseObject(arg.getValue(), Object.class);
            }
        }
        arr[i] = object;
    }
    return arr;
}
```









