# NIO学习

## 1. 认识NIO

### 1.1  nio的名字是什么?

​		有些人称为 **NEW IO** ,还有一些人叫做 **NOBLOCKING IO** , 其实就我认为他是站在两个角度去看待这个问题的,因为从jdk的源码去看 nio位于 java.nio包下,与之对立的是 java.io 包,所以此时从官方的角度就是 NEW IO , 但是我们知道 java中 io 分为三种 :    BIO , NIO , AIO ,其中我不去做解释了 , 兴趣者百度 , 名字不重要 ,重要的是理解 .

### 1.2 初步认识NIO的编程 

​		我认为学习 nio 没必要直接 进行 nio编程 , 理解才最为重要 , 直接模仿别人的代码只能让你感觉你不知道你在做啥. 所以我们就一步一步起步慢慢学习吧.

 		其中它包含了三个概念 , 一个是 selector ,一个是 channel ,还有就是 buffer ,我们拿 一张图去解释 .

​		![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-16/db9aa2dc-66d9-4e9c-ac88-29c7cad570c5.png?x-oss-process=style/template01)

其中这三个对象 是 nio 编程中 **最为核心的三个角色** 所以我们必须牢记 这三个角色.



### 1.3 认识 buffer

​	java中 一共 8个基本类型,对应着7种buffer对象(没有 BooleanBuffer) , 如何初始化一个buffer对象 `ByteBuffer buffer = ByteBuffer.allocate(100);`, 那么怎么进行读写操作. allocate 在C语言编程中也叫分配空间

```java
// 创建一个 buffer对象 ,设置其大小为 50
IntBuffer buffer = IntBuffer.allocate(20);
// 往 buffer中写入数据
for (int i = 0; i < 10; i++) {
    buffer.put(new SecureRandom().nextInt(10));
}
// clear是做啥 ? 
// buffer.clear();	
// 进行翻转 ,如果不使用这个 会发现什么 ,---> 其实这个作用就是让
// buffer.flip();  
// 读  remaining是剩余的意思
while (buffer.remaining()>0) {
    System.out.println(buffer.get());
}

// 注意指定索引位置不改变position位置的

1. 当 buffer.flip(); 不执行会发生什么 ? 
  输出:  0	0	0	0	0	0	0	0	0	0	
   我们发现 他取到的正好不是我们想要的那部分数据,那是为什么呢 ,因为我们看源码发现他只是返回一个数组,按索引值返回.... 那么 索引是哪个呢,发现是 position++ (position< limit) 这就明白了,flip就是将 option的位置放到0,limit等于原来Postion
2.当 buffer.clear()执行,flip不执行会发生什么 ?     
   输出 :  7	0	0	7	1	5	1	0	6	1	0	0	0	0	0	0	0	0	0	0	
  奥原来是直接把整个数组输出了,那么为什么呢,put不是让position++ 吗,那为啥能读到前面数据呢? 
  原来是将三个指针复位了,reset,
3. 所以我们知道了 filp 和 clear做了什么 
4. put() 方法 position++; (position<limit)
```

​		**我们发现buffer 就是一个缓冲区,那么它究竟原理是什么? flip() 又执行了什么?buffer的方法时线程安全的吗**? 

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-16/1874ecb1-3315-4ba1-8fe0-a8772ff7af27.jpg?x-oss-process=style/template01)

我们发现 Buffer 中的 flip() 方法涉及到 Buffer 中的capacity、position、limit三个概念. 其中每当插入数据时 Postion指针 一直向尾移动, limit和 capacity 位于尾部进行限制大小, 而当执行行  flip() 方法时, 让`limit = position;` 使得 `position = 0;`, 此时`buffer.remaining()>0` 执行的又是什么,他返回的是 ` limit - position;` 所以当大于0时,返回的就是我们所插入的数据 .  

- capacity：在读/写模式下都是固定的(不可变的)，就是我们分配的缓冲大小（容量）。
- position：类似于读/写指针，表示将要去读(写)的下一个位置的索引。
- limit：在写模式下表示最多能写入多少数据，此时和capacity相同。在读模式下表示最多能读多少数据，此时和缓存中的实际 数据大小相同。     

