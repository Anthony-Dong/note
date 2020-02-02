# Golang -IO 操作

Go 官方提供了一些 API，支持对**内存结构**，**文件**，**网络连接**等资源进行操作

> ​	这些IO操作涉及到一些权限问题. 所以最好linux 上写代码 . 怎么说呢. 反正最好吧. 我用的cloudstudio

## IO操作

[IO基本操作可以参考这篇文章了](https://segmentfault.com/a/1190000015591319)   https://segmentfault.com/a/1190000015591319



## os包

#### `os.Open("test.log")`  

这个是打开文件 , 只有读取权限 . 是 os.O_RDONLY



#### `func OpenFile(name string, flag int, perm FileMode)` 

第二个参数 `flag `是下面的这几个 , 多个用 `|` 分开

```go
const (
	// Exactly one of O_RDONLY, O_WRONLY, or O_RDWR must be specified.
	O_RDONLY int = syscall.O_RDONLY // open the file read-only.
	O_WRONLY int = syscall.O_WRONLY // open the file write-only.
	O_RDWR   int = syscall.O_RDWR   // open the file read-write.
	// The remaining values may be or'ed in to control behavior.
	O_APPEND int = syscall.O_APPEND // append data to the file when writing.
	O_CREATE int = syscall.O_CREAT  // create a new file if none exists.
	O_EXCL   int = syscall.O_EXCL   // used with O_CREATE, file must not exist.
	O_SYNC   int = syscall.O_SYNC   // open for synchronous I/O.
	O_TRUNC  int = syscall.O_TRUNC  // truncate regular writable file when opened.就是如果有append和trunc,那么打开文件就清空
)
```



第三个参数 `FileMode`  , 我也不知道做啥

```go
const (
	// The single letters are the abbreviations
	// used by the String method's formatting.
	ModeDir        FileMode = 1 << (32 - 1 - iota) // d: is a directory
	ModeAppend                                     // a: append-only
	ModeExclusive                                  // l: exclusive use
	ModeTemporary                                  // T: temporary file; Plan 9 only
	ModeSymlink                                    // L: symbolic link
	ModeDevice                                     // D: device file
	ModeNamedPipe                                  // p: named pipe (FIFO)
	ModeSocket                                     // S: Unix domain socket
	ModeSetuid                                     // u: setuid
	ModeSetgid                                     // g: setgid
	ModeCharDevice                                 // c: Unix character device, when ModeDevice is set
	ModeSticky                                     // t: sticky
	ModeIrregular                                  // ?: non-regular file; nothing else is known about this file

	// Mask for the type bits. For regular files, none will be set.
	ModeType = ModeDir | ModeSymlink | ModeNamedPipe | ModeSocket | ModeDevice | ModeCharDevice | ModeIrregular

	ModePerm FileMode = 0777 // Unix permission bits
)
```

我们看看这个玩意是啥 

```go
func main() {
	fmt.Println(os.FileMode(777), 777)
	fmt.Println(os.FileMode(0777), 0777)
}
```

输出一下

```go
-r----x--x 777
-rwxrwxrwx 511
```

发现为啥  777 不对 . 

linux中的权限r w x分别对应4 2 1，相加的值为7，习惯了linux中权限命令使用，会将os.FileMode(777) 误解等价于777权限，但是将777传入os.FileMode，你会发现打印出来的不是 -rwxrwxrwx , 这个是为啥呢 ? 

因为他是八进制, 当你输入777 , 实际上是把他看成了8进制. 懂了吧. 

```go
i, _ := strconv.ParseInt("777", 8, 0)

fmt.Println(i) //511
```

所以就是 rx-x-x 的权限. 



文件权限在linux如下显示. 

```go
[root@iz2zegua78a74kqn8clokfz user]# ll
total 28388
drwxr-xr-x 9 user user     4096 Aug 27 15:26 elasticsearch
-rw-r--r-- 1 user root 29056810 Aug 27 15:00 elasticsearch-6.2.4.tar.gz
-rw-r--r-- 1 user root      291 Sep  1 17:57 null
drwxr-xr-x 2 user root     4096 Sep  1 19:07 script

–       rw-     r–    r–    9          user                user      4096   
// - : 文件,  d : 目录  , l : 软连接 ,所以表示
普通文件 用户权限 组权限 其他用户 表示文件个数 该文件或目录的拥有者  表示所属的组   文件大小 
```

其实日常使用的就是  `0777`  , `0644`  , `0664`  基本足够了 . 



#### `os.Chmod("test.log",0644)`

修改文件权限



#### `os.Mkdir("D:/代码库/golang/src/com.io.test/a",0644)`

创建文件夹其实就是  mkdir a



#### `os.MkdirAll("D:/代码库/golang/src/com.io.test/a/b",0644)`

创建文件夹其实就是  mkdir -p a/b  , 如果没有改目录会递归创建



#### `os.RemoveAll(path)`

递归删除

#### `os.Remove(file/dirname)`

非递归



#### `ioutil包`

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/72b640d2-65be-450b-b946-89eaac682503.png?x-oss-process=style/template01)



## filepath 包

#### `filepath.Walk(path , fun)`

最有用的递归遍历文件夹

```go
filepath.Walk("D:/代码库/golang/src/com.io.test", func(path string, info os.FileInfo, err error) error {
    fmt.Println("path : ", path)
    fmt.Println("is dir : ", info.IsDir())
    return nil
})
```

