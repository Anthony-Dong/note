# 你真正了解java集合吗

## 1. list学习

### 1.主要问题

- 了解一下ArrayList和CopyOnWriteArrayList的**增删改查**实现原理
- 看看为什么说ArrayList查询快而增删慢？
- CopyOnWriteArrayList为什么并发安全且性能比Vector好
- LinkedList对比ArrayList
- Arrays.asList(....)的使用问题
- Collections这个工具类
- System.arraycopy()和Arrays.copy()区别
- java9+  List.of()方法  map , set 同理 都有,不多写了

### 2.为什么arraylist 不安全

- 我们查看源码发现 arraylist 的 CRUD 操作 并么有涉及到锁之类的东西
- 底层是数组，初始大小为10
- 插入时会判断数组容量是否足够，不够的话会进行扩容
- 所谓扩容就是新建一个新的数组，然后将老的数据里面的元素复制到新的数组里面(所以增加较慢)
- 移除元素的时候也涉及到数组中元素的移动，删除指定index位置的元素，然后将index+1至数组最后一个元素往前移动一个格

### 3.CopyOnWriteArrayList  有什么特点?

- 它是List接口的一个实现类,在 **java.util.concurrent**( 简称  JUC ,后面我全部改成 juc ,大家注意下)
- 内部持有一个ReentrantLock lock = new ReentrantLock(); 对于 **增删改** 操作都是 先加锁 再 释放锁 线程安全.并且锁只有一把，而读操作不需要获得锁，支持**并发**。
- 读写分离，写时复制出一个新的数组，完成插入、修改或者移除操作后将新数组赋值给array

### 4.CopyOnWriteArrayList为什么并发安全且性能比Vector好

- Vector是**增删改查**方法都加了**synchronized**，保证同步，但是每个方法执行的时候都要去获得锁，性能就会大大下降，而CopyOnWriteArrayList 只是在**增删改**上加**锁**，但是读不加锁，在读方面的性能就好于Vector，CopyOnWriteArrayList支持读多写少的并发情况。
- Vector和CopyOnWriteArrayList都是 List接口 的一个实现类

### 5.CopyOnWriteArrayList适用于什么情况

- 我们看源码不难发现他每次增加一个元素都要进行一次拷贝,此时严重影响了增删改的性能,其中和arraylist 差了好几百倍 我自己测试过,
- 所以对于读多写少的操作  CopyOnWriteArrayList 更加适合 ,而且线程安全
- **DriverManager**  这个类 就使用到了CopyOnWriteArrayList

### 6. LinkedList 对比 arraylist 的优缺点

- LinkedList 呢是一个 双向链表,他的**添加和删除**优于ArrayList,但是对于**查的功能**差与arraylist
- 只要ArrayList的当前容足够大，add()操作向数组的尾部的效率非常高的，当向数组指定位置添加数据时，会进行大量的数组移动复制操作。而数组复制时，最终将调用System.arraycopy()方法，因此add()操作的效率还是相当高的。尽管这样当向指定位置添加数据时也还是比Linkedlist慢，后者添加数据只需要改变指针指向即可。Arraylist删除数组也需要移动数组，效率较慢。
- Linkedlist基于链表的动态数组，数据添加删除效率高，只需要改变指针指向即可，但是访问数据的平均效率低，需要对链表进行遍历。
- 对于随机访问get和set，ArrayList优于LinkedList，因为LinkedList要移动指针。 对于新增和删除操作add和remove，LinedList比较占优势，因为ArrayList要移动数据。

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-07/4fe202ae-4013-44e0-86fd-8e382149b8a0.png?x-oss-process=style/template01)

### 7. arrays.aslist 是不可变的吗?

- ```java
          List<Integer> integers = Arrays.asList(1, 2, 3, 4, 5);
          integers.set(2, 5);
  //        integers.add(6);
          integers.forEach(System.out::println);
  
  1. 很显然我们是可以修改 list集合的 可以使用set方法
  2. 但是当我们尝试去使用add() 方法时,会抛出 java.lang.UnsupportedOperationException 的异常,
  	不支持操作的异常,这个异常竟然是在 AbstractList 类里,也很厉害,这么超前的思想,
  3.当我们使用 java9+时  可以使用 List.of()方法 ,他就是彻彻底底的不可修改的
  ```

