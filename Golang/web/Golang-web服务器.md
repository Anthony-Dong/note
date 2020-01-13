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

