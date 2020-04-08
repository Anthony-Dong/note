# MySQL 数据库使用注意的细节

## 1. 中文问题

> ​	今天遇见一个哥们 , 代码写的么问题 , 但是就是查询不出来 . 

```java
@SpringBootApplication
public class SpringDataApplication implements CommandLineRunner {
    @Autowired
    private JdbcTemplate template;
    public static void main(String[] args) {
        SpringApplication.run(SpringDataApplication.class, args);
    }
    @Override
    public void run(String... args) throws Exception {
        template.query("select count(*) as 'count' from words where tmxz like ?", new Object[]{"%词"}, new RowCallbackHandler() {
            @Override
            public void processRow(ResultSet rs) throws SQLException {
                String anInt = rs.getString("count");
                System.out.println(anInt);
            }
        });
    }
}
```

我简单的翻版了一下. 其实就是因为 count 查询出来的数字是 0 , 但是数据库直接查询确实可以的. 那是为什么呢, 我当时也不知道. 于是我一顿操作猛如虎, 也写了同样的代码. 其次就是`JdbcTemplate` 没有设置SQL的日志打印. 所以很坑. 此时我就去开启了 数据库的日志. 结果发现

```java
2020-03-23T13:19:57.061031Z	   42 Query	select count(*) as 'count' from words where tmxz like '%?'
```

为啥是 '%?' 呢 , 我当时也很懵逼. 于是我突然想到设置编码. 

于是修改一下就好了. 

```java
spring.datasource.url=jdbc:mysql://localhost:3306/test?useSSL=false&useUnicode=true&characterEncoding=utf-8
```

## limit 问题

我们知道 limit 查询是 `limit m, n `  , 而m是开始, n是多少条.  m为缺省值 . 也就是一个的话就是n. 

那么如果我92589 条数据, 我们去查询 `limit 90000,10` . 那么他会执行什么, 他的type是`ALL` , 表示全表扫描. 没有使用索引.  

```sql
mysql> EXPLAIN SELECT * FROM words LIMIT 90000, 10;
+----+-------------+-------+------------+------+---------------+------+---------+------+-------+----------+-------+
| id | select_type | table | partitions | type | possible_keys | key  | key_len | ref  | rows  | filtered | Extra |
+----+-------------+-------+------------+------+---------------+------+---------+------+-------+----------+-------+
|  1 | SIMPLE      | words | NULL       | ALL  | NULL          | NULL | NULL    | NULL | 92589 |   100.00 | NULL  |
+----+-------------+-------+------------+------+---------------+------+---------+------+-------+----------+-------+
1 row in set, 1 warning (0.00 sec)
```



我们发现他一共查询了 92589 多条信息. 

其次.我们修改成`SELECT * FROM words where id>90000 LIMIT 10;` , 其中只进行查询了3312条. 他的type是`range` , 代表范围扫描 , 使用主键索引

```sql
mysql> EXPLAIN SELECT * FROM words where id>90000 LIMIT  10;
+----+-------------+-------+------------+-------+---------------+---------+---------+------+------+----------+-------------+
| id | select_type | table | partitions | type  | possible_keys | key     | key_len | ref  | rows | filtered | Extra       |
+----+-------------+-------+------------+-------+---------------+---------+---------+------+------+----------+-------------+
|  1 | SIMPLE      | words | NULL       | range | PRIMARY       | PRIMARY | 4       | NULL | 3312 |   100.00 | Using where |
+----+-------------+-------+------------+-------+---------------+---------+---------+------+------+----------+-------------+
1 row in set, 1 warning (0.00 sec)
```



具体字段还不理解可以看看这篇文章 : [https://www.cnblogs.com/ambitionutil/p/11278600.html](https://www.cnblogs.com/ambitionutil/p/11278600.html)  , 还有就是5.7缺省就会输出filtered,  它指返回结果的行占需要读到的行(rows列的值)的百分比.

## 开启日志(定位问题)

对于数据库来说, 我们有的时候真的不知道啥问题. 此时就需要开启查询的日志了. 此时我们需要修改一下

查询一下日志的位置.  根目录通过 ` show variables like "%datadir%"`  查询. 

```sql
mysql> show variables like 'general_log%';
+------------------+---------------------+
| Variable_name    | Value               |
+------------------+---------------------+
| general_log      | OFF                 |
| general_log_file | DESKTOP-7H84K93.log |
+------------------+---------------------+
2 rows in set, 1 warning (0.00 sec)
```

windows默认是在 `C:\ProgramData\MySQL\MySQL Server 5.7\Data` 里面 . 

第二步就是将日志开启. 

```sql
mysql> set global general_log=on;
Query OK, 0 rows affected (0.01 sec)
```

当我们使用完一定要关闭奥 . ` set global general_log=off;`



慢查询 : 

```sql
mysql> SHOW VARIABLES LIKE 'slow_query%';
+---------------------+--------------------------+
| Variable_name       | Value                    |
+---------------------+--------------------------+
| slow_query_log      | ON                       |
| slow_query_log_file | DESKTOP-7H84K93-slow.log |
+---------------------+--------------------------+
2 rows in set, 1 warning (0.00 sec)
```

这里是慢查询日志和是否启用了. 

修改就是

```java
mysql> set global slow_query_log=on;
Query OK, 0 rows affected (0.01 sec)
```



关于日志的更多介绍可以看看  [https://www.cnblogs.com/myseries/p/10728533.html]( https://www.cnblogs.com/myseries/p/10728533.html)





## binlog

还有binlog 如何开启

配置文件

```java
# Binary Logging.
log-bin="logbin.log"
```



然后执行

```sql
mysql> SHOW VARIABLES LIKE 'log_bin';
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| log_bin       | ON    |
+---------------+-------+
```

这个代表开启了



然后查看binlog 位置

```java
mysql> SHOW MASTER logs;
+---------------+-----------+
| Log_name      | File_size |
+---------------+-----------+
| logbin.000001 |       154 |
+---------------+-----------+
1 row in set (0.00 sec)
```



然后就代表开启了. 此时我们需要读取. 

需要借助一个工具`mysqlbinlog` , 默认情况下都会有的. 

```sql
mysqlbinlog --no-defaults --database=test --base64-output=decode-rows -v logbin.000001
```

长啥样子呢

```sql
#200323 23:26:36 server id 1  end_log_pos 1767 CRC32 0xf1d602b5         Update_rows: table id 108 flags: STMT_END_F
### UPDATE `test`.`words`
### WHERE
###   @1=1
###   @2='nihao'
###   @3='111'
###   @4=1
###   @5='1'
###   @6='1'
###   @7='1'
###   @8='1'
### SET
###   @1=1
###   @2='nihao'
###   @3='111'
###   @4=1
###   @5='1'
###   @6='1'
###   @7='1111'
###   @8='1'
# at 1767
```

看不懂吧. 也就是字段没法直接显示. 

