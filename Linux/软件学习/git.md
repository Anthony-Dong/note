# git 以及 github 学习

## 1.介绍

![](https://git-scm.com/images/logo@2x.png)

> ​ [Git 的网址](https://git-scm.com/)(https://git-scm.com/); Git 是一个开源的分布式版本控制系统可以有效、高速地处理从很小到非常大的项目版本管理 . 同时国内的 git 仓库是[码云](https://gitee.com/)(https://gitee.com/) , 国外的是[github](https://github.com/Anthony-Dong)(https://github.com/),同时还有很多,比如阿里云的`code.aliyun` 还有 腾讯的 `coding.net` , 都很优秀

## 2.概念

> Git 工作区、版本库和暂存区概念
>
> 1.工作区（workspace）是对项目的某个版本独立提取出来的内容。这些从 Git 仓库的压缩数据库中提取出来的文件，放在磁盘上供你使用或修改。
>
> 2.暂存区域（Staging Area）是一个文件，保存了下次将提交的文件列表信息，一般在 Git 仓库目录中。有时候也被称作“索引”，不过一般说法还是叫暂存区域。
>
> 3.版本库也叫本地库（Repository）目录是 Git 用来保存项目的元数据和对象数据库的地方。这是 Git 中最重要的部分，从其它计算机克隆仓库时，拷贝的就是这里的数据.

![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/f725411f-1fbf-4c4c-8baa-5167e15181c5.jpg)

注意 : LF(unix 的回车换行符) 与 CRLF(windows 的回车换行符)

## 3. git 常用指令

### 1. GIT 安装(linux 版本)

```java
// 1. 下载安装包
sudo wget https://github.com/git/git/archive/v2.8.0.tar.gz

// 2. 解压
sudo tar -zxvf v2.8.0.tar.gz -C /opt/git

// 3. 安装依赖
sudo yum -y install zlib-devel openssl-devel cpio expat-devel gettext-devel curl-devel perl-ExtUtils-CBuilder perl-ExtUtils-MakeMaker

// 4. 安装在 /usr/local/git
sudo make prefix=/usr/local/git all
sudo make prefix=/usr/local/git install

// 5. 修改 /etc/profile 配置环境变量
export GIT_HOME=/usr/local/git
export PATH=$PATH:$GIT_HOME/bin

// 6. 介意配置免密登录 --- 看我下面的 ssh 免密登录
```

### 2. git config

```java
// 1. 查看配置信息
git config --list

// 2. 全局配置(以后每个项目都是)
git config --global user.name "Anthony-Dong"
git config --global user.email "574986060@qq.com"

// 3. 项目配置
git config user.email "574986060@qq.com"
git config user.name "Anthony-Dong"
```

注意一点 以下的 `<>:<>` 这个语句 , 必须不能有空格,例如 `<> : <>` ,这样子会报错,因为他们是一部分

### 3. git init

初始化 git 仓库 , 还有就是 github 上有一个文件 叫 `README.md` 文件,是你的项目介绍文件

### 4. git add test.txt

将工作区文件放入暂存区

### 5. git commit -m "first commit"

将暂存区 提交到 版本库中

### 6. git remote add <主机名> <网址>

```java
// 1. 远程远程git仓库名为 origin 添加进来
git remote add server_name ssh://admin@47.94.234.232:/home/admin/git/test.git

// 2. 用于远程主机的改名。
git remote rename <原主机名> <新主机名>

// 3. 命令用于删除远程主机。
git remote rm <主机名>

// 4. 可以查看该主机的详细信息
git remote show <主机名>
```

### 7. git push <远程主机名> <本地分支名>:<远程分支名>

```java
//1. 将本地的newbranch分支推送到origin主机的master分支
git push origin newbranch:master

//2. 其实一般都是写 将本地的master分支推送到origin的git仓库,如果master不存在则创建
git push origin master
```

### 8. git remote -v

查看 remote 的内容

```java
$ git remote -v
origin  https://github.com/Anthony-Dong/test.git (fetch)    表示这个地址取回
origin  https://github.com/Anthony-Dong/test.git (push)		表示这个地址推送
```

### 9. git clone <版本库的网址> <本地目录名>

```java
// 1. 意思就是克隆 https://github.com/Anthony-Dong/test.git 到我本地的 D:/MyDesktop/git/test 路径下
git clone https://github.com/Anthony-Dong/test.git D:/MyDesktop/git/test

// 2. 到当前目录
git clone https://github.com/Anthony-Dong/test.git


// 3. 克隆到当前目录 ,远程主机名自定义为originhost, 默认是 origin
git clone  -o originhost https://github.com/Anthony-Dong/test.git
```

### 10. git fetch <远程主机名> <分支名>

`git fetch`命令通常用来查看其他人的进程，因为它取回的代码对你本地的开发代码没有影响。

```java
// 1. 将远程分支全部拉到本地
git fetch origin

// 2. 取回origin主机的master分支。
git fetch origin master

// 3. 可以通过这个命令查看 更新的信息
git log -p FETCH_HEAD
```

### 11. git merge <远程主机名>/<分支名>

```java
// 1. 将远程的origin/master 分支合并到本地
git merge origin/master

// 2. 和上面这句话含义一样
git rebase origin/master

// 3. 如果出现拒绝合并
git merge origin/master --allow-unrelated-histories
```

### 12. git pull <远程主机名> <远程分支名>:<本地分支名>

```java
// 1. 将origin主机的next分支，与本地的master分支合并
git pull origin next:master

// 2. 如果远程分支是与当前分支合并，则冒号后面的部分可以省略。就是将远程的next分支和当前分支合并 , 如果远程没有next分支他会自己创建
git pull origin next

// 3. 拉去远程分支 , 到本地库(origin本地分支),不会合并分支
git fetch origin master

// 4. 将origin本地分支和next分支合并
git merge origin/next

// 5. 如果你第一次拉去项目, 最好这么拉去.不然拉去失败 , 因为不可能你的项目是一个空项目.
git merge origin master --allow-unrelated-histories

// 6. 可以通过git status 查看当前合并后需要处理的问题.
```

### 13. fork 协助开发

> ​ 1. 先到 git 上 fork 别人的项目
>
> 2. 此时你的 github 上仓库就会有这个项目
> 3. git clone 到本地 ,修改完提交到自己的仓库
> 4. 在 github 项目页上发送 pull request 进行发送 推送请求
> 5. 项目人会在在 github 上会 收到 pullrequest ;此时就有 merger 选项,可以进行合并冲突

### 14. config 文件

> ​ 在 .git 里面

```properties
[core]
	repositoryformatversion = 0
	filemode = false
	bare = false
	logallrefupdates = true
	symlinks = false
	ignorecase = true
[user]
	name = Anthony-Dong
	email = fanhaodong516@qq.com
[remote "coding"]
	url = git@e.coding.net:anthony-dong/blog.git
	fetch = +refs/heads/*:refs/remotes/coding/*
[remote "github"]
	url = git@github.com:Anthony-Dong/anthony-dong.github.io.git
	fetch = +refs/heads/*:refs/remotes/github/*
[remote "gitee"]
	url = git@gitee.com:Anthony-Dong/Anthony-Dong.git
	fetch = +refs/heads/*:refs/remotes/gitee/*
[branch "master"]
	remote = coding
	remote = github
	remote = gitee
	merge = refs/heads/master
```

像我这个配置 , 他就有三个远程地址 ,

## 4. git 的版本控制

> ​ 进行版本前进后退操作 head(指向当前分支的一个指针;master 指向当前版本的一个指针) 可以利用 head 对版本进行前进后退 .

```java
// 1. 回退到8ba6b3d分支中去
git reset --hard 8ba6b3d
HEAD is now at 8ba6b3d third

// 2. 查看其他命令
git help reset
--soft  Does not touch the index file(暂存区域) or the working tree(工作区域) at all
--mixed Resets the index but not the working tree
--hard Resets the index and working tree
```

## 5. git 的分支

![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/75007332-e8b9-4d0a-ae85-a476f2ed407d.jpg?x-oss-process=style/template01)

master 是主分支(各个节点代表版本)
feature_game 主要是功能分支(模块开发的分支)
hot_fix 主要是 bug 修复分支
On branch master 代表是在主分支上

### 1. git branch

创建分支

```java
// 1.查看当前的远程分支
git branch -r
// 2.查看当前的所有分支
git branch -a

// 3.创建新的分支,分支名为 hot_fix
git branch hot_fix

// 4.删除名为hot_fix的分支
git branch -d hot_fix
```

### 2. git checkout

切换分支

```java
// 1.在原有的 origin/master 分支上创建一个新的分支
git checkout -b newBrach origin/master

// 2.切换分支到hot_fix
git checkout hot_fix
```

### 3. 解决多人开发问题

```java
// 1. 我们可以创建一个新的分支'other' , 在我们当前master分支上
git branch other

// 2. 切换到other 分支上
git checkout  other

// 3. 我们将远程的master分支拉去到本地
git fetch origin master

// 然后 看看远程分支
git remote -r

// 4. 将远程origin master分支合并到当前分支中去.
git merge origin/master

// 5. 将处理完的other推送到远程的master分支中去
git push origin other:master
```

## 6. git 原理

#### 1. hash 算法(加密算法)

> ​ 主要主流的有 sha1 md5 crc32 算法;其主要特点就是 文件经过特定的 hash 算法生成的结果长度都是固定的; 而且是不可逆操作; 还有只要修改文件;hash 值就要发生改变

#### 2. 分支管理过程

​ 1.git 底层采用的 SHA-1 算法 ;进行文件校验的;保证数据的完整性

2. 提交对象(版本节点) 包含有 {commit ,tree:{blob,blob} ,parent;author ,committer} ;tree 包含了 所有的文件对象的 hash 值;同时 tree 也指向一个 hash;所以 每次 commit 都会产生一个版本对象
3. 提交对象与父对象形成链条(版本链条) 就类似于一个区块链;每个节点都保存有父节点的信息;不能修改;
4. 版本的每一次提交（commit），git 都将它们根据提交的时间点串联成一条线。刚开始是只有一条时间线，即 master 分支，HEAD 指向的是当前分支的当前版本

![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/6369eee5-e702-49e6-8c3b-3c8438fe371a.jpg?x-oss-process=style/template01)

5. 分支: master 分支 会指向 我们 每次提交的版本对象 ;

6. 当创建了新分支，比如 dev 分支（通过命令 git branch dev 完成），git 新建一个指针 dev，dev=master，dev 指向 master 指向的版本，然后切换到 dev 分支（通过命令 git checkout dev 完成），把 HEAD 指针指向 dev。

![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/e7a7d04d-d3cd-492b-9b96-fb7bf7d70028.jpg?x-oss-process=style/template01)

7. 每次切换指针 只是改变了 head 指针 指向的 分支对象

8.在 dev 分支上编码开发时，都是在 dev 上进行指针移动，比如在 dev 分支上 commit 一次，dev 指针往前移动一步，但是 master 指针没有变

![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/95202ec9-9762-47f8-a57c-e106cc1cb86b.jpg?x-oss-process=style/template01)

9.当我们完成了 dev 分支上的工作，要进行分支合并，把 dev 分支的内容合并到 master 分支上（通过首先切换到 master 分支，git branch master，然后合并 git merge dev 命令完成）。其内部的原理，其实就是先把 HEAD 指针指向 master，再把 master 指针指向现在的 dev 指针指向的内容

![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/57f63f82-5cde-47e7-a5fc-f3075a8ffa47.jpg?x-oss-process=style/template01)

10.当 merge 分支时 ;如果产生版本不对应的问题; 产生 div 分支不在主分支上; 这时 git 会 合并俩分支;交给人来处理冲突;然后 add commit 完成合并

![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/f162c8ed-c0e9-4643-9ba2-0bc52888a82b.jpg?x-oss-process=style/template01)
![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/0785b90c-2846-4988-8770-b836f5eeb935.jpg?x-oss-process=style/template01)

11.合并完成后，就可以删除掉 dev 分支（通过 git branch -d dev 命令完成）

![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/50f2244b-72dc-485a-a062-06aab6f6ca46.jpg?x-oss-process=style/template01)

## 7. git 和 svn 的区别

- svn 的集中式的文件管理机制 基于 增量式版本控制 :每次只保存 修改了的内容;需要的时候需要把前面全部加起来

- git 的文件管理机制 基于快照的方式 ;如果数据没有修改只是保存一个链接指向之前存储的文件
- Subversion 原理上只关心文件内容的具体差异。每次记录有哪些文件作了更新，以及都更新了哪些行的什么内容.Git 记录版本历史只关心文件数据的整体是否发生变化。Git 不保存文件内容前后变化的差异数据。
- Subversion 的工作区和版本库是截然分开的，而 Git 的工作区和版本库是如影随形的。

### 1. SVN 优缺点

**优点：**
1、 管理方便，逻辑明确，符合一般人思维习惯。
2、 易于管理，集中式服务器更能保证安全性。
3、 代码一致性非常高。
4、 适合开发人数不多的项目开发。

**缺点：**
1、 服务器压力太大，数据库容量暴增。
2、 如果不能连接到服务器上，基本上不可以工作，看上面第二步，如果服务器不能连接上，就不能提交，还原，对比等等。
3、 不适合开源开发（开发人数非常非常多，但是 Google app engine 就是用 svn 的）。但是一般集中式管理的有非常明确的权限管理机制（例如分支访问限制），可以实现分层管理，从而很好的解决开发人数众多的问题。

### 2. git 优缺点

**优点：**
1、适合分布式开发，强调个体。
2、公共服务器压力和数据量都不会太大。
3、速度快、灵活。
4、任意两个开发者之间可以很容易的解决冲突。
5、离线工作。

**缺点：**
1、学习周期相对而言比较长。
2、不符合常规思维。
3、代码保密性差，一旦开发者把整个库克隆下来就可以完全公开所有代码和版本信息。

## 8. git 的工作流

1. 集中式工作流 和 svn 类似(以中央仓库作为项目所要修改的单点实体,所有修改都要提交到 master 分支上)

2. gitflow 工作流 ,充分利用了 git 的分布式(分支,协作开发)特性

主干分支 master
开发分支 develop
bug 修复分支 hot_fix
测试分支(提交到主分支前的测试) release
功能分支 feature

![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/f56932a4-a86a-438c-9ddb-3355b7352e3f.jpg?x-oss-process=style/template01)

## 9. ssh 免密登录

秘钥进行连接 方式 进行 本地仓库 提交到 远程仓库

### 1. 进行免密登录

```java
// 1. 一路回车到底,其实就是生成一个密钥对
ssh-keygen -t rsa
```

linux 目录在你`当前用户的空间下的 .shh目录`下 , 配置免密登录应该都会这个 , windows 目录在你的`C:\Users\12986\.ssh`, 如果你不修改默认配置的话就是这里 ,然后 去到 github 上 ,把公钥输入进去

### 2. 进行 调用远程仓库

```java
// 1. github 的
git remote add origin git@github.com:Anthony-Dong/tyut-ssm.git

// 2. 自己的git仓库 .....
git remote add origin ssh://git@192.168.58.129:/home/user/git
```

## 10. git 常见命令的区别

git commit：是将本地修改过的文件提交到本地库中；
git push：是将本地库中的最新信息发送给远程库；
git pull：是从远程获取最新版本到本地，并自动 merge；
git fetch：是从远程获取最新版本到本地，不会自动 merge；
git merge：是用于从指定的分支合并到当前分支，用来合并两个分支；
git pull 相当于 git fetch + git merge。

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-31/e87b4c1e-15b9-4b67-b503-bdbcd7711b4e.jpg?x-oss-process=style/template01)

repository 是本地版本库

workspace 是本地工作区,就是我们工作的目录

Remoto 就是远程的仓库

## 11. 创建 git 服务器

第一步 , 当然是服务器安装 git 了 , 很简单

```java
sudo yum install -y git
```

第二步就是 , 要么你使用 root 用户 要么自己用户也行. 都可以 .

我就是 admin 目录

```java
[admin@iz2zegua78a74kqn8clokfz test2.git]$ pwd
/home/admin/git/test.git
```

创建一个文件夹 , 我是都放在 git 里面了 , 所以后面这个才是 test.git 才是我们的 git 项目文件

初始化

```java
git init --bare test.git
```

此时你的 test.git 仓库就创建好了 , 然后就是拉去和提交了 , ssh 地址是下面这个

```java
ssh://admin@47.94.234.232:/home/admin/git/test.git
```

很简单反正 . 空项目就是正常流程 . 我不写了.

配置免密登录就是将你的公钥, 方到你服务器用户的公钥中. 就这个. 也不展示了 .

在 admin 用户的 .ssh 下的

```java
[admin@iz2zegua78a74kqn8clokfz .ssh]$ pwd
/home/admin/.ssh
```

修改或者创建一个 `authorized_keys` 这个文件 , 里面放入你的 windows 电脑或者 mac 电脑上的 公钥

```java
[admin@iz2zegua78a74kqn8clokfz .ssh]$ cat authorized_keys
ssh-rsa AAVAB3CzaC1yc2EABBBADAQABAAABAQDBQ68eK8/cc0GV8h4+gy4s2bMfoyU/8cIl3rfsFBj0pLiFTxiqT75yH9JSj/OUPNWhIKGlBynC/0w24dTV3baaFmNeGP9aSAbBihb2JoTS+0JGLIp4hfuUx3VUgxagrLU2I5K8w9GMP5P9zT7dpk9LyfbXje7+HkYY/CnrwsJkiR4zRhUazwoJi7GZJJIvMu1AvRtSBTO61WrO0syPBXrgxcDOowwhiNAnA2OsSMrJHJI4vImJnFuZQO1un19U+B/+M6i7RWZ43ZkZNpW2k+eu3d3JrNHCnZ5O6glwR94JZw7xA61Q1MuxzQJBW/rDhdSEdizM8pFLolDBo6PYB1ep 12986@DESKTOP-7H84K93
```

windows 的公钥在 目录里面. 如何生成 , 百度有. 一个命令`ssh-keygen`就行了. 回车到底 , 或者自己随意输入.

```java
C:\Users\12986\.ssh
```
