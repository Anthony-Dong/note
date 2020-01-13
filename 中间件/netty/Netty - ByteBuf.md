# Netty - ByteBuf

> ​	Netty的数据包对象, 很重要 ,他不是使用的Java中Nio中的ByteBuff对象, 是一种特殊的结构体, 下面可以看看

## 1. 基本概念

> ​	ByteBuf提供了两个指针变量 去支持有序的读写操作 (readerIndex) 和 (writerIndex)  .
>
> ​	其实现的ReferenceCounted是一个引用计数接口,实现垃圾回收的,他不是由GC控制的,所以使用时注意不然会出现内存溢出的现象 , 所以用Netty需要注意ByteBuf的释放问题
>
> ​	当一个对象初始化完成后，它的reference count 是1, 调用retain()加1，调用release()减1并且如果reference count是0的话就释放。使用被释放掉的对象将会报错。
>
>  数据结构 :   R+W指针 ,  分为 三块区域 { discardable bytes (已读,可废弃区), readable bytes (可读区), writable bytes (可写)}

基本示意图 , 提供的 

    +-------------------+------------------+------------------+
    | discardable bytes |  readable bytes  |  writable bytes  |
    |                   |     (CONTENT)    |                  |
    +-------------------+------------------+------------------+
    |                   |                  |                  |
    0      <=      readerIndex   <=   writerIndex    <=    capacity

## 2. 基本使用

> ​	他有一个工具类, 可以去了解一下 ByteBufUtil , 

### 1. 实例化

```java
@Test
public void testInstance() {
    System.out.println("===============堆内存======================");
    ByteBuf buf = Unpooled.copiedBuffer("HELLO WORLD !", CharsetUtil.UTF_8);
    System.out.println("是否是直接内存 : " + buf.isDirect());
    String data = buf.toString(CharsetUtil.UTF_8);
    System.out.println("数据 : " + data);


    System.out.println("================直接内存====================");
    ByteBuf buffer = Unpooled.directBuffer(4);
    buffer.writeInt(10);
    System.out.println("是否是直接内存 : " + buffer.isDirect());
    System.out.println("数据 : " + buffer.readInt());

}
```

输出 : 

```java
===============堆内存======================
是否是直接内存 : false
数据 : HELLO WORLD !
================直接内存====================
是否是直接内存 : true
数据 : 10
```



### 2. 基本参数

#### 1. 写指针

> ​	`write...()` 写只能顺写, 不能指定位置写, 如果需要指定位置写,需要修改写指针位置 , 会改变写指针位置
>
> ​	使用`set...`方法,这样子不改变写指针位置

```java
@Test
public void testWriterIndex(){
    ByteBuf buffer = Unpooled.buffer(10);
    buffer.writeInt(10);
    buffer.writeInt(9);
    buffer.writeShort(8);

    System.out.println("写指针位置 : "+buffer.writerIndex());

    // 初始化 , 并不会清空数据
    buffer.writerIndex(0);
    System.out.println("写指针位置 : "+buffer.writerIndex());
}
```

输出

```java
写指针位置 : 10
写指针位置 : 0
```

#### 2. 读指针

> ​	`read...()`读也是顺读, 需要的注意的是, 改变读指针位置可以改变读额位置  , 会改变写指针位置
>
> ​	或者使用 `get...()` 方法, 这个不会改变读指针位置

```java
@Test
public void testReadIndex(){
    ByteBuf buffer = Unpooled.buffer(10);
    buffer.writeCharSequence("helloworld", CharsetUtil.UTF_8);

    byte b = buffer.readByte();
    System.out.println("第一位 : "+(char) b);
    System.out.println("读指针位置 : " + buffer.readerIndex());
}
```

输出

```java
第一位 : h
读指针位置 : 1
```

#### 3. 计数引用

