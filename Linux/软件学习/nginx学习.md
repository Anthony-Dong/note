# nginx
### 1.概念:	
	nginx是一款自由的、开源的、高性能的HTTP服务器和反向代理服务器；同时也是一个IMAP、POP3、SMTP代理服务器；nginx可以作为一个HTTP服务器进行网站的发布处理，另外nginx可以作为反向代理进行负载均衡的实现。 我们将静态资源放到nginx中，动态资源转发到tomcat服务器中
	网站 http://nginx.org/
### 2.淘宝的Tengine
	某宝网站中大部分功能也是直接使用nginx进行反向代理实现的，并且通过封装nginx和其他的组件之后起了个高大上的名字：Tengine
	Tengine是由淘宝网发起的Web服务器项目。它在Nginx的基础上，针对大访问量网站的需求，添加了很多高级功能和特性。Tengine的性能和稳定性已经在大型的网站如淘宝网，天猫商城等得到了很好的检验。它的最终目标是打造一个高效、稳定、安全、易用的Web平台。从2011年12月开始，Tengine成为一个开源项目，Tengine团队在积极地开发和维护着它。Tengine团队的核心成员来自于淘宝、搜狗等互联网企业。Tengine是社区合作的成果，我们欢迎大家参与其中，贡献自己的力量。
	网站 http://tengine.taobao.org/
