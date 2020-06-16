# Dokcer-compose 

> ​	我们发现dockerfile确实不人性化，不管是书写还是什么， 所以docker-composer 出现了，通过yaml配置话的方式就可以解决容器构建了

### 简单认识语法

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

## springboot+mysql 如何构建

下面是个例子 , 其中环境变量在同级目录下的`.env`中 ，如下图所示，为了打包方便，我们使用镜像挂载的方式。

```java
MYSQL_VERSION=5.7
MYSQL_PATH=/Users/xxx/docker/spring-boot/mysql
JAR_PATH=/Users/xxx/java/code/spring-demo/spring-boot-demo/target
Spring_ENV=dev
```

mysql的 `Dockerfile`如下所示

```dockerfile
ARG MYSQL_VERSION=5.7
FROM mysql:${MYSQL_VERSION}
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

此后，构建就可以了， 这里不需要使用 link来链接mysql ， 我们只需要我们的Springboot配置文件中用mysql代替mysql服务器的域名就可以了，

比如`spring.datasource.url=jdbc:mysql://mysql:3306/jpa?characterEncoding=utf8&useSSL=false` 便可以了