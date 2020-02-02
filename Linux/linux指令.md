# Linux进阶版学习

## 1. 用户权限学习

​	为什么要用户,因为实际开发的时候你不可能是root用户,所以需要进行权限学习,不能执行就 sudo 哈哈哈

- 新建用户 `useradd testuser `

- 创建密码 `passwd testuser`

- 删除用户  `userdel testuser`

- 切换用户 `su - user`

- 添加文件权限  `chown -R user /home/user`

- 高级版 `useradd -s /bin/sh -d /home/gem -g group –G adm,root gem`  

  ​	此命令新建了一个用户gem，该用户的登录Shell是/bin/sh，他的用户主目录在 /home/gem,它属于group用户组，同时又属于adm和root用户组，其中group用户组是其主组,

- 修改用户 `usermod -s /bin/ksh -d /home/z –g developer sam` 

  ​	此命令将用户sam的登录Shell修改为ksh，主目录改为/home/z，用户组改为developer。

- 增加组 `groupadd -g 101 group2`

  ​	此命令向系统中增加了一个新组group2，同时指定新组的组标识号是101。

- 删除组 `groupdel group1`

  ​	此命令从系统中删除组group1.

- 修改组 `groupmod -g 102 group2`

  ​	此命令将组group2的组标识号修改为102。

- 让普通用户添加sudo权限 

  ```shell
  vim /etc/sudoers
  
  找到这一行："root ALL=(ALL) ALL"在起下面添加"xxx ALL=(ALL) ALL"(这里的xxx是你的用户名)，然后保存退出。
  ```
  
  ​	

1. 给用户添加文件权限

   - ```shell
     chown -R user /home/user/   		递归的方式给 /home/user 目录下的全部给 user用户
     ```

2. 查看文件用户权限

   - ```shell
     [root@iz2zegua78a74kqn8clokfz user]# ll
     total 28388
     drwxr-xr-x 9 user user     4096 Aug 27 15:26 elasticsearch
     -rw-r--r-- 1 user root 29056810 Aug 27 15:00 elasticsearch-6.2.4.tar.gz
     -rw-r--r-- 1 user root      291 Sep  1 17:57 null
     drwxr-xr-x 2 user root     4096 Sep  1 19:07 script
     
     –       rw-     r–    r–    9          user                user      4096   
     -文件 d目录 l软连接
     普通文件 用户权限 组权限 其他用户 表示文件个数 该文件或目录的拥有者  表示所属的组   文件大小 
     ```

3. 确定了一个文件的访问权限后，用户可以利用Linux系统提供的chmod命令来重新设定不同的访问权限。也可以利用chown命令来更改某个文件或目录的所有者。利用chgrp命令来更改某个文件或目录的用户组。

   >  1. chmod命令是非常重要的，用于改变文件或目录的访问权限。用户用它控制文件或目录的访问权限。
   >
   >   ```shell
   >   chmod ［who］ ［+ | – | =］ ［mode］ 文件名
   >   
   >   mode 有 x w r 
   >   其中  x 对应的是 1
   >   	  w 对应的是 2
   >   	  r 对应的是 4
   >   	  
   >  所以  chmod 777 ./test.sh   就是 所有用户都可以进行读写操作
   >  	 chmod 754 是 用户是 xwr ,组是 xr , 其他用户是 r
   >  ```
   >  
   >   
   >
   >  2. chown 命令  功能：更改某个文件或目录的属主和属组。
   >
   >  ```shell
   >chown -R user：group  filename    -R是 递归处理 -v显示执行过程
   >   ```
   >   
   >   
   >

## 2. top 命令学习

1. top 命令界面展示

   ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-03/50f47a7f-b4d8-4a28-84b3-2aab183bcf4e.png?x-oss-process=style/template01)

