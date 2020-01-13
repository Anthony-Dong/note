# Golang -IO 操作

Go 官方提供了一些 API，支持对**内存结构**，**文件**，**网络连接**等资源进行操作

## IO操作

[IO操作可以参考这篇文章了](https://segmentfault.com/a/1190000015591319)   https://segmentfault.com/a/1190000015591319



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

## 文件路径

> ​	filepath 工具包

```go
path, e := filepath.Abs("a.txt")
if e != nil {
    //
}
fmt.Println("日志文件创建成功 : ", path)
```



## 获取文件信息

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