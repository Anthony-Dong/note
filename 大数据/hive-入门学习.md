# HIVE

## 1. 介绍

![](https://hive.apache.org/images/hive_logo_medium.jpg)

[官方地址](https://hive.apache.org/)  :  https://hive.apache.org/ , `HIVE` 汉语意思是 `蜂巢`

> ​	The Apache Hive  data warehouse software facilitates reading, writing, and managing large datasets residing in distributed storage using SQL. Structure can be projected onto data already in storage. A command line tool and JDBC driver are provided to connect users to Hive.
>
> HIVE数据仓库软件可以使用SQL语句方便的读取,写入和管理分布存储中的大型数据集。(`reside in`  属于 , 居住于) ; **数据和结构(SQL)可以相互映射起来(类似于mybatis的mapper)** , 提供了一个命令行工具和JDBC驱动程序来将用户连接到Hive。
>
> 

## 2. 快速学习

> ​	如果建表语句不会的话可以往后翻去稍微学习一下............

### 1. 安装

```shell
1. 这里不推荐用 2.x 的版本 一堆坑
tar -zxvf apache-hive-1.2.1-bin.tar.gz -C /opt/software/

mv hive-env.sh.template hive-env.sh

vim hive-env.sh
export HADOOP_HOME=/opt/software/hadoop-2.7.2
export HIVE_CONF_DIR=/opt/software/apache-hive-1.2.1-bin/conf


2. 在conf/目录下面创建一个 hive-site.xml 这个文件 ,里面添加 

<?xml version="1.0"?>
<?xml-stylesheet type="text/xsl" href="configuration.xsl"?>
<configuration>
	<property>
	  <name>javax.jdo.option.ConnectionURL</name>
	  <value>jdbc:mysql://hadoop1:3306/metastore?createDatabaseIfNotExist=true</value>
	  <!--  
     如果你是 mysql  5.5.45+, 5.6.26+ and 5.7.6+ , 需要改成 `&amp;` 意思就是 `&` 的意思不过xml
     需要写成这样子 .....
     jdbc:mysql://hadoop1:3306/metastore?createDatabaseIfNotExist=true&amp;useSSL=false
      -->
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


在/lib 目录, 放入jdbc 驱动包 , 然后mysql 创建 metastore 数据库 , 注意版本必须一致
我的是 mysql5.7 用的jar包是 mysql-connector-java-5.1.47.jar  这里不对后期很坑

3. 配置日志文件
在 conf 目录下 : 
mv hive-log4j.properties.template hive-log4j.properties
vim hive-log4j.properties 
hive.log.dir=/opt/software/apache-hive-1.2.1-bin/logs

最后记得创建文件夹 .... 默认是在 /tmp/${user}/hive.log


4. 然后 bin/hive 启动

5. 查看配置信息 : 
hive>set;
太多了 ....
```

默认配置文件：hive-default.xml 

用户自定义配置文件：hive-site.xml

 注意：用户自定义配置会覆盖默认配置。另外，Hive也会读入Hadoop的配置，因为Hive是作为Hadoop的客户端启动的，Hive的配置会覆盖Hadoop的配置。配置文件的设定对本机启动的所有Hive进程都有效。

### 2. bin/hive - 本地交互

> ​	最简单的命令行交互

### 3. bin/hiveserver2 - 支持第三方连接

> ​	hiveserver2  其实就是一个服务器端, 可以连接多个远程第三方客户端 

### 4. bin/beeline 作为第三方连接

> ​	他可以作为第三方进行连接 hive

```java
[admin@hadoop1 apache-hive-1.2.1-bin]$ bin/beeline 
Beeline version 1.2.1 by Apache Hive
// 连接命令 `!connect jdbc:hive2://hadoop1:10000` , 后面输入数据库用户名 密码
beeline> !connect jdbc:hive2://hadoop1:10000
Connecting to jdbc:hive2://hadoop1:10000
Enter username for jdbc:hive2://hadoop1:10000: root
Enter password for jdbc:hive2://hadoop1:10000: ***********
Connected to: Apache Hive (version 1.2.1)
Driver: Hive JDBC (version 1.2.1)
Transaction isolation: TRANSACTION_REPEATABLE_READ , 就好了
0: jdbc:hive2://hadoop1:10000> show databases;
+----------------+--+
| database_name  |
+----------------+--+
| default        |
+----------------+--+
1 row selected (0.777 seconds)
0: jdbc:hive2://hadoop1:10000> 
```

### 5. 命令行交互 

> ​	有的时候我们不需要客户端, 希望写一个脚本去执行 , 这时候就可以用下面说得了

```java
1. 查看命令介绍  ,常用 -e  -f
[admin@hadoop1 apache-hive-1.2.1-bin]$ bin/hive -help
usage: hive
 -d,--define <key=value>          Variable subsitution to apply to hive
                                  commands. e.g. -d A=B or --define A=B
    --database <databasename>     Specify the database to use
 -e <quoted-query-string>         SQL from command line
 -f <filename>                    SQL from files
 -H,--help                        Print help information
    --hiveconf <property=value>   Use value for given property
    --hivevar <key=value>         Variable subsitution to apply to hive
                                  commands. e.g. --hivevar A=B
 -i <filename>                    Initialization SQL file
 -S,--silent                      Silent mode in interactive shell
 -v,--verbose                     Verbose mode (echo executed SQL to the
                                  console)

2. 命令行运行
[admin@hadoop1 apache-hive-1.2.1-bin]$ bin/hive -e  "select * from stu3;"
Logging initialized using configuration in jar:file:/opt/software/apache-hive-1.2.1-bin/lib/hive-common-1.2.1.jar!/hive-log4j.properties        
OK
1	xiaoli
2	xiaowang
3	xiaoming
4	anthony
5	flex
6	xiaofan
Time taken: 2.804 seconds, Fetched: 6 row(s)    
    
3. 执行sql脚本 
[admin@hadoop1 data]$ cat sql.sff 
use default;
select * from stu3;

[admin@hadoop1 data]$ hive -f ./sql.sff 
Logging initialized using configuration in jar:file:/opt/software/apache-hive-1.2.1-bin/lib/hive-common-1.2.1.jar!/hive-log4j.properties
OK
Time taken: 1.113 seconds
OK
1	xiaoli
2	xiaowang
3	xiaoming
4	anthony
5	flex
6	xiaofan
Time taken: 1.023 seconds, Fetched: 6 row(s)   
```



## 3. HIVE的数据类型 

### 1. 基本数据类型

| Hive数据类型 | Java数据类型 | 长度                                      | 例子               |
| ------------ | ------------ | ----------------------------------------- | ------------------ |
| TINYINT      | byte         | 1byte有符号整数   8位                     | 20                 |
| SMALINT      | short        | 2byte有符号整数   16位                    | 20                 |
| INT          | int          | 4byte有符号整数    32位                   | 20                 |
| BIGINT       | long         | 8byte有符号整数    64位                   | 20                 |
| BOOLEAN      | boolean      | 布尔类型，true或者false                   | TRUE  FALSE        |
| FLOAT        | float        | 单精度浮点数    4byte  32位  8个有效数字  | 11.323444          |
| DOUBLE       | double       | 双精度浮点数    8byte  64位  17个有效数字 | 11.323444366455078 |
| STRING       | string       | 字符系列                                  | “You look great”   |
| TIMESTAMP    |              | 时间类型                                  |                    |
| BINARY       |              | 字节数组                                  |                    |

对于Hive的String类型相当于数据库的varchar类型，该类型是一个可变的字符串，不过它不能声明其中最多能存储多少个字符，理论上它可以存储2GB的字符数。

### 2. 集合类型

| 数据类型 | 描述                                                         | 语法示例                                          |
| -------- | ------------------------------------------------------------ | ------------------------------------------------- |
| STRUCT   | 和c语言中的struct类似，都可以通过“点”符号访问元素内容。例如，如果某个列的数据类型是STRUCT{first STRING, last   STRING},那么第1个元素可以通过字段.first来引用。 | struct()   例如struct<street:string, city:string> |
| MAP      | MAP是一组键-值对元组集合，使用数组表示法可以访问数据。例如，如果某个列的数据类型是MAP，其中键->值对是’first’->’John’和’last’->’Doe’，那么可以通过字段名[‘last’]获取最后一个元素 | map()   例如map<string, int>                      |
| ARRAY    | 数组是一组具有相同类型和名称的变量的集合。这些变量称为数组的元素，每个数组元素都有一个编号，编号从零开始。例如，数组值为[‘John’, ‘Doe’]，那么第2个元素可以通过数组名[1]进行引用。 | Array()   例如array<string>                       |

Hive有三种复杂数据类型**ARRAY、MAP 和 STRUCT**。ARRAY和MAP与Java中的Array和Map类似，而STRUCT与C语言中的Struct类似，它封装了一个命名字段集合，复杂数据类型允许任意层次的嵌套。

#### 练习一下

这是我们的数据 , 需要上传到hive中

```json
{
    "name": "songsong",
    "friends": ["bingbing" , "lili"] ,       //列表Array, 
    "children": {                      //键值Map,
        "xiao song": 18 ,
        "xiaoxiao song": 19
    }
    "address": {                      //结构Struct,
        "street": "hui long guan" ,
        "city": "beijing" 
    }
}

其实 struct 和 map其实很相似 , 只是一个需要key值
```

我们将表改成可以分割的格式

```text
songsong,bingbing_lili,xiao song:18_xiaoxiao song:19,hui long guan_beijing
yangyang,caicai_susu,xiao yang:18_xiaoxiao yang:19,chao yang_beijing
```

建表语句 , 如果不会后期会讲解 ,我感觉应该可以看得懂

```sql
create table test(
name string,
friends array<string>,
children map<string, int>,
address struct<street:string, city:string>
)
row format delimited fields terminated by ',' //列分隔符
collection items terminated by '_' //map,array和struct分隔符(几个内部列分隔符其实是)
map keys terminated by ':' // map kv分隔符
lines terminated by '\n';  // 行分隔符
```

插入这个文档并且查询 : 

```java
hive> load data local inpath '/opt/software/apache-hive-1.2.1-bin/data/test.txt' into table test;
Loading data to table default.test
Table default.test stats: [numFiles=1, totalSize=144]
OK
Time taken: 1.584 seconds
hive> select * from test;
OK
songsong	["bingbing","lili"]	{"xiao song":18,"xiaoxiao song":19}	{"street":"hui long guan","city":"beijing"}
yangyang	["caicai","susu"]	{"xiao yang":18,"xiaoxiao yang":19}	{"street":"chao yang","city":"beijing"}
Time taken: 0.578 seconds, Fetched: 2 row(s)
```

发现没有啥问题 ........... 数据展示格式如上 .

### 3. 数据类型转换

（1）任何整数类型都可以隐式地转换为一个范围更广的类型，如TINYINT可以转换成INT，INT可以转换成BIGINT。**可以向上转换**

（2）所有整数类型、FLOAT和STRING类型都可以隐式地转换成DOUBLE。

（3）TINYINT、SMALLINT、INT都可以转换为FLOAT。

（4）BOOLEAN类型不可以转换为任何其它的类型。

#### 测试

```sql
hive> select '1'+2, cast('1'as int) + 2;
OK
3.0	3
Time taken: 0.286 seconds, Fetched: 1 row(s)
```

如果一个 string 类型和一个数字相互加减 , 自动转换为double类型 ,(前提是string可以转换成功,别'aa'去做转换)

可以通过cast('1' as int) 显示的进行数据类型转换