```java
jdk中说 flip() It sets the limit to the current position and then sets the position to zero

// Invariants: mark <= position <= limit <= capacity (很有内涵的)
private int mark = -1;
private int position = 0;
private int limit;
private int capacity;    

public final Buffer flip() {
    limit = position;
    position = 0;
    mark = -1;
    return this;
}

public final int remaining() {
    return limit - position;
}
// nextGetIndex() 方法 就是让position索引+1
//offset是偏移量 默认值为0,
// nb是一个初始化capacity大小的数组
protected int ix(int i) {
    return i + offset;
}

public int get() {
    return hb[ix(nextGetIndex())];
}

final int nextGetIndex() {                          // package-private
    if (position >= limit)
        throw new BufferUnderflowException();
    return position++;
}
public final Buffer clear() {
    position = 0;
    limit = capacity;
    mark = -1;
    return this;
}
public IntBuffer put(int x) {
    hb[ix(nextPutIndex())] = x;
    return this;
}
final int nextPutIndex() {                          // package-private
    if (position >= limit)
        throw new BufferOverflowException();
    return position++;
}	
```

​	当我们翻阅API时,发现buffer 并不是 一个线程安全的, clear方法并不会使得数组清空,只是让参数变成初始化的状态.还有就是不会看源码,看注释 jdk的源码注释真的很详细

### 1.4 nio读取文件的方式

​		这里我写一段代码 大家应该可以理解

```java
// model <tt>"r"</tt>, <tt>"rw"</tt>, <tt>"rws"</tt>
RandomAccessFile input = new RandomAccessFile("input.txt", "r");
RandomAccessFile output = new RandomAccessFile("output.txt", "rw");
// 打开一个管道
FileChannel readChannel = input.getChannel();
FileChannel writeChannel = output.getChannel();

// allocateDirect(100)对外内存   allocate(100)
ByteBuffer buffer = ByteBuffer.allocateDirect(100);

while (true) {
    buffer.clear();
    int read = readChannel.read(buffer);
    System.out.println(read);
    if (read == -1) {
        break;
    }
    buffer.flip();
    writeChannel.write(buffer);
}
input.close();
output.close();

问题就是在  sun.nio.ch.IOUtil类中的这个断言和下面的那个判断语句
private static int readIntoNativeBuffer(FileDescriptor var0, ByteBuffer var1, long var2, NativeDispatcher var4) throws IOException {
int var5 = var1.position();
int var6 = var1.limit();

assert var5 <= var6;  // position <= limit 显然无语,完美躲避

int var7 = var5 <= var6 ? var6 - var5 : 0;   这里返回0
if (var7 == 0) {
    return 0;   所以read得到的是0
} else {................}
clear的目的就是复位, 如果不复位的话,那么 position 和 limit 在一起,没有容量,read=0,那为什么是read=0呢,我们发现最终源码,发现 
```

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-16/76867428-fad2-4523-a31b-edfe38c086ff.png?x-oss-process=style/template01)

### 1.5 buffer 对象的 两种实现方式 Direct 与 Heap

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-17/c963a764-01f2-4839-a13c-c100301f2573.png?x-oss-process=style/template01)

 	当我们去看实现方式 大致分为两种 一种是 **heap** ,一种是 **native** ,其中 **DirectByteBufferR**是只读buffer,就是不能进行修改操作的buffer,我们的重点在 heap和direct 上 .

- direct类 我们翻看源码 

  ```java
  ByteBuffer buffer = ByteBuffer.allocateDirect(100);
  
  buffer 源码
  // Used only by direct buffers
  // NOTE: hoisted here for speed in JNI GetDirectBufferAddress
  long address;  // 指向的就是 本地内存的地址
  
  DirectByteBuffer() 我的实现代码我也 讲不清楚,设计到好多 sun公司的代码, sum开头的代码它不是开源的,大家常用的jdk ,其实分为两类 一个是 java,javax 这都是开源的,sun 就不是
  这个类实现实基于jni的,
  ```

- **MappedByteBuffer**使用(A direct byte buffer whose content is a memory-mapped region of a file. 他也是 direct方式的)

  ```java
   RandomAccessFile file = new RandomAccessFile("modify.txt", "rw");
   MappedByteBuffer buffer = file.getChannel().map(FileChannel.MapMode.READ_WRITE, 0, 10);
   buffer.put(0, (byte) 'a');
   buffer.put(4, (byte) 'a');
  
   file.close();
  ```

  

- 如下是我做的两个的对比 ,有兴趣的可以看一下,主要区别就是 **DirectByteBuffer 实现了 零拷贝** 

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-17/2236d4a5-0a31-495d-bda5-40131121f2b7.png?x-oss-process=style/template01)



### 1.6 认识 selector

- **selector**

> ​		A multiplexor(多路复用器) of {@link SelectableChannel} objects. A selector may be created by invoking the {@link  open} method of this class, which will use the system's default {@link SelectorProvider } to create a new selector.
>
> ​	是一个SelectableChannel 的多路复用器 ,其实就是 一个channel的注册器, 然后selector 去复用他.
>
> ​	如何创建?
>
> ​	Selector selector = Selector.open();
>
> ​	如何使用 ?
>
> ​	serverSocketChannel.register(selector, SelectionKey.OP_ACCEPT);

