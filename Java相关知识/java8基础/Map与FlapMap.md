# Map与FlatMap

> ​	 如果你学过大数据开发的话, 尤其是spark , 对于`FlatMap`就不陌生了 , 如果让我说 Map与FlatMap的区别, 可能就是



## 1. FlatMap 究竟做啥了

### 1. 例子一

举个例子 , 我们经常遇见一堆叠加对象 , 比如下面这些数据

```java
list.add("1,2");
list.add("3,4");
list.add("5,6");
list.add("7,8");
```

我们需要将它分隔开, 然后存入一个 List中

```java
ArrayList<String> list = new ArrayList<>();
list.add("1,2");
list.add("3,4");
list.add("5,6");
list.add("7,8");
List<Object> list1 = list.stream().flatMap(new Function<String, Stream<?>>() {
    @Override
    public Stream<?> apply(String s) {
        return Stream.of(s.split(","));
    }
}).collect(Collectors.toList());
```

结果就是 1 2 3 4 5 6 7 8 

### 2. 例子二

