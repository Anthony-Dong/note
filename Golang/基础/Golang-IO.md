# Golang -IO 操作

Go 官方提供了一些 API，支持对**内存结构**，**文件**，**网络连接**等资源进行操作 , 

​	其中IO是一个编程语言的核心, 希望掌握这些核心操作 . 相对于Java他真的很丰富. Java的话推荐使用 . Guava包或者RandomAccessFile. 这些先进的API.  而Golang更是接近系统操作. 更加灵活, 

> ​	这些IO操作涉及到一些权限问题. 所以最好linux 上写代码 . 怎么说呢. 反正最好吧. 我用的cloudstudio

## IO流

> ​	golang的 IO流  , 讲解. 其实就是俩接口  , write. read ,还有一个close . 其他都是封装 . 所以对应着三个接口. 

[IO基本概念可以参考这篇文章了](https://segmentfault.com/a/1190000015591319)   https://segmentfault.com/a/1190000015591319

## os包

> ​	操作文件的基本包 , 所以是 操作系统(Operating System，简称OS)  , 懂了吗  .  

#### `os.Open(name)`  

这个是打开文件 , 只有读取权限 . 是 os.O_RDONLY , 查看源码 发现就一个readonly

```go
func Open(name string) (*File, error) {
	return OpenFile(name, O_RDONLY, 0)
}
```

#### `os.Create(name)` 

答案就是确实矿建了 , 权限呢 读写. 没有时就创建. 有就清空文件内容

```go
func Create(name string) (*File, error) {
	return OpenFile(name, O_RDWR|O_CREATE|O_TRUNC, 0666)
}
```

#### `func OpenFile(name string, flag int, perm FileMode)` 

第二个参数 `flag `是下面的这几个 , 多个用 `|` 分开

```go
const (
	// Exactly one of O_RDONLY, O_WRONLY, or O_RDWR must be specified.
	O_RDONLY int = syscall.O_RDONLY // open the file read-only.
	O_WRONLY int = syscall.O_WRONLY // open the file write-only.
	O_RDWR   int = syscall.O_RDWR   // open the file read-write.
	// The remaining values may be or'ed in to control behavior.
    O_APPEND int = syscall.O_APPEND // append data to the file when writing.(继续写)
	O_CREATE int = syscall.O_CREAT  // create a new file if none exists.
	O_EXCL   int = syscall.O_EXCL   // used with O_CREATE, file must not exist.
	O_SYNC   int = syscall.O_SYNC   // open for synchronous I/O.
    O_TRUNC  int = syscall.O_TRUNC  // truncate regular writable file when opened.(清空文件)
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



## log 包

> ​	log , 可以创建日志文件 .  很方便. 

```go
f, err := os.OpenFile(fileName, os.O_WRONLY|os.O_CREATE|os.O_APPEND, 0644)
if err != nil {
    log.Fatal(err)
}
defer f.Close()
// 设置日志输出路径
log.SetOutput(f)
log.Println("hello log)
```



## fmt 包

> ​	这个包操作文件可舒服了. 超级舒服

打印信息就不说了 . 对应着三个  `fmt.Println()` , `fmt.Print()` , `fmt.Printf()`

其实读写也对应着三个, 

首先看写吧 

```go
func main() {
	file, _ := os.Create("txt.log")
	defer file.Close()
	fmt.Fprintf(file,"Now %s\n",time.Now().Format("2020-01-01 12.00.00"))
}
```

然后文件就按照格式写进去了, 还有俩方法也是 `fmt.Fprint()`  和 `fmt.Fprintln()`  估计人都懂. 



然后就是读取文件了.  很简单

```go
func main() {
	file, _ := os.Open("txt.log")
	defer file.Close()
	var content string
    fmt.Fscan(file, &content)//遇到空格就停住了. // 输出 :  Now
}
```



我们要打印全部就 . 

```go
func main() {
	file, _ := os.Open("txt.log")
	defer file.Close()
	var content string
	for {
		n, _ := fmt.Fscan(file, &content)
		if n == 0 {
			break
		}
		fmt.Printf("%s\n", content)
	}
}
```

```go
Now
2020-2-2
22.84.84
```

## bufio 包

> ​	这个是将文件读到一个缓冲区中, 基于缓冲区操作. 所以很方便. 

#### 输入 . 模拟控制栏输入

```go
func main() {
	input := bufio.NewScanner(os.Stdin)

	for input.Scan() {
		fmt.Println(input.Text())
	}
}
```



#### 按行读取文件.  

```go
func main() {
	file, _ := os.Open("txt.log")

	defer file.Close()

	input := bufio.NewReader(file)

	line, _, _ := input.ReadLine()

	fmt.Printf("%s\n",line)
}
```

输出 :

```go
Now :  2020-02-02 22.15.15
```



注意这个读取完整文件需要如何做  , 这个方法是不会报错的, 我测试是. 唯一判断结束的就是判断. line这个切片的长度. 如果是0. 就直接break.

```go
func main() {

	file, _ := os.Open("txt.log")

	defer file.Close()

	input := bufio.NewReader(file)

	for {
		line, isPrefix, err := input.ReadLine()
		if len(line) == 0 {
			break
		}
		fmt.Printf("%s , %v , %v\n", line, isPrefix, err)
	}
}
```

输出 

```go
Now :  2020-02-02 22.15.15 , false 
Now :  2020-02-02 22.15.15 , false 
Now :  2020-02-02 22.15.15 , false 
```



#### `ReadSlice(byte)` 

这个操作其实很nice .  可以切分

```go
func main() {

	file, _ := os.Open("txt.log")

	fmt.Println("\\n : ",byte('\n'))
	defer file.Close()

	input := bufio.NewReader(file)

	for  {
		line, err := input.ReadSlice(byte('\n'))

		if err!=nil {
			fmt.Println(err)
			break
		}
		fmt.Println(line)
	}
}
```

输出 

```go
\n :  10
[78 111 119 32 58 32 32 50 48 50 48 45 48 50 45 48 50 32 50 50 46 49 53 46 49 53 10]
[78 111 119 32 58 32 32 50 48 50 48 45 48 50 45 48 50 32 50 50 46 49 53 46 49 53 10]
[78 111 119 32 58 32 32 50 48 50 48 45 48 50 45 48 50 32 50 50 46 49 53 46 49 53 10]
[10]
EOF
```

很有用的, 他可以按照给定的换行符进行切分 .  这里如果是windows换行符就恶心了 .  其实也无所谓结尾都是 `\n`

, 对于 windows平台 `\r\n`是换行符. UNIX是 `\n`   , 所以么办法.  其中 `\n`=10 . `\r`=13

后面还有很多操作 `ReadString` 和 `ReadBytes`  , 都很有用



## ioutil  包

> ​	这个包是个工具包. 包含方便的文件读写.  方便的新建临时目录 ,其实很简单. 我们看主要方法 



![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/5d835a4b-10c1-40c1-b162-13f10065e186.png?x-oss-process=style/template01)



其中 write 和 read 就不解释. 相信都懂. 



我们就看看 `TempFile`   和 `TempDir`  ,其实你理解了 `Temp`指的是临时(Template)的意思就明白了  .  

```go
dirName, _ := ioutil.TempDir("", "example")
fmt.Println(dirName)


file, _ := ioutil.TempFile("D:/代码库/golang/src/com.io.test", "example")
defer file.Close()
fileName := file.Name()
fmt.Println(fileName)
```

输出

```go
C:\Users\12986\AppData\Local\Temp\example029719187
D:\代码库\golang\src\com.io.test\example522218718
```

其实默认路径是在用户的tmp目录下的.  指定的话就是你指定的了. 



`writer := ioutil.Discard`  则是一个空的writer 接口 ,就是只写了接口方法,实际上啥也没做, 所以么啥用,明白了吧 . 

```go
var _ io.ReaderFrom = devNull(0)

func (devNull) Write(p []byte) (int, error) {
	return len(p), nil
}

func (devNull) WriteString(s string) (int, error) {
	return len(s), nil
}
```

## 文件信息 Stat

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