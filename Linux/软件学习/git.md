
# git以及github学习
## 1.介绍
	git的网址(https://git-scm.com/);Git是一个开源的分布式版本控制系统，可以有效、高速地处理从很小到非常大的项目版本管理.
	同时 国内的git仓库是码云(https://gitee.com/) 国外的是github(https://github.com/)
![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/93753cd3-5f4b-436a-b297-e93f1ba3305a.jpg?x-oss-process=style/template01)	
## 2.概念
	Git工作区、版本库和暂存区概念
	
	1.工作区（Working Directory）(working tree)是对项目的某个版本独立提取出来的内容。这些从Git仓库的压缩数据库中提取出来的文件，放在磁盘上供你使用或修改。
	2.暂存区域（Staging Area）(index file )是一个文件，保存了下次将提交的文件列表信息，一般在 Git 仓库目录中。有时候也被称作“索引”，不过一般说法还是叫暂存区域。
	3.版本库也叫本地库（Repository）目录是Git用来保存项目的元数据和对象数据库的地方。这是Git中最重要的部分，从其它计算机克隆仓库时，拷贝的就是这里的数据。
![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/f725411f-1fbf-4c4c-8baa-5167e15181c5.jpg?x-oss-process=style/template01)

LF(unix的回车换行符) 与 CRLF(windows的回车换行符)

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
export PATH=$PATH:/usr/local/git/bin
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

### git commit -m 'new file a.txt' a.txt  (跟文件名是局部提交 partial commit ;当 要合并conflict时 不能 跟文件名)
	提交 到 本地库
	第一次提交 会显示 root-commit 

### git status   当前状态信息
	绿色信息  表示 暂存区和工作区 与 本地库不在一条线上  (只需要 commit)   
	红色信息  表示 工作区 与 暂存区和本地库 不在一条线上    (这时候 需要 add 和 commit)
	modify 有修改 
	new file 有新文件需要
	delete 删除操作

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
### git 的指针操作 进行版本前进后退操作  head(指向当前分支的一个指针;master指向当前版本的一个指针) 可以利用head 对版本进行前进后退

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

### git工作区删除文件 也需要 进行add 和 commit
	比如 rm c.txt 此时查看status 会有红色字样;所以需要 add commit 进行 delete记录;方便版本控制
	
	还有一种情况 你 删除的操作文件提交到暂存区;但是又想要撤回到工作区  需要 下面的操作;他也会提示你回到哪个版本 : 
	$ git reset --hard head
	HEAD is now at a3e945a new file d.txt

### git diff a.txt
	会提示 此文件哪行修改了(因为git对文本是以行为单位);与暂存区的文件进行比较
	
	但是 git diff HEAD a.txt 是与本地库的进行比较 ;
	同时也可以  git diff HEAD^ a.txt  可以和当前版本 前一个版本进行比较
	
	不指定文件名 会比较全部文件

## 5. git的分支
![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/75007332-e8b9-4d0a-ae85-a476f2ed407d.jpg?x-oss-process=style/template01)	
	
	master 是主分支(各个节点代表版本)
	feature_game 主要是功能分支(模块开发的分支)
	hot_fix 主要是bug修复分支
	
	On branch master 代表是在主分支上
#### git branch -v  查看所有分支信息

	$ git branch -v
	  hot_fix a3e945a new file d.txt
	* master  a3e945a new file d.txt
	// * 代表当前所在的分支
#### git branch hot_fix  创建新的分支

#### git checkout hot_fix  切换分支 

#### 合并分支 此时必须 切换到 需要合并的分支上;一般要切换到master分支上
	1. $ git checkout master  切换到主分支
	2. $ git merge hot_fix	  合并 你要合并的分支
#### 解决分支冲突(比如说master分支我修改了一次;而hot_fix分支我也修改了一次;此时 master分支合并hot_fix分支的时候就需要解决冲突)
	1. $ git merge hot_fix
		Auto-merging d.txt
		CONFLICT (content): Merge conflict in d.txt
		Automatic merge failed; fix conflicts(冲突) and then commit the result.
		这里会提示 auto_merging 的文件名 所以 我们需要 修改 次文件
	2. 解决冲突的文件
		然后 add and commit  记得此时 commit 不能跟文件名
#### git branch -d hot_fix  删除分支

## 6. github使用
#### 1. git init  初始化git仓库
#### 2. touch README.md  创建一个 描述文件 ;你可以对其进行修改;就是 github 下repository的 描述内容就是
#### 3. git commit -m "first commit" 提交一下操作 
#### 4. git remote add origin https://github.com/Anthony-Dong/test.git 绑定一个远程仓库
#### 5. git push origin master  [远程仓库地址的别名] + [远程的分支名称]  

#### $ git remote -v  查看 remote的内容
	$ git remote -v
	origin  https://github.com/Anthony-Dong/test.git (fetch)  表示这个地址取回
	origin  https://github.com/Anthony-Dong/test.git (push)		表示这个地址推送
#### git clone https://github.com/Anthony-Dong/test.git  远程clone一个项目;他会携带 远程项目的 所有信息

#### git fetch origin master 拉去远程仓库 ;但是这个不改变 工作区的内容版本 ; 此时下载的文件在 origin/master 的分支上;不在 当前分支
		可以通过 git checkout origin/master 切换分支查看内容
#### git merge origin/master 合并 分支  [远程仓库地址的别名/远程的分支名称]

#### git pull origin master  = git fetch +git merge 
	git pull origin <remote_branch>：<local_branch>	
#### 当本地仓库 和 远程仓库 commit 对象 不一样时;push会发生冲突;无法push
	此时 需要 从远程仓库拉去 服务 同时 merge 提示的那个 auto_merge的文件;修改完成后 commit (这次 commit后面不能加文件名);然后再push

#### fork 团队开发 需要把公司的文件;或者别人的文件 copy到你的仓库;此时 在你的仓库的 那个文件你就有使用权限;但是此时并不能push给公司代码进行合并; 
	1.此时你的github上仓库名字下面会写有远程仓库的地址
	2. 修改你本地拉去完成仓库代码,push到自己的远程仓库后
	3.发送 pull request  (在github项目的页面) 进行发送给公司
	4.公司/个人 在 github上会 收到 pullrequest ;此时就有 merger选项,可以进行合并冲突 ; 还有就是 可以进行 chat

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
### 5．SVN优缺点
	优点： 
	1、 管理方便，逻辑明确，符合一般人思维习惯。 
	2、 易于管理，集中式服务器更能保证安全性。 
	3、 代码一致性非常高。 
	4、 适合开发人数不多的项目开发。 
	缺点： 
	1、 服务器压力太大，数据库容量暴增。 
	2、 如果不能连接到服务器上，基本上不可以工作，看上面第二步，如果服务器不能连接上，就不能提交，还原，对比等等。 
	3、 不适合开源开发（开发人数非常非常多，但是Google app engine就是用svn的）。但是一般集中式管理的有非常明确的权限管理机制（例如分支访问限制），可以实现分层管理，从而很好的解决开发人数众多的问题。
### 6. git 优缺点
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
#### 1.集中式工作流 和 svn类似(以中央仓库作为项目所要修改的单点实体,所有修改都要提交到master分支上)
#### 2.gitflow 工作流 ,充分利用了 git的分布式(分支,协作开发)特性
		主干分支 master
		开发分支 develop
		bug 修复分支 hot_fix
		测试分支(提交到主分支前的测试) release
		功能分支 feature
![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-14/f56932a4-a86a-438c-9ddb-3355b7352e3f.jpg?x-oss-process=style/template01)



## 10. ssh连接
	秘钥进行连接 方式 进行 本地仓库 提交到 远程仓库
### 进行生成秘钥设置
	ssh-keygen -t rsa
	一路回车到底 到 user .shh目录下 找到 公钥(public的那个);输入到github上自己的(setting目录下)秘钥管理里;这个只用设置一次
	下面的步骤其实新建项目时都有
### 进行 调用远程仓库
	git remote add origin git@github.com:Anthony-Dong/tyut-ssm.git
	
	git remote add origin ssh://git@192.168.58.129/home/user/git

### 进行远程推送
	git push -u origin master


### 这个其实可以不用设置 没用......

## 11. git常见命令的区别
```
	git commit：是将本地修改过的文件提交到本地库中；
 	git push：是将本地库中的最新信息发送给远程库；
 	git pull：是从远程获取最新版本到本地，并自动merge；
	git fetch：是从远程获取最新版本到本地，不会自动merge；
	git merge：是用于从指定的commit(s)合并到当前分支，用来合并两个分支；
	$ git merge -b  // 指将 b 分支合并到当前分支
	git pull 相当于 git fetch + git merge。
```

