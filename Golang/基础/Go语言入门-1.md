# Go语言入门 - 1

### 标准输入输出

```java
// @date : 2020/1/9 11:27
// @author : <a href='mailto:fanhaodong516@qq.com'>Anthony</a>

/**
	go语言标准输入输出
 */
package main

import (
	"fmt"
	"os"
)

func main() {
	strings := os.Args[1:]
	fmt.Println(strings)
}
```

```java
D:\代码库\golang\src\github.anthonydong.io\main>go build TestArgs.go

D:\代码库\golang\src\github.anthonydong.io\main>TestArgs.exe "bbbbbbb" "aa" 1
[bbbbbbb aa 1]

D:\代码库\golang\src\github.anthonydong.io\main>TestArgs.exe "bbbbbbb" "aa" 12
[bbbbbbb aa 12]
```



### 基本数据类型

```go
bool //默认值为false
string //默认值为空字符串
int int8 int16 int32 int64 //默认值为0
uint uint8 uint16 uint32 uint64 uintptr //默认值为0
byte // uint8 的别名
rune // int32 的别名
float32 float64 //默认值为0
complex64 complex128 //默认值为0
```



类型申明

```go
%d										十进制整数
%x,	%o,	%b								十六进制，八进制，二进制整数。
%f,	%g,	%e								浮点数：	3.141593	3.141592653589793	3.141593e+00 
%t										布尔：true或false 
%c										字符（rune）	(Unicode码点) 
%s										字符串 
%q										带双引号的字符串"abc"或带单引号的字符'c'
%v										变量的自然形式（natural	format） 
%T										变量的类型 
%%										字面上的百分号标志（无操作数）
%p										打印地址
```





### 类型判断

```go
counts := make(map[string]int)
counts["aa"] = 1

// 返回类型
of := reflect.TypeOf(counts)

fmt.Println(of)
```



## 切片

```go

```



