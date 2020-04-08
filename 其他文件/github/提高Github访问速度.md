# 如何提高Github访问速度

## 1. 修改Host方式

首先在你的host文件中加入

WINDOSW平台是在 `C:\Windows\System32\drivers\etc` 这个路径下的 `hosts`文件中
需要管理员可以修改

```properties
# github
13.250.177.223 github.com
185.199.109.153 assets-cdn.github.com
74.86.151.162 github.global.ssl.fastly.net
```


linux 或者 mac上 
可以在 
```shell
[admin@iz2zegua78a74kqn8clokfz ~]$ sudo vim /etc/hosts
# 往里面加入
13.250.177.223 github.com
185.199.109.153 assets-cdn.github.com
74.86.151.162 github.global.ssl.fastly.net

[admin@iz2zegua78a74kqn8clokfz ~]$ sudo source /etc/hosts
```
就可以了

其中这三个域名的查找方式可以使用
`http://tool.chinaz.com/dns` 网站查询 , 输入域名 , 如果你发现其他网页还有卡的, 可以多配置几个

或者你直接放弃使用gitpage , 使用腾讯的codingpage, 网址就是 `https://coding.net`  还有Gitee也行. 
我个人使用的Gitee, 一个仓库推送两个网站所以比较麻烦 . 

## 2. 借助梯子

梯子有很多. 我用的2N , 直接百度就有. 