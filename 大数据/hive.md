# HIVE

## 介绍

![](https://hive.apache.org/images/hive_logo_medium.jpg)

[官方地址](https://hive.apache.org/)  :  https://hive.apache.org/

> ​	The Apache Hive ™ data warehouse software facilitates reading, writing, and managing large datasets residing in distributed storage using SQL. Structure can be projected onto data already in storage. A command line tool and JDBC driver are provided to connect users to Hive.

## 快速开始

```shell
这里不推荐用 2 的版本 一堆坑
tar -zxvf apache-hive-1.2.1-bin.tar.gz -C /opt/software/

mv hive-env.sh.template hive-env.sh
vim hive-env.sh
export HADOOP_HOME=/opt/software/hadoop-2.7.2
export HIVE_CONF_DIR=/opt/software/apache-hive-1.2.1-bin/conf

创建一个  hive-site.xml 这个文件 ,里面添加 
<?xml version="1.0"?>
<?xml-stylesheet type="text/xsl" href="configuration.xsl"?>
<configuration>
	<property>
	  <name>javax.jdo.option.ConnectionURL</name>
	  <value>jdbc:mysql://hadoop1:3306/hive?createDatabaseIfNotExist=true</value>
	  <description>JDBC connect string for a JDBC metastore</description>
	</property>

	<property>
	  <name>javax.jdo.option.ConnectionDriverName</name>
	  <value>com.mysql.jdbc.Driver</value>
	  <description>Driver class name for a JDBC metastore</description>
	</property>

	<property>
	  <name>javax.jdo.option.ConnectionUserName</name>
	  <value>root</value>
	  <description>username to use against metastore database</description>
	</property>

	<property>
	  <name>javax.jdo.option.ConnectionPassword</name>
	  <value>123456</value>
	  <description>password to use against metastore database</description>
	</property>
</configuration>

在lib 目录, 放入jdbc 驱动包 , 然后mysql 创建 hive 数据库

```







## bash 交互

```shell
load data local inpath '/opt/module/datas/student.txt' into table student;
```



### DDL

#### 数据库创建语句

```shell
CREATE DATABASE [IF NOT EXISTS] database_name
[COMMENT database_comment]
[LOCATION hdfs_path]
[WITH DBPROPERTIES (property_name=property_value, ...)];


1. 创建数据库语句 , 指定位置在 hdfs 上的 /test 目录下
create database test2 location '/test2';
create database if not exists test;

2. 删除数据库
drop database test;

// 非空的
drop database test cascade;

3.查询
show databases;
show databases like 'test*'

4.查询信息
desc database test;
desc database extended test;

5.修改信息
alter database db_hive set dbproperties('createtime'='20170830');
```

#### 表创建语句

```shell
CREATE [EXTERNAL] TABLE [IF NOT EXISTS] table_name 
[(col_name data_type [COMMENT col_comment], ...)] 
[COMMENT table_comment] 
[PARTITIONED BY (col_name data_type [COMMENT col_comment], ...)] 
[CLUSTERED BY (col_name, col_name, ...) 
[SORTED BY (col_name [ASC|DESC], ...)] INTO num_buckets BUCKETS] 
[ROW FORMAT row_format] 
[STORED AS file_format] 
[LOCATION hdfs_path]
[TBLPROPERTIES (property_name=property_value, ...)]
[AS select_statement]


1. 创建表 
// 意思是 字段 id , name  , 用 \t 分割开 , 存储为 textfile , 路径是 `/user/hive/warehouse/student2`
create table if not exists student2(
id int, name string
)
row format delimited fields terminated by '\t'
stored as textfile
location '/user/hive/warehouse/student2';

2. 根据查询结果创建表
create table if not exists student3 as select id, name from student;

3. 根据已经存在的表结构创建表
create table if not exists student4 like student;

4. 查询表类型
desc formatted student2
Table Type:             MANAGED_TABLE  
```



