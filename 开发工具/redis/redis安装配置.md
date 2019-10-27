# redis安装和配置

## 1.安装
- 下载安装包
上次课前资料提供的安装包，或者:[官网下载](https://redis.io/download)
建议上传到我们的home下：/home/leyou/

- 解压
```shell
 tar -xvf redis-4.0.9.tar.gz
```

- 编译安装
```shell
 mv redis-4.0.9 redis
 cd redis
 make && make install
```

## 2.配置
修改安装目录下的redis.conf文件
```shell
vim redis.conf
```

修改以下配置：
```shell
#bind 127.0.0.1 # 将这行代码注释，监听所有的ip地址，外网可以访问
protected-mode no # 把yes改成no，允许外网访问
daemonize yes # 把no改成yes，后台运行
```

## 3.启动或停止
redis提供了服务端命令和客户端命令：
- redis-server 服务端命令，可以包含以下参数：
  start 启动
  stop 停止
- redis-cli 客户端控制台，包含参数：
  -h xxx 指定服务端地址，缺省值是127.0.0.1
  -p xxx 指定服务端端口，缺省值是6379

## 4.设置开机启动

1) 输入命令，新建文件

```sh
vim /etc/init.d/redis
```

输入下面内容：

```sh
#!/bin/sh
# chkconfig:   2345 90 10
# description:  Redis is a persistent key-value database
PATH=/usr/local/bin:/sbin:/usr/bin:/bin

REDISPORT=6379
EXEC=/usr/local/bin/redis-server
REDIS_CLI=/usr/local/bin/redis-cli

PIDFILE=/var/run/redis.pid

CONF="/home/leyou/redis/redis.conf"

case "$1" in  
    start)  
        if [ -f $PIDFILE ]  
        then  
                echo "$PIDFILE exists, process is already running or crashed"  
        else  
                echo "Starting Redis server..."  
                $EXEC $CONF  
        fi  
        if [ "$?"="0" ]   
        then  
              echo "Redis is running..."  
        fi  
        ;;  
    stop)  
        if [ ! -f $PIDFILE ]  
        then  
                echo "$PIDFILE does not exist, process is not running"  
        else  
                PID=$(cat $PIDFILE)  
                echo "Stopping ..."  
                $REDIS_CLI -p $REDISPORT SHUTDOWN  
                while [ -x ${PIDFILE} ]  
               do  
                    echo "Waiting for Redis to shutdown ..."  
                    sleep 1  
                done  
                echo "Redis stopped"  
        fi  
        ;;  
   restart|force-reload)  
        ${0} stop  
        ${0} start  
        ;;  
  *)  
    echo "Usage: /etc/init.d/redis {start|stop|restart|force-reload}" >&2  
        exit 1  
esac

```

然后保存退出

注意：以下信息需要根据安装目录进行调整：

> EXEC=/usr/local/bin/redis-server # 执行脚本的地址
>
> REDIS_CLI=/usr/local/bin/redis-cli # 客户端执行脚本的地址
>
> PIDFILE=/var/run/redis.pid # 进程id文件地址
>
> CONF="/usr/local/src/redis-3.0.2/redis.conf" #配置文件地址

2）设置权限

```sh
chmod 755 /etc/init.d/redis
```



3）启动测试

```sh
/etc/init.d/redis start
```

启动成功会提示如下信息：

```
Starting Redis server...
Redis is running...
```



4）设置开机自启动

```sh
chkconfig --add /etc/init.d/redis
chkconfig redis on
```

