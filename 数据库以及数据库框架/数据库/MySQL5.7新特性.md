# MySQL 5.7 - Json

> ​	MySQL 5.7提供了对于json的支持 , 所以可以学学, [官方文档链接](https://dev.mysql.com/doc/refman/5.7/en) : https://dev.mysql.com/doc/refman/5.7/en , 有兴趣可以学学 , 此时需要注意如果你不喜欢命令窗口的, 你可以用navicat ,注意要使用12+的版本 . 

## 1. JSON_ARRAY

> ​	就是将字符串转为数组 , 语法`JSON_ARRAY([val,[val,val...],val ...])` , 或者语法糖直接 `[1,2,3,4,[5,6,7,8],9]`

### 1. 简单使用

1) 基本写法

```sql
mysql> SELECT JSON_ARRAY(1, "abc", NULL, TRUE, CURTIME());

+---------------------------------------------+
| JSON_ARRAY(1, "abc", NULL, TRUE, CURTIME()) |
+---------------------------------------------+
| [1, "abc", null, true, "16:20:47.000000"]   |
+---------------------------------------------+
1 row in set (0.00 sec)
```

2) 嵌套写法

```sql
mysql> select json_array(1,2,3,4,json_array(1,2,3),10);
+------------------------------------------+
| json_array(1,2,3,4,json_array(1,2,3),10) |
+------------------------------------------+
| [1, 2, 3, 4, [1, 2, 3], 10]              |
+------------------------------------------+
1 row in set (0.00 sec)
```

3) 自动识别

```sql
mysql> select '[1,2,3,4,[1,2,3,4],5]';
+-----------------------+
| [1,2,3,4,[1,2,3,4],5] |
+-----------------------+
| [1,2,3,4,[1,2,3,4],5] |
+-----------------------+
1 row in set (0.00 sec)
```

### 2. 表中插入数据

1) 基本写法

```sql
mysql> insert into tj10 (a,b) values (JSON_ARRAY(1,2,3,4,6),111);
Query OK, 1 row affected (0.01 sec)

mysql> select * from tj10 where b=111 ;
+-----------------+------+
| a               | b    |
+-----------------+------+
| [1, 2, 3, 4, 6] |  111 |
+-----------------+------+
1 row in set (0.00 sec)
```

2)  自动识别写法

```sql
mysql> insert into tj10(a,b) values ('[1,2,3,4,[1,2,3,4]]',2);
Query OK, 1 row affected (0.01 sec)

mysql> select a from tj10 where b=2;
+----------------------------+
| a                          |
+----------------------------+
| [1, 2, 3, 4, [1, 2, 3, 4]] |
+----------------------------+
1 row in set (0.00 sec)
```

3) 查询写法

```sql
mysql> select a->"$[1]" from tj10 where b=2;
+-----------+
| a->"$[1]" |
+-----------+
| 2         |
+-----------+
1 row in set (0.00 sec)
```

## 2. JSON_OBJECT

对象结构 , 写法 : `JSON_OBJECT(key, val,[ key, val] ...)`

```sql
mysql> SELECT JSON_OBJECT('id', 87, 'name', 'carrot');
+-----------------------------------------+
| JSON_OBJECT('id', 87, 'name', 'carrot') |
+-----------------------------------------+
| {"id": 87, "name": "carrot"}            |
+-----------------------------------------+
1 row in set (0.00 sec)
```

直接写法 

```sql
mysql> select ('{"name":"xiaowang","age":18}') as info;
+------------------------------+
| info                         |
+------------------------------+
| {"name":"xiaowang","age":18} |
+------------------------------+
1 row in set (0.00 sec)
```

## 3. JSON_QUOTE

> ​	用法 :  `JSON_QUOTE(String)`

将一个字符串转换为json ,,直白一点就是加引号

```sql
mysql> SELECT JSON_QUOTE('null'), JSON_QUOTE('"null"');
+--------------------+----------------------+
| JSON_QUOTE('null') | JSON_QUOTE('"null"') |
+--------------------+----------------------+
| "null"             | "\"null\""           |
+--------------------+----------------------+
1 row in set (0.00 sec)
```

