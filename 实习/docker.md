# Docker 和 docker-compose  学习

[TOC]



## 1. Docker 基本命令

### -d  后台运行

> ​	其实就是类似于Java -jar 一样,前台运行,  但是可以用nohup  &, 懂了吧, 就是这个效果

### -t 和 -i  合起来就是 -it

> ​	主要是用于进入bash 界面, 比如 `docker exec -it con_id  /bin/bash`  ， 还可以启动的时候进入比如`docker run -it images_id /bin/bash` , 也可以进入

### --rm

> ​	容器退出后, 自动删除容器 

### --name

> ​	指定容器的名称

```java
[admin@iZ2zecfz1rrahf8ysv6hxiZ ~]$ docker run -d -p80:80 --rm --name m-nginx nginx
4745b4dc17937f789a70a727a520afee2c07904183617a188f00e86795ee86f7
[admin@iZ2zecfz1rrahf8ysv6hxiZ ~]$ docker ps
CONTAINER ID        IMAGE               COMMAND                  CREATED             STATUS              PORTS                NAMES
4745b4dc1793        nginx               "nginx -g 'daemon of…"   9 seconds ago       Up 8 seconds        0.0.0.0:80->80/tcp   m-nginx
```

下面有个nginx 

### -p hostport:containerport

> ​	一个容器的端口映射关系,  所以这里要记住 docker 只支持单个端口通信, 所以比如说 什么rpc调用哇, 就需要关联了. 

`docker run -d -p80:8080 --rm --name m-nginx nginx`  这个意思就是 主机80 映射到对应的8080端口

### -e

> ​	设置环境变量的

```java
[admin@iZ2zecfz1rrahf8ysv6hxiZ ~]$ docker run -d -e NAME=123 -e Pass=123 nginx
fd248d6d407e678364027715c401dbc172c30bc9024915fd968c99c56359224f
[admin@iZ2zecfz1rrahf8ysv6hxiZ ~]$ docker ps
CONTAINER ID        IMAGE               COMMAND                  CREATED             STATUS              PORTS               NAMES
fd248d6d407e        nginx               "nginx -g 'daemon of…"   6 seconds ago       Up 5 seconds        80/tcp              vibrant_franklin
[admin@iZ2zecfz1rrahf8ysv6hxiZ ~]$ docker exec -it fd248d6d407e /bin/bash
root@fd248d6d407e:/# echo $NAME
123
```

### -v  当前主机目录:容器目录

> ​	做一层映射关系  ,可以写多个  , 加上多个 -v就可以了 ， 是一个挂载的意思， 因为很多重要的东西需要挂载，防止容器销毁数据没了

### --link A容器的ID:你给你映射的id

```java
[root@iZ2zecfz1rrahf8ysv6hxiZ php]# docker run -dit --link fd248d6d407e:node  alpine
082398db2b719e21c75a3245b44978ee5db6c377c86a15033071e9d108e29ca5

[root@iZ2zecfz1rrahf8ysv6hxiZ php]# docker ps
CONTAINER ID        IMAGE               COMMAND                  CREATED             STATUS              PORTS                               NAMES
082398db2b71        alpine              "/bin/sh"                2 seconds ago       Up 1 second                                             jovial_gould
6b298c6e0c97        mysql:5.7           "docker-entrypoint.s…"   55 minutes ago      Up 55 minutes       0.0.0.0:3306->3306/tcp, 33060/tcp   mysql
fd248d6d407e        nginx               "nginx -g 'daemon of…"   58 minutes ago      Up 58 minutes       80/tcp                              vibrant_franklin


[root@iZ2zecfz1rrahf8ysv6hxiZ php]# docker exec -it 0823 sh
/ # ping node
PING node (172.18.0.2): 56 data bytes
64 bytes from 172.18.0.2: seq=0 ttl=64 time=0.144 ms
64 bytes from 172.18.0.2: seq=1 ttl=64 time=0.128 ms
64 bytes from 172.18.0.2: seq=2 ttl=64 time=0.113 ms
^C
--- node ping statistics ---
3 packets transmitted, 3 packets received, 0% packet loss
round-trip min/avg/max = 0.113/0.128/0.144 ms

/ # cat /etc/hosts
127.0.0.1	localhost
::1	localhost ip6-localhost ip6-loopback
fe00::0	ip6-localnet
ff00::0	ip6-mcastprefix
ff02::1	ip6-allnodes
ff02::2	ip6-allrouters
172.18.0.2	node fd248d6d407e vibrant_franklin
172.18.0.3	082398db2b71
```

