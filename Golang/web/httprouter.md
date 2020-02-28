# Golang - HttpRouter (核心框架)

>HttpRouter is a lightweight high performance HTTP request router (also called *multiplexer* or just *mux* for short) for [Go](https://golang.org/).
>
>HttpRouter是Go的轻量级高性能HTTP请求路由器（也称为多路复用器或简称为mux）。

[GitHub地址](https://github.com/gin-gonic/gin) : https://github.com/gin-gonic/gin

  这个框架地位很高 , 因为他处理路由的能力很不错, 也符合人们的写法, 对于golang流行的框架 比如说 gin框架, 也是使用了router . 



## 快速使用

> ​	我们知道golang提供的开发包的 http 模块已经很棒了 , 基本用户只需要关注路由和功能实现. 但是对于路由这块确实处理起来不方便, 各种解析 , httprouter确实不错. 



```go
func main() {
	router := httprouter.New()
	
	router.GET("/get/*path", func(writer http.ResponseWriter, request *http.Request, params httprouter.Params) {
		name := params.ByName("path")
		fmt.Fprintf(writer, "get : %s!", name)
	})

    router.DELETE("/delete/:path", func(writer http.ResponseWriter, request *http.Request, params httprouter.Params) {
		name := params.ByName("path")
		fmt.Fprintf(writer, "delete : %s!", name)
	})

	http.ListenAndServe(":8080", router)
}
```

测试 : 

```go
C:\Users\12986\Desktop>curl -X GET http://localhost:8080/get/a/b/c        
path : /a/b/c!
```

```go
C:\Users\12986\Desktop>curl -X DELETE http://localhost:8080/delete/a      
path : a!
```



路由规则主要是 : 

`:` 规则

```go
Pattern: /user/:user

 /user/gordon              match
 /user/you                 match
 /user/gordon/profile      no match
 /user/                    no match
```

`*` 规则

```go
Pattern: /src/*filepath

 /src/                     match
 /src/somefile.go          match
 /src/subdir/somefile.go   match
```



## router.GlobalOPTIONS

```go
func main() {

	// 创建
	router := httprouter.New()

	// 必须匹配 ,并且请求方式为option , 同时会响应中加入[GET, OPTIONS]
	router.GlobalOPTIONS = http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintln(w, w.Header()["Allow"], "OK")
	})

	// 必须匹配
	router.GET("/hello/*path", func(writer http.ResponseWriter, request *http.Request, params httprouter.Params) {
		fmt.Println(params.ByName("path"))
	})

	log.Fatal(http.ListenAndServe(":8080", router))
}
```

```go
D:\代码库\go\src\com.test.gin\file_agent>curl -X OPTIONS http://localhost:8080/hello/test
[GET, OPTIONS] OK
```



## SaveMatchedRoutePath

> ​	有些时候我们需要拿到路由 , 所以 httprouter 提供了很方便的方法 . 

```go
router := httprouter.New()

// 先声明记录
router.SaveMatchedRoutePath = true

router.GET("/get/*path", func(writer http.ResponseWriter, request *http.Request, params httprouter.Params) {
    // 这里就可以取到 /get/*path了 
    path := params.MatchedRoutePath()
    name := params.ByName("path")
    fmt.Fprintf(writer, "path : %s , name : %s!", path, name)
})
```

我们试试

```go
C:\Users\12986\Desktop>curl -X GET http://localhost:8080/get/aaaa/aaa     
path : /get/*path , name : /aaaa/aaa!
```

完全OK . 



