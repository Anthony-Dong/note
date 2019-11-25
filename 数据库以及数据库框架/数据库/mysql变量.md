# MySQL - 变量

## 1. 用户变量 - set 语句赋值  

### 1. set @variable = 0 ;

```sql
SET @variable =1 ;
SELECT @variable ;
+-----------+
| @variable |
+-----------+
|         1 |
+-----------+
```

### 2. set @variable := 0 ; 

```sql
SET @variable :=1 ;
SELECT @variable as variable;
+----------+
| variable |
+----------+
|        1 |
+----------+
```



## 2. 用户变量 - select 语句赋值 

### 1. SELECT @variable := 0 AS variable ;

select 语句赋值必须使用 `:=`   , 中间没有空格

```sql
SELECT @variable := 0 AS variable ;

+----------+
| variable |
+----------+
|        0 |
+----------+


必须为 `:=` , 不然无法赋值,不赋值,默认为null,
SELECT @t1 := (@t2 := 1) +@t3 := 4 ,@t1 ,@t2 ,@t3; 
+-----------------------------+------+------+------+
| @t1 := (@t2 := 1) +@t3 := 4 | @t1  | @t2  | @t3  |
+-----------------------------+------+------+------+
|                           5 | 5    | 1    | 4    |
+-----------------------------+------+------+------+


如果我们不用 `:=`
SELECT @t1 = (@t2 = 1) + (@t3 = 4) ,@t1 ,@t2 ,@t3; 
+-----------------------------+------+------+------+
| @t1 = (@t2 = 1) + (@t3 = 4) | @t1  | @t2  | @t3  |
+-----------------------------+------+------+------+
|                        NULL | NULL | NULL | NULL |
+-----------------------------+------+------+------+
```

### 2. 小案例,做一个排名

```sql

CREATE TABLE `students` (
  `sid` int(2) NOT NULL AUTO_INCREMENT,
  `name` varchar(50) NOT NULL,
  `score` int(2) NOT NULL,
  PRIMARY KEY (`pid`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1;
 
INSERT INTO `students` (`sid`, `name`, `score`) VALUES
(1, 'Samual', 85),
(2, 'Vino', 80),
(3, 'John', 80),
(4, 'Andy', 72),
(5, 'Brian', 71),
(6, 'Dew', 74),
(7, 'Kris', 75),
(8, 'William', 76),
(9, 'George', 73),
(10, 'Peter', 99),
(11, 'Tom', 90),
(12, 'Andre', 60);
```

**按年龄排名 , 将上面的表, 并显示出排名**

```sql
SELECT
	s.sid,
	s. NAME,
	s.score,
	@count := @count + 1 AS rank
FROM
	students s,
	(SELECT @count := 0) c
ORDER BY
	s.score DESC
	
即使 set 变量存在于整个会话,但是每次都会赋初始值,所以不会影响下一次的调用. 

+-----+---------+-------+------+
| sid | NAME    | score | rank |
+-----+---------+-------+------+
|  10 | Peter   |    99 |    1 |
|  11 | Tom     |    90 |    2 |
|   1 | Samual  |    85 |    3 |
|   2 | Vino    |    80 |    4 |
|   3 | John    |    80 |    5 |
|   8 | William |    76 |    6 |
|   7 | Kris    |    75 |    7 |
|   6 | Dew     |    74 |    8 |
|   9 | George  |    73 |    9 |
|   4 | Andy    |    72 |   10 |
|   5 | Brian   |    71 |   11 |
|  12 | Andre   |    60 |   12 |
+-----+---------+-------+------+	
```



## 3. 局部变量   - 自定义函数

局部变量存在与 begin - end 之间 , 有效期仅仅限制于  begin - end 之间 ,

declare 语句专门用于定义局部变量，可以使用default来说明默认值 , 主要是申明变量的作用.

set 设置不同类型的变量  , set语句既可以用于局部变量的赋值，也可以用于用户变量的申明并赋值。

```
declare var_name [, var_name]... data_type [ DEFAULT value ];

例如 : 
declare c int default 0;

set c = a + b ; 
```



```sql
-- 创建程序
CREATE PROCEDURE csum (a INT)

BEGIN
-- 申明变量 , 赋值 
DECLARE sum INT DEFAULT 0;
DECLARE i INT DEFAULT 1;

-- 循环开始
WHILE i <= a DO	
SET sum = sum + i;
SET i = i + 1;
END WHILE;
-- 循环结束

-- 输出结果
SELECT sum;
END;

-- 调用程序
call csum(100);

-- 删除程序
drop procedure if exists csum；
```

procedure : 程序的意思 .....

我参考的[链接](https://www.jianshu.com/p/cb0152efac32) :  https://www.jianshu.com/p/cb0152efac32



## 4. set  与 declare 的区别

> set 关键字声明的变量 ,可以在一个会话的任何地方声明，作用域是整个会话，称为用户变量
>
> declare关键字声明的变量，只能在存储过程中使用，称为存储过程变量 . 
>
> 
>
> 两者的区别是： 
>
> ​		在调用存储过程时，以declare声明的变量都会被初始化为null。而会话变量（即@开头的变量）则不会被再初始化，在一个会话内，只须初始化一次，之后在会话内都是对上一次计算的结果，就相当于在是这个会话内的全局变量。





## 5. 系统变量

### 全局变量

全局变量影响服务器整体操作。当服务器启动时，它将所有全局变量初始化为默认值。

```sql
1. 设置全局变量 , 全局变量一般是 @@  
set global var_name = value;  
set @@global.var_name = value; 

2. 查看会话变量 , 通配符 '_ 或  %'
show global variables;   
show global variables like “%var%”;  


select @@global.var_name;
select @@var_name;
```

### 会话变量

服务器为每个连接的客户端维护一系列会话变量

```sql
1. 设置会话变量 
set session var_name = value;    
set @@session.var_name = value;  

2. 查看 会话变量 
show session variables;
show session variables like "%var%";

select @@var_name;
select @@session.var_name;
```

