# Http协议的 SSE 实现推流

> SSE全称叫 `Server-Sent Events（简称 SSE）` , 可以实现服务器端向客户推送消息. 但是客户端无法向服务器反馈消息. 只能反馈就是一个关闭流的信息 , 其实意思就是你中断网页请求 , 或者关闭浏览器. 服务器会写不出去消息,因为就中断了.  

对比一下 websocket来说 

SSE 与 WebSocket 作用相似，都是建立浏览器与服务器之间的通信渠道，然后服务器向浏览器推送信息。

总体来说，WebSocket 更强大和灵活。因为它是全双工通道，可以双向通信；SSE 是单向通道，只能服务器向浏览器发送，因为流信息本质上就是下载。如果浏览器向服务器发送信息，就变成了另一次 HTTP 请求。

但是，SSE 也有自己的优点。

> - SSE 使用 HTTP 协议，现有的服务器软件都支持。WebSocket 是一个独立协议。
> - SSE 属于轻量级，使用简单；WebSocket 协议相对复杂。
> - SSE 默认支持断线重连，WebSocket 需要自己实现。
> - SSE 一般只用来传送文本，二进制数据需要编码后传送，WebSocket 默认支持传送二进制数据。
> - SSE 支持自定义发送的消息类型。

因此，两者各有特点，适合不同的场合。





我们以一个例子来说吧. 客户端不断向服务器端每隔100MS推送当前时间. 

```java
@Configuration
public class ServerConfig {

    @Bean
    public ServletRegistrationBean<HttpServlet> servletServletRegistrationBean() {

        ServletRegistrationBean<HttpServlet> registrationBean = new ServletRegistrationBean<>();

        registrationBean.setServlet(new HttpServlet() {
            @Override
            protected void doGet(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
                resp.setHeader("Content-Type", "text/event-stream;charset=UTF-8");
                resp.setHeader("Cache-Control", "no-cache, no-store, max-age=0, must-revalidate");
//                resp.setHeader("Pragma", "no-cache");
                resp.setHeader("Connection", "keep-alive");
                final PrintWriter writer = resp.getWriter();

                while (!writer.checkError()) {
                    try {
                        TimeUnit.MILLISECONDS.sleep(1000);
                    } catch (InterruptedException e) {
                        //
                    }
                    writer.write(String.format("cur-time : %s\n\n", new Date()));
                    writer.flush();
                }
            }
        });
        registrationBean.setUrlMappings(Collections.singletonList("/get"));
        return registrationBean;
    }
}
```

浏览器屏幕会出现 : 

```java
cur-time : Thu Feb 13 21:27:43 CST 2020

cur-time : Thu Feb 13 21:27:44 CST 2020

cur-time : Thu Feb 13 21:27:45 CST 2020

cur-time : Thu Feb 13 21:27:46 CST 2020

    ... 不断的出现....
```

所以很方便实现了推送消息.   其中也可以使用 spring-mvc的 Mapping 进行处理, 我这里没有使用.





node.js如何做呢

```js
var http = require("http");

http.createServer(function (req, res) {
    var path = "." + req.url;
    if (path === "./stream") {
        res.writeHead(200, {
            "Content-Type": "text/event-stream",
            "Cache-Control": "no-cache",
            "Connection": "keep-alive",
            "Access-Control-Allow-Origin": '*',
        });
        res.write("retry: 10000\n");
        res.write("event: connecttime\n");
        res.write("data: " + (new Date()) + "\n\n");
        res.write("data: " + (new Date()) + "\n\n");

        interval = setInterval(function () {
            res.write("data: " + (new Date()) + "\n\n");
        }, 1000);

        req.connection.addListener("close", function () {
            clearInterval(interval);
        }, false);
    }
}).listen(8844, "127.0.0.1");
```

最后 `node server.js`  运行就行了.



