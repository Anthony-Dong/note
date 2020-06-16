## Go mod

`go modules` 是 golang 1.11 新加的特性。现在1.12 已经发布了，是时候用起来了。Modules官方定义为：模块是相关Go包的集合。modules是源代码交换和版本控制的单元。 go命令直接支持使用modules，包括记录和解析对其他模块的依赖性。modules替换旧的基于GOPATH的方法来指定在给定构建中使用哪些源文件。

更多关于 gomod的信息，可以看看官方的wiki ： https://github.com/golang/go/wiki/Modules

### 1、快速创建一个go mod项目

直接init就可以了

```go
sgcx015@SGCX015deMacBook-Pro:~/go/code/my_test_mod % go mod init my_pro
go: creating new go.mod: module my_pro
sgcx015@SGCX015deMacBook-Pro:~/go/code/my_test_mod % ls
go.mod
```

go.mod文件一旦创建后，它的内容将会被go toolchain全面掌控。go toolchain会在各类命令执行时，比如go get、go build、go mod等修改和维护go.mod文件。

现在我们在 cmd目录下创建main.go

```go
package main

import (
	"fmt"
	"github.com/robfig/cron"
	"log"
	"sync"
)

func main() {
	group := sync.WaitGroup{}
	group.Add(1)
	c := cron.New()
	err := c.AddFunc("*/2 * * * * *", func() {
		fmt.Println("hello cron")
	})
	if err != nil {
		log.Fatal(err)
	}
	c.Start()
	group.Wait()
}
```



然后执行 命令：

```go
// 导出一下国内源
sgcx015@SGCX015deMacBook-Pro:~/go/code/my_test_mod % export GOPROXY=https://goproxy.cn
// build
sgcx015@SGCX015deMacBook-Pro:~/go/code/my_test_mod % go build cmd/main.go
go: finding github.com/robfig/cron v1.2.0
go: downloading github.com/robfig/cron v1.2.0
go: extracting github.com/robfig/cron v1.2.0
```

他会自动find ， down ，exec等

最后看一下mod文件

```go
sgcx015@SGCX015deMacBook-Pro:~/go/code/my_test_mod % go mod edit -print
module my_pro

go 1.13

require github.com/robfig/cron v1.2.0
```

自动的生成了一个文件。那么还有一个问题，它放哪了。。。。 ,显然是一个cache中，所以还需要我们显示的指定，但是没事，go-vendor，很棒，所以执行`go mod vendor ` ,会将依赖全部打包在工作目录下的vendor下。

```shell
sgcx015@SGCX015deMacBook-Pro:~/go/code/my_test_mod % go mod vendor
sgcx015@SGCX015deMacBook-Pro:~/go/code/my_test_mod % ls
cmd    go.mod go.sum main   vendor
sgcx015@SGCX015deMacBook-Pro:~/go/code/my_test_mod % ls vendor
github.com  modules.txt
```

注意：`go mod vendor` 会复制modules下载到vendor中, 貌似只会下载你代码中引用的库，而不是go.mod中定义全部的module。



### 其他指令

go.mod 提供了`module`, `require`、`replace`和`exclude` 四个命令

- `module` 语句指定包的名字
- `require` 语句指定的依赖项模块
- `replace` 语句可以替换依赖项模块
- `exclude` 语句可以忽略依赖项模块



### 其他操作指令

```shell
sgcx015@SGCX015deMacBook-Pro:~/go/code/my_test_mod % go help mod
Go mod provides access to operations on modules.

Note that support for modules is built into all the go commands,
not just 'go mod'. For example, day-to-day adding, removing, upgrading,
and downgrading of dependencies should be done using 'go get'.
See 'go help modules' for an overview of module functionality.

Usage:

	go mod <command> [arguments]

The commands are:

	download    download modules to local cache
	edit        edit go.mod from tools or scripts
	graph       print module requirement graph
	init        initialize new module in current directory
	tidy        add missing and remove unused modules
	vendor      make vendored copy of dependencies
	verify      verify dependencies have expected content
	why         explain why packages or modules are needed

Use "go help mod <command>" for more information about a command.
```



### 加载规则

`GO111MODULE` 有三个值：`off`, `on`和`auto（默认值）`。

- `GO111MODULE=off`，go命令行将不会支持module功能，寻找依赖包的方式将会沿用旧版本那种通过vendor目录或者GOPATH模式来查找。
- `GO111MODULE=on`，go命令行会使用modules，而一点也不会去GOPATH目录下查找。
- `GO111MODULE=auto`，默认值，go命令行将会根据当前目录来决定是否启用module功能。这种情况下可以分为两种情形：
  - 当前目录在GOPATH/src之外且该目录包含go.mod文件
  - 当前文件在包含go.mod文件的目录下面。

