### Linux
    linux直接yum -y install httpd-tools，然后ab -V测试
### Windows
    1查看80端口有没有被占用，netstat -ano | findstr "80"
    2下载地址https://www.apachehaus.com/cgi-bin/download.plx
    3解压后，找到安装目录下的httpd.conf，修改为自己的安装目录 
    4 启动服务  
    5 命令示例(并发200，一共1000个请求)
    D:\apache24\bin>ab -c 200 -n 1000 http:nginx负载均衡/压力方法
    6 测试结果
    研究redisson的分布式锁在并发下的效果

Ab命令参数
-n即requests，用于指定压力测试总共的执行次数。
-c即concurrency，用于指定压力测试的并发数。
-t即timelimit，等待响应的最大时间(单位：秒)。
-b即windowsize，TCP发送/接收的缓冲大小(单位：字节)。
-p即postfile，发送POST请求时需要上传的文件，此外还必须设置-T参数。
-u即putfile，发送PUT请求时需要上传的文件，此外还必须设置-T参数。
-T即content-type，用于设置Content-Type请求头信息，例如：application/x-www-form-urlencoded，默认值为text/plain。
-v即verbosity，指定打印帮助信息的冗余级别。
-w以HTML表格形式打印结果。
-i使用HEAD请求代替GET请求。
-x插入字符串作为table标签的属性。
-y插入字符串作为tr标签的属性。
-z插入字符串作为td标签的属性。
-C添加cookie信息，例如："Apache=1234"(可以重复该参数选项以添加多个)。
-H添加任意的请求头，例如："Accept-Encoding: gzip"，请求头将会添加在现有的多个请求头之后(可以重复该参数选项以添加多个)。
-A添加一个基本的网络认证信息，用户名和密码之间用英文冒号隔开。
-P添加一个基本的代理认证信息，用户名和密码之间用英文冒号隔开。
-X指定使用的代理服务器和端口号，例如:"126.10.10.3:88"。
-V打印版本号并退出。
-k使用HTTP的KeepAlive特性。
-d不显示百分比。
-S不显示预估和警告信息。
-g输出结果信息到gnuplot格式的文件中。
-e输出结果信息到CSV格式的文件中。
-r指定接收到错误信息时不退出程序。
-h显示用法信息，其实就是ab -help。

测试
测试代码
1 新建一个测试redisson控制层，加入测试代码
@RequestMapping("lockTest")
@ResponseBody
public String lockTest(){
    Jedis jedis = redisUtil.getJedis();// redis链接
    RLock lock = redissonClient.getLock("redis-lock");//分布锁
    //加锁
    lock.lock();
    try {
        String v = jedis.get("k");//获取value
        System.err.print("==>"+v);//打印value
        if(StringUtil.isBlank(v)){
            v = "1";
        }
        int inum = Integer.parseInt(v);//获得value的值
        jedis.set("k", inum+1+"");//value增加1
        jedis.close();
    } finally {
        lock.unlock();
    }
    return "success";
}

2 启动n个测试redisson控制层的微服务
 

3 配置nginx的负载均衡到测试redisson控制层的微服务
A配置nginx的upstream
	upstream redisTest {
		server   127.0.0.1:8071 weight=3;
		server   127.0.0.1:8072  weight=3;
		server   127.0.0.1:8073  weight=3;
	}
B配置nginx的默认代理地址
        location / {
             #root D:/software/upload;	
			 proxy_pass http://redisTest;
             index  index.html index.htm; 
        }

4 用apache的ab压力测试命令访问nginx来压测分布式的redisson控制层的微服务
同时观察redis的被并发的k的增长情况

