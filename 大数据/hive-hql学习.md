# HIVE-入门学习二

> **DDL : **(Data Definition Language)  数据定义语言 , 比如 create alter drop truncate ,这些语句
>
> **DML : ** (Data Manipulation Language) 数据操作语言   其实就是我们所说的 CRUD 操作 
>
> **DCL** : (Data Control Language)  数据控制语言  , 比如一些事务操作  ,数据库权限之类的
>
> 我们的重点主要是前两个

## DDL语句

### 1. 数据库创建语句

```sql
CREATE DATABASE [IF NOT EXISTS] database_name
[COMMENT database_comment]
[LOCATION hdfs_path]
[WITH DBPROPERTIES (property_name=property_value, ...)];
```

使用 : 

```shell
1. 创建数据库语句 , 指定位置在 hdfs 上的 /test 目录下
create database test2 location '/test2';

create database if not exists test;

2. 删除数据库
drop database test;

删除非空数据库
drop database test cascade;

3.查询
show databases;
show databases like 'test*'

4.查询信息
desc database test;
desc database extended test;

hive> desc database default;
OK
default	Default Hive database	hdfs://hadoop1:9000/user/hive/warehouse	public	ROLE	
Time taken: 0.055 seconds, Fetched: 1 row(s)

5.修改数据库属性操作
alter database db_hive set dbproperties('createtime'='20170830');
```

### 2. 表创建语句

```sql
CREATE [EXTERNAL] TABLE [IF NOT EXISTS] table_name //external 外部表`EXTERNAL_TABLE`,删除表时只会删除元信息,不会删除数据. 默认是管理表`MANAGED_TABLE`
[(col_name data_type [COMMENT col_comment], ...)] //字段属性
[COMMENT table_comment] //添加注释
[PARTITIONED BY (col_name data_type [COMMENT col_comment], ...)] 
[CLUSTERED BY (col_name, col_name, ...) 
[SORTED BY (col_name [ASC|DESC], ...)] INTO num_buckets BUCKETS] 
[ROW FORMAT row_format] 
[STORED AS file_format]  // sequencefile(二进制) textfile(文本) rcfile(列式存储)
[LOCATION hdfs_path]  // HDFS 上的位置
[TBLPROPERTIES (property_name=property_value, ...)]
[AS select_statement]  // 根据查询结果创建表 或者用like可以复制表结构,但是不复制数据
```

使用 : 

```shell
1. 创建表 
// 字段 id 和 name  , 列分隔符 `\t` 分割开 , 存储格式为 textfile , 路径是 `/user/hive/warehouse/student2`
create table if not exists student2(
id int, name string
)
row format delimited fields terminated by '\t'
stored as textfile
location '/user/hive/warehouse/student2';

2. 根据查询结果创建表 , 此时执行mr ,同时也新建了一个表 ,, 也就是一个文件夹 ,同时把数据也拷贝过去了
create table if not exists student3 as select id, name from student;

3. 根据已经存在的表结构创建表 , 此时也是新建了一个表 , 也就是一个文件夹
create table if not exists student4 like student;

4. 查询表类型
desc formatted student2
Table Type:             MANAGED_TABLE  

6. 查询建表语句
show create table student;

7. 删除表
drop table student;

8.修改表为外部表
alter table student set tblproperties('EXTERNAL'='TRUE');

9. 查看属性
desc formatted student2;
```

#### 插入数据 - 本地/HDFS文件插入

```shell
1. 创建一张表
create table stu3(id int , name string) row format delimited fields terminated by '\t';

2. 创建文件
[admin@hadoop1 data]$ cat stu3.txt 
1	xiaoli
2	xiaowang
3	xiaoming

3.上传到stu3 表中 
load data  local inpath '/opt/software/apache-hive-1.2.1-bin/data/stu3.txt' into table stu3;

4.查询
hive> select * from stu3;
OK
1	xiaoli
2	xiaowang
3	xiaoming
```

查看HDFS , 发现规律 ............

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-21/4ad88603-641d-418e-aa38-955cbe928db4.jpg?x-oss-process=style/template01)

我们查看一下我们的建表语句 , 发现`/user/hive/warehouse/stu3` 就是在这里 , 所以他其实做的就是将你的` stu3.txt` 文件通过HDFS上传到了这里 

