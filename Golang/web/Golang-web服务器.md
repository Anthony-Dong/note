# Golang-web

> ​	Golang原生上支持的web服务器就很厉害, 不需要框架就可以很好的处理问题

## 1. 最简单的Demo

```go
type handler struct {
}
func (s *handler) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	fmt.Printf("请求方式 : %v\n", r.Method)
	fmt.Printf("请求路径 : %s\n", r.URL.Path)

	forms := r.Form
	if forms != nil {
		for k, v := range forms {
			fmt.Println("key : ", k, " , value : ", v)
		}
	}

	bytes := make([]byte, r.ContentLength)
	r.Body.Read(bytes)
	fmt.Println("请求体 : ", string(bytes))
    // 必须先写头
    w.Header().Set("Access-Control-Allow-Origin", "*")
    w.Header().Set("Content-Type", "application/json")
    // 再响应码
    w.WriteHeader(201)
    // 再数据
    w.Write([]byte(`{"msg": "hello golang !"}`))
}

func main() {
	hand := &handler{}
    
	listener, _ := net.Listen("tcp", "127.0.0.1:9090")
    // 核心代码就是 server , 他需要一个监听器和一个handler
	http.Serve(listener, hand)
}
```

这就是一个golang的web服务器 . 

`func Serve(l net.Listener, handler Handler) error`  方法

还有种写法一般网上是这种

```go
http.HandleFunc("/get", func(writer http.ResponseWriter, request *http.Request) {
    //do
})         
http.ListenAndServe("127.0.0.1:9090", nil)
```

其实他的执行逻辑跟第一个一样

```go
func Serve(l net.Listener, handler Handler) error {
	srv := &Server{Handler: handler}
    // 先不看这个
	return srv.Serve(l)
}

// 监听启动
func ListenAndServe(addr string, handler Handler) error {
	// 实例化一个对象
    server := &Server{Addr: addr, Handler: handler}
    // 他监听启动
	return server.ListenAndServe()
}

func (srv *Server) ListenAndServe() error {
	if srv.shuttingDown() {
		return ErrServerClosed
	}
	addr := srv.Addr
	if addr == "" {
		addr = ":http"
	}
    // 监听端口
	ln, err := net.Listen("tcp", addr)
	if err != nil {
		return err
	}
    // 他包装了一层,因为golang中的con,是不会自动断开的,就算客户端断开连接,所以需要自己封装
	return srv.Serve(tcpKeepAliveListener{ln.(*net.TCPListener)})
}
```



```go
func (srv *Server) Serve(l net.Listener) error {
	if fn := testHookServerServe; fn != nil {
		fn(srv, l) // call hook with unwrapped listener
	}

	l = &onceCloseListener{Listener: l}
    
     // 最后监听器关闭
	defer l.Close()

    // 开启
	if err := srv.setupHTTP2_Serve(); err != nil {
		return err
	}

	if !srv.trackListener(&l, true) {
		return ErrServerClosed
	}
	defer srv.trackListener(&l, false)

	var tempDelay time.Duration     // how long to sleep on accept failure
	baseCtx := context.Background() // base is always background, per Issue 16220
	ctx := context.WithValue(baseCtx, ServerContextKey, srv)
    
    // 无脑循环中... 服务器都是
	for {
        // 接收一个 conn
		rw, e := l.Accept()
		if e != nil {
			select {
			case <-srv.getDoneChan():
				return ErrServerClosed
			default:
			}
			if ne, ok := e.(net.Error); ok && ne.Temporary() {
				if tempDelay == 0 {
					tempDelay = 5 * time.Millisecond
				} else {
					tempDelay *= 2
				}
				if max := 1 * time.Second; tempDelay > max {
					tempDelay = max
				}
				srv.logf("http: Accept error: %v; retrying in %v", e, tempDelay)
				time.Sleep(tempDelay)
				continue
			}
			return e
		}
		tempDelay = 0
        // 封装一个conn
		c := srv.newConn(rw)
		c.setState(c.rwc, StateNew) // before Serve can return
        // 开启一个 goruntime进行后续处理,这里就是处理逻辑了
		go c.serve(ctx)
	}
}
```



