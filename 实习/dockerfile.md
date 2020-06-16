# docker file

docker file 是用来构建docker image 的工具，我们可以看看官方的文档很详细[https://docs.docker.com/engine/reference/builder/#buildkit](https://docs.docker.com/engine/reference/builder/#buildkit)



> ​	首先看我下面这个例子 ， alpine是一个小型的linux系统，只有10m左右

```java
FROM alpine
ENV MYSQL_ROOT_PASSWORD=root
RUN echo 'we are running some # of cool things' >>/opt/hello
RUN echo $MYSQL_ROOT_PASSWORD >>/opt/hello
```

### FROM

> ​	就是拉去镜像的意思

### ENV

> ​	环境变量 ， 可以写多个

### RUN

> 值得是运行的命令，也就是执行shell ， 可以写多个



其次就是执行这个脚本了`docker build .` 进入dockerfile目录就可以了

```java
% docker build .
Sending build context to Docker daemon  2.048kB
Step 1/4 : FROM alpine
 ---> a187dde48cd2
Step 2/4 : ENV MYSQL_ROOT_PASSWORD=root
 ---> Running in 629241f84747
Removing intermediate container 629241f84747
 ---> 14ef04bd9641
Step 3/4 : RUN echo 'we are running some # of cool things' >>/opt/hello
 ---> Running in 3409a0aa3ecc
Removing intermediate container 3409a0aa3ecc
 ---> 868e409aace8
Step 4/4 : RUN echo $MYSQL_ROOT_PASSWORD >>/opt/hello
 ---> Running in 9131a6825abd
Removing intermediate container 9131a6825abd
 ---> 6e0040043495
Successfully built 6e0040043495
```

每一步都回去构建一个镜像，然后对应着一个hash值，执行每一步执行命令，然后在移除前面那个镜像，最后再build

```java
% docker images
REPOSITORY              TAG                 IMAGE ID            CREATED             SIZE
<none>                  <none>              6e0040043495        7 seconds ago       5.6MB
```

最后详情如上



### copy

`COPY ./Dockerfile  /opt/` 意思就是将当前文件目录下的dockfile 移动到镜像中的/opt/目录中

### ARG

```java
ARG MYSQL_VERSION=latest
FROM mysql:${MYSQL_VERSION}
```

上面的含义就是 一个变量环境， 这个arg只会在 build期间有效， 同时可以通过 `--build-arg <参数名>=<值> ` 来覆盖。

> ​	以上这几个命令全部是build阶段进行的 ，一下几个在run的时候执行的



### CMD [ "echo","hello world" ]

`CMD [ "echo","hello world" ]  意思就是启动的时候执行 echo helloworld ， 多个参数空分号间隔开

可以写多个cmd



### VOLUME

> ​	这个经常遇见， 挂载么 ， 也就是输出会在这里输出



### EXPOSE 

> ​	暴漏的端口  ，也可以通过 run  -p 命令添加 , 但是我们这个expose只是申明方式，必须我们用 -P来随机启动， 我们可以通过 `docker inspect continer_id. 查看具体的端口信息



### ENTRYPOINT

> ​	entrypoint 类似于cmd ，但是cmd可能被覆盖

```java
FROM alpine
ENV MYSQL_ROOT_PASSWORD=root
ARG NAME=anthony
WORKDIR /opt/
RUN echo 'we are running some # of cool things hhhhh' >>./hello
RUN echo $NAME >>./hello
COPY ./Dockerfile /opt/
ENTRYPOINT [ "echo","${NAME}" ]
EXPOSE 8888/tcp
```

构建后， 执行 `docker run --rm alpine:mini hello world` 会输出 ${NAME} hello world ， 

其实就是为了补充命令的， 而cmd 没有这个功能。

### -t

> ​	`docker build -t alpine:mini .`  这个是指定name 和 tag的， 你生存的镜像的



### 