```shell
hive> show create table stu3;
OK
CREATE TABLE `stu3`(
  `id` int, 
  `name` string)
ROW FORMAT DELIMITED 
  FIELDS TERMINATED BY '\t' 
STORED AS INPUTFORMAT 
  'org.apache.hadoop.mapred.TextInputFormat' 
OUTPUTFORMAT 
  'org.apache.hadoop.hive.ql.io.HiveIgnoreKeyTextOutputFormat'
LOCATION
  'hdfs://hadoop1:9000/user/hive/warehouse/stu3'
TBLPROPERTIES (
  'COLUMN_STATS_ACCURATE'='true', 
  'numFiles'='1', 
  'totalSize'='31', 
  'transient_lastDdlTime'='1574342187')
Time taken: 0.257 seconds, Fetched: 16 row(s)
```

因此我们此时可以将 我们本地的文件 直接以建表规则的方式,通过HDFS上传到`/user/hive/warehouse/stu3` 目录下面, 

```shell
[admin@hadoop1 data]$ cat stu3_2.txt 
4	anthony
5	flex
6	xiaofan

[admin@hadoop1 data]$ hadoop fs -put stu3_2.txt /user/hive/warehouse/stu3

hive> select * from stu3;
OK
1	xiaoli
2	xiaowang
3	xiaoming
4	anthony
5	flex
6	xiaofan
```

发现完全 OK , 此时我们想 ,我们可以让 HDFS 上的文件 加载到`stu3`这张表中 , 此时

```java
1. 上传到 HDFS 
[admin@hadoop1 data]$ cp stu3.txt stu4.txt
[admin@hadoop1 data]$ ls
1.txt  stu3_2.txt  stu3.txt  stu4.txt  stu.txt
[admin@hadoop1 data]$ hadoop fs -put stu4.txt /
    
2. 将HDFS 上的'/stu4.txt' 加载到 表stu3中
hive> load data inpath '/stu4.txt' into table stu3;
Loading data to table default.stu3
Table default.stu3 stats: [numFiles=3, totalSize=89]
OK
Time taken: 0.392 seconds

3. 查询一下
hive> select * from stu3;
OK
1	xiaoli
2	xiaowang
3	xiaoming
4	anthony
5	flex
6	xiaofan
1	xiaoli
2	xiaowang
3	xiaoming
Time taken: 0.138 seconds, Fetched: 9 row(s)    
```

查看一下 HDFS : 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-21/4079531a-6b16-4d0a-8e08-83b94d096db6.jpg?x-oss-process=style/template01)

查看一下源文件 发现是移动到了 stu3 目录下面  ,但是其实是改了NameNode的元数据信息 ....

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-11-21/88392e80-3b57-4ba6-a57b-1db3847cfdf3.jpg?x-oss-process=style/template01)

### 3. 分区表

>分区类似于索引 , 可以提高查询速度 , 因为每一个分区就是一个文件夹 , 多级分区也是多级文件夹,这些都保存在数据库的元信息中

建表语句

```sql
hive (default)> create table dept_partition(
deptno int, dname string, loc string
)
partitioned by (month string) //分区规则,可以有多个,用','分开就行了
row format delimited fields terminated by '\t'
location '/hive/dept_partition';
```

插入数据 ..

```sql
load data inpath '/hive/dept.txt' into table default.dept_partition partition(month='201709');
```

查询语句 : 

```sql
select * from dept_partition where month='201709';
```

增加分区 

```sql
alter table dept_partition add partition(month='201706') ;

alter table dept_partition add partition(month='201705') partition(month='201704');
```

删除分区 

```sql
 alter table dept_partition drop partition (month='201704');
 
 alter table dept_partition drop partition (month='201705'), partition (month='201706');
```

查看分区 

```sql
show partitions dept_partition;
```



创建多级分区 : 

```sql
hive (default)> create table dept_partition2(
               deptno int, dname string, loc string
               )
               partitioned by (month string, day string)
               row format delimited fields terminated by '\t';    
```

添加数据:

```sql
hive (default)> load data local inpath '/opt/module/datas/dept.txt' into table default.dept_partition2 partition(month='201709', day='13');
```

查询

```sql
select * from dept_partition2 where month='201709' and day='13';
```

直接上传到HDFS 上 

1) 上传后修复数据 