- 怎么保证安全

  ```java
  1. 使用 Collections这个工具类
  	List<Integer> integers1 = Collections.synchronizedList(integers);
  
  2. java5+ 变成 CopyOnWriteArrayList
    CopyOnWriteArrayList<Integer> integers2 = (CopyOnWriteArrayList<Integer>) integers;
  
  3. java9+ ,使用 List.of() 变成只读对象
  ```

### 8. System.arraycopy()和Arrays.copy()区别

- System.arraycopy（）是浅拷贝
- Arrays.copy（） 深拷贝 ,但是底层采用的是 System.arraycopy（）,,**public static <T,U> T[] copyOf(U[] original, int newLength, Class<? extends T[]> newType)**  他返回被copy的对象,
- 切记不能 复制 List对象 只能复制 数组 比如 Integer[] list ,

## 2.set 学习

### 1. 问题

- HashSet、TreeSet和LinkedHashSet三种类型什么时候使用它们
- hashset的实现方式? 那怎么实现一个线程安全的 hashset ,jdk没有 ConcurrentHashSet
- CopyOnWriteArraySet的实现 

### 2.HashSet、TreeSet和LinkedHashSet三种类型什么时候使用它们

- 如你的需求是要一个能快速访问的Set，那么就要用HashSet,如果你要一个排序Set，那么你应该用TreeSet,如果你要记录下插入时的顺序时，你应该使用LinedHashSet
- Set集合中不能包含重复的元素，每个元素必须是唯一的，你只要将元素加入set中，重复的元素会自动移除。
- HashSet是采用hash表算法来实现的，其中的元素没有按顺序排列.
- TreeSet是采用树结构实现(称为红黑树算法)，元素是按顺序进行排列，
- LinkedHashSet正好介于HashSet和TreeSet之间，它也是一个基于散列函数和双向链表的集合，但它同时维护了一个双链表来记录插入的顺序，基本方法的复杂度为O(1)。
- HashSet,LinkedHashSet,TreeSet的元素都只能是对象. 
- 三者 都不支持 同步, 需要使用 Collections.synchronizedSet(new HashSet(…));

### 3. HashSet和LinkedHashSet,TreeSet判定元素重复的原则

首先会去判断对象的 hashcode是否相同 ,如果hashcode相同,则会调用 equals方法进行继续判断 ,

### 3. TreeSet判定元素重复的原则

根据实现的Comparable接口的compareTo(Object  o) ,o是前一个对象 , 进行比较返回结果进行去重.

所以对象必须实现Comparable接口

### 5. HashSet的实现方式? 那怎么实现一个线程安全的 hashset 

> ​	我们看源码 会发现 他里面有一个 HashMap ,那为什么要用(用transient关键字标记的成员变量不参与序列化过程。) 为什么呢 ,因为 HashMap 已经实现了Serializable,

怎么实现一个  ConcurrentHashSet 

- 自己写一个 实现类 实现 Set  ,里面 定义一个 ConcurrentHashSet  ,和 hashset的方式一样 
- 直接用 第三方库 ----- com.alibaba.dubbo.common.utils.ConcurrentHashSet ,阿里的库 ....

```java
package com.alibaba.dubbo.common.utils;
import java.util.AbstractSet;
import java.util.ConcurrentModificationException;
import java.util.Iterator;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

public class ConcurrentHashSet<E> extends AbstractSet<E> implements Set<E>, java.io.Serializable {

    private static final long serialVersionUID = -8672117787651310382L;

    private static final Object PRESENT = new Object();

    private final ConcurrentMap<E, Object> map;

    public ConcurrentHashSet() {
        map = new ConcurrentHashMap<E, Object>();
    }

    public ConcurrentHashSet(int initialCapacity) {
        map = new ConcurrentHashMap<E, Object>(initialCapacity);
    }
    
 ...............   
}
```

很显然跟我说的好像 一模一样 ,哈哈哈 ,我也是看别人学的,只是看你用的巧不巧,他继承了AbstractSet这个抽象类,重写了 他部分想要改的方法, 同时也实现了 set接口

### 6.CopyOnWriteArraySet的实现  

```jav
public CopyOnWriteArraySet() {
     al = new CopyOnWriteArrayList<E>();
 }
```

很显然翻源码我们发现 他实现了  CopyOnWriteArrayList<E>();  哈哈哈,是不是很吃惊 ,底层就是用的CopyOnWriteArrayList 

## 3. map学习

### 1. 问题

