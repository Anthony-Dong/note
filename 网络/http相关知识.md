

# HTTP 一些概念和操作简单学习(误区)

> ​	HTTP是五层协议中最顶层 ,属于应用层协议 ,基于TCP编写的

> ​	我下面测试的工具是 `Spring-MVC `+ `insomnia` 进行测试的

## 1. HTTP无状态协议

> ​		HTTP无状态协议，是指协议对于事务处理没有记忆能力。缺少状态意味着如果后续处理需要前面的信息，则它必须重传，这样可能导致每次连接传送的数据量增大。另一方面，在服务器不需要先前信息时它的应答就较快。
>
> ​		Cookie是通过客户端保持状态的解决方案。从定义上来说，Cookie就是由服务器发给客户端的特殊信息，而这些信息以文本文件的方式存放在客户端，然后客户端每次向服务器发送请求的时候都会带上这些特殊的信息。服务器在向客户端回传相应的超文本的同时也会发回这些个人信息,存放于HTTP响应头（Response Header）,当客户端浏览器接收到来自服务器的响应之后，浏览器会将这些信息存放在一个统一的位置;自此，客户端再向服务器发送请求的时候，都会把相应的Cookie再次发回至服务器。而这次，Cookie信息则存放在HTTP请求头（Request Header）了

## 2. Get 和 Post 区别

### 1. 看看规范 

规范规定 : get 请求是将参数以key=value 的形式拼接在url的?后面 , post请求是将参数放在请求体里 , 难道只能认识这些 , 难道get不能放入请求体 ? post不能url传参? 

### 2. 带着疑惑

```java
@PostMapping("/post")
public String post(@RequestParam("name") String name) {
    System.out.println(name);
    return "post 请求";
}


@GetMapping("/get")
public String get(@RequestBody Map<String, String> map) {

    Set<String> strings = map.keySet();
    strings.forEach(s -> {
        System.out.println(s + " : " + map.get(s));
    });
    return "get请求";
}
```

这是两个方法 ,一个get 请求 , 一个 post , 我们使用一下 , 输出结果 

- post请求 , 请求行携带参数 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-05/662ebd9e-2ddb-49d5-a4e3-5bd33f1bca77.jpg?x-oss-process=style/template01)

- get请求 , 请求体携带参数

![]( https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-05/7e11a64d-7ef7-40ce-bd84-72c16e30c00b.jpg?x-oss-process=style/template01)

### 3. 总结

说明7种请求方式 ,只是规范的规定 , 其实每一种都可以做很多事情 , 所以我们不要被所谓的规范而不敢去尝试 , 

## 2. PUT和POST的区别

官方的定义是PUT他是一个幂等性操作,而post不是, 

`幂等性` 的意思是 不管1次或者N次变化结果是相同的 . 

而PUT和POST区别就在这里 ,比如我们去更新或者添加一个东西 ,  连续发送多次 , 比如说我要 update/article , 连续发送了两次 ,put只是覆盖第一次的结果  , 而post代表每次都会产生新的结果 ,所以对于这个文章更新 , post结果是产生了两篇文章 ,而 put是一篇 .

## 3. 表单传值格式

#### 1. application/x-www-form-urlencoded

#### 2. multipart/form-data

#### 3. application/json

这三者的区别是什么 ? 

### 1. application/x-www-form-urlencoded 使用

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-05/cbe38fbe-b94b-4e2c-a2e7-74231eeceb96.png?x-oss-process=style/template01)

### 2. multipart/form-data

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-05/a2a877c3-7a21-44d6-a93d-52757561dce1.png?x-oss-process=style/template01)

### 3. application/json

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-05/b9ea5099-72bd-4945-a95d-7e7e43c80e5d.png?x-oss-process=style/template01)

### 4. 总结

其实 `application/___ `, 其实有很多形式的 , 比如XML , Yaml , Json等等 , 他们对于springmvc 中都需要一个 body去接收, 因为他们在传递的时候确实是在请求体中, 

然后就是常见的 `application/x-www-form-urlencoded` , 这个有点不同, 他不能使用请求体去接收, 因为他的传递的请求体,其实个get的很像 , 只是放在了请求体里 , 也是用 `&`来连接 , 参数的格式为`key=value&key=value` ,  是浏览器默认的编码格式 . 

然后就是 ` multipart/form-data` , 这种一般是来做文件上传的 ,他就有点特殊了 , 你可以看看他就请求体分割了, 然后通过特定个格式编码获取的. 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-05/64978f44-e855-4b36-83e2-d467ac3ebbaf.jpg?x-oss-process=style/template01)



## 重定向

我们只知道 重定向是跳转页面.  前端跳转很简单. 直接一个a标签就行了. 

那么后端呢 , 后端如何跳转. 你请求了一个api接口. 

这里就告诉大家 , 这就是重定向.

```java
@GetMapping("/go/baidu")
public void redirect(HttpServletResponse response) {
    response.setStatus(301);
    response.setHeader("Location", "http://www.baidu.com");
}
```

我们发现响应是 .

```java
HTTP/1.1 301
Location: http://www.baidu.com
Content-Length: 0
Date: Sun, 01 Mar 2020 09:05:43 GM
```

这都是浏览器一种根据协议处理的一种行为表现

比如我们访问 `http://www.baidu.com`  , 此时会被跳转到

```java
HTTP/1.1 307 Internal Redirect
Location: https://www.baidu.com/
Non-Authoritative-Reason: HSTS
```



## 如何解决Mapping重复

```java

```

