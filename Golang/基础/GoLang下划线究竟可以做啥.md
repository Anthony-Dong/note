# GoLang下划线究竟可以做啥

下划线确实还是很有作用的, 关注源码的话, 一堆使用`_` 来申明变量的. 确实还是很有用 . 因此我找了一篇文章专门介绍一下 : [https://studygolang.com/articles/17479?fr=sidebar](https://studygolang.com/articles/17479?fr=sidebar)  , 下文基本大部门引用自改文章. 

## 用在 import

```go
import _  "net/http/pprof"
import _ "github.com/go-sql-driver/mysql"
```

这种用法，会调用包中的`init()`函数，让导入的包做初始化，但是却不使用包中其他功能。

类似于Java的ClassForName 去加载一个类. 但是不使用它. 

## 用在返回值



## 用在变量

大多数是为了断言. 确定正确性. 