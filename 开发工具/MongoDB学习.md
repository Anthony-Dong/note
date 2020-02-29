## MongoDB学习

### 1.什么是MongoDB? 和redis的关系是什么

#### 1.1  概念 

MongoDB是一个基于分布式文件存储的数据库。由[C++](https://baike.baidu.com/item/C%2B%2B)语言编写。旨在为WEB应用提供可扩展的高性能数据存储解决方案。

MongoDB是一个介于[关系数据库](https://baike.baidu.com/item/关系数据库)和非关系数据库之间的产品，是非关系数据库当中功能最丰富，最像关系数据库的。它支持的数据结构非常松散，是类似[json](https://baike.baidu.com/item/json)的[bson](https://baike.baidu.com/item/bson)格式，因此可以存储比较复杂的数据类型。Mongo最大的特点是它支持的查询语言非常强大，其语法有点类似于面向对象的查询语言，几乎可以实现类似关系数据库单表查询的绝大部分功能，而且还支持对数据建立[索引](https://baike.baidu.com/item/索引)。

#### 1.2 区别

就Redis和MongoDB来说，大家一般称之为**Redis缓存、MongoDB数据库**。这也是有道有理有根据的，

Redis主要把数据存储在内存中，其“缓存”的性质远大于其“数据存储“的性质，其中数据的增删改查也只是像变量操作一样简单；

MongoDB却是一个“存储数据”的系统，增删改查可以添加很多条件，就像SQL数据库一样灵活，这一点在面试的时候很受用。

- 性能上  R>M
- 可操作性,即 功能性上 M>R
- 可靠性,即数据持久化方式上   M > R
- 数据一致性上 ,redis支持事务操作   R>M
- 数据分析以及应用场景方面  M>R
- 可用性上,即拓展性上 M>R

#### 1.3 个人理解

我感觉他和 elasticsearch很相似 只是 e是基于 [Lucene](https://baike.baidu.com/item/Lucene/6753302)实现的,其实就是java

### 2.MongoDB 安装(linux版本)

#### 2.1 创建仓库文件: 

`vi /etc/yum.repos.d/mongodb-org-3.4.repo`

添加

```
[mongodb-org-3.4]
name=MongoDB Repository
baseurl=https://repo.mongodb.org/yum/redhat/$releasever/mongodb-org/3.4/x86_64/
gpgcheck=1
enabled=1
gpgkey=https://www.mongodb.org/static/pgp/server-3.4.asc
```

#### 2.2  yum安装

1. `yum install -y mongodb-org`

   修改配置文件

2. `vi /etc/mongod.conf`

   修改 bindIP 为 0.0.0.0

   dbpath #数据文件存放目录

   logpath  日志文件存放目录

   port  端口

   fork  后台启动

#### 2.3 启动、停止、重启

​	MongoDB默认将数据文件存储在`/var/lib/mongo`目录，默认日志文件在`/var/log/mongodb`中。如果要修改,可以在 `/etc/mongod.conf` 配置中指定备用日志和数据文件目录。

启动命令:

```
`service mongod start`
```

 停止命令:

```
`service mongod stop`
```

 重启命令:

```
`service mongod restart`
```

可以通过查看日志文件

```
`cat /``var``/log/mongodb/mongod.log`
```

```
[initandlisten] waiting for connections on port <port>
```

<port> 是mongodb运行端口

也可以通过下面命令检查是否启动成功

```
`chkconfig mongod ``on`
```

其中启动会提示   WARNING 怎么办?

```shell
echo never >>  /sys/kernel/mm/transparent_hugepage/enabled
echo never >>  /sys/kernel/mm/transparent_hugepage/defrag
```

#### 2.4 如何使用

```
[root@instance-d0nk2r2c ~]# mongo
```

 

```
## 查看数据库
> show dbs;
```

 

```
## 查看数据库版本
> db.version();
```

 

```
## 常用命令帮助
> db.help();
```

 #### 2.5 卸载移除mongo

```
`yum erase $(rpm -qa | grep mongodb-org)`
```

#### 2.6 移除数据库文件和日志文件

```
`rm -r /``var``/log/mongodb``rm -r /``var``/lib/mongo`
```

### 3. MongoDB 学习和使用(http://www.mongoing.com/docs/)

#### 3.1 MongoDB 关系

![1566359549793](C:\Users\12986\AppData\Roaming\Typora\typora-user-images\1566359549793.png)

其中 MongoDB 分为  数据库 -- 集合 -- 文档(db-- collection -- doc)

其中 三者 关系 和 关系型数据库中的  db- table - data 一致 

#### 3.2 特点

其中 生成 db 直接 use db_name  他会等到你向数据库中插入一个 doc自动生成数据库 

#### 3.3 基本指令

1. show dbs  查看当前所有数据库  也可以用 show databases
2. use db_name  创建数据库
3. db  查看当前所处的数据库
4. show collections  显示数据库中所有的集合
5. 

#### 3.4 crud 操作(http://www.mongoing.com/docs/)

1. ```json
  db.<collection_name>.insert({doc:'json'})
  ```

    	向集合中插入数据  : db.student.insert({name:'anthony',age:18,gender:'male'})  如果我们没有插入_id 则自动生成一个 _id:ObjectID() 
- insertOne()
- insertMany()
2. ```json
   db.<collection_name>.find({_id:'1'}) 
   ```

    查询集合中所有的数据  : db.student.find()  

   - findOne().name  可以指定对象的属性名称
   - find({}).count()  聚合操作

3. ```json
   db.<collection_name>.update(<query>,<update>{'新对象':'?'}) 
   ```

   - ```json 
     db.student.update({name:'anthony'},{age:50})    全部替换  (update  默认只会修改一个)
     ```

   - ```json
     db.student.update(
         {name:'anthony'},
         {$set:{
             age:100
         }})      									只会设置修改值
     ```

   - ```json
     db.student.update(
         {name:'anthony'},
         {$unset:{
             age:随意都行
         }})    										删除数据的部分属性
     ```

   - ```json
     db.student.updateMany(
         {name:'anthony'},
         {$set:{
             age:随意都行
         }})    										可以修改多个
     ```

4. ```json
   db.<collection_name>.remove(<query>)    他删除符合条件的全部数据
   ```

   - ```json
     db.<collection_name>.remove(<query>,<justone>)   第二个参数传递一个true 就可以了  
     ```

     - db.student.remove({})  请空全部  这么比较慢
     - db.student.drop()  删除全部 