```sql
 msck repair table dept_partition2;
```

2) 上传后添加分区

```sql
 alter table dept_partition2 add partition(month='201709',day='11');
```

3) 添加数据直接

```sql
load data local inpath '/opt/module/datas/dept.txt' into table dept_partition2 partition(month='201709',day='10');
```



### 4. 修改表

1) 修改表的名字

```sql
alter table table_name rename to new_table_name ; 
```

2) 添加列

```sql
alter table table_name add columns(deptdesc string);
```

3) 更新列

```sql
alter table dept_partition change column deptdesc desc int;
```

## DML 语句

### 1. 添加数据

```sql
load data [local] inpath '/opt/module/datas/student.txt' [overwrite] into table student [partition (partcol1=val1,…)];

（1）load data:表示加载数据
（2）local:表示从本地加载数据到hive表；不写则从HDFS加载数据
（3）inpath:表示加载数据的路径
（4）overwrite:表示覆盖表中已有数据，否则表示追加
（5）into table:表示加载到哪张表
（6）student:表示具体的表
（7）partition:表示上传到指定分区

 create table student(id int, name string) stored as textfile partitioned by (month string) row format delimited fields terminated by '\t' location '/hive/student';
```

### 2. 插入语句

```sql
1. 基本插入
insert into table  student partition(month='201709') values(1,'wangwu'),(2,’zhaoliu’);

2. 根据查询结果覆盖
insert overwrite table student partition(month='201708') select id, name from student where month='201709';

3. 创建表
create table if not exists student3 as select id, name from student;


```

### 3. 查询语句

1) 支持 as

```sql
select ename AS name, deptno dn from emp;
```

2) 支持运算符操作

```sql
select sal +1 from emp;
```

3) 支持常见的聚合函数操作

```sql
count(*)
max(sal)
min(sal)
 sum(sal)
 avg(sal)
```

4) 支持 limit语句

```sql
select * from emp limit 5;
```

5) 支持where

```sql
select * from emp where sal >1000;
```

6) 比较符  

| 操作符                  | 支持的数据类型 | 描述                                                         |
| ----------------------- | -------------- | ------------------------------------------------------------ |
| A=B                     | 基本数据类型   | 如果A等于B则返回TRUE，反之返回FALSE                          |
| A<=>B                   | 基本数据类型   | 如果A和B都为NULL，则返回TRUE，其他的和等号（=）操作符的结果一致，如果任一为NULL则结果为NULL |
| A<>B,   A!=B            | 基本数据类型   | A或者B为NULL则返回NULL；如果A不等于B，则返回TRUE，反之返回FALSE |
| A<B                     | 基本数据类型   | A或者B为NULL，则返回NULL；如果A小于B，则返回TRUE，反之返回FALSE |
| A<=B                    | 基本数据类型   | A或者B为NULL，则返回NULL；如果A小于等于B，则返回TRUE，反之返回FALSE |
| A>B                     | 基本数据类型   | A或者B为NULL，则返回NULL；如果A大于B，则返回TRUE，反之返回FALSE |
| A>=B                    | 基本数据类型   | A或者B为NULL，则返回NULL；如果A大于等于B，则返回TRUE，反之返回FALSE |
| A [NOT] BETWEEN B AND C | 基本数据类型   | 如果A，B或者C任一为NULL，则结果为NULL。如果A的值大于等于B而且小于或等于C，则结果为TRUE，反之为FALSE。如果使用NOT关键字则可达到相反的效果。 |
| A IS NULL               | 所有数据类型   | 如果A等于NULL，则返回TRUE，反之返回FALSE                     |
| A IS NOT   NULL         | 所有数据类型   | 如果A不等于NULL，则返回TRUE，反之返回FALSE                   |
| IN(数值1, 数值2)        | 所有数据类型   | 使用 IN运算显示列表中的值                                    |
| A [NOT]   LIKE B        | STRING 类型    | B是一个SQL下的简单正则表达式，也叫通配符模式，如果A与其匹配的话，则返回TRUE；反之返回FALSE。B的表达式说明如下：‘x%’表示A必须以字母‘x’开头，‘%x’表示A必须以字母’x’结尾，而‘%x%’表示A包含有字母’x’,可以位于开头，结尾或者字符串中间。如果使用NOT关键字则可达到相反的效果。 |
| A RLIKE B, A REGEXP B   | STRING 类型    | B是基于java的正则表达式，如果A与其匹配，则返回TRUE；反之返回FALSE。匹配使用的是JDK中的正则表达式接口实现的，因为正则也依据其中的规则。例如，正则表达式必须和整个字符串A相匹配，而不是只需与其字符串匹配。 |

