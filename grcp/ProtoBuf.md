# Protobuf 文件书写格式

前期准备 :  https://github.com/protocolbuffers/protobuf/releases 下载编译器. protoc 编译器. 



官方文档 , 也就是proto3 / proto2文档, 在 https://developers.google.com/protocol-buffers/docs/proto3 这里, 如果英文水平差的同志呢, 可以看看这个  https://www.jianshu.com/p/bdd94a32fbd1 ,  其实proto3 更加写起来方便.   



我写了一篇文章 https://github.com/Anthony-Dong/protobuf-start  , 就是基本的使用.  



> ​	其中要区分开 message 和 service. 前者是序列化, 后者提供了rpc服务.  大部分场景只需要message . 同时编译service. 也需要插件进行编译 . 普通的protoc --go_out  --java_out 都不行. 

如果玩 golang的 , 可以使用 [gofast](https://github.com/gogo/protobuf)  插件编译 . 也不错. 但是和官方自带的一些库冲突.  最好别混合使用. 

对比两张性能图

![]( https://tyut.oss-accelerate.aliyuncs.com/image/2020-40-44/f13213fe-e186-46bc-ad92-4c9c176b7c9e.png?x-oss-process=style/template01)

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-40-44/95d83bfb-adcd-425d-aca7-6e79fc5cea33.png?x-oss-process=style/template01)

## 文件结构

#### 基本结构

其实我大致说一下protobuf文件的基本结构 . 

写protobuf文件 , 使用vscode 写比较棒  . 有代码提示, 下载一个插件就行了.  我觉得protobuf文件最好和业务抽离出来. 别混在一起. 

```java
syntax = "proto3"; // 语法版本

// 这个属于package . 别的protobuf文件引用的时候需要指定这个. 最好自己给 protobuf定义一个path.
package com.anthony.protobuf;

// Java选项
// Java类输出的目的地
option java_package = "com.example.grpc.api";
// 输出的Class文件. 如果java_multiple_files=false,则只输出这一个文件
// 否则输出 三个文件. Builder , RPCDateRequest ,Request 三个文件, 所以 java_outer_classname不能和RPCDateRequest名字重复
option java_outer_classname = "Request";
// 上面解释过了
option java_multiple_files = true;

// Go 选项 , 也是它指的是生成后的 ,目录位置. 相对于protoc指定的地址.
option go_package = "github.com/golang/protobuf/ptypes/timestamp";


// 定义消息 , 这里需要注意的是这个对象可以嵌套 互相引用.
message RPCDateRequest {
    uint64 id = 1;
    string msg = 2;
}
```

#### 如何编译 

> ​	最好熟练掌握编译命令 . 

第三步. 就是编译了 .  `protoc` 命令

```java
protoc -I=$SRC_DIR --go_out=$DST_DIR $SRC_DIR/addressbook.proto
```

第一个意思是 :  `-I ` 和 `--proto_path`  含义一样  . 有些时候我们需要导入别的proto文件. 此时就需要使用这个了 .  这个可以指定多个路径. 

第二个就是 ,  `$DST_DIR` 指定的是 `go_package`的输出路径. 或者`java_package` 输出路径

第三个就是.  `SRC_DIR` 指的是 我们protobuf源文件的地址.  

第四个比如说 `--go_out` 指的是 go的输出路径.   `--go_out=plugins=grpc:` 这个指的是生成 grpc 相关的配置. 

基本就是上诉四个命令. 



## Message

> ​	其中 message 有三个修饰词, 但是在proto3中, 只留下了一个, 默认一个缺醒值 .  `required ` 表示必须出现一次, `optional`  出现零次或者一次,  `repeated` : 出现零次或者多次 , 其实就是数组. 也就是前面三个, 但是proto3中只保留了 第三个`repeated` .  默认就是`optional` ,不需要显示写出来,写出来就异常了.   
>
> ​	同时proto2支持默认模式, 也就是 `default` 关键字 , 但是在 proto3中取消了. 

#### 基本写法

```java
syntax="proto3";

package  com.anthony.protobuf;

// 引入其他的protobuf文件
import "google/protobuf/empty.proto";

option java_multiple_files=true;
option java_package="com.anthony.api";
option java_outer_classname="PeopleApi";

message People{
    // 1 ,2 ,3 ,4 的意思是index , 所以传输的时候可以直接考这个传输位置. 这个是死的, 千万别随意改变. 
    // 1-15 占用一个字节.  后面的占用两个字节. 所以最好使用前面的15个字段.
    string name=1;
    int32 age=2;
    // 枚举
    enum Gender{
        // 这个必须从0开始
        Female = 0;
        Male = 1;
        Null=2;
    }
    Gender gender=3 ;
    // 0-1个 ,repeated 其实就是一个数组.
    // 还可以使用其他类型.
    repeated Hobby hobbies=4;

    // map
    map<string,Salary>  job=5;
}

message Salary{
     int32 salary=1;
}

service PeopleService{
    rpc addUser (People) returns (google.protobuf.Empty){}
}

message Hobby{
    string name=1;
}

message Student{
    
}
```



#### 保留字 reserved 用法

比如 我的第一代版本中, 有一个字段, 比如下面这种情况

```java
syntax="proto2";

package  com.anthony.protobuf;

option go_package="api";

message People{
    required string name=1;
    optional int32 age=2;
    // 第二个版本取消掉.
    optional string info=3;
}
```

执行 `protoc -I ./src --go_out=./go_out  ./src/com/anthony/protobuf/user.proto`然后我们为了验证. 此时需要保存到文件中 , 调用这个方法. 

```go
func write() {
	people := api.People{}
	name := "tom"
	people.Name = &name
	age := int32(20)
	people.Age = &age
	info := "是个男的"
	people.Info = &info

	// 序列化
	bytes, _ := proto.Marshal(&people)

	// 保存到文件中
	file, e := os.OpenFile("save.txt", os.O_CREATE|os.O_WRONLY|os.O_TRUNC, 0644)
	if e != nil {
		log.Fatal(e)
	}
	file.Write(bytes)
	file.Close()
}
```



此时下一代来了, 我们舍弃掉了 info字段. 此时修改了proto文件 . 

```java
syntax="proto3";
package  com.anthony.protobuf;
option go_package="api";

message People{
    string name=1;
    int32 age=2;
    // 第一代版本中的.
    // string info=3;
    reserved 3;
}
```

改成了上诉这个, 然后编译 

此时 调用这个方法. 

```go
func read() {
	bytes, _ := ioutil.ReadFile("save.txt")
	var rest api.People
	proto.Unmarshal(bytes,&rest)
	fmt.Printf("%#v\n",rest)
	marshal, _ := json.Marshal(&rest)
	fmt.Printf("%s", marshal)
}
```

输出 

```go
api.People{Name:"tom", Age:20, XXX_NoUnkeyedLiteral:struct {}{}, XXX_unrecognized:[]uint8{0x1a, 0xc, 0xe6, 0x98, 0xaf, 0xe4, 0xb8, 0xaa, 0xe7, 0x94, 0xb7, 0xe7, 0x9a, 0x84}, XXX_sizecache:0}
{"name":"tom","age":20}
```

说明个问题, 完全兼容的.   其实就算是不加上 reserved  . 也没啥事, 不信你删了它试试.   可能是我理解的出入问题. 

官方的意思是 : **如果删除了某一个字段，protobuf允许重新使用该数值作为新的属性的标签，但是为了保证向后兼容，读取旧的数据的时候不会出现问题，一般使用reserved来声明该数值为保留，不能被使用。  反正就是个保留字, 是一种协议, 为了拓展罢了. 告诉对方这个是保留位.** 



#### 支持JSON

所以反正推荐使用 proto3 , 初次使用的话.  同时他还支持json. 原因是因为 , tag标签中有 json.

```go
type People struct {
	Name                 string   `protobuf:"bytes,1,opt,name=name,proto3" json:"name,omitempty"`
	Age                  int32    `protobuf:"varint,2,opt,name=age,proto3" json:"age,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}
```

Java如何做json呢. 

需要 如下这么做. 使用第三方显然不可能.  需要使用 `JsonFormat` 工具类

```java
public static void main(String[] args) throws InvalidProtocolBufferException {
    NPack hello = NPack.newBuilder().setBody(ByteString.copyFrom("helloworld".getBytes()))
            .setUrl("hello")
            .setTimeStamp(System.currentTimeMillis()).build();


    // 序列化成 json
    String print = JsonFormat.printer().print(hello);
    System.out.println(print);


    // 反序列化成 result
    NPack.Builder result = NPack.newBuilder();
    JsonFormat.parser().merge(print, result);

    NPack build = result.build();
    System.out.println(build);
}
```



## service 

> ​	记住一点就行了, service 不能有普通字段, 必须是 包装类型, 也就是必须是message. 也不能返回多个对象. 



## 书写规范

#### message字段

使用驼峰命名法（首字母大写）命名 message，例子：**SongServerRequest**
使用下划线命名字段，栗子：**song_name**

```java
message SongServerRequest {
  required string song_name = 1;
}
```

#### 枚举

使用驼峰命名法（首字母大写）命名枚举类型，使用 “大写_下划线_大写” 的方式命名枚举值：

```java
enum Foo {
  FIRST_VALUE = 0;
  SECOND_VALUE = 1;
}
```

### service 字段

如果你在 .proto 文件中定义 RPC 服务，你应该使用驼峰命名法（首字母大写）命名 RPC 服务以及其中的 RPC 方法：

```java
service FooService {
  rpc GetSomething(FooRequest) returns (FooResponse);
}
```

## 总结

多写 , 多敲. 别把protobuf 作为rpc.  而是当做序列化协议. 我们大部分场景只需要它的序列化功能. 