其实是加了一组映射罢了.

如果你的容器想重复使用， 就用 `docker start container_id`

## 2. 创建MySQL 数据库

```java
docker run -p 3306:3306 --name mysql \
-v /usr/local/docker/mysql/conf:/etc/mysql \
-v /usr/local/docker/mysql/logs:/var/log/mysql \
-v /usr/local/docker/mysql/data:/var/lib/mysql \
-e MYSQL_ROOT_PASSWORD=123456 \
-d mysql:5.7
```



## 3.Dockerfile

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

### COPY

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

可以写多个cmd ， 同时如果想运行多个命令，可以看看后面那个解释， 也可以用&& ，但是很多并不能成立；

### VOLUME

> ​	这个经常遇见， 挂载么 ， 也就是输出会在这里输出

### EXPOSE 

> ​	暴漏的端口  ，也可以通过 run  -p 命令添加 , 但是我们这个expose只是申明方式，必须我们用 -P来随机启动我们的暴漏端口， 我们可以通过 `docker inspect continer_id`. 查看具体的端口信息 ，用户也可以自己指定用  `-p 主机端口:容器端口`

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

## 4. docker-composer

> ​	我们发现dockerfile确实不人性化， 所以docker-composer 出现了，通过yaml就可以解决容器构建了

### 简单使用

```yaml
version: "3"
services:
  nginx:
  	container_name: my-nginx
    image: nginx:alpine
    ports:
    - 80:80
    volumes:
    - /home/admin/docker:/user/shares/nginx/html
  	environment:
    - MYSQL_ROOT_PASSWORD=123456
    command: java -jar /opt/app/spring-boot-rest-framework-1.0.0.jar
```

整体结构很简单 : 

1)  version 申明版本号

2)  services 申明需要的多个构建的服务

3)  我们这个是构建 nginx , image是 nginx:alpine , 端口是 80映射, 文件映射是 /home/admin , 环境变量是... , `container_name` 是镜像名称 . 

基本就是这, 可以配置多个 service 跟nginx同级, 而且相互之间可以依赖.

### 和dockerfile组合使用

其中可能使用到 docker-compose 和 dockerfile组合使用 ，下面就是，会执行build里面指定的数据，context是指的是context路径，其次就是dockerfile的文件名 ，其次就是 args（同dockerfile的ARG）

```docker
mysql:
  build:
    context: ./mysql
    dockerfile: Dockerfile
    args:
      - MYSQL_VERSION:5.7
```

关于docker-compose 执行多条命了， 具体解决方法可以看一下以下的两个链接 ： 

[https://segmentfault.com/q/1010000014461396](https://segmentfault.com/q/1010000014461396)

[https://github.com/docker-library/redmine/issues/52](https://github.com/docker-library/redmine/issues/52)

是一个issues ， 

## springboot+mysql 如何使用

下面是个例子 , 其中环境变量在 同级目录下的`.env`中

```java
MYSQL_VERSION=5.7
MYSQL_PATH=/Users/xxx/docker/spring-boot/mysql
JAR_PATH=/Users/xxx/java/code/spring-demo/spring-boot-demo/target
Spring_ENV=dev
```

`docker-compose.yml` 文件

```yml
version: "3"
services:
  mysql:
    build:
      context: ./mysql
      dockerfile: Dockerfile
    volumes:
      - ${MYSQL_PATH}/data:/var/lib/mysql
    ports:
      - 3306:3306
    environment:
      - MYSQL_ROOT_PASSWORD=123456
      - MYSQL_DATABASE=jpa
  spring-boot:
    image: openjdk:8u252-jre
    volumes:
      - ${JAR_PATH}:/opt/jar
    ports:
      - 8888:8888
    command: java -jar -Dspring.profiles.active=${Spring_ENV} /opt/jar/spring-boot-demo-0.0.1-SNAPSHOT.jar
```

此后，构建就可以了， 这里不需要 link链接mysql ， 我们只需要我们的Springboot配置文件中用 mysql代替mysql服务器的域名就可以了