2. 界面 描述

   >1.  task  是进程数
   >
   >  1. total　　 　　 进程总数
   >  2. running 　　 正在运行的进程数
   >  3. sleeping 　　睡眠的进程数
   >  4. stopped 　　停止的进程数
   >  5. zombie 　　  僵尸进程数
   >
   >2.  CPU 是 CPU 的使用情况  
   >
   >  1. id 是指 空闲的 内存百分比
   >  2. us 是指 用户百分比
   >  3. sy 是指 内核百分比
   >
   >3. Mem  是内存使用情况 比如说 1883724 可以以三位分隔开 1,883,724, 就是 1.8个G 最小单位是KB
   >
   >  1. total 是全部 物理内存
   >  2. free 是空闲物理内存
   >  3. used 是指 使用的物理内存
   >  4. buffers：  用作内核缓存的内存量
   >
   >4. swap 是指的是一个交换分区或文件。在Linux上可以使用swapon -s命令查看当前系统上正在使用的交换空间
   >
   >  1. 我没搞清楚这个是啥
   >
   >***
   >
   >5. 开始 进程 详情信息
   >
   >  1.  pid  指的是 进程id
   >  2. ppid 是 父进程id
   >  3. USER 进程所有者用户
   >  4. PR 指的是优先级
   >  5. NI 指的是 nice   负值表示高优先级，正值表示低优先级
   >  6. VIRT 进程使用的虚拟内存总量，单位kb。VIRT=SWAP+RES(其实就是占用地址空间的大小;就是数据大小)
   >     1. 现代操作系统里面分配虚拟地址空间操作不同于分配物理内存。在64位操作系统上，可用的最大虚拟地址空间有16EB，即大概180亿GB。那么在一台只有16G的物理内存的机器上，我也能要求获得4TB的地址空间以备将来使用。
   >     2. VIRT高是因为分配了太多地址空间导致。一般来说不用太在意VIRT太高，因为你有16EB的空间可以使用。如果你实在需要控制VIRT的使用，设置环境变量MALLOC_ARENA_MAX，例如hadoop推荐值为4，因为YARN使用VIRT值监控资源使用。
   >  7.  SWAP  进程使用的虚拟内存中，被换出的大小，单位kb
   >  8.  RES  进程使用的、未被换出的物理内存大小，单位kb。RES=CODE+DATA  (其实就是 物理内存大小)  我们主要看这个
   >  9. COMMAND  命令名/命令行
   >  10. S  进程状态。D=不可中断的睡眠状态 R=运行 S=睡眠 T=跟踪/停止 Z=僵尸进程
   >  11. TIME+    进程使用的CPU时间总计，单位1/100秒
   >
   >6. 相关 top 界面指令(在 top界面 敲 这些 字母就行)
   >
   >  1.  "1" 显示 每个 CPU 状况
   >
   >  2. "b"  打开和关闭 高亮效果
   >
   >  3. "x"  打开自己设定的排序 高亮显示 (或关闭)
   >
   >  4. 可以看到现在是按"%MEM"进行排序的，可以按”shift+>”或者”shift+<”左右改变排序序列。
   >
   >  5. "q" 退出top程序
   >
   >  7. M:根据驻留内存大小进行排序。
   >
   >  8. P 根据 cpu排序 和 shift> < 一样
   >
   >  9. T 根据 时间累加排序
   >
   >  10. "f"  进入 另外一个界面
   >
   >  11. "k" 终止一个进程。
   >
   >****
   >
   >
   >
   >7. 以及  top 的相关参数
   >
   >   "-d"  更新周期  单位S  默认是5S刷新一次  例如 top -d 10  表示 10S刷新一次
   >
   >   "-n"  表示更次次数  top -n 2 表示 只更新两次
   >
   >   "-p"  通过指定监控进程ID来仅仅监控某个进程的状态。
   >
   >   "-c"  显示整个命令行而不只是显示命令名   比如说我查看 top -p 6379 -c 就显示 了上面的cpu等信息
   >
>   q  该选项将使top没有任何延迟的进行刷新。如果调用程序有超级用户权限，那么top将以尽可能高的优先级运行。 
   >
>   S  指定累计模式 
   >
>   s  使top命令在安全模式中运行。这将去除交互命令所带来的潜在危险。 
   >
>   i  使top不显示任何闲置或者僵死进程。 
   >

   

   ## 3. free 命令学习

   1. 命令界面
   
      ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-03/60ff85ee-4785-491d-a2c3-a7dc7640cbb5.png?x-oss-process=style/template01       )
   
   2. 命令参数
   
   3. 很简单  只需要 记住  

      1. "-m"  以 M 为单位显示
   2. "-k"  是以KB为单位显示  其他的  b  g 都是一样不展示了
      
      3. "-t"  显示的更细
   4. "-s "  跟S
      
      5. "-h" 显示 单位
   6. swap  就是 虚拟内存
      
      7. men  是物理内存
   8. 可用内存=used  +  free + buff/cache  这三个之和





## 4.ps命令与 ps -ef   以及 ps -aux 命令pstree 和 ps -axjf



