# Go-kit 玩法

首先，玩go-kit，他不是一个mvc框架，他的架构层次比较多一些。

大致上分为`transport`   ,  `endpoint` , `service` 

## http-demo

```go
import (
	"context"
	"encoding/json"
	"fmt"
	transport "github.com/go-kit/kit/transport/http"
	"net/http"
)

type InfoDto struct {
	Version string
}

func main() {

	var endpoint = func(ctx context.Context, request interface{}) (response interface{}, err error) {
		dto := request.(*InfoDto)
		fmt.Println("version :", dto.Version)
		response = map[string]interface{}{
			"data": "ok",
		}
		err = nil
		return
	}

	// transport  需要将 service+编解码 柔和起来
	hand := transport.NewServer(endpoint, func(i context.Context, req *http.Request) (request interface{}, err error) {
		request = &InfoDto{}
		err = json.NewDecoder(req.Body).Decode(&request)
		return
	}, func(i context.Context, writer http.ResponseWriter, response interface{}) (err error) {
		err = json.NewEncoder(writer).Encode(response)
		return
	})

	http.Handle("/", hand)
	http.ListenAndServe(":8888", nil)

}
```



简单的来看，其实他就是将  transport 抽象了一下，其实做的事controller的事情，endpoint可以理解为是service。



## 定义middlerware

> 中间件其实是修饰的 endpoint ，类似于Spring的interceptor

```go
// 限流
type Limier interface {
	Allow() bool
}

type defaultLimiter struct {
}

func (*defaultLimiter) Allow() bool {
	return rand.Intn(3) == 1
}

// 添加限流
func addLimier(limier Limier) endpoint.Middleware {
	return func(next endpoint.Endpoint) endpoint.Endpoint {
		return func(ctx context.Context, request interface{}) (response interface{}, err error) {
			if limier.Allow() {
				return next(ctx, request)
			}
			return endpoint.Nop(ctx, request)
		}
	}
}

// 使用
end = addLimier(&defaultLimiter{})(end)
```





## 定义前置、后置器等

> 类似于Java的Servlet的Filter，但是不具备拦截功能

```go
// option 的方法，很好的解决go的重载
option := transport.ServerBefore(func(ctx context.Context, request *http.Request) context.Context {
  fmt.Println("http before")
  return ctx
})
```



然后看看zipkin的组合实现 ,其实就是在前后结束实现了一个拦截，但是真的符合我们的要求吗。显然不符合。

```go
func HTTPServerTrace(tracer *zipkin.Tracer, options ...TracerOption) kithttp.ServerOption {
	serverBefore := kithttp.ServerBefore(
	)

	serverAfter := kithttp.ServerAfter(
	)

	serverFinalizer := kithttp.ServerFinalizer(
	)
	return func(s *kithttp.Server) {
		serverBefore(s)
		serverAfter(s)
		serverFinalizer(s)
	}
}
```



## go-kit http整体设计

#### 结构

```go
type Server struct {
	e            endpoint.Endpoint //service
	dec          DecodeRequestFunc // 编解码
	enc          EncodeResponseFunc
	before       []RequestFunc //前置处理器
	after        []ServerResponseFunc //后置处理器
	errorEncoder ErrorEncoder// error处理器
	finalizer    []ServerFinalizerFunc
	errorHandler transport.ErrorHandler// error处理器
}
```



#### 处理逻辑

