# linux命令学习

> 如果你还没有linux系统 - > 阿里云购买 ,或者 自己装vm 安装虚拟机 -> 教程我下面有(我只用 centos 7)

### linux良好入门第一步,放弃root用户

一位优秀的linux用户  应该不去使用 root用户 

如何创建一个用户组 `groupadd -g 101 admin `    ,删除就是`groupdel admin`   用户也是

如何去新建一个 用户  `useradd  admin -s /bin/bash -g admin` 

如何修改密码 `passwd admin`

切换用户呢 :  `su - admin`

如何添加sudo权限`  vim /etc/sudoers `  添加  `找到这一行："root ALL=(ALL) ALL"在起下面添加"xxx ALL=(ALL) ALL"(这里的xxx是你的用户名)，然后保存退出。`  

此时 愉快的开启了 非root用户

如何 修改 用户权限文件所有权 `chown `       修改文件执行权限 `chmod  `    修改用户 `usermod` 权限

`chown  -R admin:admin /home/admin`  意思就是将 `/home/admin/`下文件的所有权给 admin用户,admin组

`chmod +x  test.sh`  就是给admin用户 加执行操作  `xwr` 执行 写 读操作 ,`chmod 777 test.sh` 是 用户 ,用户组,其他用户都有`xwr`权限  , 7 的含义是 :  x 1  w 2  r 4  加起来的 就是7 

`usermod admin -s /bin/bash -g admin`    就是给 admin用户的shell 改成 /bin/bash  用户组改成admin

### shutdown 关机

```shell
shutdown(选项)(参数)

-r：shutdown之后重新启动；
-h：将系统关机；
-n：不调用init程序进行关机，而由shutdown自己进行；

指定现在立即关机: 
shutdown -h now
```

#### echo 
	">"意思为重定向，会清除文件时所有以前数据，
  	">>"为内容追加，是在文件尾部追加需要的内容，还有一个较复杂不常用的类似命令printf.
  	-n	不换行输出内容
  	-e	解析转义字符
  	\n	换行 \r	回车 \t	制表符（tab） \b	退格 \v	纵向制表符
 	例如   `echo 'hello world' >> hello.txt `

#### cat 
	cat hello.txt
	查看文件;不进行编辑
	
	文件输入
	cat  <<EOF >>1.txt
	>1
	>2
	>3
	>EOF
	或者 都可以
	cat >>1.txt  <<EOF 
	>1
	>2
	>3
	>EOF


​	文件拷贝
​	`cat pyhton.py >> test.sh`

### tail
	tail -n 3 hello.txt
	显示最后三行
	
	持续显示 
	tail  -F 文件
### tar
```shell
 tar zxf git-2.5.0.tar.gz  没有具体信息
 tar zxvf git-2.5.0.tar.gz 有具体信息  
 -v 显示过程
```
### mv
	可以实现 移动和重命名
	mv before.txt after.txt
	将/a目录移动到/b下，并重命名为c
	mv /a /b/c
### rm
	删除命令
	rm 文件名 （功能：删除文件） 会提示
	rm *   （功能：删除该路径下的所有文件）
	rm -f  文件名 （功能：删除文件，不再询问是否删除）
	rm -r 文件夹 （功能：递归删除文件夹）
	rm -rf 文件或者目录 （功能：删除文件获得递归删除目录，最常用
### df 命令 查看 硬盘使用情况

```shell
df  -h 
[admin@localhost apollo]$ df  -h
Filesystem               Size  Used Avail Use% Mounted on
/dev/mapper/centos-root   18G  2.9G   15G  17% /
devtmpfs                 908M     0  908M   0% /dev
tmpfs                    914M     0  914M   0% /dev/shm
tmpfs                    914M   17M  897M   2% /run
tmpfs                    914M     0  914M   0% /sys/fs/cgroup
/dev/sda1                497M   98M  400M  20% /boot
```

### 多屏显示控制

	空格 向下翻页
	b 向上翻页
	q 退出
### vim编辑器  / vi 编辑器
```+
:wq 保存并且退出
:set nu 显示行号
/condition 查询
CTL + b :上翻
CTL + f : 下翻
dd 删除

shift+insert   粘贴
ctrl+insert   复制

撤销操作 esc  u
```
### cp 文件拷贝
	cp命令可以拷贝一个文件和目录到指定的目录下。也可以一次复制多个文件，最后一个参数为目标目录。
	源文件：被拷贝的文件或目录。如果是目录的话，需要使用 -r 选项，多个源文件，要用空格分割。
	
	cp -r /opt/source  /opt/destination
### scp 远程文件拷贝

```shell
cp -r 主机ip:/opt/source  远程主机ip:/opt/destination

将  远程source 拷贝 到  远程destination
此时需要输入密码输入正确的密码就行了  两个是远程主机 
```

### rsync 远程文件拷贝

```shell
           源					  目的地
// 将IP(192.168.58.131)用户为admin的/opt/source文件夹  复制到本地的/opt/destination    
rsycn -av admin@192.168.58.131:/opt/source    /opt/destination

// 将本地的复制到远程
rsycn -av /opt/source    admin@192.168.58.131:/opt/destination

-a 归档拷贝  -v显示执行过程
将 远程source 拷贝到 本地的destination , 不能拷贝俩远程.
```

### ln 

