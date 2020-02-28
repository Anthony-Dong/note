# Java - I/O体系应用 - 续集

[Java - I/O体系从原理到应用](https://anthony-dong.gitee.io/post/flc5KkhMY/)  写的就不错, 这一篇只是相应的补充 . 



## 1.  RandomAccessFile 的mode

> ​	`RandomAccessFile  `和 `FileInputStream`  都可以拿到一个`FileChannel` , 这个channel是NIO的产物, 是一个双向的流.

| Value     | Meaning                                                      |
| --------- | ------------------------------------------------------------ |
| `"r"`     | Open for reading only. Invoking any of the `write` methods of the resulting object will cause an [`IOException`](http://www.mpi-inf.mpg.de/departments/d5/teaching/ss05/is05/javadoc/java/io/IOException.html) to be thrown. |
| `"rw"`    | Open for reading and writing. If the file does not already exist then an attempt will be made to create it. |
| `"rws"`   | Open for reading and writing, as with `"rw"`, and also require that every update to the file's content or metadata be written synchronously to the underlying storage device. |
| `"rwd"  ` | Open for reading and writing, as with `"rw"`, and also require that every update to the file's content be written synchronously to the underlying storage device. |



## 2. channel.read(buffer) 返回值

`int read = rchannel.read(buffer);`  的返回值有些时候需要判断. 

- `-1`  : 负一表示读不到数据  , 代表channel中没有可读数据
- `0`  :  代表buffer 的 limit和position相等, 无法写入
- `>0`  :  代表从channel中读取多少个字节数据. 

## 3. Java的 sendfile实现

> ​	其实就chanel 的直接拷贝的实现 , Java send file api 是 transferTo 方法和 transferFrom 方法。
>
> 为了理解 sendfile 的意义，了解数据从文件到套接字的常见数据传输路径就非常重要：
>
> 1. 操作系统从磁盘读取数据到内核空间的 pagecache
> 2. 应用程序读取内核空间的数据到用户空间的缓冲区
> 3. 应用程序将数据(用户空间的缓冲区)写回内核空间到套接字缓冲区(内核空间)
> 4. 操作系统将数据从套接字缓冲区(内核空间)复制到通过网络发送的 NIC 缓冲区
>
> 这显然是低效的，有四次 copy 操作和两次系统调用。使用 sendfile 方法，可以允许操作系统将数据从 pagecache 直接发送到网络，这样避免重新复制数据。所以这种优化方式，只需要最后一步的copy操作，将数据复制到 NIC 缓冲区。

[文章链接](https://www.jianshu.com/p/713af3a13bde) :  https://www.jianshu.com/p/713af3a13bde

对于 `sendfile` 只能使用` transferTo()`  ,from不可以使用 , 只能将文件复制到网卡.

```java
SocketChannel socketChannel = serverSocketChannel.accept();
FileInputStream file = new FileInputStream("resp.text");
FileChannel readChannel = file.getChannel();
// 这里可以直接文件数据复制到NIC缓冲区(UNIX) , 叫 `sendfile`
readChannel.transferTo(0, readChannel.size(), socketChannel);
// close 释放资源
```

## 4. NIO 基本感受

nio 与 普通的io 区分就是在于io是基于stream流的, 而nio是基于channel的. channel是一个双向的流.

对比传统的io编程中, 使用到的 `ServerSocket` 和 `Socket`   对应到 NIO中就是 `ServerSocketChannel` 和 `SocketChannel`  , 但是NIO的核心是selector (多路复用) , 是的传统的阻塞编程变得非阻塞. 这个很重要. 可以看看[Java -IO体系](https://anthony-dong.gitee.io/post/flc5KkhMY/)那篇文章感受一下. 



## 5. MMAP  使用

[百度百科 :](https://baike.baidu.com/item/mmap/1322217) https://baike.baidu.com/item/mmap/1322217

[关于MMAP的这主要实现以及理论 : ](https://www.jianshu.com/p/c3afc0f02560)   https://www.jianshu.com/p/c3afc0f02560

> ​	mmap是一种内存映射文件的方法，即将一个文件或者其他对象映射到进程的地址空间，实现文件磁盘地址和进程虚拟地址空间中一段虚拟地址的一一对应关系；实现这样的映射关系后，进程就可以采用指针的方式读写操作这一块内存，而系统会自动回写脏页面到对应的文件磁盘上，即完成了对文件的操作而不必调用read，write等系统调用函数，相反，内核空间堆这段区域的修改也直接反应到用户空间，从而可以实现不同进程间的文件共享。

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/efe52780-3d1c-4c9e-ae65-57d1328661e5.png)



对于正常的IO操作 . 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-10-11/6834f5a6-015b-4cbc-8bd1-82113a41d4d6.webp)



**需要四次拷贝才能修改文件 , 而MMAP只需要 一次. **

Java中使用`mmap `

基于NIO的 FileChannel . 

首先我的测试文件`test.properties`内容是 `Hello World` , 然后我们使用mmap.

```java
public static void main(String[] args) throws IOException {

    // 1. 获取 FileChannel
    RandomAccessFile file = new RandomAccessFile("test.properties", "rw");

    FileChannel channel = file.getChannel();

    // 2. 指定1024大小的空间.这个是直接内存(不属于GC管理范围,所以节省了GC回收).
    // 第一个是mode . 第二个是文件映射起始位置(其实就是文件内容的offset), 第三个是文件映射长度.
    MappedByteBuffer buffer = channel.map(FileChannel.MapMode.READ_WRITE, 0, 1024);

    // 读取
    byte[] reader = new byte[20];
    buffer.get(reader);
    System.out.printf("read : %s\n", new String(reader, StandardCharsets.UTF_8));

    // 重置位置. 然后我们继续写入
    buffer.clear();
    buffer.put("我写入的内容".getBytes(StandardCharsets.UTF_8));


    // 重置位置. 然后我们继续读取,不用filp的目的是懒得进行位置修改
    buffer.clear();
    buffer.get(reader);
    System.out.printf("read : %s\n", new String(reader, StandardCharsets.UTF_8));
}
```

输出 : 

```go
read : Hello World      
read : 我写入的内容  
```