- **Set<SelectionKey> selectionKeys = selector.keys();**

> 这个集合 是什么 ?
>
> ​	它是 注册在 selector上的 channel 对象, 记住 你注册的时候用啥,你获取的时候就取注册的类型,向下转型一般. 例如  ServerSocketChannel  serverChannel = (ServerSocketChannel)selectionKey.channel();

- **SelectionKey**

> ​	A token representing the registration of a {@link SelectableChannel} with a {@link Selector}.
>
> ​	一个 SelectableChannel 的 token (标记) ;
>
> ​	有四个状态 `OP_ACCEPT `   `OP_CONNECT `  `OP_WRITE `  `OP_READ` 



## 2. NIO编写服务器

```java
// 创建一个 SelectableChannel 的对象
ServerSocketChannel serverSocketChannel = ServerSocketChannel.open();
// 这一步必须设置为 非阻塞的
serverSocketChannel.configureBlocking(false);

// 绑定一个socket对象 , 让他去监听 8899 端口 , 跟BIO一样,传统socket编程
ServerSocket socket = serverSocketChannel.socket();
socket.bind(new InetSocketAddress(8899));

// 创建一个 Selector对象
Selector selector = Selector.open();
// 把连接事件注册到 Selector对象上
serverSocketChannel.register(selector, SelectionKey.OP_ACCEPT);
ByteBuffer buffer = ByteBuffer.allocate(1024);

// 死循环,不断轮询
while (true) {

    // 每次进行轮询
    selector.select(); //阻塞

    // 当监听到事件时 执行一下代码
    Set<SelectionKey> selectionKeys = selector.keys();

    selectionKeys.forEach(selectionKey -> {
        // 如果是已连接的
        if (selectionKey.isAcceptable()) {

         // 获取到连接对象
         ServerSocketChannel  serverChannel = (ServerSocketChannel)selectionKey.channel();

            SocketChannel socketChannel = null;
            try {
                // 打开 socketChannel (网络编程入口)
                socketChannel = serverChannel.accept();//阻塞

                if (null != socketChannel) {
                    socketChannel.configureBlocking(false);
                    socketChannel.register(selector, SelectionKey.OP_READ);
                }
            } catch (IOException e) {
//                        e.printStackTrace();
            }

        } else if (selectionKey.isReadable()) {
            SocketChannel channel = (SocketChannel) selectionKey.channel();
            Integer read=0;
            try {
                // 读取到buffer中
                 read = channel.read(buffer);
            } catch (IOException e) {
//                        e.printStackTrace();
            }

            if (read >0) {
                buffer.flip();
                Charset charset = Charset.forName("utf-8");
                char[] array = charset.decode(buffer).array();

                String msg = new String(array);

                System.out.println(msg);

                buffer.clear();
            }
        }
    });
}
```



## 3. 编写日志读写

写 , 每100ms 写一行

```java
public class TestWriter {


    public static void main(String[] args) throws IOException {

        BufferedWriter writer = new BufferedWriter(new FileWriter("C:\\Users\\12986\\Desktop\\file.txt"));

        StringBuilder builder = new StringBuilder(20);
        AtomicInteger integer = new AtomicInteger(0);

        new Timer().schedule(new TimerTask() {
            @Override
            public void run() {
                try {
                    // 设置他的写指针
                    builder.setLength(0);
                    
                    // 添加数据
                    builder.append("当前行 : " + integer.incrementAndGet() + "\n");
                    
                    // 写数据
                    writer.write(builder.toString());
                    
                    // 刷洗记得 , 不然无法保存
                    writer.flush();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }, 0, 100);

    }

}
```

读取 , 读操作

```java
public class TestRead {

    public static void main(String[] args) throws IOException, InterruptedException {

        RandomAccessFile file = new RandomAccessFile(new File("C:\\Users\\12986\\Desktop\\file.txt"), "r");

        FileChannel channel = file.getChannel();
        ByteBuffer allocate = ByteBuffer.allocate(100);

        // 开始长度
        int start = 0;

        while (true) {
            // 1. 清空
            allocate.clear();
            // 2. 读
            int read = channel.read(allocate, start);

            // 3. 如果读取数据为-1 返回
            if (read == -1) continue;

            // 4. start=start+读取长度
            start += read;

            // 变成数组 -> 由于需要读取不需要0拷贝
            byte[] array = allocate.array();

            // 5.读取日志
            String log = new String(array, 0, read, Charset.forName("utf8"));

            // 6. log
            System.out.println(log);
        }
    }
}
```

