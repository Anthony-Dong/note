# LinkedHashMap原理分析

## 1. 我们先写一段代码体验一下 LinkedHashMap

```java
        LinkedHashMap<String, Integer> lmap = new LinkedHashMap<>();
        lmap.put("语文", 1);
        lmap.put("数学", 2);
        lmap.put("英语", 3);
        lmap.put("历史", 4);
        lmap.put("政治", 5);
        lmap.put("地理", 6);
        lmap.put("生物", 7);
        lmap.put("化学", 8);
        for(Map.Entry<String, Integer> entry : lmap.entrySet()) {
            System.out.println(entry.getKey() + ": " + entry.getValue());
        }

		语文: 1
        数学: 2
        英语: 3
        历史: 4
        政治: 5
        地理: 6
        生物: 7
        化学: 8
            
          
```

- 我们debug看看他的数据结构

- ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-13/471fb2d5-e190-436a-9217-18a4aaeccd3b.png?x-oss-process=style/template01)

- 当我们去看到 打印结果 发现他是一个有序,根本不符合**hashmap的无序性**

- 那他究竟是为什么呢,我们看看LinkedHashMap的源码

  > ```
  > Hash table and linked list implementation of the Map interface, with predictable iteration order. This implementation differs from HashMap in that it maintains a doubly-linked list running through all of its entries. This linked list defines the iteration ordering, which is normally the order in which keys were inserted into the map (insertion-order).
  > 
  > 这里主要说了 它是由Hashtable 和 linkedlist 实现的, 而且是双向链表(没一个entry都有 pre ,next) ,所以很好实现
  > ```

- 它继承与hashmap ,那么它究竟重写和添加了什么方法呢,下一节引出

## 2. LinkedHashMap 的三个主要实现函数

- 我们去看他的父类发现,hashmap有三个回调函数

- ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-13/a7fdb106-4982-4fdf-ac32-b5274d34e52e.png?x-oss-process=style/template01)

- 所以问题的关键就在这里,我们试着看一下 他是咋重写这 三个**回调函数**的

- ```java
  1. afterNodeRemoval  这个函数是在移除节点后调用的，就是将节点从双向链表中删除。
      
      void afterNodeRemoval(Node<K,V> e) { // unlink
          LinkedHashMap.Entry<K,V> p =(LinkedHashMap.Entry<K,V>)e,
              b = p.before, // p前
              a = p.after;	// p后
          // 解除关系(类似与C的指针)    
          p.before = p.after = null;
          // b 如果为空(意思就是 p原来是头结点),a为头,否者 b的后面结点为a;
          // 如果a为空,那么p就是尾巴,此时 移除p ,b就为尾巴, 否者a的前面是b
          if (b == null)
              head = a;
          else
              b.after = a;
          if (a == null)
              tail = b;
          else
              a.before = b;
      }
      
  2. afterNodeInsertion  节点插入后做的操作 
      
      // evict逐出的意思, 默认是true 这段代码我并么有看懂
      void afterNodeInsertion(boolean evict) { // possibly remove eldest
          LinkedHashMap.Entry<K,V> first;
          if (evict && (first = head) != null && removeEldestEntry(first)) {
              K key = first.key;
              removeNode(hash(key), key, null, false, true);
          }
      }
  
  
  3. afterNodeAccess()  每次执行get操作时会调用 afterNodeAccess方法 ,我后面写个demo就和这个类似,
  	写demo的时候并没有研究 ,哈哈哈
  
  
      void afterNodeAccess(Node<K,V> e) { // move node to last
          LinkedHashMap.Entry<K,V> last;
          if (accessOrder && (last = tail) != e) {
              LinkedHashMap.Entry<K,V> p =
                  (LinkedHashMap.Entry<K,V>)e, b = p.before, a = p.after;
              p.after = null;
              if (b == null)
                  head = a;
              else
                  b.after = a;
              if (a != null)
                  a.before = b;
              else
                  last = b;
              if (last == null)
                  head = p;
              else {
                  p.before = last;
                  last.after = p;
              }
              tail = p;
              ++modCount;
          }
      }
  
  ```



## 3. put和get函数

- put函数 并未实现 ,只是实现了get函数

- 我们看看get函数

  ```java
      public V get(Object key) {
          Node<K,V> e;
          if ((e = getNode(hash(key), key)) == null)
              return null;
          if (accessOrder)
              afterNodeAccess(e);
          return e.value;
      }
  
  注意: 如果定义了 accessOrder ,那么每次执行会调用回调函数(就是把最近访问的元素放到队尾)
          
  举个栗子: 
  	   LinkedHashMap<Object, Object> map = new LinkedHashMap<>(10, 1, true);
  
  
          map.put(1, 1);
          map.put(2, 2);
          map.put(3, 3);
  
          for(Map.Entry<Object, Object> entry : map.entrySet()) {
              System.out.println(entry.getKey() + ": " + entry.getValue());
          }
          map.get(2);
          for(Map.Entry<Object, Object> entry : map.entrySet()) {
              System.out.println(entry.getKey() + ": " + entry.getValue());
          }
  输出:
  	1: 1
      2: 2
      3: 3
    执行get之后      
      1: 1
      3: 3
      2: 2
  ```

  

## 4. 总结

> ​	只能说 linkedhashmap 实现 完全基于 父类的 hashmap ,只是重写了一些方法,
>
> ​	还有就是 jdk源码太优秀了 , hashmap设计的太好了



## 5. 我的问题

- ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-13/27eb9d63-285c-48c8-b6de-480c3414238e.png?x-oss-process=style/template01)

- 代码实现 (这里就和 linkedhashmap 很相似)

- ```java
  public class LRUcache {
  
      private  LinkedHashMap<Integer, Integer> map = new LinkedHashMap<>();
  
      private int capacity;
  
      public LRUcache(int capacity) {
          this.capacity = capacity;
      }
      public Integer get(Integer key){
          Integer integer = map.get(key);
          if (null == integer) {
              return -1;
          }
          // 每次用的时候先删除,然后把他插入到队未
          map.remove(key);
          map.put(key, integer);
          return integer;
      }
      public void put(Integer key, Integer value) {
          map.put(key, value);
          if (map.size() > capacity) {
              Set<Integer> integers = map.keySet();
              Optional<Integer> first = integers.stream().findFirst();
              map.remove(first.get());
          }
      }
  }    
  ```

  





**本文参考自**

[引用自--------某位不知名大佬]([https://yikun.github.io/2015/04/02/Java-LinkedHashMap%E5%B7%A5%E4%BD%9C%E5%8E%9F%E7%90%86%E5%8F%8A%E5%AE%9E%E7%8E%B0/](https://yikun.github.io/2015/04/02/Java-LinkedHashMap工作原理及实现/))