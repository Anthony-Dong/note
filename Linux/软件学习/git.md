
# git以及github学习
## 1.介绍

![](https://git-scm.com/images/logo@2x.png)

> ​		[Git的网址](https://git-scm.com/)(https://git-scm.com/); Git是一个开源的分布式版本控制系统可以有效、高速地处理从很小到非常大的项目版本管理 . 同时国内的git仓库是[码云](https://gitee.com/)(https://gitee.com/)  , 国外的是[github](https://github.com/Anthony-Dong)(https://github.com/)	

## 2.概念
> Git工作区、版本库和暂存区概念
>
> 1.工作区（workspace）是对项目的某个版本独立提取出来的内容。这些从Git仓库的压缩数据库中提取出来的文件，放在磁盘上供你使用或修改。
>
> 2.暂存区域（Staging Area）是一个文件，保存了下次将提交的文件列表信息，一般在 Git 仓库目录中。有时候也被称作“索引”，不过一般说法还是叫暂存区域。
>
> 3.版本库也叫本地库（Repository）目录是Git用来保存项目的元数据和对象数据库的地方。这是Git中最重要的部分，从其它计算机克隆仓库时，拷贝的就是这里的数据.

![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/f725411f-1fbf-4c4c-8baa-5167e15181c5.jpg)

注意 :  LF(unix的回车换行符) 与 CRLF(windows的回车换行符)

## 3. git常用指令

### GIT安装(linux版本)

```shell
1. 下载安装包
sudo wget https://github.com/git/git/archive/v2.8.0.tar.gz

2. 解压
sudo tar -zxvf v2.8.0.tar.gz -C /opt/git  

3. 安装依赖
sudo yum -y install zlib-devel openssl-devel cpio expat-devel gettext-devel curl-devel perl-ExtUtils-CBuilder perl-ExtUtils-MakeMaker

4. 安装在 /usr/local/git
sudo make prefix=/usr/local/git all
sudo make prefix=/usr/local/git install

5. 修改 /etc/profile
配置环境变量
export GIT_HOME=/usr/local/git
export PATH=$PATH:$GIT_HOME/bin

6. 介意配置免密登录 --- 看我下面的 ssh 免密登录
```

### GIT安装(windows版)

```shell
下载地址 
https://git-scm.com/download/win

无脑回车到底, 记住一点安装路径千万被有中文

windows就别配置环境变量了, 用git bash 就不错.

介意配置免密登录 --- 看我下面的 ssh 免密登录
```

### 查看git的全局配置
	 git config --list
	
	 必须添加的 信息 前面全局设置 后面是次repository设置
	 git config --global user.name "Anthony-Dong"
	 git config --global user.email "574986060@qq.com"
	 git config user.email "574986060@qq.com"
	 git config user.name "Anthony-Dong"

### git add a.txt
	提交a.txt从工作区到暂存区

### git rm --cached a.txt
	将新建的 a.txt 从 暂存区 中撤回来;放到工作区;

### git commit -m 'new file a.txt' a.txt
	提交 到 本地库
	第一次提交 会显示 root-commit 

### git status   当前状态信息
	绿色信息  表示 暂存区和工作区 与 本地库不在一条线上  (只需要 commit)   
	红色信息  表示 工作区 与 暂存区和本地库 不在一条线上    (这时候 需要 add 和 commit)
	modify   有修改 
	new file 有新文件需要
	delete   删除操作

### git log 可以查看 提交的具体历史记录	
	一行显示 比较舒服 用起来 : 
	
	$ git log --pretty=oneline
	8ba6b3df90bfb25ab289bee5248f07c96ce7a2d7 (HEAD -> master) third
	93fcb68263ca04851e638536e951ee7dd126b994 second
	a98370e63b0daa156074f1bbedf6641f23a7b2c6 first
	
	$ git log --oneline
	8ba6b3d (HEAD -> master) third
	93fcb68 second
	a98370e first

### git reflog  可以显示 git中head的移动步数
	第一条显示的是 目前版本指针指向的位置
	$ git reflog
	608deb0 (HEAD -> master) HEAD@{0}: reset: moving to 608deb0
	8ba6b3d HEAD@{1}: reset: moving to 8ba6b3d
	608deb0 (HEAD -> master) HEAD@{2}: commit: forth
	8ba6b3d HEAD@{3}: commit: third
	93fcb68 HEAD@{4}: commit: second
	a98370e HEAD@{5}: commit (initial): first

## 4. git的版本控制
### git 的指针操作 

​		进行版本前进后退操作  head(指向当前分支的一个指针;master指向当前版本的一个指针) 可以利用head 对版本进行前进后退

	$ git reset --hard 8ba6b3d   (后面的索引是hash的一部分;可以通过 git reflog 查看)
	HEAD is now at 8ba6b3d third
	
	$ git reset --hard head^ (一个^ 代表回退一个版本)
	HEAD is now at 8ba6b3d third
	
	$ git reset --hard head~2 (head~2 指定后退的几步 3-1 =2 步)
	HEAD is now at a98370e first
	
	git help reset 其他命令可以参照help
	--soft  Does not touch the index file(暂存区域) or the working tree(工作区域) at all 
	--mixed Resets the index but not the working tree
	--hard Resets the index and working tree

### git 删除文件需要注意

git工作区删除文件 也需要 进行add 和 commit

```
比如 rm c.txt 此时查看status 会有红色字样;所以需要 add commit 进行 delete记录;方便版本控制

还有一种情况你删除的操作文件提交到暂存区;但是又想要撤回到工作区需要下面的操作;他也会提示你回到哪个版本 : 
$ git reset --hard head
HEAD is now at a3e945a new file d.txt
```

### git diff a.txt
	会提示 此文件哪行修改了(因为git对文本是以行为单位);与暂存区的文件进行比较
	
	但是 git diff HEAD a.txt 是与本地库的进行比较;
	同时也可以  git diff HEAD^ a.txt  可以和当前版本 前一个版本进行比较
	
	不指定文件名 会比较全部文件

## 5. git的分支
![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/75007332-e8b9-4d0a-ae85-a476f2ed407d.jpg?x-oss-process=style/template01)	
	
	master 是主分支(各个节点代表版本)
	feature_game 主要是功能分支(模块开发的分支)
	hot_fix 主要是bug修复分支
	
	On branch master 代表是在主分支上
#### git branch

创建分支

```shell
git branch -r    查看当前的远程分支
git branch -a    查看当前的所有分支

git branch hot_fix  创建新的分支,分支名为 hot_fix

git branch -d hot_fix  删除名为hot_fix的分支
```
#### git checkout

切换分支

```shell
git checkout -b newBrach origin/master   在原有的 origin/master 分支上创建一个新的分支

git checkout hot_fix  切换分支到hot_fix
```

#### git merge

合并分支

```shell
git merge origin/master   将 origin/master 合并到 当前分支
```

#### 解决多人开发问题
	1. 我们可以创建一个新的分支'other' , 在我们当前master分支上
	git branch other 
	
	2. 切换到other 分支上
	git checkout  other 
	
	3. 我们将远程的拉去到本地,先查看远程分支
	git fetch origin master
	
	然后 看看远程分支
	git remote -r
	
	4. 将远程分支合并到当前分支,此时分支为 other
	
	git merge origin/master 
	
	5. 此时我们再解决冲突,然后add, commit ,然后push 将本地的other分支给远程master分支
	
	git push origin other:master


## 6. github使用

注意一点 以下的 `<>:<>` 这个语句 , 必须不能有空格,例如 ` <> : <> ` ,这样子会报错,因为他们是一部分

#### 1. git init 

 初始化git仓库 , 还有就是 github上有一个文件 叫 `README.md` 文件,是你的项目介绍文件

#### 2. git add test.txt

将工作区文件放入暂存区

#### 3. git commit -m "first commit" 

将暂存区 提交到 版本库中

#### 4. git remote add <主机名>  <网址>

```shell
git remote add origin https://github.com/Anthony-Dong/test.git

远程主机名为 origin 添加进来

git remote rename <原主机名> <新主机名>  用于远程主机的改名。

git remote rm <主机名>  命令用于删除远程主机。

git remote show <主机名> 可以查看该主机的详细信息
```

#### 5. git  push  <远程主机名>  <本地分支名>:<远程分支名>  

```shell
git push origin master  将本地的master分支推送到origin主机的master分支。如果远程master不存在，则会被新建。


git push origin :master 等同于 git push origin --delete master   删除origin主机的master分支。


git push --all origin  将所有本地分支都推送到origin主机。
```

#### 6.  git remote -v  

查看 remote的内容

	$ git remote -v
	origin  https://github.com/Anthony-Dong/test.git (fetch)    表示这个地址取回
	origin  https://github.com/Anthony-Dong/test.git (push)		表示这个地址推送
#### 7. git clone <版本库的网址> <本地目录名>

```shell
git clone https://github.com/Anthony-Dong/test.git D:/MyDesktop/git/test

意思就是 克隆 https://github.com/Anthony-Dong/test.git 到我本地的 D:/MyDesktop/git/test 路径下,记住不能放到已经存在的路径下面

git clone https://github.com/Anthony-Dong/test.git
克隆到当前目录

git clone  -o originhost https://github.com/Anthony-Dong/test.git
克隆到当前目录 ,远程主机名自定义为originhost, 默认是 origin
```

####  8. git fetch <远程主机名>  <分支名> 

`git fetch`命令通常用来查看其他人的进程，因为它取回的代码对你本地的开发代码没有影响。

```shell
git fetch origin 将远程分支全部拉到本地

git fetch origin master  取回origin主机的master分支。

git log -p FETCH_HEAD    可以通过这个命令查看 更新的信息
```

#### 9. git merge  <远程主机名>/<分支名> 

```shell
git merge origin/master  将远程的origin/master 分支合并到本地

git rebase origin/master  和上面这句话含义一样
```

#### 10. git pull <远程主机名> <远程分支名>:<本地分支名>

	git pull origin next:master   将origin主机的next分支，与本地的master分支合并
	
	git pull origin next  如果远程分支是与当前分支合并，则冒号后面的部分可以省略。就是将远程的next分支和当前分支 合并
	
	git fetch origin
	git merge origin/next
#### 11. fork 协助开发
	1. 先到git上fork 别人的项目
	2. 此时你的github上仓库就会有这个项目
	2. git clone 到本地 ,修改完提交到自己的仓库
	3. 在github项目页上发送 pull request进行发送 推送请求
	4. 项目人会在在 github上会 收到 pullrequest ;此时就有 merger选项,可以进行合并冲突

## 7. git原理
#### 1.hash算法(加密算法)
	主要主流的有 sha1 md5 crc32 算法;其主要特点就是 文件经过特定的hash算法生成的结果长度都是固定的; 而且是不可逆操作; 还有只要修改文件;hash值就要发生改变
#### 2.分支管理过程	
	1.git底层采用的 SHA-1 算法 ;进行文件校验的;保证数据的完整性
	2. 提交对象(版本节点) 包含有 {commit ,tree:{blob,blob} ,parent;author ,committer} ;tree包含了 所有的文件对象的hash值;同时 tree也指向一个hash;所以 每次 commit都会产生一个版本对象
	3. 提交对象与父对象形成链条(版本链条) 就类似于一个区块链;每个节点都保存有父节点的信息;不能修改;
	4. 版本的每一次提交（commit），git都将它们根据提交的时间点串联成一条线。刚开始是只有一条时间线，即master分支，HEAD指向的是当前分支的当前版本
![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/6369eee5-e702-49e6-8c3b-3c8438fe371a.jpg?x-oss-process=style/template01)

	5. 分支: master分支 会指向 我们 每次提交的版本对象 ;
	
	6. 当创建了新分支，比如dev分支（通过命令git branch dev完成），git新建一个指针dev，dev=master，dev指向master指向的版本，然后切换到dev分支（通过命令git checkout dev完成），把HEAD指针指向dev。
![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/e7a7d04d-d3cd-492b-9b96-fb7bf7d70028.jpg?x-oss-process=style/template01)

	7. 每次切换指针 只是改变了 head指针 指向的 分支对象
	
	8.在dev分支上编码开发时，都是在dev上进行指针移动，比如在dev分支上commit一次，dev指针往前移动一步，但是master指针没有变
![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/95202ec9-9762-47f8-a57c-e106cc1cb86b.jpg?x-oss-process=style/template01)

	9.当我们完成了dev分支上的工作，要进行分支合并，把dev分支的内容合并到master分支上（通过首先切换到master分支，git branch master，然后合并git merge dev命令完成）。其内部的原理，其实就是先把HEAD指针指向master，再把master指针指向现在的dev指针指向的内容
![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/57f63f82-5cde-47e7-a5fc-f3075a8ffa47.jpg?x-oss-process=style/template01)

	10.当merge 分支时 ;如果产生版本不对应的问题; 产生div分支不在主分支上; 这时 git 会 合并俩分支;交给人来处理冲突;然后add commit 完成合并
![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/f162c8ed-c0e9-4643-9ba2-0bc52888a82b.jpg?x-oss-process=style/template01)
![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/0785b90c-2846-4988-8770-b836f5eeb935.jpg?x-oss-process=style/template01)

	7.合并完成后，就可以删除掉dev分支（通过git branch -d dev命令完成）
![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/50f2244b-72dc-485a-a062-06aab6f6ca46.jpg?x-oss-process=style/template01)

## 8. git和svn的区别
	1.svn的集中式的文件管理机制 基于 增量式版本控制 :每次只保存 修改了的内容;需要的时候需要把前面全部加起来
		
	2.git 的文件管理机制  基于快照的方式 ;如果数据没有修改只是保存一个链接指向之前存储的文件
	3. Subversion原理上只关心文件内容的具体差异。每次记录有哪些文件作了更新，以及都更新了哪些行的什么内容.Git记录版本历史只关心文件数据的整体是否发生变化。Git 不保存文件内容前后变化的差异数据。
	4. Subversion的工作区和版本库是截然分开的，而Git的工作区和版本库是如影随形的。
### 1. SVN优缺点
	优点： 
	1、 管理方便，逻辑明确，符合一般人思维习惯。 
	2、 易于管理，集中式服务器更能保证安全性。 
	3、 代码一致性非常高。 
	4、 适合开发人数不多的项目开发。 
	缺点： 
	1、 服务器压力太大，数据库容量暴增。 
	2、 如果不能连接到服务器上，基本上不可以工作，看上面第二步，如果服务器不能连接上，就不能提交，还原，对比等等。 
	3、 不适合开源开发（开发人数非常非常多，但是Google app engine就是用svn的）。但是一般集中式管理的有非常明确的权限管理机制（例如分支访问限制），可以实现分层管理，从而很好的解决开发人数众多的问题。
### 2. git 优缺点
	优点： 
	1、适合分布式开发，强调个体。 
	2、公共服务器压力和数据量都不会太大。 
	3、速度快、灵活。 
	4、任意两个开发者之间可以很容易的解决冲突。 
	5、离线工作。 
	缺点： 
	1、学习周期相对而言比较长。 
	2、不符合常规思维。 
	3、代码保密性差，一旦开发者把整个库克隆下来就可以完全公开所有代码和版本信息。

## 9. git的工作流
1. 集中式工作流 和 svn类似(以中央仓库作为项目所要修改的单点实体,所有修改都要提交到master分支上)

2. gitflow 工作流 ,充分利用了 git的分布式(分支,协作开发)特性

		主干分支 master
		开发分支 develop
		bug 修复分支 hot_fix
		测试分支(提交到主分支前的测试) release
		功能分支 feature
![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/f56932a4-a86a-438c-9ddb-3355b7352e3f.jpg?x-oss-process=style/template01)



## 10. ssh免密登录
	秘钥进行连接 方式 进行 本地仓库 提交到 远程仓库
### 进行免密登录
	ssh-keygen -t rsa
	一路回车到底,其实就是生成一个密钥对
	linux 目录在你当前用户的空间下的 .shh目录下 , 配置免密登录应该都会这个
	windows 目录在你的 C:\Users\12986\.ssh , 如果你不修改默认配置的话就是这里 ,
	然后 去到github上 ,把公钥输入进去
### 进行 调用远程仓库
	// github 的 
	git remote add origin git@github.com:Anthony-Dong/tyut-ssm.git
	
	// 自己的git仓库 ..... 
	git remote add origin ssh://git@192.168.58.129/home/user/git

## 11. git常见命令的区别
```
	git commit：是将本地修改过的文件提交到本地库中；
 	git push：是将本地库中的最新信息发送给远程库；
 	git pull：是从远程获取最新版本到本地，并自动merge；
	git fetch：是从远程获取最新版本到本地，不会自动merge；
	git merge：是用于从指定的分支合并到当前分支，用来合并两个分支；
	git pull 相当于 git fetch + git merge。
```

![]( https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-31/e87b4c1e-15b9-4b67-b503-bdbcd7711b4e.jpg?x-oss-process=style/template01)

repository 是本地版本库

workspace 是本地工作区,就是我们工作的目录

Remoto 就是远程的仓库