```shell
1. 软连接
ln [参数][源文件或目录][目标文件或目录]

给文件创建软链接，为log2013.log文件创建软链接link2013，如果log2013.log丢失，link2013将失效：
ln -s log2013.log link2013

结果是俩文件

删除直接 就是 rm link2013 -f

2. 硬链接文件除了文件名与源文件不一样，其余所有信息都是一样的。类似于 cp 复制操作。但是又和复制不一样，硬链接可以同步更新。
ln log2013.log ln2013
结果是三个文件

`不允许将硬链接指向目录；不允许跨分区创建硬链接`
```



### yum 命令 (Ubuntu 是 opt)

```shell
yum(选项)(参数)

1. 选项 : 
-y：对所有的提问都回答“yes”；
-c：指定配置文件；

2.参数 
install：安装rpm软件包；
update：更新rpm软件包；
remove：删除指定的rpm软件包;
list：显示软件包的信息；
clean：清理yum过期的缓存

3.设置yum源为阿里云  个人习惯
sudo yum-config-manager --add-repo http://mirrors.aliyun.com/docker-ce/linux/centos/docker-ce.repo
```



### find 查找文件名

```shell
查找名字 以 env.sh 结尾的字符
find /opt/bigdata/hadoop-2.7.7/etc/hadoop -name '*env.sh'

```

###  文件使用习惯
	opt/fastdfs 数据存储目录
	usr/local/fdfs 启动文件目录
	etc/fdfs 配置文件目录
	usr/bin/fdfs_trackerd 启动配置
	etc/init.d/fdfs_trackerd 启动服务脚本
	usr/include/fads   插件目录

### jdk8配置

下载地址 我提供了 : 网盘链接  `链接: https://pan.baidu.com/s/1WV7xDa2UN6pOXVnLiSjP-g 提取码: 7yhk 复制这段内容后打开百度网盘手机App，操作更方便哦` 

	1.tar -zxvf jdk-8u171-linux-x64.tar.gz
	2.vi /etc/profile
	文件最后添加  	
	export JAVA_HOME=/home/elk1/jdk1.8/jdk1.8.0_171
	export PATH=:$PATH:$JAVA_HOME/bin
	export JRE_HOME=$JAVA_HOME/jre
	export PATH=$PATH:$JRE_HOME/bin
	export CLASSPATH=.:$JAVA_HOME/lib:$JRE_HOME/lib:$CLASSPATH
	
	日常开发 前两行就行了 ------- 没必要jre 和 classpath
					
	3.source /etc/profile
	4.java -version

###  linux 常见问题 换行符 
	linux执行sh报错：$’\r’: 未找到命令的解决
	背景
	　　执行.sh脚本时出现$’\r’: 未找到命令，
	原因
	　　是因为命令直接从windows 复制过来导致的
	解决
	　　yum install dos2unix
	　　dos2unix **.sh 进行转换
	　　再次执行即可
### linux 安装 ifconfig 

```shell
yum install net-tools.x86_64

最小化安装啥都没有
```

### 安装一台linux主机

> centos7 的镜像版 http://mirrors.aliyun.com/centos/7/isos/x86_64/
>
> 最小版本下载地址	http://mirrors.aliyun.com/centos/7/isos/x86_64/CentOS-7-x86_64-Minimal-1908.iso

> 下载 mini版本的 centos  别说为啥 , 因为我们是开发,没必要安装太多没用的 , 安装直接百度或者无脑next

> 修改静态 ip -> 别问为什么 -> 因为我们需要远程连接

```shell
1、更改 ip地址 -> 前提是你设置了nat网络(就是虚拟网卡)
cd /etc/sysconfig/network-scripts/
vim ifcfg-eno16777736

BOOTPROTO=static #这里讲dhcp换成ststic
ONBOOT=yes #将no换成yes
其中 gateway & ipaddress 都不能重复 
IPADDR=192.168.58.129
NETMASK=255.255.255.0
GATEWAY=192.168.58.2
// 这个可以不填写
DNS1=192.168.1.1

保存退出  esc  :wq 退出
重启网络
service network restart

修改机器名 -> 默认localhost
vim /etc/hostname
```

> ​	**安装 shell远程连接 工具**  百度网盘连接 `链接: https://pan.baidu.com/s/1PTdQzhu3mNSpNTs39LVnHg 提取码: p92d 复制这段内容后打开百度网盘手机App，操作更方便哦`

> 安装 **网络工具**
>
> ```
> yum  -y：对所有的提问都回答“yes”；
> ```
>
> `yum install net-tools.x86_64`

> ​	安装 **vim**  编辑器
>
> `yum  install  vim`

> ​	安装**OpenSSL 开发包**
>
> `sudo yum install openssl-devel`

> ​	安装 **gcc**
>
> `sudo yum install gcc`     
>
> 

> ​	**安装 git** 需要 `yum -y install curl-devel expat-devel gettext-devel openssl-devel zlib-devel gcc perl-ExtUtils-MakeMaker` 
>
> 执行  `sudo make prefix=/usr/local/git all`
>
> `sudo make prefix=/usr/local/git install`



> 学会 **配置** `/etc/profile`  
>
> ```shell
> export JAVA_HOME=安装路径
> export PATH=$PATH:$JAVA_HOME/bin
> 
> 一般软件都是这个 ,然后 
>  source /etc/profile
> 实现生效 
> ```
>
> 



### 如何 克隆虚拟机  ->前提是你有一台完整的虚拟机

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-22/959ff2ef-8b7a-4235-9f31-f043bd097367.png?x-oss-process=style/template01)