```java
@Test
public void testReferenceCounted(){
    ByteBuf buffer = Unpooled.buffer(10);
    buffer.writeCharSequence("helloworld", CharsetUtil.UTF_8);

    buffer.retain(); // 计数引用加一
    System.out.println(buffer.refCnt()); // 2

    buffer.release(); // 计数引用减一
    System.out.println(buffer.refCnt());  // 1
    
    // 当等于零时 ,这个数据会自动被彻底清空 再调用会抛出IllegalReferenceCountException异常
}
```

## 3. 深浅拷贝

> ​	深拷贝的意思就是我修改我拷贝的数据不改变原来的数据 , 浅拷贝就是我修改数据会改变原来的数据

快速举个例子

```java
@Test
public void testCopy(){
    ByteBuf buffer = Unpooled.buffer(10);
    buffer.writeCharSequence("helloworld", CharsetUtil.UTF_8);

    System.out.println("=========深拷贝===========");

    ByteBuf copy = buffer.copy(0, 1);
    copy.setByte(0, 1);
    System.out.println(copy.getByte(0)); // 1
    System.out.println(buffer.getByte(0)); // 104


    System.out.println("=========浅拷贝===========");
    ByteBuf slice = buffer.slice(0, 1);
    slice.setByte(0, 1);
    System.out.println(slice.getByte(0)); // 1
    System.out.println(buffer.getByte(0)); // 1 
}
```

这就是区别

### 1. 视图 / 副本 - 浅拷贝方法

> You can create a view of an existing buffer by calling one of the following methods:
>
> - duplicate()
> - slice()
> - slice(int, int)
> - readSlice(int)
> - retainedDuplicate()
> - retainedSlice()
> - retainedSlice(int, int)
> - readRetainedSlice(int)

### 2. 视图 - 浅拷贝

#### 1. slice() = duplicate()  只展示一种

```java
// Unpooled 是非池化的意思 ,官方推荐自己定义使用非池化 , 可以看看文档去
ByteBuf buf = Unpooled.buffer(10);
buf.writeShort(100);
buf.writeInt(1000);
buf.writeInt(10000);
// 读取一个 r -> 2
System.out.println("buf.readShort() = " + buf.readShort());

System.out.println("buf.slice() = " + buf.slice());

// 再读取一个 r- > 4
System.out.println("buf.readInt() = " + buf.readInt());
```

输出结果 :  我们发现就是截取了从 r -> capacity 的长度

```java
buf.readShort() = 100
buf.slice() = UnpooledSlicedByteBuf(ridx: 0, widx: 8, cap: 8/8, unwrapped: UnpooledByteBufAllocator$InstrumentedUnpooledUnsafeHeapByteBuf(ridx: 2, widx: 10, cap: 10))
buf.readInt() = 1000
```

#### 2. slice(int index, int len)  与 readSlice(int length)

```java
ByteBuf buf = Unpooled.buffer(10);
buf.writeShort(100);
buf.writeInt(1000);
buf.writeInt(10000);
// 读取一个 r -> 2
System.out.println("buf.readShort() = " + buf.readShort());

// 2开始 截取 4个长度
ByteBuf slice = buf.slice(2, 4);

// 这个buf上 从0开始设置一个 数值为66666占四个字节的数字
slice.setInt(0, 66666);

// 在原来的上面从 r指针开始 4个字节长度 , 他相当于 slice() 方法第一个参数是 buf.readerIndex() , 
ByteBuf buf1 = buf.readSlice(4);

//输出读到的
System.out.println("slice.readInt() = " + buf1.readInt());
```

输出结果 :  我们发现副本值是相互影响的 ,所以不会产生额外对象

```java
buf.readShort() = 100
slice.readInt() = 66666
```

#### 3.  加了retained 前缀的方法

```java
ByteBuf buf = Unpooled.buffer(10);
buf.writeShort(100);
buf.writeInt(1000);
buf.writeInt(10000);

System.out.println("buf.refCnt() = " + buf.refCnt());

buf.retainedSlice(2, 4);

System.out.println("buf.refCnt() = " + buf.refCnt());

buf.slice(2, 4);

System.out.println("buf.refCnt() = " + buf.refCnt());
```