```go
// ServeHTTP implements http.Handler.
func (s Server) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	ctx := r.Context()

  // defer
	if len(s.finalizer) > 0 {
		iw := &interceptingWriter{w, http.StatusOK, 0}
		defer func() {
			ctx = context.WithValue(ctx, ContextKeyResponseHeaders, iw.Header())
			ctx = context.WithValue(ctx, ContextKeyResponseSize, iw.written)
			for _, f := range s.finalizer {
				f(ctx, iw.code, r)
			}
		}()
		w = iw
	}

  // before
	for _, f := range s.before {
		ctx = f(ctx, r)
	}

  // 解码
	request, err := s.dec(ctx, r)
	if err != nil {
    // 异常处理器
		s.errorHandler.Handle(ctx, err)
		s.errorEncoder(ctx, err, w)
		return
	}

  // 处理器，异常处理器
	response, err := s.e(ctx, request)
	if err != nil {
		s.errorHandler.Handle(ctx, err)
		s.errorEncoder(ctx, err, w)
		return
	}

  // 后置处理器
	for _, f := range s.after {
		ctx = f(ctx, w)
	}

  // 解码
	if err := s.enc(ctx, w, response); err != nil {
		s.errorHandler.Handle(ctx, err)
		s.errorEncoder(ctx, err, w)
		return
	}
}
```



#### 基本流程

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-60-66/1c467543-9f9c-49a3-8130-c576416f6ef4.png)



## go-kit 整合grpc

> ​	首先整合过程很麻烦，它基本是基于service实现的，

```go
syntax = "proto3";

package grpc_demo;

service Add {
    rpc Sum (SumRequest) returns (SumReply) {
    }
}

message SumRequest {
    int64 a = 1;
    int64 b = 2;
}

message SumReply {
    int64 v = 1;
    string err = 2;
}
```

脚本

```shell
#!/usr/bin/env sh

# Install proto3 from source
#  brew install autoconf automake libtool
#  git clone https://github.com/google/protobuf
#  ./autogen.sh ; ./configure ; make ; make install
#
# Update protoc Go bindings via
#  go get -u github.com/golang/protobuf/{proto,protoc-gen-go}
#
# See also
#  https://github.com/grpc/grpc-go/tree/master/examples

protoc addsvc.proto --go_out=plugins=grpc:.
```

程序

```go
package grpc_demo

import (
	"context"
	"fmt"
	grpctransport "github.com/go-kit/kit/transport/grpc"
)

// 实现sum方法
type grpcServer struct {
	sum    grpctransport.Handler
	concat grpctransport.Handler
}


//生成器
func NewGRPCServer(service endpoint.Endpoint) AddServer {
	return &grpcServer{
		sum: grpctransport.NewServer(
			service,
			decodeGRPCSumRequest,
			encodeGRPCSumResponse,
		),
	}
}


//代码生成器生成
func (s *grpcServer) Sum(ctx context.Context, req *SumRequest) (*SumReply, error) {
	_, rep, err := s.sum.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	return rep.(*SumReply), nil
}

// 代码生成器生成
func decodeGRPCSumRequest(_ context.Context, grpcReq interface{}) (interface{}, error) {
	req, _ := grpcReq.(*SumRequest)
	return &SumRequest{A: int64(req.A), B: int64(req.B)}, nil
}

// 代码生成器
func encodeGRPCSumResponse(_ context.Context, response interface{}) (interface{}, error) {
	resp, _ := response.(*SumReply)
	return resp, nil
}
```

以上代码均可以使用代码生成器生成



一下，是主方法

```go
func Main() {
	grpcListener, err := net.Listen("tcp", ":8888")
	if err != nil {
		panic(err)
	}
	g := grpc.NewServer()

	RegisterAddServer(g, NewGRPCServer(func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*SumRequest)
		return &SumReply{V: req.B + req.A, Err: ""}, nil
	}))
	err = g.Serve(grpcListener)
	if err != nil {
		panic(err)
	}
}
```



客户端方法

```go
func NewClient() {
	// 1. 创建一个连接
	conn, err := grpc.Dial(":8888", grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Fatalf("did not connect: %v", err)
	}
	defer conn.Close()

	// 2. 然后创建客户端
	client := NewAddClient(conn)

	// 3. rpc调用
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()
	response, err := client.Sum(ctx, &SumRequest{A: 1, B: 2})
	if err != nil {
		fmt.Printf("超时 , 原因 : %s\n", err)
	} else {
		fmt.Printf("结果 : %v\n", response.V)
	}
}
```

