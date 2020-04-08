# GRPC  go & java 混合平台测试

这个项目链接在 :  https://github.com/Anthony-Dong/protobuf-start  这里, 文章就是这个项目的 ReadMe . 我放到博客里了. 

如果你使用protobuf序列化,此时一定需要统一方式去构建protobuf,并不是随处构建.  protobuf解决了序列化问题, 同时也解决了rpc问题. 

需要学习的两个go 和 Java版本  . 我主要学习这两个版本. 

java的grpc 链接 :  https://github.com/grpc/grpc-java

go的 grpc 链接 : https://github.com/grpc/grpc-go 

这些都包含了, protobuf的使用  . 

### 书写规范

本项目目录 .  src 指的是 package的根目录, 

```java
D:\protobuf>tree
卷 文件盘 的文件夹 PATH 列表
卷序列号为 40D1-2584
D:.
├─bin
├─demo
│  ├─go
│  │  └─com.anthonydong.grpc
│  └─java
│      └─grpc-study-demo02
├─go_out
├─java_out
└─src
    ├─com
    │  └─anthony
    │      └─protobuf
    └─google
        └─protobuf
```

项目结构如上图所示 , 其中 `google.protobuf` 是 google官方的库, 其中已经实现了大部分类.  所以拿来直接用. 

`go_out` 指的是 go编译后的文件输出目录. 

`java_out` 指的是Java编译后的文件输出目录

`demo`  指的是我下面写的demo. 

`bin`  是两个编译protobuf的 go插件. 记得加入path路径. 

### protobuf文件

比如此时我们编译 我自己的项目 . 

比如文件是  `D:\protobuf\src\com\anthony\protobuf\RPCServiceProto.proto` 在这个目录下面

```java
syntax = "proto3"; // 语法版本

// 当前目录
package com.anthony.protobuf;

// 导入proto文件
import "google/protobuf/timestamp.proto";

// 定义一些属性
option java_package = "com.anthonydong.grpc.api";
option java_outer_classname = "RPCServiceApi";
option java_multiple_files = true;
option go_package = "com.anthonydong.grpc/api";


// 请求体
message RPCRequest {
    string msg = 1;
    int32 code = 2;
    google.protobuf.Timestamp time=3;
}
// 响应头
message RPCResponse {
    string msg = 1;
    int32 code = 2;
    google.protobuf.Timestamp time = 3;
}

// 服务接口
service RPCService{
    rpc send (RPCRequest) returns (RPCResponse) {
    }
}
```

打开终端执行   , 这里注意使用 go_out , 别使用 gofast_out , 因为官方库使用的是 go_out编译的, 存在出入. 所以如果你使用 go_fast_out , 最后把官方的也事先编译一遍.

```java
D:\protobuf>protoc -I ./src --go_out=plugins=grpc:./go_out ./src/com/anthony/protobuf/RPCServiceProto.proto
```

这个命令 : 

第一个 :  `-I ` 和 `--proto_path`  含义一样  . 有些时候我们需要导入别的proto文件. 此时就需要使用这个了 .  这个可以指定多个路径. 

第二个 ,  `$DST_DIR` 指定的是 `go_package`的输出路径. 或者`java_package` 输出路径

第三个.  `SRC_DIR` 指的是 我们protobuf源文件的地址.  

第四个 `--go_out` 指的是 go的输出路径.   如果加上 `--go_out=plugins=grpc:` 指的是还要生成grpc相关的代码. 懂了吧,  只写 `--go_out` 只会编译message, 但是如果改成`pluging` 则会编译其他信息.  

基本就是上诉的四个命令.  

### go版本 

然后go编写 服务端代码

```go
package main

import (
	"com.anthonydong.grpc/rpc/api"
	"context"
	"fmt"
	"github.com/golang/protobuf/ptypes/timestamp"
	"google.golang.org/grpc"
	"log"
	"net"
	"time"
)

type server struct {
	api.RPCServiceServer
}

func (*server) Send(ctx context.Context, req *api.RPCRequest) (*api.RPCResponse, error) {
	msg := req.Msg
	s := fmt.Sprintf("Golang服务器已收到你的消息\"%s\"", msg)
	response := &api.RPCResponse{Msg: s, Code: 400, Time: &timestamp.Timestamp{Seconds: int64(time.Now().Second()), Nanos: int32(time.Now().UnixNano())}}
	//time.Sleep(time.Second * 2)
	return response, nil
}

const (
	port = ":9999"
)

func main() {
	// 创建一个 tcp服务器
	lis, err := net.Listen("tcp", port)
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	// 创建一个server
	ser := grpc.NewServer()
	// 注册信息
	api.RegisterRPCServiceServer(ser, &server{})

	// 启动服务
	if err := ser.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}
```

客户端代码

```go
package main

import (
	"com.anthonydong.grpc/rpc/api"
	"context"
	"fmt"
	"google.golang.org/grpc"
	"log"
	"time"
)

const (
	address = "localhost:9999"
)

func main() {

	// 1. 创建一个连接
	conn, err := grpc.Dial(address, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Fatalf("did not connect: %v", err)
	}
	defer conn.Close()

	// 2. 然后创建客户端
	client := api.NewRPCServiceClient(conn)

	// 3. rpc调用
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()
	response, err := client.Send(ctx, &api.RPCRequest{Msg: "hello world"})
	if err != nil {
		fmt.Printf("超时 , 原因 : %s\n", err)
	} else {
		fmt.Printf("结果 : %s , 状态码 : %d.\n", response.Msg, response.Code)
	}
}
```