输出结果 :  retain 使得计数引用加一 

```java
buf.refCnt() = 1
buf.refCnt() = 2
buf.refCnt() = 2
```

### 3. 深拷贝 - copy()

```java
ByteBuf buf = Unpooled.buffer(10);

buf.writeShort(100);
buf.writeInt(1000);
buf.writeInt(10000);

// 复制
ByteBuf copy = buf.copy();
copy.setShort(0, 1000);
System.out.println("buf.readShort() = " + buf.readShort() + " : " + buf.readerIndex()+" : "+buf.refCnt());

//重置r指针 , 不能使用buf.clear();会使得 w指针也重置为0
buf.readerIndex(0);
// 视图
ByteBuf duplicate = buf.duplicate();
duplicate.setShort(0, 1000);
System.out.println("buf.readShort() = " + buf.readShort() + " : " + buf.readerIndex()+" : "+buf.refCnt());
```

输出 : 

```java
buf.readShort() = 100 : 2 : 1
buf.readShort() = 1000 : 2 : 1
```

所以copy()方法是在原来的基础上直接拷贝了一份 , 所以修改对原来不影响 ,但此时容易造成内存泄漏,而视图确实不一样 他修改会改变原来的, 看情况选择两者的使用

### 4. 深拷贝 - buf.readBytes(int len);

```java
ByteBuf buf = Unpooled.buffer(2);

//设置一个值
buf.writeShort(100);

// 读取两个字节返回一个 buf1
ByteBuf buf1 = buf.readBytes(2);

System.out.println("buf1 = " + buf1);

// buf1 中设置值为2000
buf1.setShort(0, 2000);

//输出设置的值
System.out.println("buf1.readShort() = " + buf1.readShort());

// 调整reader指针位置
buf.readerIndex(0);

//输出buf的值
System.out.println("buf.readShort() = " + buf.readShort());
```

结果 : 所以此时获取到的是  : 

```java
buf1 = UnpooledByteBufAllocator$InstrumentedUnpooledUnsafeNoCleanerDirectByteBuf(ridx: 0, widx: 2, cap: 2)
buf1.readShort() = 2000
buf.readShort() = 100
```



## 4. 大端小端 

[参考文章](https://blog.csdn.net/jason_cuijiahui/article/details/79010257) : https://blog.csdn.net/jason_cuijiahui/article/details/79010257 对于概念讲的比较细

> 1. 大端模式是指**数据的低位保存在内存的高地址中，而数据的高位保存在内存的低地址中**.
> 2. 小端模式是指**数据的低位保存在内存的低地址中，而数据的高位保存在内存的高地址中**。

```java
System.out.println(ByteOrder.nativeOrder());


ByteBuf buf = Unpooled.buffer(10);

// 这个必须用返回的视图操作 ,netty中文档说了 , 
ByteBuf order = buf.order(ByteOrder.LITTLE_ENDIAN);

order.writeShort(100);
order.writeInt(1000);
order.writeInt(10000);


System.out.println("buf.readerIndex() = " + buf.readerIndex());


System.out.println("buf.readerIndex(2) = " + buf.readerIndex(2));

// 小端读取 的意思 LITTLE_ENDIAN的缩写
System.out.println("buf.readIntLE() = " + buf.readIntLE());
// 大端读取 
System.out.println("buf.readInt() = " + buf.readInt());
```

输出 : X86目前都是小端模式 ,netty默认是大端模式

```java
LITTLE_ENDIAN
buf.readerIndex() = 0
buf.readerIndex(2) = UnpooledByteBufAllocator$InstrumentedUnpooledUnsafeHeapByteBuf(ridx: 2, widx: 10, cap: 10)
buf.readIntLE() = 1000
buf.readInt() = 270991360
```