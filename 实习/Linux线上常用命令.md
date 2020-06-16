# Linux线上命令

## 认识环境

1、联调环境其实是我们对外的开发环境，也就是比如本地开发的话，使用开发配置，那么此时调用别人的接口，就是联调环境的接口。那么日志也就是在本地的docker内，可以进去看

2、测试环境，测试环境就有测试环境的机器了，主要是用于测试，一般开发人员也可以使用测试环境的东西

3、预发环境，预发环境是走的线上的数据（数据库、缓存等），主要是用于线上最后一步测试，也可以用来跑数据，但是我们有统计库，但是那个库好多数据没有

4、线上环境 , 就是线上，一般上线上至少得有回滚方案。

操作流程可以看：[http://wiki.corp.ttyongche.com:8360/pages/viewpage.action?pageId=26565486](http://wiki.corp.ttyongche.com:8360/pages/viewpage.action?pageId=26565486)

## 环境机器

全部的发布都是在盖亚系统上，可以根据发布信息查看发不到那台机器了。

登陆这些机器需要跳板机，可以自己查找wiki，同时需要申请一下。

项目源文件在  /home/ttyc/system/项目名称

项目日志在 /data/log/项目名称



## 线上操作注意

1、只能读，不能写（可以写到/tmp下面，对于/home下的代码无法修改）

2、禁止使用 vim操作，或者一些容易挂了的操作





## grep/fgrep命令学习

> ​	grep可以支持正则， fgrep不支持正则，但是快哇。
>
> ​	官方文档：[https://man.linuxde.net/grep](https://man.linuxde.net/grep)

- 这个会将 包含`TemplateList`的字符串全部打印出来，以一整句（也就是\n分割）为单位

```shell
grep "TemplateList" ./business.log
```

- 不区分大小写怎么做, 加`-i`参数

```shell
grep  -i "templatelist" ./business.log
```

- 如果想显示行数：可以加`-n` 参数

```shell
sgcx015@172-15-70-185 ebike-city-report % grep  -i -n "templatelist" ./business.log
13:[2020-05-08 14:39:49] local.ERROR: peccancy.go:36 [TemplateList] get templateList change warehouse params error, Key: 'TemplateListRequest.Params.PageSize' Error:Field validation for 'PageSize' failed on the 'required' tag
```

- 如果想高亮显示，可以加`--color`参数 ，与`--color=auto` 含义相同

```go
grep  -i -n --color "templatelist" ./business.log
```

- 如果想查看一共出现了多少次，可以使用`-c`参数

```shell
grep  -i -c "templatelist" ./business.log
15
```

- 如果你想查看出现的我们查到的前几行，后几行的话，可以使用`-B5` 意思就是 Before 5行，查看前面的5行， 使-用`-A10`，看之后的几行， 那么前后都想看就是`-C` ， combine的意思。

```shell
sgcx015@172-15-70-185 ebike-city-report % grep  -i -B2 --color "templatelist" ./business.log
--
[2020-05-08 15:57:15] local.INFO: uuid.go:38 init mac addr: 3C15C2D23E34
[2020-05-08 15:57:15] local.INFO: main.go:46 Running with 8 CPUs
[2020-05-08 15:57:18] local.INFO: peccancy.go:38 [TemplateList] start, params: {Page:1 PageSize:10}
```

- `-w` 可以精确匹配，也就是

```shell
grep  -w -i -B2 "templateli" --color ./business.log
```

- `-E` 正则匹配 , 我不会写，好像`^`开头， `$`结束

```shell
grep  -E "^templateli$" --color ./business.log
```

- `-v` 反查询，也就是不包含的







### wc 命令

> ​	主要是用于统计文件行数、字数等

```shell
# 统计行数
sgcx015@172-15-70-185 ebike-city-report % wc -l business.log 
      36 business.log
# 统计多少个字 
sgcx015@172-15-70-185 ebike-city-report % wc -w business.log
    1514 business.log 
# 统计字节数
sgcx015@172-15-70-185 ebike-city-report % wc -c business.log
   18139 business.log
```



## 