- 最常见的问题就是 hashmap的底层实现,jdk7和jdk8的差别 ,这个 我不讲了,如果想要看,自己百度,我推荐 必应(国内), 我提供一个我自己写的一个hashmap源码
- Hashtable、HashMap  以及ConcurrentHashMap 的区别
- 深度学习 ConcurrentHashMap  和  HashMap   靠你们自己了 ,这俩研究透, 你已经向大神进阶了
- ConcurrentSkipListMap 与 TreeMap 的选择
- LinkedHashMap的使用

### 2. Hashtable的学习

- **Hashtable和ConcurrentHashMap以及ConcurrentSkipListMap 以及TreeMap  不允许key 和 value值 为空**,,但是 **hashmap却可以 key 和value值都可以为空,**
- Hashtable的方法 都加了 synchronized 关键字修饰,所以线程安全
- 它是 数组+链表的实现

### 3.ConcurrentHashMap 问题

- 取消segments字段，直接采用transient volatile HashEntry<K,V>[] table保存数据，
- 采用table数组元素作为锁，从而实现了对每一行数据进行加锁，进一步减少并发冲突的概率。
-  把Table数组＋单向链表的数据结构   变成为  Table数组 ＋ 单向链表 ＋ 红黑树的结构。
- 当链表长度超过8以后，单向链表变成了红黑数；  在哈希表扩容时，如果发现链表长度小于 6，则会由红黑树重新退化为链表。
- 对于其他详细我不吹,看懂的么几个 ,他比HashMap 还要难,
- 对于线程安全环境下 介意使用 ConcurrentHashMap  而不去使用 Hashtable

### 4. 为什么不去使用 Hashtable,而去使用ConcurrentHashMap ?

​		HashTable容器使用synchronized来保证线程安全，但在线程竞争激烈的情况下HashTable的效率非常低下。因为当一个线程访问HashTable的同步方法时，其他线程访问HashTable的同步方法时，可能会进入阻塞或轮询状态。如线程1使用put进行添加元素，线程2不但不能使用put方法添加元素，并且也不能使用get方法来获取元素，所以竞争越激烈效率越低。

### 5. HashMap 问题

- 其中部分信息咱们还能聊聊,不会的我就算了

- 内部的Node是HashMap的一个内部类，实现了Map.Entry接口

- 存储结构是 数组+链表 或者 数组+ 红黑树 实现,有个 阈值,当链表长度大于8，大于8的话把链表转换为红黑树,

  当小于等于6时会自动转成链表

  > 原因： (反正我看不懂,只是解决碰撞概率的问题,数学问题这个是)
  >
  > 　　红黑树的平均查找长度是log(n)，长度为8，查找长度为log(8)=3，链表的平均查找长度为n/2，当长度为8时，平均查找长度为8/2=4，这才有转换成树的必要；链表长度如果是小于等于6，6/2=3，虽然速度也很快的，但是转化为树结构和生成树的时间并不会太短。
  >
  > 还有选择6和8的原因是：
  >
  > 　　中间有个差值7可以防止链表和树之间频繁的转换。假设一下，如果设计成链表个数超过8则链表转换成树结构，链表个数小于8则树结构转换成链表，如果一个HashMap不停的插入、删除元素，链表个数在8左右徘徊，就会频繁的发生树转链表、链表转树，效率会很低。

- Node[] table的初始化长度length(默认值是16)，Load factor为负载因子(默认值是0.75)，

- 实现链接,大家不会写可以看看

  [hashmap链表+数组实现]: http://note.youdao.com/noteshare?id=2392292d22c867e806266a8bfcbf719e

- HashMap是非synchronized ,线程不安全

- 大家可以看看高能讲解:

  [美团大佬写的]: https://zhuanlan.zhihu.com/p/21673805


### 6. ConcurrentSkipListMap 与 TreeMap 的选择

- ConcurrentSkipListMap提供了一种线程安全的并发访问的排序映射表。内部是SkipList（跳表）结构实现，利用底层的插入、删除的CAS原子性操作，通过死循环不断获取最新的结点指针来保证不会出现竞态条件。在理论上能够在O(log(n))时间内完成查找、插入、删除操作。调用ConcurrentSkipListMap的size时，由于多个线程可以同时对映射表进行操作，所以映射表需要遍历整个链表才能返回元素个数，这个操作是个O(log(n))的操作。
- 在JDK1.8中，ConcurrentHashMap的性能和存储空间要优于ConcurrentSkipListMap，但是ConcurrentSkipListMap有一个功能： **它会按照键的自然顺序进行排序**。
- 故需要对**键值排序**，则我们可以使用TreeMap，在**并发场景**下可以使用ConcurrentSkipListMap。
- 所以 我们并不会去 纠结 ConcurrentSkipListMap 和 ConcurrentHashMap 两者的选择,因为我解释的很好了

