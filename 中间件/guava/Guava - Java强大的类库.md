# Guava - Java强大的类库

> ​	`Guava `是Google推出的Java通用包, 很强大的类库 , 基本包含了 IO,多线程 ,集合, 字符串, 空指针检测, 很多



> ​	[官方链接 : ](https://github.com/google/guava/wiki)  https://github.com/google/guava/wiki  ,可以挨着看.......



## 1. 文件IO

> ​	`ByteSource` , source是源的意思 , `ByteSink` 是下层的意思 ,  所以一个是进, 一个是出, 一个是读, 一个是写 , 同时你不用关系IO的关闭 , 

### 1. 快速开始

下面这个代码就是一个文件拷贝 , 从 in.txt 拷贝到 out.txt

```java
@Test
public void testStart() throws IOException {
    ByteSource source = Files.asByteSource(new File("file/in.txt"));
    source.copyTo(Files.asByteSink(new File("file/out.txt")));
   
}
```

是不是相对于Java的Stream 方便了太多 , 

### 2. Files

>  他是文件操作的源 , 提供了很多静态方法
>
>  主要实现的方法有以下几个 , 常用的流转换他都提供了 , 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-08/179d1a99-e468-4d94-9f75-48e8b54589ff.jpg?x-oss-process=style/template01)

### 2. ByteSource

> ​	这个对象代表一个数据源,  可以将一个文件读取成一个字节流 , 其实就是Java的`InputStream` , 

他的 `ByteSource#read`方法可以转换成字节数组 , 他的`ByteSource#openBufferedStream` 可以打开一个输入流, 他的`ByteSource#slice` 还可以切割输入自己流

```java
@Test
public void testSource() throws IOException {

    ByteSource source = Files.asByteSource(new File("file\\in.txt"));

    source.sizeIfKnown();// 防止空指针

    source.copyTo(new FileOutputStream("file/out.txt")); // 拷贝

    source.copyTo(Files.asByteSink(new File("file/out.txt"), FileWriteMode.APPEND)); // 拷贝

    source.slice(0, 10); // 切割

    source.read();  // 读取一个流

    source.asCharSource(Charset.forName("utf-8")); // 读取一个CharSource流(字符流)

    source.hash() // 求hash码

    source.openStream(); // 返回一个 inputStream 对象

}
```

### 3. CharSource

> ​	 字符流

### 4. ByteSink

> ​	这个可以比作成一个 `OutputStream` 对象 , 他执行写操作

```java
@Test
public void testSink() throws IOException {

    ByteSink byteSink = Files.asByteSink(new File(""));

    byteSink.openStream();  // 返回一个 OutputStream 对象

    byteSink.write(new byte[1024]);  // 往文件中写入字节数组

    byteSink.writeFrom(new FileInputStream(""));  // 写入一个文件输入流

    byteSink.asCharSink();  // 转换成 CharSink
}
```

### 5. CharSink

> ​	可以很轻松的写入日志 , 比如 .....

```java
CharSink charSink = Files.asCharSink(new File("file\\out4.txt"), Charset.forName("utf-8"), FileWriteMode.APPEND);
// 写入操作很方便
charSink.write(new StringBuilder("sb"));
```



## 2. EventBus

就不讲解了 , 我的有一篇文章中写过 , [链接](https://anthony-dong.github.io/post/jian-ting-zhe-mo-shi) : https://anthony-dong.github.io/post/jian-ting-zhe-mo-shi



## 3. 