输出 :  , 这个方法可以递归的遍历

```go
path : D:/代码库/golang/src/com.io.test , is dir : true .
path : D:\代码库\golang\src\com.io.test\.idea , is dir : true .
path : D:\代码库\golang\src\com.io.test\.idea\com.io.test.iml , is dir : false .
path : D:\代码库\golang\src\com.io.test\.idea\inspectionProfiles , is dir : true .
path : D:\代码库\golang\src\com.io.test\.idea\misc.xml , is dir : false .
path : D:\代码库\golang\src\com.io.test\.idea\modules.xml , is dir : false .
path : D:\代码库\golang\src\com.io.test\.idea\workspace.xml , is dir : false .
```



#### `filepath.Abs(path)`

输出文件全路径

```go
s, _ := filepath.Abs("1.jpg")
fmt.Println(s)  // D:\代码库\golang\src\com.io.test
```



#### ` filepath.Clean(path)`

清洗路径 . 很好地处理 / 或者 \\  之类的. 

```go
clean := filepath.Clean("D:\\代码库/golang//src\\com.io.test/")

fmt.Println(clean) 
```

#### `filepath.Split(path)`

```java
dir, file := filepath.Split("D:/代码库/golang/src/com.io.test/1.jpg")

fmt.Printf("dir : %s , file : %s \n", dir, file)
```

输出

```go
dir : D:/代码库/golang/src/com.io.test/ , file : 1.jpg 
```

#### `filepath.Join(...)`

```go
path := filepath.Join("D:/代码库/golang/src/com.io.test", "1.jpg")
fmt.Println(path)
//   D:\代码库\golang\src\com.io.test\1.jpg
```



#### 很多

```go
i := filepath.Dir("D:/代码库/golang/src/com.io.test")
fmt.Println(i)  D:\代码库\golang\src
```



```go
Code:
paths := []string{
    "/a/b/c",
    "/b/c",
    "./b/c",
}
base := "/a"

fmt.Println("On Unix:")
for _, p := range paths {
    // base(文件路径)  ,path(当前路径)  输出当前路径到文件路径的相对路径
    rel, err := filepath.Rel(base, p)
    fmt.Printf("%q: %q %v\n", p, rel, err)
}
Output:
On Unix:
"/a/b/c": "b/c"  
"/b/c": "../b/c"  
"./b/c": "" Rel: can't make ./b/c relative to /a
```



## 日志

> ​	log

```go
f, err := os.OpenFile(fileName, os.O_WRONLY|os.O_CREATE|os.O_APPEND, 0644)
if err != nil {
    log.Fatal(err)
}
// 设置日志输出路径
log.SetOutput(f)
log.Println("hello log)
defer f.Close()
```

## 文件信息

其实就是 `Stat` 信息, 知道这个其他随意

### 1.文件修改时间

```go
fileInfo, _ := os.Stat("test.log")
//修改时间
modTime := fileInfo.ModTime()
fmt.Println(modTime)
```

### 2.文件是否存在

```go
_, err := os.Stat("test.log")
if(os.IsNotExist(err)) {
    fmt.Println("file not exist!")
}
```

### 3.文件是否是目录

```go
fileInfo, _ := os.Stat("test.log")
//是否是目录
isDir := fileInfo.IsDir()
fmt.Println(isDir)
```

### 4.文件权限

```go
fileInfo, _ := os.Stat("test.log")
//权限
mode := fileInfo.Mode()
fmt.Println(mode)
```

### 5.文件名

```go
fileInfo, _ := os.Stat("test.log")
//文件名
filename:= fileInfo.Name()
fmt.Println(filename)
```

### 6.文件大小

```go
fileInfo, _ := os.Stat("test.log")
//文件大小
filesize:= fileInfo.Size()
fmt.Println(filesize)//返回的是字节
```

### 7.文件创建时间

文件的创建时间并没有直接的方法返回，翻看源代码才知道如何获取

```go
fileInfo, _ := os.Stat("test.log")
fileSys := fileInfo.Sys().(*syscall.Win32FileAttributeData)
nanoseconds := fileSys.CreationTime.Nanoseconds() // 返回的是纳秒
createTime := nanoseconds/1e9 //秒
fmt.Println(createTime)
```

### 8.文件最后写入时间

```go
fileInfo, _ := os.Stat("test.log")
fileSys := fileInfo.Sys().(*syscall.Win32FileAttributeData)
nanoseconds := fileSys.LastWriteTime.Nanoseconds() // 返回的是纳秒
lastWriteTime := nanoseconds/1e9 //秒
fmt.Println(lastWriteTime)
```

### 9.文件最后访问时间



```go
fileInfo, _ := os.Stat("test.log")
fileSys := fileInfo.Sys().(*syscall.Win32FileAttributeData)
nanoseconds := fileSys.LastAccessTime.Nanoseconds() // 返回的是纳秒
lastAccessTime:= nanoseconds/1e9 //秒
fmt.Println(lastAccessTime)
```

### 10.文件属性

```go
fileInfo, _ := os.Stat("test.log")
fileSys := fileInfo.Sys().(*syscall.Win32FileAttributeData)
fileAttributes:= fileSys.FileAttributes
fmt.Println(fileAttributes)
```