# nginx
## 1.概念:	
> nginx是一款自由的、开源的、高性能的HTTP服务器和反向代理服务器；同时也是一个IMAP、POP3、SMTP代理服务器；nginx可以作为一个HTTP服务器进行网站的发布处理，另外nginx可以作为反向代理进行负载均衡的实现。 我们将静态资源放到nginx中，静态资源可以转发到服务器FS, 动态请求可以代理到tomcat之类的服务器
>
> 网站 http://nginx.org/

## 2. 对代理服务器而言的正向代理与反向代理
#### 1.正向代理
![image](https://imgconvert.csdnimg.cn/aHR0cHM6Ly91cGxvYWQtaW1hZ2VzLmppYW5zaHUuaW8vdXBsb2FkX2ltYWdlcy82MTUyNTk1LWQ3OGJiOWE3NzZiYmUxMmIucG5nP2ltYWdlTW9ncjIvYXV0by1vcmllbnQvc3RyaXAlN0NpbWFnZVZpZXcyLzIvdy8xMjQw)

	客户端想要访问一个服务器，但是它可能无法直接访问这台服务器，这时候这可找一台可以访问目标服务器的另外一台服务器，而这台服务器就被当做是代理人的角色 ，称之为代理服务器，于是客户端把请求发给代理服务器，由代理服务器获得目标服务器的数据并返回给客户端。客户端是清楚目标服务器的地址的，而目标服务器是不清楚来自客户端，它只知道来自哪个代理服务器，所以正向代理可以屏蔽或隐藏客户端的信息。
![image](https://imgconvert.csdnimg.cn/aHR0cHM6Ly91cGxvYWQtaW1hZ2VzLmppYW5zaHUuaW8vdXBsb2FkX2ltYWdlcy82MTUyNTk1LWYxZTdlZTA5MDdiZjJhMTUucG5nP2ltYWdlTW9ncjIvYXV0by1vcmllbnQvc3RyaXAlN0NpbWFnZVZpZXcyLzIvdy8xMjQw)
#### 2.反向代理
从上面的正向代理，你会大概知道代理服务器是为客户端作代理人，它是站在客户端这边的。
其实反向代理就是代理服务器为服务器作代理人，站在服务器这边，它就是对外屏蔽了服务器的信息，常用的场景就是多台服务器分布式部署，像一些大的网站，由于访问人数很多，就需要多台服务器来解决人数多的问题，这时这些服务器就由一个反向代理服务器来代理，客户端发来请求，先由反向代理服务器，然后按一定的规则分发到明确的服务器，而客户端不知道是哪台服务器。常常用nginx来作反向代理。

![image](https://imgconvert.csdnimg.cn/aHR0cHM6Ly91cGxvYWQtaW1hZ2VzLmppYW5zaHUuaW8vdXBsb2FkX2ltYWdlcy82MTUyNTk1LWYxZTdlZTA5MDdiZjJhMTUucG5nP2ltYWdlTW9ncjIvYXV0by1vcmllbnQvc3RyaXAlN0NpbWFnZVZpZXcyLzIvdy8xMjQw)
## 3.负载均衡的实现
> - 轮询
> - 权重
> - ip_hash
> - rt速度
> - url_hash

```shell
// 一般默认采用的轮询方式;但是也可以设置为ip_hash

// 1.轮询（默认）每个请求按时间顺序逐一分配到不同的后端服务器，如果后端服务器down掉，能自动剔除。

// 2.weight 指定轮询几率，weight和访问比率成正比，用于后端服务器性能不均的情况。
upstream bakend {  
  server 192.168.0.14 weight=10;  
  server 192.168.0.15 weight=10;  
}

// 3.ip_hash  每个请求按访问ip的hash结果分配，这样每个访客固定访问一个后端服务器，可以解决session的问题。
upstream bakend {  
  ip_hash;  
  server 192.168.0.14:88;  
  server 192.168.0.15:80;  
}

// 4.fair 按后端服务器的响应时间来分配请求，响应时间短的优先分配。
upstream backend {  
  fair;  
  server server1;  
  server server2;  	  
}

// 5. url_hash 按访问url的hash结果来分配请求，使每个url定向到同一个后端服务器，后端服务器为缓存时比较有效。
upstream backend {  
  server squid1:3128;  
  server squid2:3128;  
  hash $request_uri;  
  hash_method crc32;  
}
```

## 4.Nginx配置(nginx.conf)

### 1. 基本格式

		main                                # 全局配置 main只是一个代表的意思;写的时候不用往上写main
		 
		events {                            # nginx工作模式配置
	
		}
		http {                              # http设置
		    server {                        # 服务器主机配置
		        listen       80;
	            server_name  localhost;
		        ....
		        location /static{                    # 路由配置
		           alias  /opt/img;
		           index  404.html
	             }
		 
		        location path {
		            ....
		        }
		 
		        location otherpath {
		            ....
		        }
		    }	 
		    upstream name {                    # 负载均衡配置
		        ....
		    }
		}
		https{} https配置

### 2. 区域作用
	main：用于进行nginx全局信息的配置
	events：用于nginx工作模式的配置
	http：用于进行http协议信息的一些配置
	server：用于进行服务器访问信息的配置
	location：用于进行访问路由的配置
	upstream：用于进行负载均衡的配置
### 3. Main区域

```properties
# user nobody nobody;  // 当前操作的用户
worker_processes 2;  // 工作线程=当前CPU的core
# error_log logs/error.log
# error_log logs/error.log notice
# error_log logs/error.log info
# pid logs/nginx.pid
worker_rlimit_nofile 1024;
```
### 4. events 模块

	event {
	    worker_connections 1024;
	    multi_accept on;
	    use epoll;
	}
	worker_connections 指定最大可以同时接收的连接数量,最大连接数 = worker_connections*worker_processes
	multi_accept 配置指定nginx在收到一个新连接通知后尽可能多的接受更多的连接
	use epoll 配置指定了线程轮询的方法，如果是linux2.6+，使用epoll，如果是BSD如Mac请使用Kqueue
### 5. http - server区域

#### 1. http 配置

配置一些媒体类型, 已经压缩方式 , 和超时 , 日志记录

#### 2. 静态资源代理

比如我们现在有一个需求就是 , 我们服务器上有一个图片库 , 比如在 `opt/static/img` 目录下面 ,我们要求我们访问`http://www.xxx.com/static/a.png` 可以代理到我们 `opt/static/img/a.png` , 可以这么配置

```java
server {
    listen       80;  // 监听的端口 ,浏览器在未指定端口的前提下, 默认访问的是80端口
    server_name  localhost; //指定ip地址或者域名，多个配置之间用空格分隔 manage.tyut.com

    #charset koi8-r;

    #access_log  logs/host.access.log  main;

    location /static {
      alias opt/static/img;
      index error.png;
    }
}
```

#### 3. 配置个别404 重定向

还是上诉的那个代理 .我们可以将它访问 `http://www.xxx.com/static` 下面所有为找到的资源,重定向到 `opt/static/img/404.html` , 

```java
location /static {
  alias /opt/test/img/;
  index error.png;
  error_page 404 404.html;
}
```

#### 4. 配置全局 404 

```shell
error_page  500 502 503 504  /50x.html;
location = /50x.html {
    root   html;
}
```

这个意思就是 , 我访问的错误码如果是 `500 502 503 504` 之类的, 会将请求转发到 `/50x.html`中 , 然后去当前目录下的 nginx/html文件中找 到 `50x.html`,  就是这么玩了

```java
[xxx html]# pwd
/opt/nginx/nginx-1.14.2/html
[xxx html]# ls
50x.html  index.html
```

比如 404 哇, 你也可以这么配置

```java
error_page  404  /404.html;
location = /404.html {
    root   html;
}
```

然后 `/opt/nginx/nginx-1.14.2/html` 中新建一个 404.html 文件



#### 5. 正则匹配

正则表达式中 `^`表示以什么开头, `$` 以什么结束 , `* `代表任意字符  `~` 区分大小写匹配

`~*` 不区分大小写匹配

`location ~*.(gif|jpg|jpeg)$` 匹配以.gif、.jpg 或 .jpeg 结尾的请求


	location ^~/static {
	      alias /opt/test/img;
	      index index.html;
	}


#### 6. 请求转发	

    location / {
        proxy_pass http://www.xxx.com;
        proxy_connect_timeout 1000;
        proxy_read_timeout 1000;
    }


​    
    upstream www.xxx.com{
      server 127.0.0.1:8050 weight=2;
      server 127.0.0.1:8060 weight=1;
    }