## 2. http 客户端

其实不管怎么使用都是调用的是` func (c *Client) Do(req *Request) (*Response, error) `这个方法. 

简单使用 :  

就是这么简单 . 都是默认,有何不好呢.  但是对于特殊情况  , 比如说客户端连接超时机制 . 

```go
resp, err := http.Get("http://localhost:8888")
```

// DefaultClient 是一个var DefaultClient = &Client{} 单例对象

```go
func Get(url string) (resp *Response, err error) {
	return DefaultClient.Get(url)
}
```

// 继续走  , 需要`NewRequest()` 方法实例化一个request对象. 

```go
func (c *Client) Get(url string) (resp *Response, err error) {
	req, err := NewRequest("GET", url, nil)
	if err != nil {
		return nil, err
	}
	return c.Do(req)
}
```

` c.Do(req)` - > `Client.send()` 方法  , 前面都是对cookie之类的东西进行不断的处理 .  也就是不断的封装request对象. 

最后还是来到了  `RoundTripper.RoundTrip(req)` 方法.  

```go
// Transport specifies the mechanism by which individual
// HTTP requests are made.
// If nil, DefaultTransport is used. 默认使用的是DefaultTransport
Transport RoundTripper 
```

他每次会调用 :  当你没有就会返回一个 DefaultTransport

```go
func (c *Client) transport() RoundTripper {
	if c.Transport != nil {
		return c.Transport
	}
	return DefaultTransport
}
```

```go
var DefaultTransport RoundTripper = &Transport{
	Proxy: ProxyFromEnvironment,
	DialContext: (&net.Dialer{
		Timeout:   30 * time.Second,
		KeepAlive: 30 * time.Second,
		DualStack: true,
	}).DialContext,
	MaxIdleConns:          100,
	IdleConnTimeout:       90 * time.Second,
	TLSHandshakeTimeout:   10 * time.Second,
	ExpectContinueTimeout: 1 * time.Second,
}
```

这里携带了大量的信息 . 包括超时之类的. 

```go
resp, didTimeout, err = send(req, c.transport(), deadline)
```

最后还是来到了 `RoundTrip(*Request) (*Response, error)`   , `Transport`实现了这个接口 . 





加入超时机制.  就需要自己定义了`http.Client` .  加入 `Timeout `.   

```go
import (
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"time"
)

func main() {

	// 定义客户端
	client := &http.Client{Timeout: time.Second * 10}

    // 定义请求
	req, _ := http.NewRequest("GET", "http://localhost:8888", nil)

	// 开始时间
	start := time.Now().UnixNano() / 1e6

	// 发送请求
	response, e := client.Do(req)

	// 异常
	if e != nil {
		fmt.Printf("cost : %d", time.Now().UnixNano()/1e6-start)
		os.Exit(-1)
	}

	// 关闭流
	defer response.Body.Close()

	// 打印
	body, _ := ioutil.ReadAll(response.Body)
	fmt.Printf("cost : %d , res : %s\n", time.Now().UnixNano()/1e6-start, body)
}
```

其实也可以使用 `http.Transport` 的CancelRequest(req)

```go
func main() {

	// 定义一个超时 ctx
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*2)
	defer cancel()

	// 定义一个客户端
	tr := &http.Transport{}
	client := &http.Client{Transport: tr}
	req, _ := http.NewRequest("GET", "http://localhost:1111", nil)

	// 定义一个管道去接收 响应
	ch := make(chan struct {
		*http.Response
		error
	})

	go func() {
		// 发送请求
		resp, err := client.Do(req)
		ch <- struct {
			*http.Response
			error
		}{resp, err}
	}()

	// 阻塞获取结果: 要么拿去结果. 要么超时
	select {
	case resp, _ := <-ch:
		defer resp.Body.Close()
		bytes, _ := ioutil.ReadAll(resp.Body)
		fmt.Printf("response : %s \n", bytes)
	case <-ctx.Done():
		// 取消请求
		tr.CancelRequest(req)
		// 获取取消结果
		_, e := <-ch
		fmt.Println("time out", e)
	}
}
```



两种方式各有各的好处 . 