### 7. LinkedHashMap的使用

- 主要是为了解决读取的有序性,
- 基于 HashMap 实现的 

## 4.queue队列学习

> ​	队列 在于你走向高级工程师必须走的一步 . 一开始我们对于他并不了接,但是你会发现并发包里面一堆关于队列的类,你就知道了他的关键所在
>
> ​	通过我这段时间的学习,我发现在线程池这块,还有这消息队列,还有在数据库连接池这块都需要队列.这些中间件对于队列的依赖性太过于强烈.
>
> ​	所以学会队列是很重要的一步.这些内容我会慢慢补充的.

### 1. 入门

#### 1.queue 跟 list and set  有啥区别

- 我们都知道**队列**(Queue)是一种**先进先出**(FIFO)的数据结构，Java中定义了`java.util.Queue`接口用来表示队列。Java中的`Queue`与`List`、`Set`属于同一个级别接口，它们都是继承于`Collection`接口。

  Java中还定义了一种双端队列`java.util.Deque`，我们常用的`LinkedList`就是实现了`Deque`接口。

#### 2.Deque 是什么

- 它是一个双端队列
- 我们用到的 linkedlist 就是 实现了  deque的接口
- 支持在两端插入和移除元素
- 区别与 循环队列 [循环队列实现讲解](https://www.cnblogs.com/chenliyang/p/6554141.html)

#### 3.队列分类 -  非阻塞队列

- **LinkedList**

  `LinkedList`是双相链表结构，在添加和删除元素时具有比`ArrayList`更好的性能。但在 Get 与 Set 方面弱于`ArrayList`。当然，这些对比都是指数据量很大或者操作很频繁的情况下的对比。

- **PriorityQueue**

  PriorityQueue维护了一个有序列表，存储到队列中的元素会按照自然顺序排列。当然，我们也可以给它指定一个实现了 `java.util.Comparator` 接口的排序类来指定元素排列的顺序。

- **ConcurrentLinkedQueue**

  `ConcurrentLinkedQueue` 是基于链接节点的并且线程安全的队列。因为它在队列的尾部添加元素并从头部删除它们，所以只要不需要知道队列的大小 `ConcurrentLinkedQueue` 对公共集合的共享访问就可以工作得很好。收集关于队列大小的信息会很慢，需要遍历队列。

### 2. (重点)ArrayBlockingQueue与LinkedBlockingQueue的区别,哪个性能好呢

- ArrayBlockingQueue是有界queue

- LinkedBlockingQueue 是无界的也可能是无界的(因为构造器的区别,当你指定了大小,他就是有界的)

- 但是当 take 和 put操作时 , **array 速度要快于 linked** .**原因是什么**

  > 1.队列中的锁的实现不同
  >
  > ​       ArrayBlockingQueue中的锁是没有分离的，即生产和消费用的是同一个锁；
  >
  > ​       LinkedBlockingQueue中的锁是分离的，即生产用的是putLock，消费是takeLock
  >
  > 2.在生产或消费时操作不同
  >
  > ​     ArrayBlockingQueue基于数组，在生产和消费的时候，是直接将枚举对象插入或移除的，不会产生或销毁任何额外的对象实例；
  >
  > ​     LinkedBlockingQueue基于链表，在生产和消费的时候，需要把枚举对象转换为Node<E>进行插入或移除，会生成一个额外的Node对象，这在长时间内需要高效并发地处理大批量数据的系统中，其对于GC的影响还是存在一定的区别。

- **问题有哪些**

  > - **在使用LinkedBlockingQueue时，若用默认大小且当生产速度大于消费速度时候，有可能会内存溢出。**
  > - 在使用ArrayBlockingQueue和LinkedBlockingQueue分别对1000000个简单字符做入队操作时，
  >
  > ​       LinkedBlockingQueue的消耗是ArrayBlockingQueue消耗的10倍左右，
  >
  > ​       即LinkedBlockingQueue消耗在1500毫秒左右，而ArrayBlockingQueue只需150毫秒左右。
  >
  > - 按照实现原理来分析，**ArrayBlockingQueue完全可以采用分离锁，从而实现生产者和消费者操作的完全并行运行**。Doug Lea之所以没这样去做，也许是因为ArrayBlockingQueue的数据写入和获取操作已经足够轻巧，以至于引入独立的锁机制，除了给代码带来额外的复杂性外，其在性能上完全占不到任何便宜。
  >
  > ​	

- FIFO( First Input First Output)简单说就是指先进先出。就是指队列

### 3.LinkedTransferQueue与LinkedBlockingQueue的区别

- 前者比后者性能好

### 4. (重点)BlockingQueue的问题 以及  ConcurrentLinkedQueue 的问题

- `BlockingQueue` 可以是限定容量的。

- `BlockingQueue` 实现主要用于生产者-使用者队列，但它另外还支持collection接口。

- `BlockingQueue` 实现是线程安全的

  **区别**

  > ​	    **LinkedBlockingQueue**是一个线程安全的阻塞队列，基于链表实现，一般用于生产者与消费者模型的开发中。采用锁机制来实现多线程同步，提供了一个构造方法用来指定队列的大小，如果不指定大小，队列采用默认大小（Integer.MAX_VALUE，即整型最大值）。
  >
  > ​	  **ConcurrentLinkedQueue**是一个线程安全的非阻塞队列，基于链表实现。java并没有提供构造方法来指定队列的大小，因此它是无界的。为了提高并发量，它通过使用更细的锁机制，使得在多线程环境中只对部分数据进行锁定，从而提高运行效率。他并没有阻塞方法,take和put方法.注意这一点

### 5.简要概述BlockingQueue常用的五个实现类

- **ArrayBlockingQueue** 

  ArrayBlockingQueue:规定大小的BlockingQueue,其构造函数必须带一个int参数来指明其大小.其所含的对象是以**FIFO**(先入先出)顺序排序的.

- **LinkedBlockingQueue**

  大小不定的BlockingQueue,若其构造函数带一个规定大小的参数,生成的BlockingQueue有大小限制,若不带大小参数,所生成的BlockingQueue的大小由Integer.MAX_VALUE来决定.其所含的对象是以**FIFO**(先入先出)顺序排序的

- **PriorityBlockingQueue** 

  类似于LinkedBlockQueue,但其所含对象的排序不是FIFO,而是依据对象的**自然排序顺序**或者是**构造函数的Comparator**决定的顺序.

- **DelayQueue** 

  一个内部由优先级堆支持的、基于时间的调度队列。队列中存放Delayed元素，只有在延迟期满后才能从队列中提取元素。当一个元素的getDelay()方法返回值小于等于0时才能从队列中poll中元素，否则poll()方法会返回null。　　

- **SynchronousQueue** 

  对其的操作必须是放和取交替完成的.

### 6.(重点**小顶堆**)优先队列 PriorityQueue与PriorityBlockingQueue 

> ​	小顶堆是什么 :  任意一个非叶子节点的权值，都不大于其左右子节点的权值

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-28/b9f34cf3-a483-4440-9a4b-99ec627c3059.png?x-oss-process=style/template01)

