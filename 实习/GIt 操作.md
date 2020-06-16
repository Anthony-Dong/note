## 项目组的GIT操作流程

1. 本地拉取项目，首先得`git checkout -b new_branch` , 就是在拉取到的分支上新建一个分支，不要在主分支上开发 （后期如果需要合到主分支，需要申请leader进行merge）；

2. 提交前,也就是`git add .`前，看看git diff是不是自己想要加的修改，然后add ， commit ， push 当前的分支，也就是远程新建一个分支； 比如上面是 new_branch ,那么我们push的时候需要`git push origin new_branch`

3. 其次就是联调和测试环境了， 联调直接去盖亚系统上，选择自己的项目分支，将自己的项目分支加到当前的项目分支中，然后选择**手动分支构建** ， 构建完成后会有一个**发布按钮**， 我们可以通过 单车联调环境进行和其它部门的联调 ，测试环境需要申请机器（**跳板机**） ，然后发不到测试环境中。

4. **预发布环境**：需要我们进行tag操作，**每一次预发布都需要新建一个tag** ， 预发机器也需要申请 ， 为了防止tag打的多，最好先确定好再发布。

   ```shell
   # 1.找到最下面那个tag， 然后加1，现在这种就是 v4.9.5.4
   git tag
   # ...
   v4.9.5.2
   v4.9.5.3
   
   # 2.新建一个tag
   git tag v4.9.5.4
   
   # 3.发布tag
   git push origin v4.9.5.4
   ```

   其次就是我们的进入盖亚的系统， 选择手动tag构建，最后进行发布到slave环境，然后进到机器中测试就行了。

5. 线上环境， 后期再说，还没接触到。

6. 最后留一个我们项目组git[开发的操作流程](http://wiki.corp.ttyongche.com:8360/pages/viewpage.action?pageId=26565486) 。

## Gitignore 的问题

​	当远程文件拉取到本地，或者将我们本地的数据已经有一份提交过了，此时再去创建.gitignore时，会出现无效的问题，是因为我们本地有个缓存，[为什么gitignore无效] ，此时我们需要修改一下`git rm -r --cached .` 执行此命令， 清空本地的缓存；

​	关于gitigonre的用法，[可以看看这一篇博客](https://www.cnblogs.com/kevingrace/p/5690241.html)  ，其中如果不嫌弃麻烦的话，可以建立多个gitignore文件，其中下面是，忽略和.gitignore文件同级目录下的文件；

```git
.idea
.env
```


## Git 如何做撤销操作

```shell
# 撤销工作区中当前目录中的所有更改
git checkout .
 
# 撤销工作区中指定文件的修改
git checkout -- filename
```

> ​	git checkout 命令只能撤销工作区中的修改，而不能撤销暂存区中的修改。

```shell
# 切换到指定的分支（如果分支在本地不存在，会自动从远程仓库拉取下来）
git checkout develop
 
# 切换分支的同时，创建分支
git checkout -b my-dev
```

其他操作看这篇文章 [https://blog.csdn.net/lamp_yang_3533/article/details/80357505](https://blog.csdn.net/lamp_yang_3533/article/details/80357505)  



假如你修改了工作区，并把工作区中的修改 git add 到了暂存区。

如果你想撤销工作区和暂存区中的修改。

```shell
git reset --hard head
# 简写为
git reset --hard
```

如果你仅仅只是想要撤销暂存区中的修改。

```shell
git reset --mixed
# 简写为
git reset
```

如果你已经commit了， 你想回退到上个版本

```shell
git reset --hard head^
```





## 提交

```shell
# 切换到主分枝
git checkout master 

# 拉取远程master
git pull

# 切换到我们的开发分支,合并master
git checkout dev
git merger master
```