直白一点就是去引号 .........有给你去, 没有不给你去 , 就是这么屌 , 

```sql
mysql> select json_unquote( "\"null\"");
+---------------------------+
| json_unquote( "\"null\"") |
+---------------------------+
| null                      |
+---------------------------+
1 row in set (0.00 sec)
```



## 4. JSON_EXTRACT

> ​	 用法 : `JSON_EXTRACT(json_doc, path ,path ...)`

> ​	Returns data from a JSON document, selected from the parts of the document matched by the *path* arguments.
>
> ​	从JSON中返回数据 , 从给定的路径 ,`$`代表当前对象

### 1. 简单使用

1) 例子一

```sql
mysql> SELECT JSON_EXTRACT('[10, 20, [30, 40]]', '$[1]');
+--------------------------------------------+
| JSON_EXTRACT('[10, 20, [30, 40]]', '$[1]') |
+--------------------------------------------+
| 20                                         |
+--------------------------------------------+
1 row in set (0.00 sec)
```

2) 例子二

```sql
mysql> SELECT JSON_EXTRACT('[10, 20, [30, 40]]', '$[2][*]');
+-----------------------------------------------+
| JSON_EXTRACT('[10, 20, [30, 40]]', '$[2][*]') |
+-----------------------------------------------+
| [30, 40]                                      |
+-----------------------------------------------+
1 row in set (0.00 sec)
```

3) 例子三

```sql
mysql> SELECT JSON_EXTRACT('[10, 20, [30, 40]]', '$[1]', '$[0]');
+----------------------------------------------------+
| JSON_EXTRACT('[10, 20, [30, 40]]', '$[1]', '$[0]') |
+----------------------------------------------------+
| [20, 10]                                           |
+----------------------------------------------------+
1 row in set (0.00 sec)

```

### 2. 语法糖  : column->path

文档写的太好我懒得翻译了 ........

> ​	In MySQL 5.7.9 and later, the [`->`](https://dev.mysql.com/doc/refman/5.7/en/json-search-functions.html#operator_json-column-path) operator serves as an alias for the [`JSON_EXTRACT()`](https://dev.mysql.com/doc/refman/5.7/en/json-search-functions.html#function_json-extract) function when used with two arguments, a column identifier on the left and a JSON path on the right that is evaluated against the JSON document (the column value). You can use such expressions in place of column identifiers wherever they occur in SQL statements.

字段属性 : 

```sql
mysql> select * from tj10 where b=222;
+------------------------------+------+
| a                            | b    |
+------------------------------+------+
| {"age": 2, "name": "xiaoli"} |  222 |
+------------------------------+------+
1 row in set (0.00 sec)
```

查询name

```sql
mysql> select a->"$.name" from tj10 where b=222;
+-------------+
| a->"$.name" |
+-------------+
| "xiaoli"    |
+-------------+
1 row in set (0.00 sec)
```

可以用在where语句中

```sql
mysql> select * from tj10 where a->"$.name"="xiaoli";
+------------------------------+------+
| a                            | b    |
+------------------------------+------+
| {"age": 1, "name": "xiaoli"} |   10 |
| {"age": 2, "name": "xiaoli"} |  222 |
+------------------------------+------+
2 rows in set (0.00 sec)
```



### 3. 语法糖 : column->>path

> ​	This is an improved, unquoting extraction operator available in MySQL 5.7.13 and later. Whereas the `->` operator simply extracts a value, the `->>` operator in addition unquotes the extracted result. In other words, given a [`JSON`](https://dev.mysql.com/doc/refman/5.7/en/json.html) column value *column* and a path expression *path*, the following three expressions return the same value:
>
> - 开始 : `JSON_UNQUOTE( JSON_EXTRACT(column, path) )`
> - 进阶 : `JSON_UNQUOTE(column ->path)`
> - 最后 : `column->>path`
>
> The `->>` operator can be used wherever `JSON_UNQUOTE(JSON_EXTRACT())` would be allowed. This includes (but is not limited to) `SELECT` lists, `WHERE` and`HAVING` clauses, and `ORDER BY` and `GROUP BY` clauses.

其实就是去除引号 ... quote就是引号的意思

```sql
mysql> select a->>"$.name" from tj10 where b=222;
+--------------+
| a->>"$.name" |
+--------------+
| xiaoli       |
+--------------+
1 row in set (0.00 sec)
```



## 5. JSON_KEYS

> ​	写法 :  `JSON_KEYS(json_doc , [path])`

> ​	Returns the keys from the top-level value of a JSON object as a JSON array, or, if a *path* argument is given, the top-level keys from the selected path. 

1) 不嵌套的

```sql
mysql> select json_keys(a) from tj10 where b=10;
+-----------------+
| json_keys(a)    |
+-----------------+
| ["age", "name"] |
+-----------------+
1 row in set (0.00 sec)
```

2)  嵌套的