1. ps-ef  用标准的格式显示
   
      ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-03/3689d15b-e530-4bda-8268-3c7858e6777d.png?x-oss-process=style/template01)
   
   2. ps-aux 是用BSD的格式来显示 
   
   3. 我们只用关注的是 RSS  他是 比如说 2788 他其实是 2,788 单位为KB  实际大小是 2.7M
   
   4. 以及  pid 进程号
   
   5. TTY ：该 process 是在那個终端上面运作
   
   
   
   ## 5. netstat 学习 查看服务及监听端口
   
   >```shell
   >netstat命令各个参数说明如下：
   >
   >　　-t : 指明显示TCP端口
   >
   >　　-u : 指明显示UDP端口
   >
   >　　-l : 仅显示监听套接字(所谓套接字就是使应用程序能够读写与收发通讯协议(protocol)与资料的程序),仅列出有在 Listen (监听) 的服務状态
   >
   >　　-p : 显示进程标识符和程序名称，每一个套接字/端口都属于一个程序。
   >
   >　　-n : 不进行DNS轮询(可以加速操作)
   >　　-a :(all)显示所有选项， netstat默认不显示LISTEN相关
   >　
   >```
   
   ```shell
   
   netstat -nlp |grep LISTEN   //查看当前所有监听端口·(比较常用)
   
   netstat -nlp |grep 80   //查看所有80端口使用情况·
   
   netstat -an | grep 3306   //查看所有3306端口使用情况·
   
   netstat -apn | grep ssh   并不是所有的进程都能找到，没有权限的会不显示，使用 root 权限查看所有的信息。
   
   netstat -an | grep ':22'  找出运行在指定端口的进程
   
   netstat -ie  跟  ifconfig 效果相似
   
   
   windows的 查看进程 
   netstat -ano | findstr "80"
   ```
   

## 6. linux 查看 开放的端口号

- ```
  1.查看开放的端口号 
  firewall-cmd --zone=public --list-ports
  
  2.添加端口号
  firewall-cmd --zone=public --add-port=9876/tcp --permanent
  
  3.重新启动
  firewall-cmd --reload
  
  4.再次查看 ok了
  
  5. 阿里云上防火墙就是关闭的,需要我们手动去网站开启
  ```

## 7. linux 下 cache和buff 过高

- 手动释放

  ```shell
  echo 1 > /proc/sys/vm/drop_caches;
  echo 2 > /proc/sys/vm/drop_caches;
  echo 3 > /proc/sys/vm/drop_caches;
  ```

- 如何查看 top 和 free都可以

  ```shell
  [root@iz2zegua78a74kqn8clokfz shell]# free -m
                total        used        free      shared  buff/cache   available
  Mem:           1839         783         966           8          89         928
  Swap:             0           0           0
  
  
  [root@iz2zegua78a74kqn8clokfz shell]# free -h
                total        used        free      shared  buff/cache   available
  Mem:           1.8G        783M        966M        8.6M         90M        927M
  Swap:            0B          0B          0B
  ```

- buff/cache是用于加速程序运行的，有一些硬盘/网络上的数据加载的速度比较慢，因此会被缓存到高速的内存中，以免去重复加载的时间消耗。buff/cache在内存占用过高的时候会被系统自动清理， 但是这种清缓存的工作也并不是没有成本。清缓存必须保证cache中的数据跟对应文件中的数据一致，所以伴随着cache清除的行为的，一般都是要做大量硬盘读写的。因为内核要对比cache中的数据和对应硬盘文件上的数据是否一致，如果不一致需要写回，之后才能回收。 硬盘读写就意味着高延迟，这个延迟通常会达到几毫秒到数秒。



## 7. linux 添加定时任务

### 1. 使用

```shell
crond status：查看crond服务状态
crond start：启动crond服务
crond stop：关闭crond服务
crond restart：重启crond服务
crontab -u：设定某个用户的crond服务
crontab -l：列出某个用户的crond内容
crontab -r：删除某个用户的crond服务
crontab -e：编辑某个用户的crond服务内容
*有时在启动crond时会出现crond: can't lock /var/run/crond.pid, otherpid may be 15340: Resource temporarily unavailable的提示，只需用killall crond结束所有crond服务再重新启动即可

然后通过 pgrep crond 查看一下
若没有 则启动  crond start  就 OK了

```

### 2. 例子

```shell
vim clear.sh
#!/bin/bash
echo 1 > /proc/sys/vm/drop_caches
echo 2 > /proc/sys/vm/drop_caches
echo 3 > /proc/sys/vm/drop_caches

crontab -e
0 1 * * * /home/admin/shell/clear.sh

crontab -l  查看有 就OK了 ,然后每天凌晨1点自动清理缓存


software
```





## firewall 

```shell
1. 添加端口
# firewall-cmd --permanent --add-port=80/tcp
# firewall-cmd --permanent --add-port=443/tcp


2. 重启
# firewall-cmd --reload

3. 查看当前防火墙上的设置，运行 –list-services


1.firewalld的基本使用
启动：  systemctl start firewalld
查状态：systemctl status firewalld 
停止：  systemctl disable firewalld
禁用：  systemctl stop firewalld
在开机时启用一个服务：systemctl enable firewalld.service
在开机时禁用一个服务：systemctl disable firewalld.service
查看服务是否开机启动：systemctl is-enabled firewalld.service
查看已启动的服务列表：systemctl list-unit-files|grep enabled
查看启动失败的服务列表：systemctl --failed


service iptables stop
chkconfig iptables off 
```