- PriorityQueue是非线程安全的，PriorityBlockingQueue是线程安全的

- 两者都使用了堆，算法原理相同

- PriorityBlockingQueue可以在queue为空时阻塞take操作

- PriorityBlockingQueue使用重入锁，每一个操作都需要加锁

- PriorityQueue的逻辑结构是一棵完全二叉树，存储结构其实是一个数组。逻辑结构层次遍历的结果刚好是一个数组。

- PriorityQueue() 使用默认的初始容量（11）创建一个 `PriorityQueue`，并根据其元素的comparable接口实现进行排序

- [PriorityQueue讲解1](https://blog.csdn.net/learn_tech/article/details/86510599)

- [PriorityQueue讲解2](https://www.cnblogs.com/lanhaicode/p/10546257.html)

- ```java
  1. 首先java 中的 PriorityQueue 是优先队列,使用的是小顶堆实现
  2. 什么是小顶堆 (父节点,永远小于左右子节点) ,因此结果不一定是完全升序
  3. 什么是大顶堆 跟 小顶堆相反,
  4. 优先队列中 对于当offer操作,当插入的元素此时长度大于默认长度会进行数组扩容(system.copyarr()方法)
      所以他其实是一个无界数列
  4. 所以  优先队列 是数组实现,他不需要占用太大的物理空间,而是进行了深度的排序
  ```

- ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-12/a8561cae-59b1-474d-bc44-62c7414a2ee4.png?x-oss-process=style/template01)

### 7.常用的方法

- **add**          增加一个元索(其实调用的就是offer 方法)    如果队列已满，则抛出一个IIIegaISlabEepeplian异常 
- **remove**   移除并返回队列头部的元素    如果队列为空，则抛出一个NoSuchElementException异常 　　
- **element**  返回队列头部的元素               如果队列为空，则抛出一个NoSuchElementException异常 　　
- **offer**        添加一个元素并返回true        如果队列已满，则返回false 　　
- **poll**          移除并返问队列头部的元素    如果队列为空，则返回null 　　
- **peek**        返回队列头部的元素                如果队列为空，则返回null 　　
- **put**           添加一个元素                           如果队列满，则阻塞 　(像队尾插入一个元素)　 block有
- **take**         移除并返回队列头部的元素   如果队列为空，则阻塞 (像队头移除一个元素,并且整体向前移动,保证对头不为空)　 block有



主要是 put和take这两个方法 ,决定了blockqueue作为队列的性质.

```java
 		LinkedBlockingQueue<Connection> connections = new LinkedBlockingQueue<Connection>(5);
        connections.offer(new Connection("1"));
        connections.offer(new Connection("2"));
        connections.offer(new Connection("3"));
        connections.offer(new Connection("4"));
        connections.offer(new Connection("5"));
        //首先赋值5个连接
        System.out.println("start");
        connections.forEach(System.out::println);
        
        // 移除掉头部的,然后再像尾部put一个
        Connection connection = connections.take();
        connections.put(connection);
        System.out.println("start");
        connections.forEach(System.out::println);
```



但是我们日常使用的无非是 **LinkedBlockingQueue** 和 **ArrayBlockingQueue** 但是哪个性能好呢,我们试着比较一下: 

```java
  LinkedBlockingQueue<Connection> connections = new LinkedBlockingQueue<Connection>(5);
        connections.offer(new Connection("1"));
        connections.offer(new Connection("2"));
        connections.offer(new Connection("3"));
        connections.offer(new Connection("4"));
        connections.offer(new Connection("5"));

        ArrayBlockingQueue<Connection> connections1 = new ArrayBlockingQueue<>(5);
        connections1.offer(new Connection("1"));
        connections1.offer(new Connection("2"));
        connections1.offer(new Connection("3"));
        connections1.offer(new Connection("4"));
        connections1.offer(new Connection("5"));


        long start1 = System.currentTimeMillis();
        for (int x = 0; x < 1000000; x++) {
            Connection connection = connections.take();
            connections.put(connection);
        }
        System.out.println(System.currentTimeMillis() - start1);

        long start2 = System.currentTimeMillis();
        for (int x = 0; x < 1000000; x++) {
            Connection connection = connections1.take();
            connections1.put(connection);
        }
        System.out.println(System.currentTimeMillis() - start2);

结果 : 
117
62
```

结果 比较 还是很明显的. **ArrayBlockingQueue** 优于 **LinkedBlockingQueue** .在这种情况下

**但是再怎么比也没有 非阻塞的 ConcurrentLinkedQueue 快, 代码不放了,结果我已经说了.**





### 7. 劝退版代码

```java
1. 版本1 请大佬复制运行
	PriorityBlockingQueue<Integer> queue = new PriorityBlockingQueue<>(2);

        queue.add(9);
        queue.add(1);
        queue.add(8);

        System.out.println("queue.size() = " + queue.size());

        // 尝试取消掉下一行
        System.out.println("queue.take() = " + queue.take());

        System.out.println("queue = " + queue);

	1. put方法 并不阻塞
    2. offer 方法 也不限制
    3. 插入对象会排序,由compareable的实现

2. 进阶版 demo

 public static void main(String[] args) throws Exception{

        offer(new ArrayBlockingQueue<Integer>(2));
        offer(new LinkedBlockingDeque<Integer>(2));
        offer(new PriorityBlockingQueue<Integer>(2));
        offer(new SynchronousQueue<Integer>());

    }

    private static void offer(BlockingQueue<Integer> queue) throws Exception {

        System.out.println("queue.getClass().getName() = " + queue.getClass().getName());
        System.out.println("queue.offer(1) = " + queue.offer(1));
        System.out.println("queue.offer(2) = " + queue.offer(2));
        System.out.println("queue.offer(3) = " + queue.offer(3));
        System.out.println("queue.size() = " + queue.size());
        System.out.println("queue.take() = " + queue.take());
    }
尝试运行一下

```


