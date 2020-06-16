# Go mod 开发中常用玩法

## 什么是GoMod

`go modules` 是 golang 1.11 新加的特性。

那么问题来了，它解决了什么问题，其实核心的问题就是，他可以没有GoPath这个概念，所以很方便，但是当你不会使用的时候，你会觉得它很不好用。

> ​	PS：go.mod文件一旦创建后，它的内容将会被go toolchain全面掌控。go toolchain会在各类命令执行时，比如go get、go build、go mod等修改和维护go.mod文件。



## 简单使用

比如，现在有一个需求是，我们要创建一个新项目，项目名字为 go-kit-demo，此时我们需要go-kit的依赖，那么怎么办呢

第一步

```go
% mkdir go-kit-demo

% cd go-kit-demo

% go mod init go-kit-demo // 初始化一个项目名为 go-kit-demo，这个路径其实是你相对于GOPATH的路径，以后你的包就是 import "go-kit-demo/util" 之类的

% ls
go.mod // 发现多了个文件

% cat go.mod
module go-kit-demo // 模块名字

go 1.13 // go版本

% go mod edit -require=github.com/go-kit/kit@latest //添加go-kit的依赖

% go mod download //下载依赖

% cat go.mod
module go-kit-demo

go 1.13

require github.com/go-kit/kit v0.10.0 // indirect

% ls
go.mod    go.sum//校验包
```

**第二步，其实就是需要代码，让编辑器可以找到，显然目前go官方不支持的，需要第三方。**

> ​	但是这里你无法直接使用go-kit，理由是，goland工具不支持，这种模块依赖，所以必须引入到vendor中，也就是。

这里推荐一个工具，他可以将依赖打包到vendor进去，相当nice

```shell
https://github.com/nomad-software/vend
```

根据要求来就行了

```go
go get github.com/nomad-software/vend
```

就像下面这样子

```go
~/go/code/mode-test % vend
vend: copying cloud.google.com/go (v0.34.0)
vend: copying github.com/BurntSushi/toml (v0.3.1)
vend: copying github.com/Knetic/govaluate (v3.0.1-0.20171022003610-9aa49832a739+incompatible)
vend: copying github.com/Shopify/sarama (v1.19.0)
vend: copying github.com/Shopify/toxiproxy (v2.1.4+incompatible)
vend: copying github.com/VividCortex/gohistogram (v1.0.0)
vend: copying github.com/afex/hystrix-go (v0.0.0-20180502004556-fa1af6a1f4f5)
vend: copying github.com/alecthomas/template (v0.0.0-20190718012654-fb15b899a751)
vend: copying github.com/alecthomas/units (v0.0.0-20190717042225-c3de453c63f4)
vend: copying github.com/apache/thrift (v0.13.0)
vend: copying github.com/armon/circbuf (v0.0.0-20150827004946-bbbad097214e)
```

所以还是很方便的，可以将依赖全部copy到vendor下，就可以愉快的使用了。



## 其他命令

比如说国内源的问题，需要修改一下

```go
export GOPROXY=https://goproxy.cn
```

其次就是，比如低版本的go ，可能需要强制开启go mod

```go
export GO111MODULE=on
```



像其他命令我就不介绍了，比如还有常用的可能就是`go mod vendor `,将依赖copy到vendor目录下，但是有个问题就是，它只会copy代码依赖的东西，所以我们有些时候是不方便的，因此，需要使用到我上面推荐的工具。



比如说

```go
go get github.com/go-kit/kit //此时会在go.mod文件中添加依赖，也就是说不需要手动的去执行 go mod edit -require=github.com/go-kit/kit@latest了
```

还有其他命令

```go
Go mod provides access to operations on modules.

Note that support for modules is built into all the go commands,
not just 'go mod'. For example, day-to-day adding, removing, upgrading,
and downgrading of dependencies should be done using 'go get'.
See 'go help modules' for an overview of module functionality.

Usage:

	go mod <command> [arguments]

The commands are:

	download    download modules to local cache//下载
	edit        edit go.mod from tools or scripts//编辑go.mod
	graph       print module requirement graph// 打印依赖
	init        initialize new module in current directory //初始化
	tidy        add missing and remove unused modules // 删除没用的依赖
	vendor      make vendored copy of dependencies // copy到vendor
	verify      verify dependencies have expected content // 校验
	why         explain why packages or modules are needed

Use "go help mod <command>" for more information about a command.
```



go.mod 提供了`module`, `require`、`replace`和`exclude` 四个命令

- `module` 语句指定包的名字
- `require` 语句指定的依赖项模块
- `replace` 语句可以替换依赖项模块
- `exclude` 语句可以忽略依赖项模块

其实这些不需要care



## 本地仓库如何解决

需求，我们现在有另外一个项目，我们这个项目需要引入。

> ​	PS: 进入公司，一般你会将公司的代码，放在同一个目录下面，也就是上一级目录是一样的，所以我们也遵守这个规范，理由其实很简单，因为方便环境。

初始化 A\B项目，名字为 demo-1，demo-2

```go
~/go/code/test % cd demo-1
~/go/code/test/demo-1 % ls
~/go/code/test/demo-1 % go mod init demo-1
go: creating new go.mod: module demo-1
```



我们目前目录结构是这样的:

```go
~/go/code/test % tree .
.
├── demo-1
│   └── go.mod
└── demo-2
    ├── api
    │   └── userservice.go
    └── go.mod

3 directories, 3 files
```

我们现在demo-1这个项目，需要使用 user-service这个接口

```go
package api

type UserService interface{
	GetUserInfo()*UserInfo
}

type UserInfo struct{
	Name string `json:"name"`
	Age  int  `json:"age"`
}
```

此时只需要在 demo-1里面做一下替换就可以了，如下图所示

```go
module demo-1

go 1.13

require demo_2 v0.0.0

replace demo_2 => ../demo-2
```



然后我们demo-1就可以愉快的使用了

```go
package main

import (
	"demo_2/api"
	"fmt"
)

func main() {
	info := api.UserInfo{
		Name: "tom",
		Age:  18,
	}
	fmt.Println(info)
}
```



那么现实中，可能依赖包要更新，需要及时的拉去master，这个更新怎么办，很简单，只需要执行 `go mod vendor ` 从新拉去一下

> ​	注意的一点是 本地包，无法校验hash



## 为什么要必须使用vendor目录

- 1、依赖可能是本地，可能是私有仓库，无法解决
- 2、生产环境可能拉去慢
- 3、没有通用的解决方案，等go以后发展吧

其中 `go build -mod=vendor` 的意思是强制开启vendor模式，依赖只会去vendor目录找，其他都不会care，也不会网络环境找。

## 私人仓库如何解决

全部公有仓库，没啥事，但是私有怎么半

这三个方案，理论上都可行，但是都有缺陷。

goproxy代理，本身并不解决从gitlab服务器获取私有项目代码的问题，也没有成熟的鉴权机制。因此比较适合静态公共代码。 

> ​	例子可以看：https://goproxy.cn/

nginx代理，同样不解决认证的问题，我配置了一个代理，ssl认证失败（弱鸡本鸡），理论上这个方案是最好的解决方案。

go replace，需要在每个项目中都添加，更新依赖也比较复杂。

所以采用的方案是，还是本地依赖。



目前网上的解决方案都是修改本地的git，配置，通过配置一些信息之类的，自行百度，但是还是搭建代理比较好，从代理层解决，可以参考上面的go proxy