### 3.对代理服务器而言的正向代理与反向代理
#### 1.正向代理
![image](https://imgconvert.csdnimg.cn/aHR0cHM6Ly91cGxvYWQtaW1hZ2VzLmppYW5zaHUuaW8vdXBsb2FkX2ltYWdlcy82MTUyNTk1LWQ3OGJiOWE3NzZiYmUxMmIucG5nP2ltYWdlTW9ncjIvYXV0by1vcmllbnQvc3RyaXAlN0NpbWFnZVZpZXcyLzIvdy8xMjQw)

	客户端想要访问一个服务器，但是它可能无法直接访问这台服务器，这时候这可找一台可以访问目标服务器的另外一台服务器，而这台服务器就被当做是代理人的角色 ，称之为代理服务器，于是客户端把请求发给代理服务器，由代理服务器获得目标服务器的数据并返回给客户端。客户端是清楚目标服务器的地址的，而目标服务器是不清楚来自客户端，它只知道来自哪个代理服务器，所以正向代理可以屏蔽或隐藏客户端的信息。
![image](https://imgconvert.csdnimg.cn/aHR0cHM6Ly91cGxvYWQtaW1hZ2VzLmppYW5zaHUuaW8vdXBsb2FkX2ltYWdlcy82MTUyNTk1LWYxZTdlZTA5MDdiZjJhMTUucG5nP2ltYWdlTW9ncjIvYXV0by1vcmllbnQvc3RyaXAlN0NpbWFnZVZpZXcyLzIvdy8xMjQw)
#### 2.反向代理
	从上面的正向代理，你会大概知道代理服务器是为客户端作代理人，它是站在客户端这边的。
	其实反向代理就是代理服务器为服务器作代理人，站在服务器这边，它就是对外屏蔽了服务器的信息，常用的场景就是多台服务器分布式部署，像一些大的网站，由于访问人数很多，就需要多台服务器来解决人数多的问题，这时这些服务器就由一个反向代理服务器来代理，客户端发来请求，先由反向代理服务器，然后按一定的规则分发到明确的服务器，而客户端不知道是哪台服务器。常常用nginx来作反向代理。
![image](https://imgconvert.csdnimg.cn/aHR0cHM6Ly91cGxvYWQtaW1hZ2VzLmppYW5zaHUuaW8vdXBsb2FkX2ltYWdlcy82MTUyNTk1LWYxZTdlZTA5MDdiZjJhMTUucG5nP2ltYWdlTW9ncjIvYXV0by1vcmllbnQvc3RyaXAlN0NpbWFnZVZpZXcyLzIvdy8xMjQw)
### 4.负载均衡的实现
	nginx扮演了反向代理服务器的角色，它是以依据什么样的规则进行请求分发的呢？不用的项目应用场景，分发的规则是否可以控制呢？这里提到的客户端发送的、nginx反向代理服务器接收到的请求数量，就是我们说的负载量.请求数量按照一定的规则进行分发到不同的服务器处理的规则，就是一种均衡规则.所以~将服务器接收到的请求按照规则分发的过程，称为负载均衡。

	负载均衡在实际项目操作过程中，有硬件负载均衡和软件负载均衡两种，硬件负载均衡也称为硬负载，如F5负载均衡，相对造价昂贵成本较高，但是数据的稳定性安全性等等有非常好的保障，如中国移动中国联通这样的公司才会选择硬负载进行操作；更多的公司考虑到成本原因，会选择使用软件负载均衡，软件负载均衡是利用现有的技术结合主机硬件实现的一种消息队列分发机制
#### nginx的负载均衡调度算法
	![image](https://img-blog.csdn.net/20180425100012770?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L21vYWt1bg==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)
	一般默认采用的轮询方式;但是也可以设置为ip_hash
	1.轮询（默认）每个请求按时间顺序逐一分配到不同的后端服务器，如果后端服务器down掉，能自动剔除。
	2.weight 指定轮询几率，weight和访问比率成正比，用于后端服务器性能不均的情况。
		upstream bakend {  
		  server 192.168.0.14 weight=10;  
		  server 192.168.0.15 weight=10;  
		}	
	3.ip_hash  每个请求按访问ip的hash结果分配，这样每个访客固定访问一个后端服务器，可以解决session的问题。
		upstream bakend {  
		  ip_hash;  
		  server 192.168.0.14:88;  
		  server 192.168.0.15:80;  
		}
	4.fair 按后端服务器的响应时间来分配请求，响应时间短的优先分配。
		upstream backend {  
		fair;  
		  server server1;  
		  server server2;  	  
		}
	5. url_hash 按访问url的hash结果来分配请求，使每个url定向到同一个后端服务器，后端服务器为缓存时比较有效。
		upstream backend {  
		  server squid1:3128;  
		  server squid2:3128;  
		  hash $request_uri;  
		  hash_method crc32;  
		}

### 5.nginx配置(nginx.conf)
		main                                # 全局配置 main只是一个代表的意思;写的时候不用往上写main
		 
		events {                            # nginx工作模式配置

		}
		http {                                # http设置
		    ....		 
		    server {                        # 服务器主机配置
		        ....
		        location {                    # 路由配置
		            ....
		        }
		 
		        location path {
		            ....
		        }
		 
		        location otherpath {
		            ....
		        }
		    }	 
		    server {
		        ....
		        location {
		            ....
		        }
		    }		 
		    upstream name {                    # 负载均衡配置
		        ....
		    }
		}

#### 1.主要信息
	main：用于进行nginx全局信息的配置
	events：用于nginx工作模式的配置
	http：用于进行http协议信息的一些配置
	server：用于进行服务器访问信息的配置
	location：用于进行访问路由的配置
	upstream：用于进行负载均衡的配置
##### 1.1 main模块 其实不用加main 直接在最上面写就行
	# user nobody nobody;
	worker_processes 2;
	# error_log logs/error.log
	# error_log logs/error.log notice
	# error_log logs/error.log info
	# pid logs/nginx.pid
	worker_rlimit_nofile 1024;
	上述配置都是存放在main全局配置模块中的配置项

	user用来指定nginx worker进程运行用户以及用户组，默认nobody账号运行
	worker_processes指定nginx要开启的子进程数量，运行过程中监控每个进程消耗内存(一般几M~几十M不等)根据实际情况进行调整，通常数量是CPU内核数量的整数倍
	error_log定义错误日志文件的位置及输出级别【debug / info / notice / warn / error / crit】
	pid用来指定进程id的存储文件的位置
	worker_rlimit_nofile用于指定一个进程可以打开最多文件数量的描述
##### 1.2 events 模块  针对nginx服务器的工作模式的一些操作配置
	
	event {
	    worker_connections 1024;
	    multi_accept on;
	    use epoll;
	}
	worker_connections 指定最大可以同时接收的连接数量，这里一定要注意，最大连接数量是和worker processes共同决定的。
	multi_accept 配置指定nginx在收到一个新连接通知后尽可能多的接受更多的连接
	use epoll 配置指定了线程轮询的方法，如果是linux2.6+，使用epoll，如果是BSD如Mac请使用Kqueue
###### epoll
	IO多路复用(IO multiplexing)的三种机制Select，Poll，Epoll
	epoll是Linux目前大规模网络并发程序开发的首选模型。，是基于事件驱动的I/O方式,在绝大多数情况下性能远超select和poll。目前流行的高性能web服务器Nginx正式依赖于epoll提供的高效网络套接字轮询服务。但是，在并发连接不高的情况下，多线程+阻塞I/O方式可能性能更好。


#### 1.3 http模块 作为web服务器，http模块是nginx最核心的一个模块，配置项也是比较多的，项目中会设置到很多的实际业务场景，需要根据硬件信息进行适当的配置，常规情况下，使用默认配置即可！	

##### 1.4 server模块 location模块
		server模块 使我们使用的最核心的模块


		listen: 监听的端口 一般是80 
		server：一个虚拟主机的配置，一个http中可以配置多个server
		server_name：用力啊指定ip地址或者域名，多个配置之间用空格分隔 manage.tyut.com

		localhost / 路径是 =root 配置的路径  /后面的东西不算
		所以 文件路径在 /opt/static/etc  访问 http://localhost/etc 就可以访问到了
		location /etc{
			root /opt/static
			index  index.html
		}

	     server {
	        listen       80;
	        server_name  image.leyou.com;

	    	# 监听域名中带有group的，交给FastDFS模块处理
	        location ~/group([0-9])/ {
	            ngx_fastdfs_module;
	        }		
			location / {
		          root  /home/leyou/static/;
		    }

			<!-- 可以配置多个location  -->
		   	
	        <!-- 指定错误错误状态码  50x的状态码 跳转路径;也是可以配置多个 -->
	        error_page   500 502 503 504  /50x.html;
	        location = /50x.html {
	            root   html;
	        }
	        
	    }

			https的传输协议 配置; https默认端口是443
			需要购买整数实现;配置  运维配置的东西
			 # HTTPS server
		    #
		    #server {
		    #    listen       443 ssl;
		    #    server_name  localhost;

		    #    ssl_certificate      cert.pem;
		    #    ssl_certificate_key  cert.key;

		    #    ssl_session_cache    shared:SSL:1m;
		    #    ssl_session_timeout  5m;

		    #    ssl_ciphers  HIGH:!aNULL:!MD5;
		    #    ssl_prefer_server_ciphers  on;

		    #    location / {
		    #        root   html;
		    #        index  index.html index.htm;
		    #    }
		    #}


#### 配置示例

#####1.网络部署		
		server {
        listen       80;
        server_name  www.leyou.com;

        proxy_set_header X-Forwarded-Host $host;
        proxy_set_header X-Forwarded-Server $host;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;

		location /item {
        		if (!-f $request_filename) { 
            		proxy_pass http://127.0.0.1:8071;
            		break;
        		}
			}
	        location / {
				proxy_pass http://www.leyou.com;
				proxy_connect_timeout 600;
				proxy_read_timeout 600;
	        }
	    }
	
		upstream www.leyou.com{
			server 127.0.0.1:8050 weight=2;
			server 127.0.0.1:8060 weight=1;
		}

##### 静态资源部署
		   server{
				listen  80;
				server_name   www.hello.com;
			                
			    location / {
				     root  /leyou/static/protal;
				     index  index.html;
				}
	     }
###### nginx多台部署记得 他们监控的端口号不能重复;不然会出错误


		upstream name {
		    ip_hash;
		    server 192.168.1.100:8000 backup;  表示的是 你懂得 备胎;所有人gg了 才有用它		
		    server 192.168.1.100:8001 down;  down表示down机了 不参加与负载均衡
		    server 192.168.1.100:8002 max_fails=3;
		    server 192.168.1.100:8003 fail_timeout=20s;
		    server 192.168.1.100:8004 max_fails=3 fail_timeout=20s;
		    server 127.0.0.1:7070 backup;
		}

		upstream的name属性  被proxy_pass所指向,还有proxy记住一定要加http://   最后面不用跟/
		server host:port：分发服务器的列表配置
		ip_hash：指定请求调度算法，默认是weight权重轮询调度，可以指定
		-- down：表示该主机暂停服务
		-- max_fails：表示失败最大次数，超过失败最大次数暂停服务
		-- fail_timeout：表示如果请求受理失败，暂停指定的时间之后重新发起请求
		backup： 其它所有的非backup机器down或者忙的时候，请求backup机器。所以这台机器压力会最轻。


### 操作示范
	
	安装 tar -zxvf 包
	进入主目录 cd/nginx   进行./configure --prefix=/usr/local/nginx  前面是安装路径;这个直接放在bin目录下 你可以直接nginx启动

	执行 make && make install

	启动 nginx   /usr/local/nginx/sbin/nginx -c /usr/local/nginx/conf/nginx.conf -t  先进行检测

	查看 nginx进程 ps -ef | grep nginx
	[root@darian conf]# ps -ef | grep nginx
	root      29914      1  0 20:44 ?        00:00:00 nginx: master process nginx -c /opt/nginx/conf/nginx.conf
	nobody    29915  29914  0 20:44 ?        00:00:01 nginx: worker process
	root      29917      1  0 20:44 ?        00:00:00 nginx: master process nginx -c /opt/nginx/conf/nginx2.conf
	nobody    29918  29917  0 20:44 ?        00:00:00 nginx: worker process
	root      30223  28421  0 20:50 pts/0    00:00:00 grep --color=auto nginx
	其中master是主进程;后面的worker是他开启的子线程;所以我们用quit不会关闭已有连接的服务

	关闭 nginx 
	优雅的关闭 kill -QUIT 29917  (主pid master进程;不是worker进程)
	快速关闭 kill -TERM 29914  

	其实 你也可以 nginx -s stop  nginx -s reload




















		http {
		    ##
		    # 基础配置
		    ##
		 	
				include       mime.types;  在conf目录下的mime.types
			    default_type  application/octet-stream;    默认是 流类型 

			    #log_format  main  '$remote_addr - $remote_user [$time_local] "$request" '
			    #                  '$status $body_bytes_sent "$http_referer" '
			    #                  '"$http_user_agent" "$http_x_forwarded_for"';

			    #access_log  logs/access.log  main;  存放日止文件的位置

			    sendfile        on;  开启高效文件传输
			    #tcp_nopush     on;		防止网络阻塞

			    #keepalive_timeout  0;
			    keepalive_timeout  65;

			    #gzip  on;  开启压缩配置



		    sendfile on; 配置on让sendfile发挥作用，将文件的回写过程交给数据缓冲去去完成，而不是放在应用中完成，这样的话在性能提升有有好处
		    tcp_nopush on; 让nginx在一个数据包中发送所有的头文件，而不是一个一个单独发
		    tcp_nodelay on; 让nginx不要缓存数据，而是一段一段发送，如果数据的传输有实时性的要求的话可以配置它，发送完一小段数据就立刻能得到返回值，但是不要滥用哦
		    keepalive_timeout 65; 给客户端分配连接超时时间，服务器会在这个时间过后关闭连接。一般设置时间较短，可以让nginx工作持续性更好;默认是75S
		    client_header_timeout 10：设置请求头的超时时间
			client_body_timeout 10:设置请求体的超时时间
			send_timeout 10：指定客户端响应超时时间，如果客户端两次操作间隔超过这个时间，服务器就会关闭这个链接
			types_hash_max_size 2048; 
			# server_tokens off;
		 
		    # server_names_hash_bucket_size 64;
		    # server_name_in_redirect off;
		 
		    include /etc/nginx/mime.types;
		    default_type application/octet-stream;
		 

		}