> ​	If the top-level value has nested subobjects, the return value does not include keys from those subobjects.

```sql
mysql>  SELECT JSON_KEYS('{"a": 1, "b": {"c": 30}}');
+---------------------------------------+
| JSON_KEYS('{"a": 1, "b": {"c": 30}}') |
+---------------------------------------+
| ["a", "b"]                            |
+---------------------------------------+
1 row in set (0.00 sec)
```

3) 携带路径

```sql
mysql> SELECT JSON_KEYS('{"a": 1, "b": {"c": 30}}',"$.b");
+---------------------------------------------+
| JSON_KEYS('{"a": 1, "b": {"c": 30}}',"$.b") |
+---------------------------------------------+
| ["c"]                                       |
+---------------------------------------------+
1 row in set (0.00 sec)
```

## 6. JSON_SEARCH

> ​	Returns the path to the given string within a JSON document. 
>
> ​	`'one'`: The search terminates after the first match and returns one path string. It is undefined which match is considered first.
>
> ​	`'all'`: The search returns all matching path strings such that no duplicate paths are included. If there are multiple strings, they are autowrapped as an array. The order of the array elements is undefined.
>
> ​	Within the *search_str* search string argument, the `%` and `_` characters work as for the [`LIKE`](https://dev.mysql.com/doc/refman/5.7/en/string-comparison-functions.html#operator_like) operator: `%` matches any number of characters (including zero characters), and `_` matches exactly one character.

写法 : `JSON_SEARCH(json_doc, one_or_all, search_str[, escape_char[, path] ...])`



使用

```sql
mysql> SET @j = '["abc", [{"k": "10"}, "def"], {"x":"abc"}, {"y":"bcd"}]';
Query OK, 0 rows affected (0.00 sec)

mysql> select json_search(@j,'one','a%');
+----------------------------+
| json_search(@j,'one','a%') |
+----------------------------+
| "$[0]"                     |
+----------------------------+
1 row in set (0.00 sec)
```



## 7. 整合Mybatis



```java
21:33:57.176 [main] DEBUG org.apache.ibatis.transaction.jdbc.JdbcTransaction - Opening JDBC Connection
21:33:57.453 [main] DEBUG org.apache.ibatis.datasource.pooled.PooledDataSource - Created connection 1371006431.
21:33:57.464 [main] DEBUG com.mybatis.mapper.UserMapper.insertIntoUser - ==>  Preparing: INSERT INTO `user` (info) VALUES (?) 
21:33:57.506 [main] DEBUG com.mybatis.mapper.UserMapper.insertIntoUser - ==> Parameters: {"age":11,"hobby":["football","dance","song"],"name":"tom"}(String)
21:33:57.515 [main] DEBUG com.mybatis.mapper.UserMapper.insertIntoUser - <==    Updates: 1
```





```java
21:38:04.329 [main] DEBUG com.mybatis.mapper.UserMapper.findByName - ==>  Preparing: SELECT info FROM `user` WHERE info -> "$.name" = ? 
21:38:04.363 [main] DEBUG com.mybatis.mapper.UserMapper.findByName - ==> Parameters: tom(String)
21:38:04.386 [main] DEBUG com.mybatis.mapper.UserMapper.findByName - <==      Total: 1
Bean{name='tom', age='11', hobby=[football, dance, song]}
```