其中 两个编译器都可以实现这些操作



其次就是如果手动序列化 , 则需要使用 

```go
package main

import (
	"com.anthonydong.grpc/rpc/api"
	"encoding/json"
	"fmt"
	"github.com/golang/protobuf/proto"
	"github.com/golang/protobuf/ptypes/timestamp"
	"time"
)

func main() {

	res := api.RPCResponse{Msg: "hello protobuf", Code: 400, Time: &timestamp.Timestamp{Seconds: int64(time.Now().Second()), Nanos: int32(time.Now().UnixNano())}}
	bytes, _ := proto.Marshal(&res)
	fmt.Printf("proto 长度 : %d.\n", len(bytes))

	marshal, _ := json.Marshal(&res)
	fmt.Printf("json 长度 : %d.\n", len(marshal))
}
```

输出  : , 发现 protobuf 还是很短小的. 确实不赖. 

```go
proto 长度 : 34.
json 长度 : 76.
```

### Java 版本

首先编译protobuf文件

```java
D:\protobuf>protoc -I ./src --java_out=./java_out ./src/com/anthony/protobuf/RPCServiceProto.proto
```

这里有个问题 就是如何编译 rpc . 

其中根据官方提供的  :  [https://github.com/grpc/grpc-java](https://github.com/grpc/grpc-java)

这里有个问题就是 必须使用插件  , 此时我们可以直接将我们的proto文件拷贝到我们的项目目录 , 在 `src/main/proto` 这个位置  

然后 , 编译就可以了 , 基本就是两个步骤.  生成的文件在 target目录下面. 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-30-33/89f29827-9a03-4a73-b77b-b82d090730a2.png?x-oss-process=style/template01)



此时编写代码 , Java客户端代码

```java
package com.anthonydong.grpc.client;

import com.anthonydong.grpc.api.RPCRequest;
import com.anthonydong.grpc.api.RPCResponse;
import com.anthonydong.grpc.api.RPCServiceGrpc;
import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;

import java.util.function.IntConsumer;
import java.util.stream.IntStream;

/**
 * TODO
 *
 * @date:2020/3/31 15:49
 * @author: <a href='mailto:fanhaodong516@qq.com'>Anthony</a>
 */
public class Client {

    private static final String host = "localhost";
    private static final int serverPort = 9999;

    public static void main(String[] args) throws Exception {
        ManagedChannel managedChannel = ManagedChannelBuilder.forAddress(host, serverPort).usePlaintext().build();
        try {
            RPCServiceGrpc.RPCServiceBlockingStub service = RPCServiceGrpc.newBlockingStub(managedChannel);
            IntStream.range(0, 10).forEach(new IntConsumer() {
                @Override
                public void accept(int value) {
                    test(service);
                }
            });
        } finally {
            managedChannel.shutdown();
        }
    }

    private static void test(RPCServiceGrpc.RPCServiceBlockingStub rpcDateService) {
        RPCRequest rpcDateRequest = RPCRequest
                .newBuilder()
                .setMsg("hello protobuf")
                .build();
        RPCResponse response = rpcDateService.send(rpcDateRequest);
        System.out.printf("receive msg : %s, code :%d.\n", response.getMsg(), response.getCode());
    }
}
```

此时输出 

```java
receive msg : 服务器已收到你的消息"hello protobuf", code :400.
```

所以还是相当nice的. 



服务器代码  如下 : 

第一个是接口实现类. 

```java
public class RPCServiceImpl extends RPCServiceGrpc.RPCServiceImplBase {

    @Override
    public void send(RPCRequest request, StreamObserver<RPCResponse> responseObserver) {
        try {
            String msg = request.getMsg();
            RPCResponse response = RPCResponse.newBuilder()
                    .setMsg(String.format("Java服务器收到你的消息\"%s\"", msg))
                    .setCode(400)
                    .setTime(Timestamp.newBuilder().setSeconds(System.currentTimeMillis()).build())
                    .build();

            // reactor模型, onNext 就是消费. 记得最后关闭 流
            responseObserver.onNext(response);
            responseObserver.onCompleted();

        } catch (Exception e) {
            // 异常就发送这个.
            responseObserver.onError(e);
        }
    }
}
```

第二个是服务具体实现 

```java
package com.anthonydong.grpc.server;

public class server {
    private static final int port = 10000;

    public static void main(String[] args) throws IOException, InterruptedException {
        Server server = ServerBuilder.
                forPort(port)
                .addService(new RPCServiceImpl())
                .build();
        server.start();
        System.out.println(String.format("Java服务端启动成功, 端口号: %d.", port));
        server.awaitTermination();
    }
}
```

基本就是这些了.  

这个就可以实现相互调用了 .  



其中 protobuf 主要解决的问题是序列化问题,  同时也规范了 rpc调用. 一种契约体.  