```sql
 select * from emp where sal LIKE '2%';
 select * from emp where sal LIKE '_2%';
 select * from emp where sal RLIKE '[2]';
```

7) 逻辑运算符 

| 操作符 | 含义   |
| ------ | ------ |
| AND    | 逻辑并 |
| OR     | 逻辑或 |
| NOT    | 逻辑否 |

```sql
select * from emp where sal>1000 and deptno=30;
select * from emp where sal>1000 or deptno=30;
select * from emp where deptno not IN (30, 20);
```

8) 排序 

**Order By**：全局排序，只有一个Reducer , 

```sql
select * from emp order by sal desc;
```

**Sort By**：对于大规模的数据集order by的效率非常低。在很多情况下，并不需要全局排序，此时可以使用**sort by**。Sort by为每个reducer产生一个排序文件。每个Reducer内部进行排序，对全局结果集来说不是排序。

```sql
设置reduce数量
set mapreduce.job.reduces=3;

查看数量
set mapreduce.job.reduces;

select * from emp sort by deptno desc;
```

**Distribute By**： 在有些情况下，我们需要控制某个特定行应该到哪个reducer，通常是为了进行后续的聚集操作。**distribute by** 子句可以做这件事。**distribute by**类似MR中partition（自定义分区），进行分区，结合sort by使用。 

```sql
hive (default)> set mapreduce.job.reduces=3;
hive (default)> insert overwrite local directory '/opt/module/datas/distribute-result' select * from emp distribute by deptno sort by empno desc;
```

注意：

1．  distribute by的分区规则是根据分区字段的hash码与reduce的个数进行模除后，余数相同的分到一个区。

2．  Hive要求DISTRIBUTE BY语句要写在SORT BY语句之前。





### 4. join 操作

> ​	Hive支持通常的SQL JOIN语句，但是只支持等值连接，不支持非等值连接。
>
> （1）使用别名可以简化查询。
>
> （2）使用表名前缀可以提高执行效率。

```sql
select e.empno, e.ename, d.deptno, d.dname from emp e join dept d on e.deptno = d.deptno;
```



#### 多表

```sql
hive (default)>SELECT e.ename, d.dname, l.loc_name
FROM   emp e 
JOIN   dept d
ON     d.deptno = e.deptno 
JOIN   location l
ON     d.loc = l.loc;
```

大多数情况下，**Hive会对每对JOIN连接对象启动一个MapReduce任务**。本例中会首先启动一个MapReduce job对表e和表d进行连接操作，然后会再启动一个MapReduce job将第一个MapReduce job的输出和表l;进行连接操作。

注意：为什么不是表d和表l先进行连接操作呢？这是因为Hive总是按照从左到右的顺序执行的。

优化：当对3个或者更多表进行join连接时，如果每个on子句都使用相同的连接键的话，那么只会产生一个MapReduce job。

**hive join目前不支持在on子句中使用谓词or**



### 5. 分桶 

> ​	Hive的分桶采用对分桶字段的值进行哈希，然后除以桶的个数求余的方式决定该条记录存放在哪个桶当中

```sql
create table stu_buck(id int, name string)
clustered by(id) 
into 4 buckets
row format delimited fields terminated by '\t';

desc formatted stu_buck;

必须设置前两部
set hive.enforce.bucketing=true;

set mapreduce.job.reduces=1;

insert into table stu_buck select id, name from stu;
```

#### 分桶抽样查询

对于非常大的数据集，有时用户需要使用的是一个具有代表性的查询结果而不是全部结果。Hive可以通过对表进行抽样来满足这个需求。

```sql
select * from stu_buck tablesample(bucket 1 out of 4 on id);
```

tablesample是抽样语句，语法：TABLESAMPLE(BUCKET x OUT OF y) 

x表示从哪个bucket开始抽取，如果需要取多个分区，以后的分区号为当前分区号加上y ,x的值必须小于等于y的值 , 

y必须是table总bucket数的倍数或者因子。 

