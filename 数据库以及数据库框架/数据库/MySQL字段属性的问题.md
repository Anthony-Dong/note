# MySQL 字段属性的问题

## 1. 整型

tinyint一个字节   smallint  两个字节   MEDIUMINT三个字节  int 4个字节   BIGINT 8个字节 基本就这些了

拿 int 来举例子 : 

mysql中 int占用4个字节 , 所以是 2^32= 4294967296，因为要表示正负还有0,所以最终的表示整形范围(10进制)为-2147483648~2147483647 , 

所以这个数字已经足够大了 , 如果是自增ID ,完全可以用这个 .

然后就是我们想知道的 int(1) , int(4) 的区别了 ,我们其实有个误区就是 , 括号里的数字表示的是啥 ? , 是展示的长度,  你设置成int ,注定大小已经限制了, 1和4的区别 在于展示的长度, 这点与 varchar、char 后面的数字含义是不同的。但是怎么使用呢 ?  这个就用到了 zerofill , 比如我们将 `user_id int(4) zerofill` ,现在插入1会显示成 0001 , 而不是1 , 知道了吧 

## 2. 浮点类型

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-20/a3bd35cf-2f5e-4b45-a527-66ccb83c7217.jpg?x-oss-process=style/template01)



Decimal型的取值范围和double相同。但是decimal的有效取值范围由M和D决定，而且Decimal型的字节数是M+2。也就是说，定点数的存储空间是根据其精度决定的。

MySQL中可以指定浮点数和定点数的精度。其基本形式如下：float/double/decimal数据类型(M,D) 

M参数称为精度，是数据的总长度，小数点不占位置。D参数成为标度，是指小数点后面的长度是D。

比如 float(6.4),我们插入 9999.99就是他的最大值了 , 长度为6 ,小数为2 ,  插入长度或者小数点位数超过就会报错, 

测试一下 : 

```sql
CREATE TABLE `test` (
  `id_int` int(11) NOT NULL AUTO_INCREMENT,
  `a_float` float(6,2) DEFAULT NULL,
  `b_double` double(6,2) DEFAULT NULL,
  `c_decimal` decimal(6,2) DEFAULT NULL,
  PRIMARY KEY (`id_int`) USING BTREE
) ENGINE=InnoDB;
```

```sql
INSERT INTO test (a_float,b_double,c_decimal) VALUES (1.115,1.115,1.115);
INSERT INTO test (a_float,b_double,c_   decimal) VALUES (9999.99,9999.99,9999.99);
```

查询

```sql
mysql> select * from test;
+--------+---------+----------+-----------+
| id_int | a_float | b_double | c_decimal |
+--------+---------+----------+-----------+
|      1 |    1.12 |     1.12 |      1.12 |
|      2 | 9999.99 |  9999.99 |   9999.99 |
+--------+---------+----------+-----------+
2 rows in set (0.00 sec)
```



**浮点数和定点数有其默认的精度，float和double默认会保存实际精度，但这与操作系统和硬件的精度有关 , decimal型的默认整数位为10，小数位为0，即默认为整数**



在MySQL中，定点数以字符串形式存储，因此，其精度比浮点数要高，而且浮点数会出现误差，这是浮点数一直存在的缺陷。如果要对数据的精度要求比较高，还是选择定点数decimal比较安全。

## 3. varchar 和 char

varchar(5)  这里的5 限制的是储存字符的个数，字符不分贵贱（不分 中文、英文、数字...）。

