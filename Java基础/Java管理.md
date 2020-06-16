# Java 线上小技巧

> ​	jconsole 是 windows的平台的可视化工具, 线上可没有这种待遇, 所以, 只能靠命令. 
>
> ​    一个好的开发, 要知道如何写代码和处理问题. 



## JPS

全程就是 java process status  , jdk1.5 提供的命令 . 

#### jps

> ​	最简单的工具. 直接显示

```java
C:\WINDOWS\system32>jps
10624 Launcher
// 进程ID  和 进程名称
12352 Jps
9028 Demo
1048 RemoteMavenServer
```

#### jps -l

> ​	可以显示出类的全限定名称. 

```java
C:\WINDOWS\system32>jps -l
// 类的全限定名称
1048 org.jetbrains.idea.maven.server.RemoteMavenServer
13432 org.jetbrains.jps.cmdline.Launcher
13196 sun.tools.jps.Jps
```

### jps -v

查看启动时参数.

```java
C:\WINDOWS\system32>jps  -v
7404 Demo -Xmx10m -javaagent:C:\Program Files\JetBrains\IntelliJ IDEA 2018.3.4\lib\idea_rt.jar=2381:C:\Program Files\JetBrains\IntelliJ IDEA 2018.3.4\bin -Dfile.encoding=UTF-8
```

#### jps -m

查看启动时, 传给main方法的参数.



## jinfo

jinfo用于观察、**修改**运行中的java程序的运行环境参数：参数包括Java System属性和JVM命令行参数。





## Jmap

> ​	查看内存快照的.

#### jmap -heap pid

> ​	查看堆内存的使用情况

```java
C:\WINDOWS\system32>jmap -heap 8468
Attaching to process ID 8468, please wait...
Debugger attached successfully.
Server compiler detected.
JVM version is 25.221-b11

using thread-local object allocation.
Parallel GC with 4 thread(s)

Heap Configuration:
   MinHeapFreeRatio         = 0
   MaxHeapFreeRatio         = 100
   MaxHeapSize              = 5316280320 (5070.0MB)
   NewSize                  = 111149056 (106.0MB)
   MaxNewSize               = 1772093440 (1690.0MB)
   OldSize                  = 222298112 (212.0MB)
   NewRatio                 = 2
   SurvivorRatio            = 8
   MetaspaceSize            = 21807104 (20.796875MB)
   CompressedClassSpaceSize = 1073741824 (1024.0MB)
   MaxMetaspaceSize         = 17592186044415 MB
   G1HeapRegionSize         = 0 (0.0MB)
       
Heap Usage:
PS Young Generation
Eden Space:
   capacity = 83886080 (80.0MB)
   used     = 11746288 (11.202133178710938MB)
   free     = 72139792 (68.79786682128906MB)
   14.002666473388672% used
From Space:
   capacity = 13631488 (13.0MB)
   used     = 0 (0.0MB)
   free     = 13631488 (13.0MB)
   0.0% used
To Space:
   capacity = 13631488 (13.0MB)
   used     = 0 (0.0MB)
   free     = 13631488 (13.0MB)
   0.0% used
PS Old Generation
   capacity = 222298112 (212.0MB)
   used     = 0 (0.0MB)
   free     = 222298112 (212.0MB)
   0.0% used

3528 interned Strings occupying 288160 bytes.       
```



#### jmap -histo pid

> 查看具体的类信息. 

```java
C:\WINDOWS\system32>jmap -histo 13428
//  实例数量   字节数    类名
 num     #instances         #bytes  class name
----------------------------------------------
   1:          1536        6486504  [I
   2:          2969        2216464  [B
   3:         14409        1676208  [C
   4:          9886         237264  java.lang.String
   5:          2562         143472  jdk.internal.org.objectweb.asm.Item
   6:           878         100584  java.lang.Class
   7:          2053          97032  [Ljava.lang.Object;
   8:           151          85040  [Ljdk.internal.org.objectweb.asm.Item;
   9:          1337          40656  [Ljava.lang.Class;
  10:           791          31640  java.util.TreeMap$Entry
  11:           788          31520  java.lang.invoke.MethodType
  12:           700          28000  java.util.LinkedHashMap$Entry
  13:           496          27776  java.lang.invoke.MemberName
  14:          1107          26568  java.lang.StringBuilder
```

像 [I 这种

- B代表byte 
- C代表char 
- D代表double 
- F代表float 
- I代表int 
- J代表long 
- Z代表boolean 

前边有`[`代表数组，`[I `就相当于`int[]`  , 对象用`[L+类名`表示数组 



## jvisualvm

> ​	jvisualvm 工具, 使用起来很方便, 可视化界面. 



主要是查看一下GC的效率 , 和 Jmap . Jstack 等信息吧. 





## 线上问题如何排查

https://blog.csdn.net/GitChat/article/details/79019454















