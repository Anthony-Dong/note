# hashmap

initialCapacity  -- 数组的长度,  `Node<K,V>[] tab` 的长度 , 长度默认值是16 ,   

比如我们的实例化的时候是`new HashMap<>(5);` 此时他进行位运算, 最后长度就成了 8  , 必须是2的幂次方

 

loadFactor  --  加载因子 , 默认值是0.75, 



threshold  --- 阈值 = initialCapacity  *loadFactor   , 



默认的时候 `loadFactor` 是 0.75  , ` initial capacity` 是 16  , `threshold` 因此就是 俩数字相乘 12 

```java
/**
 * Constructs an empty <tt>HashMap</tt> with the default initial capacity
 * (16) and the default load factor (0.75).
 */
public HashMap() {
    this.loadFactor = DEFAULT_LOAD_FACTOR; // all other fields defaulted
}
```



### 1. 扩容 (很重要)

对于hashmap中的扩容 以及 resize() 操作

```java
if (++size > threshold)  resize();
```

这里以这个为例子 : 

```java
HashMap<String, Integer> map = new HashMap<>(8);
```

比如`new HashMap<>(8);` 长度就是 8 , `loadfactory `是 0.75 , 因此他的`threshold`是6, 当进行一次 ` resize()` 操作 , 首先这里操作将 `newCap = oldCap << 1`  ,原来的大小乘以2  , 因此此时变成了capacity = 16

```java 
else if ((newCap = oldCap << 1) < MAXIMUM_CAPACITY &&oldCap >= DEFAULT_INITIAL_CAPACITY)
newThr = oldThr << 1; // double threshold
```

因此这一步就是 `ft = newCap*loadFactor` 此时就是 16*0.75=12 , 

```java
if (newThr == 0) {
    float ft = (float)newCap * loadFactor; // newCap*loadFactor
    newThr = (newCap < MAXIMUM_CAPACITY && ft < (float)MAXIMUM_CAPACITY ?
              (int)ft : Integer.MAX_VALUE);
}
```



所以大致流程就是  : 

我们定义一个`HashMap`的时候 , 一般受到两个参数的影响 ,一个是`initialCapacity`   , 一个是 `loadFactor` 都决定了后期的扩容

插入数据的时候, 最难受的就是`resize`阶段, 此时会进行大量的节点移动, 消耗很大运算 , 所以我们应当避免



比如初始化8 , 此时初始化的`threshold` =6 , 然后当数据长度大于6的时候 , 就会进行扩容, 此时更新完以后就是 

`Capacity=16` , `threshold=12`  ,然后再当长度大于12的时候还要扩容 , 循环往复



所以当我们定义一个 存入90个hashmap的数组时候 ,避免`resize` 应当的操作就是 , 让`threshold>100` 就可以了 , 还有`capacity必须是 2的幂次方` ,   

所以很简单, 就是 capacity = 128 . 此时 threshold就是96 , 所以刚刚好 , 



### 2. treeifyBin 

还有就是一个 `treeifyBin` 的操作 , 就是当 一个Node中的长度 > `TREEIFY_THRESHOLD` 就会进行`treeifyBin` 的操作

```java
static final int TREEIFY_THRESHOLD = 8;  // 固定死的
```

```java
for (int binCount = 0; ; ++binCount) {
    // 循环迭代, 到最后一位 , 此时长度大于8就进行treeifyBin 操作
    if ((e = p.next) == null) {
        p.next = newNode(hash, key, value, null);
        if (binCount >= TREEIFY_THRESHOLD - 1) // -1 for 1st
            treeifyBin(tab, hash);
        break;
    }
    ..... 
}
```