这里我并没有get 到 go-kit的好处，发现很多没有用，也就是编解码器，根本没有用处，我觉得他的源码中，是不是可以加入一个如果他为空，可以跳过呢。



其实性能压测来说，grpc本身的效率只和 http快2-3倍，并没有想象中那么快。







## grpc真的那么快吗

​     我自己测试，本地测试，grpc并没有http快，可能是go的http的transport比较好，复用连接，不需要重复建立，但是可以看到http在大多数场景下还是可以起到很大作用的。

我感觉dubbo会很快。

```go

import (
	"bytes"
	"encoding/json"
	"fmt"
	"go-kit/demo"
	"io"
	"io/ioutil"
	"net/http"
	"runtime"
	"time"
)

func main() {
	server()
	runtime.Gosched()
	client()
}

func server() {
	http.HandleFunc("/add", func(writer http.ResponseWriter, request *http.Request) {
		req := demo.AddReqeust{}
		err := json.NewDecoder(request.Body).Decode(&req)
		if err != nil {
			panic(err)
		}
		defer request.Body.Close()
		_ = json.NewEncoder(writer).Encode(map[string]interface{}{
			"result": req.A + req.B,
		})
	})
	go http.ListenAndServe(":8888", nil)
}

func client() {
	now := time.Now()
	count := 10000
	for x := 0; x < count; x++ {
		request()
	}
	fmt.Println(time.Now().Sub(now).Milliseconds())
}

func request() {
	reader, err := addJsonRequestParams(&demo.AddReqeust{
		A: 1,
		B: 2,
	})
	if err != nil {
		return
	}
	resp, err := http.Post("http://127.0.0.1:8888/add", "application/json", reader)
	if err != nil {
		return
	}
	defer resp.Body.Close()
	all, err := ioutil.ReadAll(resp.Body)
	fmt.Printf("%s", all)
}

func addJsonRequestParams(params interface{}) (io.Reader, error) {
	var buf bytes.Buffer
	if err := json.NewEncoder(&buf).Encode(params); err != nil {
		return nil, err
	}
	return ioutil.NopCloser(&buf), nil
}
```





grpc测试

```go
import (
	"context"
	"fmt"
	"go-kit/grep_demo"
	"google.golang.org/grpc"
	"log"
	"net"
	"runtime"
	"time"
)

type demos struct {
}

func (*demos) Sum(ctx context.Context, req *grep_demo.SumRequest) (*grep_demo.SumReply, error) {
	return &grep_demo.SumReply{V: req.A + req.B, Err: ""}, nil
}

func main() {
	rpcserver()
	runtime.Gosched()
	rpcclient()
}

func rpcserver() {
	listener, err := net.Listen("tcp", ":8888")
	if err != nil {
		return
	}
	server := grpc.NewServer()
	dd := demos{}
	grep_demo.RegisterAddServer(server, &dd)
	go server.Serve(listener)
}

func rpcclient() {
	// 1. 创建一个连接
	conn, err := grpc.Dial(":8888", grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Fatalf("did not connect: %v", err)
		return
	}
	defer conn.Close()

	// 2. 然后创建客户端
	client := grep_demo.NewAddClient(conn)
	now := time.Now()
	for x := 0; x < 10000; x++ {
		func() {
			// 3. rpc调用
			ctx, cancel := context.WithTimeout(context.Background(), time.Second)
			defer cancel()
			response, err := client.Sum(ctx, &grep_demo.SumRequest{A: 1, B: 2})
			if err != nil {
				fmt.Printf("err=%s\n", err)
			} else {
				fmt.Printf("{\"result\":%d}\n", response.V)
			}
		}()
	}
	fmt.Println(time.Now().Sub(now).Milliseconds())
}
```



