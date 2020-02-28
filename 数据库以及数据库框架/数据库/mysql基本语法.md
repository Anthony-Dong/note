# MySQL - 基本语法

## 1. MySQL 语法分类

### 1.sql92 (这个可以淘汰)

### 2.sql99(学习这个)

### 3.区别 下面来个练习

```sql
1. sql92的
SELECT ename,dname  FROM ept AS e , dept AS d WHERE e.dept_id=d.id
其中","就是笛卡尔积在sql92中

2. sql99的 需要 一个关键字 "join"  来实现笛卡尔积  (内连接 的 等值连接)
SELECT
	e.ename,
	d.dname
FROM
	ept e
JOIN dept d ON e.dept_id = d.id

只是省略了 inner 这个关键字

SELECT
	e.ename,
	d.dname
FROM
	ept e
INNER JOIN dept d ON e.dept_id = d.id
```

- 区别1  实现笛卡尔积的方式不同 一个是 ",  和 where条件"  一个是 "join 和 on 条件"

## 2. MySQL  查询语句执行循序

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-11-01/46448cf7-cabd-4a93-bbeb-eac333dc6d5a.jpg)

## 3. Join 表的连接

### 1. 内连接(jion/inner jion)

- 等值连接(连接条件是 a.filed=b.filed)

  ```sql
  1. 需求 : 根据员工表的部门名称id查询出部门表中部门名称
  
  2.查询出 员工表的员工名称和部门id
  mysql> select ename,dept_id from ept;
  +-------+---------+
  | ename | dept_id |
  +-------+---------+
  | 小明  |       1 |
  | 小王  |       2 |
  | 小李  |       3 |
  | 小钱  |       2 |
  +-------+---------
  
  3. 查询出了 部门表中部门名称和部门id
  mysql> select id , dname from dept;
  +----+-------+
  | id | dname |
  +----+-------+
  |  1 | 运维  |
  |  2 | 开发  |
  |  3 | 产品  |
  |  4 | 前端  |
  +----+-------+
  
  4. 我们可以将两个表连接起来 然后 第一章标的dept_id=B表的id 就好了,这就是等值连接
  SELECT
  	e.ename,
  	d.dname
  FROM
  	ept e
  INNER JOIN dept d ON e.dept_id = d.id
  
  +-------+-------+
  | ename | dname |
  +-------+-------+
  | 小明  | 运维  |
  | 小王  | 开发  |
  | 小李  | 产品  |
  | 小钱  | 开发  |
  +-------+-------+
  ```

  

- 非等值连接 (连接条件不是等值)

  ```sql
  SELECT
  	e.ename,e.salary,s.level
  FROM
  	ept e
  INNER JOIN
  	salary_level s
  ON
  	e.salary BETWEEN s.salary-449 AND s.salary
  ```

- 自连接(自己连自己)   把两次查询的结果当做单独的表

  ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-03/cef2c59f-27ce-4f70-a61d-728757c287ce.png?x-oss-process=style/template01)

  ```sql
  mysql> select ename,p_id from ept;
  +-------+------+
  | ename | p_id |
  +-------+------+
  | 小明  | NULL |
  | 小王  |    1 |
  | 小李  |    2 |
  | 小钱  |    1 |
  +-------+------+
  
  mysql> select id , ename from ept;
  +----+-------+
  | id | ename |
  +----+-------+
  |  1 | 小明  |
  |  2 | 小王  |
  |  3 | 小李  |
  |  4 | 小钱  |
  +----+-------+
  
  我们可以看成  第一个表为 A表 ;第二个表为B表;只要A表的 p_id 等于 B 表的 id  就可以实现了
  
  
  SELECT
  	a.ename,
  	b.ename as leader_name
  FROM
  	ept a
  INNER JOIN ept b ON a.p_id = b.id
  
  +-------+-------------+
  | ename | leader_name |
  +-------+-------------+
  | 小王  | 小明        |
  | 小钱  | 小明        |
  | 小李  | 小王        |
  +-------+-------------+
  ```

### 2. 外连接(left join / right outer join)

- 什么是外连接?

  > ​	我们不难发现 我们在等值连接中只会取到A和B表都符合条件的内容;这就是内连接
  >
  > 有的时候我们需要 我们也要查询出B表中应该有的数据但是没匹配的数据
  >
  > 所以区分 内外连接的关键字是 left 和 right

- 左外连接(LEFT JOIN/LEFT OUTER JOIN)

  ```sql
  同上面 内连接那个需求,我们需要显示出没有领导的员工
  
  SELECT
  	a.ename,
  	b.ename AS leader_name
  FROM
  	ept a
  LEFT JOIN ept b ON a.p_id = b.id
  
  +-------+-------------+
  | ename | leader_name |
  +-------+-------------+
  | 小王  | 小明        |
  | 小钱  | 小明        |
  | 小李  | 小王        |
  | 小明  | NULL        |
  +-------+-------------+
  ```

  

- 右外连接(right JOIN/right OUTER JOIN)

  ```shell
  1. 需求: 查询出部门对应的员工姓名,没有的也要显示
  
  2.查询出 员工表的员工名称和部门id
  mysql> select ename,dept_id from ept;
  +-------+---------+
  | ename | dept_id |
  +-------+---------+
  | 小明  |       1 |
  | 小王  |       2 |
  | 小李  |       3 |
  | 小钱  |       2 |
  +-------+---------
  
  3. 查询出了 部门表中部门名称和部门id
  mysql> select id , dname from dept;
  +----+-------+
  | id | dname |
  +----+-------+
  |  1 | 运维  |
  |  2 | 开发  |
  |  3 | 产品  |
  |  4 | 前端  |
  +----+-------+
  
  4. 我们就需要 B表全部显示;就算不匹配的
  
  SELECT
  	d.dname,
  	e.ename
  FROM
  	dept d
  LEFT JOIN ept e ON d.id = e.dept_id
  
  +-------+-------+
  | dname | ename |
  +-------+-------+
  | 运维  | 小明  |
  | 开发  | 小王  |
  | 产品  | 小李  |
  | 开发  | 小钱  |
  | 前端  | NULL  |
  +-------+-------+
  
  所以 不难看出来 我们的需求实现了;就算是前端么有人,但是他也全部显示了;
  
  5. 其中关于左和右 是建立在 join 这个词 左边还是右边的 你的表在join这个词左边就是左连接
  ```

  ### 3. 全连接

  ```sql
  1. 需求 我们需要 查询出每个员工对应的领导名称,以及每个员工对应的部门名称,以及每个员工对应的薪水等级
  2. 三张表结构
  mysql> select * from ept ;
  +----+-------+---------+---------+------+
  | id | ename | salary  | dept_id | p_id |
  +----+-------+---------+---------+------+
  |  1 | 小明  | 1800.00 |       1 | NULL |
  |  2 | 小王  | 1200.00 |       2 |    1 |
  |  3 | 小李  | 2500.00 |       3 |    2 |
  |  4 | 小钱  | 2000.00 |       2 |    1 |
  +----+-------+---------+---------+------+
  
  mysql> select * from dept ;
  +----+-------+
  | id | dname |
  +----+-------+
  |  1 | 运维  |
  |  2 | 开发  |
  |  3 | 产品  |
  |  4 | 前端  |
  +----+-------+
  
  mysql> select * from salary_level;
  +----+--------+-------+
  | id | salary | level |
  +----+--------+-------+
  |  1 |   2500 |     1 |
  |  2 |   2000 |     2 |
  |  3 |   1500 |     3 |
  |  4 |   1000 |     4 |
  +----+--------+-------+
  
  
  3. 我们需要 a表连接a表  a表连接b表  a表连接c表 
  
  select 
  a.ename,b.ename as leader_name,c.dname as dept,d.level as level
  from ept a
  left join ept b
  on a.p_id=b.id
  join dept c
  on a.dept_id=c.id
  join salary_level d
  on a.salary between d.salary and d.salary+499;
  
  +-------+-------------+------+-------+
  | ename | leader_name | dept | level |
  +-------+-------------+------+-------+
  | 小李  | 小王        | 产品 |     1 |
  | 小钱  | 小明        | 开发 |     2 |
  | 小明  | NULL        | 运维 |     3 |
  | 小王  | 小明        | 开发 |     4 |
  +-------+-------------+------+-------+
  
  ```

  

## 4.  子查询

### 1. 其实 就是把 子查询当做 一个 条件参数 

```sql
1. 需求 查询出员工表中薪水大于平均薪水的员工的姓名和薪水
2. 查询emp 表 情况
mysql> select * from emp;
+-------+--------+-----------+------+------------+---------+---------+--------+
| EMPNO | ENAME  | JOB       | MGR  | HIREDATE   | SAL     | COMM    | DEPTNO |
+-------+--------+-----------+------+------------+---------+---------+--------+
|  7396 | SMITH  | CLERK     | 7902 | 1980-12-17 |  800.00 |    NULL |     20 |
|  7499 | ALLEN  | SALESMAN  | 7698 | 1981-02-20 | 1600.00 |  300.00 |     30 |
|  7521 | WARD   | SALESMAN  | 7698 | 1981-02-22 | 1250.00 |  500.00 |     30 |
|  7566 | JONES  | MANAGER   | 7839 | 1981-04-02 | 2975.00 |    NULL |     20 |
|  7654 | MARTIN | SALESMAN  | 7698 | 1981-09-28 | 1250.00 | 1400.00 |     30 |
|  7698 | BLAKE  | MANAGER   | 7839 | 1981-05-01 | 2850.00 |    NULL |     30 |
|  7782 | CLARK  | MANAGER   | 7839 | 1981-06-09 | 2450.00 |    NULL |     20 |
|  7788 | SCOTT  | ANALYST   | 7566 | 1987-04-19 | 3000.00 |    NULL |     20 |
|  7839 | KING   | PRESIDENT | NULL | 1981-11-17 | 5000.00 |    NULL |     10 |
|  7844 | TURNER | SALESMAN  | 7698 | 1981-09-08 | 1500.00 |    0.00 |     30 |
|  7876 | ADAMS  | CLERK     | 7788 | 1987-05-23 | 1100.00 |    NULL |     20 |
|  7900 | JAMES  | CLERK     | 7698 | 1981-12-03 |  950.00 |    NULL |     30 |
|  7902 | FORD   | ANALYST   | 7566 | 1981-12-03 | 3000.00 |    NULL |     20 |
|  7934 | MILLER | CLERK     | 7782 | 1982-01-23 | 1300.00 |    NULL |     10 |
+-------+--------+-----------+------+------------+---------+---------+--------+

3. 就是嵌套查询

SELECT
	a.ENAME,
	a.SAL
FROM
	emp a
WHERE
	a.SAL > (
		SELECT
			AVG(emp.SAL) AS avgsal
		FROM
			emp
	)

+-------+---------+
| ENAME | SAL     |
+-------+---------+
| JONES | 2975.00 |
| BLAKE | 2850.00 |
| CLARK | 2450.00 |
| SCOTT | 3000.00 |
| KING  | 5000.00 |
| FORD  | 3000.00 |
+-------+---------+
	
```

****



### 2.  就是把子查询 当做 一个 临时表

```sql
1. 需求  找出每个部门的平均薪水,并且显示出每个部门的薪水等级

2. 分析 找出每个部门的平均薪水 就需要 先按部门分组,然后求出每个部门的平均薪水

SELECT
	e.DEPTNO as DEPTNO,
	AVG(e.SAL) AS avgsal
FROM
	emp e
GROUP BY
	e.DEPTNO
+--------+-------------+
| DEPTNO | avgsal      |
+--------+-------------+
|     10 | 3150.000000 |
|     20 | 2220.833333 |
|     30 | 1566.666667 |
+--------+-------------+

3.查看薪水表
mysql> select * from salgrade;
+-------+-------+-------+
| GRADE | LOSAL | HISAL |
+-------+-------+-------+
|     1 |   700 |  1200 |
|     2 |  1201 |  1400 |
|     3 |  1401 |  2000 |
|     4 |  2001 |  3000 |
|     5 |  3001 |  9999 |
+-------+-------+-------+
4. 我们应该把 表1当做 a表  表二当做 b表 此时做连接
select  a.DEPTNO,b.GRADE
from a
join b
on a.avgsal between b.LOSAL and b.HISAL

此时我们是不是 可以改写一下 

SELECT
	a.DEPTNO,
	a.avgsal,
	b.GRADE
FROM
	(
		SELECT
			e.DEPTNO AS DEPTNO,
			AVG(e.SAL) AS avgsal
		FROM
			emp e
		GROUP BY
			e.DEPTNO
	) a
JOIN salgrade b 
ON a.avgsal BETWEEN b.LOSAL AND b.HISAL;

+--------+-------------+-------+
| DEPTNO | avgsal      | GRADE |
+--------+-------------+-------+
|     10 | 3150.000000 |     5 |
|     20 | 2220.833333 |     4 |
|     30 | 1566.666667 |     3 |
+--------+-------------+-------+
```

## 5. union 表的连接

1.使用场景 : 连接两个一样的表;

```sql
1. 查询出工作为 "CLERK" 和 "SALESMAN"的 员工姓名

2. 第一种实现结果 
SELECT
	ename,
	job
FROM
	emp
WHERE
	JOB = "CLERK"
OR JOB = "ANALYST"

3. 第二种实现
SELECT
	ename,
	job
FROM
	emp
WHERE
	job IN ('CLERK', 'ANALYST')

4. 第三种实现

SELECT
	ename,
	job
FROM
	emp
WHERE
	job = "CLERK"
UNION
	SELECT
		ename,
		job
	FROM
		emp
	WHERE
		job = "ANALYST"

+--------+---------+
| ename  | job     |
+--------+---------+
| SMITH  | CLERK   |
| ADAMS  | CLERK   |
| JAMES  | CLERK   |
| MILLER | CLERK   |
| SCOTT  | ANALYST |
| FORD   | ANALYST |
+--------+---------+

显然 union  必须 要求两个组合的 表 必须 字段长度相同 ;而且 有意义点的就是 两个表 字段含义一样
```



## 6. constraint 表的约束

> 非空约束 **not null**
>
> 唯一性约束 , 不可重复   **unique**
>
> 给约束起别名  **constraint**  name 
>
> 联合约束主键约束   primary key  与 not no unique 的效果相同  , 
>
> 还可以添加索引 ,每张表必须有主键,不然就是无效表
>
> 主键值不能修改的单一主键联合主键(但是 一张表只能有一个主键,,只有两个一样时才会出现主键不同)自然主键业务主键自增主键值   auto_increment   只要你一旦使用过 , 就不能重复使用外键约束 foreign key 外简约束外键字段外键值

```sql
drop table if exists t_user;

create table t_user(
	id int(2) not null,
    name varchar(10) unique,
    email varchar(20),
    phone varchar(15),
    cd_card char(18) not no unique,
    constraint t_user_email_phone_unique unique(email,phone),
    primary key(id,name) 
)



constraint  英文单词 翻译叫  约束   一般写法就是 constraint + 一个字段名 +条件

如果我们给一个表添加外键约束
子 ----- 父
constraint t_student_classes_fk foreign key(classno)  references t_class(cno)

子表的数据也就是 外键值可以为空 ,称为孤儿数据
父表中的引用字段必须具有唯一性
创建表要先创建父表再创建子表
```



##  7. cascade 级联操作

```sql
1.级联删除 on delete cascade  (当我删除 父表(引用表)中的数据时,子表中的相关数据也要被删除   )
cascade 意思是 级联 ,瀑布

2.给学生表添加一个级联删除 
alter table student drop foreign key t_student_classes_fk;

alter table stadent add constraint t_student_classes_fk foreign key(classno) 
reference t_class(cno) on delete cascade; 


3.级联更新  on update cascade (更新父表,子表中引用的字段也要被更新)

alter table student add constraint t_class_student_fk foreign key(classno)
reference t_class(cno) on update cascade


* 注意就是 更新和删除建立在外键之上

```



##  8. transaction 事务

mysql数据库中 在操作数据库时 是 设置的自动提交事务

你可以 根据 	`show variables like '%commit%'`;  看到

```sql
mysql> show variables like '%commit%';
+-----------------------------------------+-------+
| Variable_name                           | Value |
+-----------------------------------------+-------+
| autocommit                              | ON    |
| binlog_group_commit_sync_delay          | 0     |
| binlog_group_commit_sync_no_delay_count | 0     |
| binlog_order_commits                    | ON    |
| innodb_api_bk_commit_interval           | 5     |
| innodb_commit_concurrency               | 0     |
| innodb_flush_log_at_trx_commit          | 1     |
| slave_preserve_commit_order             | OFF   |
+-----------------------------------------+-------+
8 rows in set, 1 warning (0.00 sec)
```



1. 开启事务

   ```sql
   1.手动开启事务
   start transaction ;
   2. 执行 DML语句
   inset into student (name) values ('小王');
   3.手动提交
   commit;
   
   
   
   
   4.如果我们关闭事务  set autocommit=off
   执行 DML语句
   
   然后 commit;
   
   
   
   
   ```

   

2. mysql的四种 隔离级别

   ```sql
   1. 查看当前会话的隔离级别  
   mysql> select @@tx_isolation;
   +-----------------+
   | @@tx_isolation  |
   +-----------------+
   | REPEATABLE-READ |
   +-----------------+
   1 row in set, 1 warning (0.00 sec)
   
   2. 查看当前 全局的隔离级别
   mysql> select @@global.tx_isolation;
   +-----------------------+
   | @@global.tx_isolation |
   +-----------------------+
   | REPEATABLE-READ       |
   +-----------------------+
   1 row in set, 1 warning (0.00 sec)
   
   
   修改全局的隔离级别
   set global transaction isolation level read committed
   
   
   3. 读未提交(read uncommitted) : 就是当A选择开启事务,执行了一条DML语句,此时未提交;然后B也开起了事务去读取数据,结果读取到了A修改后的数据,这条数据 为 脏读;
   
   4. 读已提交(read committed) : 就是A选择开起了事务,然后执行了一条DML语句,此时开着事务的B无法看到A执行后的结果,当A提交后,此时开着事务的B 就能看到 commit后的数据; 此时出现的现象叫做 不可重复读;
   
   5. 可重复读(repeatable read) : 就是A,B开启着事务,A提交的数据,B无法获取到 ;此时这种现象叫做幻象读;保证了B的数据开启事务时刻数据的一致性
   
   6. 串行化(serializable): 同一时刻操作数据库的只有一个人;但是可以多人开启事务;只能等待第一个人提交事务或者回滚事务,另外一个人才能执行数据库操作;
   
   
   ```

   







## 9. index 索引

1. 主键索引 ,添加主键会自动添加主键索引

2. 外键索引,

3. 索引检索方式

   - 全表扫描
   - 索引检索

4. 很少使用 dml操作时,适合添加索引

5. 经常出现在where语句中

6. 创建索引

   ```sql
   create index tudent_name_index on student(name);
   
   
   查看索引
   show index from table_name
   
   
   删除索引
   drop index  tudent_name_index on student;
   ```

   

7. 索引也要被存储在文件中

## 10. view 视图

1. 创建视图 

   ```sql
   1. create view student_view as select * from student
   
   2. 当我们 show create view student_view时
   
   mysql> show create view dept_view;
   
   | View      | Create View                                                                                                                                           
   | dept_view | CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `dept_view` AS select `dept`.`DEPTNO` AS `DEPTNO`,`dept`.`DNAME` AS `DNAME`,`dept`.`LOC` AS `LOC` from `dept` | gbk                  | gbk_chinese_ci 
   
   
   我们可以发现 他只是 在原有的表上创建了一个 select 语句
   
   
   3. 删除视图
   drop view dept_view;
   或
   drop view if exists dept_view;
   ```

2. 索引 视图 和 原表中的数据 是个影子关系

3. 视图的作用

   - 隐藏表的细节 比如  `create view dept_view as select depton as a ,dname as b from dept` 我们会发现 我们拿到的表 只有 a , b 字段  只能考 字段操作

   - 类似与 创建临时表,提高查询效率,以及可用性,

     ```sql
     create  view  emp_dept as
     select a.ename,b.dname
     from emp a
     join dept b
     on a.deptno=b.deptno
     
     
     然后直接 
     select* from emp_dept;
     
     这个可以提高检索效率和重复利用率
     ```

     



## 11. MySQL 存储引擎

### 1.mysql有9种存储引擎

```sql
mysql> show engines ;
+--------------------+---------+----------------------------------------------------------------+--------------+------+------------+
| Engine             | Support | Comment                                                        | Transactions | XA   | Savepoints |
+--------------------+---------+----------------------------------------------------------------+--------------+------+------------+
| InnoDB             | DEFAULT | Supports transactions, row-level locking, and foreign keys     | YES          | YES  | YES        |
| MRG_MYISAM         | YES     | Collection of identical MyISAM tables                          | NO           | NO   | NO         |
| MEMORY             | YES     | Hash based, stored in memory, useful for temporary tables      | NO           | NO   | NO         |
| BLACKHOLE          | YES     | /dev/null storage engine (anything you write to it disappears) | NO           | NO   | NO         |
| MyISAM             | YES     | MyISAM storage engine                                          | NO           | NO   | NO         |
| CSV                | YES     | CSV storage engine                                             | NO           | NO   | NO         |
| ARCHIVE            | YES     | Archive storage engine                                         | NO           | NO   | NO         |
| PERFORMANCE_SCHEMA | YES     | Performance Schema                                             | NO           | NO   | NO         |
| FEDERATED          | NO      | Federated MySQL storage engine                                 | NULL         | NULL | NULL       |
+--------------------+---------+----------------------------------------------------------------+--------------+------+------------+
9 rows in set (0.01 sec)


1. 默认是 innodb 
2. 可以修改 my.ini 设置默认存储引擎 以及一些默认配置
3. 修改表的存储引擎  alter table table_name engine=MyISAM


```

### 2. InnoDB

​	1.主要记住 innodb 支持**事务操作**

```
2. 使用**外键** 以及  级联删除和级联更新也需要使用 innodb engine
```
 	3. 行及锁定
 	4. mysql数据库崩溃 后 提供自动恢复
 	5. 表空间的概念
 	6. 提供 acid 的兼容

### 3. MyISAM

 	1. 表存储格式
     - 表结构 .frm
     - 表数据  .myd
     - 表索引  .myi
 	2. 可以压缩文件,节省空间

### 4. MEMORY

1. 存储的数据在 内存中 ,所以 很快
2. 索引也被存储在内存中
3. 字段属性不能包含text,blob等大的地段
4. 

   

## 12. 数据库设计的三范式

1. 第一范式 :  要求有主键 , 且每个字段保证其原子性,且不重复
2. 第二范式 : 数据库中的非主键字段严格和主键字段有关联 ,其自身不能产生部分依赖 (尽量不要使用联合主键)
   - 多对多 时 ;建立 中间表
3. 第三范式:非主键字段不能产生传递依赖 ; 





## 13. MySql 锁的问题

### 1.表锁(互斥锁)

- **读锁** :` lock table clazz read `; 此时只能读数据,无法修改数据;只有当 unlock tables; 才可以执行修改操作
- **写锁**:`lock table clazz write`; 此时只能写数据,无法进行读取数据;只有当 unlock tables; 才可以执行修改操作
- 这种使用场景一般是 锁表 ;做大的维护操作时;可以锁表 ; 读写互斥

### 2.行锁

- 他不锁住整个数据库,开启事务,只会锁住操作的同一张表(比如  A B 两个人同时开启事务,A对表a操作,B对表b操作,此时两个行为都可以执行,不需要等待一方commit,当B表尝试修改A表时;此时就会等待,必须等待A提交才能执行)

- 其实就是上面的四种事务;这里不做解释了;

### 3. 悲观锁

```sql
1. 就是mysql默认的事务等级 
他能保证 每一时刻只能有一个人操作数据库,除非你提交了;不然无法更新
比如 两个人 A和B 
每次执行操作 都要开启事务 默认事务是 可重复读(repeatable read)  这个级别;
count=2
当 A 执行 update oreders set count=count-1 where id=1 and count>=1;
此时 B也去执行 update oreders set count=count-2 where id=1 and count>=2;
由于A先执行 ;所以此时B 是卡住的;只有 A提交了;B才能修改;此时 B中查到的count数据已经修改过了;所以返回结果为false;



但是 在事务中 如果使用了 这个 select * from oreders for update; 他可以防止读取到的数据发生不可重复读现象;其实可以理解为 带着 update的select语句; 你执行了 update 本身就锁住了;他其实一句带着两句;他是个 互斥锁;你就算你是读,但是你执行了这个操作,别人也无法写执行;



但是 就算是我们在 开启事务中 使用select * from oreders 读到的是假数据 仍然不影响我们执行 update操作时;数据的读取问题 ;比如上面那个问题


```

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-04/510d874e-d23b-44c9-a851-579954481f6f.png?x-oss-process=style/template01)



### 4.乐观锁

```sql
1. 比如说 我们在 查询的时候 带上一个 version ,更新的时候 需要带上这个version

SELECT  count,version FROM orders WHERE id =1   把查询到version数据加给第二行,

每次更新都需要这样子,如果一个人更新成功,那么version数据就变化了,那么另外一个人拿到的锁,他打不开,找不到数据就无法更新

UPDATE orders SET count=count-5 , version=version+5 WHERE id=1 AND version=2 AND count>=5

这样可以防止超卖和加锁
```





## 14. QA 

1. between  range_1 and  range_2     是 左闭右闭

2. ```sql
   SELECT
   	AVG(emp.SAL) AS avgsal,ENAME,SAL
   FROM
   	emp
   WHERE SAL>avgsal
   切记 where 语句中不能直接套用非表格字段 ;这个需要子查询
   ```

3. limit  用法  limit a , b  (a是从0开始的,b指的是长度)    当  只有一个参数时 ;指的 是长度; 比如说 找出 第五个到第八个,我们 5 , 6 , 7 ,8  一共长度是4 ,则  limit 4 , 4,  其中limit是mysql 特有的
4. mysql 的 **varcart**类型 长度 为 1 可以输入 一个汉字 也是一个英文 一个数字 ;其实 一个varchar 就是一个字符;存储空间等于实际数据长度
5. **char** 是固定长度 一般填写 固定长度的信息好 ;存储空间是死的;不管你是1 23 4 5 我规定了10 你就给你分配10
6. **int**类型  占用4个字节  长度 好像不管写多少 数字 都么限制
7. **bigint**类型  占用8个字节  长度 好像不管写多少 数字 都么限制
8. **tinyint** 类型 长度 选择 1 但是可以到达 127 [0,128)
9. double 和 float
10. **date** 占用8个字节

### 1.技巧

1. 备份表 :  `create table emp_bak as select * from emp`

2. 查看创建表的语句:  `show create table emp_bak`

3. DML 是什么  Data Manipulation Language(数据操纵语言) ; 很显然是  delete , insert , update 

   1. `delete from table_name where  field=a`
   2. `delete from table_name`
   3. `update table_name set gender=1,emp=2 where id=1`
   4. `insert into table_name(filed1,field2) values ('','','')`

4. `str_to_date('1949-10-01','%Y-%m-%d')`  这个是日期 插入日期 ;; 其中mysql 默认的日期格式 是 `1994-10-01` 输入这个不会报错 

5. `show variables like '%char%';`

   ```sql
   +--------------------------+---------------------------------------------------------+
   | Variable_name            | Value                                                   |
   +--------------------------+---------------------------------------------------------+
   | character_set_client     | gbk                                                     |
   | character_set_connection | gbk                                                     |
   | character_set_database   | latin1                                                  |
   | character_set_filesystem | binary                                                  |
   | character_set_results    | gbk                                                     |
   | character_set_server     | latin1                                                  |
   | character_set_system     | utf8                                                    |
   | character_sets_dir       | C:\Program Files\MySQL\MySQL Server 5.7\share\charsets\ |
   +--------------------------+---------------------------------------------------------+
   
    character_set_results   字符结果集
   character_set_database   数据库的字符集
   
   
   修改很简单 直接  set character_set_results = utf8  就可以了
    
   
   ```

6. 两个表之间备份插入 ,上面提到了 全部插入 ,现在改成条件插入`insert into emp_bak select * from emp where job="manager"`

7. [数据库模式定义语言](https://baike.baidu.com/item/数据库模式定义语言/1237788)DDL(Data Definition Language)，是用于描述数据库中要存储的现实世界实体的语言。

   ```sql
   添加 alter table table_name add  email varchar(128) ;
   
   修改 alter table table_name modify no int(8);
   
   删除 alter table table_name  drop email ;
   
   修改表结构 alter table table_name change name  username varchar(32)
   
   
   ```

8. 解决 1对 1  表的问题 :  

   - 就是在一张表的外键字段上添加唯一约束
   - 中间表 外键 + 唯一

9. 查看数据库的version `SHOW VARIABLES WHERE Variable_name = 'version';`



### 13. mysql的执行顺序

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-06/f5fbfe21-3a1d-456a-be95-48677928c3e5.png?x-oss-process=style/template01)





## 15. linux 安装 mysql

> 基本按照我这个路就可以了 .

```java
[admin@hadoop1 ~]$ sudo wget -i -c http://dev.mysql.com/get/mysql57-community-release-el7-10.noarch.rpm

[admin@hadoop1 ~]$ sudo yum -y install mysql57-community-release-el7-10.noarch.rpm

[admin@hadoop1 ~]$ sudo yum -y install mysql-community-server

[admin@hadoop1 ~]$ sudo systemctl start  mysqld.service

[admin@hadoop1 ~]$ systemctl status mysqld.service

// 查看密码记得 , 这个临时密码 ,需要记住
[admin@hadoop1 ~]$ sudo  grep "password" /var/log/mysqld.log
2019-11-21T11:27:46.135337Z 1 [Note] A temporary password is generated for root@localhost: iUa3!wQ2wu:j

// ""加可以防止特殊字符
[admin@hadoop1 ~]$ mysql -uroot -p"iUa3!wQ2wu:j"

// 此时进入 mysql  ......密码不过过于简单 大小写区分开,同时有特殊符号 , 大写小写字母,数字,符号
ALTER USER 'root'@'localhost' IDENTIFIED BY 'new_password';

// 让任何主机可以访问 ....授予权限
GRANT ALL PRIVILEGES ON *.* TO 'root'@'%' IDENTIFIED BY 'new_password' WITH GRANT OPTION;


此时就可以远程登录了.....

注意 5.7以后 
需要在/etc/my.cnf 里面添加一句话
sql_mode = "STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION"
```



2、添加用户

```mysql
//只允许指定ip连接
create user 'user_name'@'localhost' identified by 'user_password';
//允许所有ip连接（用通配符%表示）
create user 'user_name'@'%' identified by 'user_password';
```

3、为新用户授权

```mysql
//基本格式如下
grant all privileges on 数据库名.表名 to '新用户名'@'指定ip' identified by '新用户密码' ;
//示例
//允许访问所有数据库下的所有表
grant all privileges on *.* to '新用户名'@'指定ip' identified by '新用户密码' ;
// grant all privileges on *.* to 'admin'@'localhost' identified by 'admin123';
//指定数据库下的指定表
grant all privileges on test.test to '新用户名'@'指定ip' identified by '新用户密码' ;
```

4、设置用户操作权限

```mysql
//设置用户拥有所有权限也就是管理员
grant all privileges on *.* to '新用户名'@'指定ip' identified by '新用户密码' WITH GRANT OPTION;
// grant all privileges on *.* to 'admin'@'localhost' identified by 'admin123' WITH GRANT OPTION;
//拥有查询权限
grant select on *.* to '新用户名'@'指定ip' identified by '新用户密码' WITH GRANT OPTION;
//其它操作权限说明,select查询 insert插入 delete删除 update修改
//设置用户拥有查询插入的权限
grant select,insert on *.* to '新用户名'@'指定ip' identified by '新用户密码' WITH GRANT OPTION;
//取消用户查询的查询权限
REVOKE select ON what FROM '新用户名';
```

5、删除用户

```mysql
DROP USER username@localhost;
```

6、修改后刷新权限

```mysql
FLUSH PRIVILEGES;
```

## 14. Oracle 新语法 start with...connect by 

> 	1.  start with 条件1
>  	2.  connect by 条件2
>  	3.  prior表示上一条记录  比如 **CONNECT BY** **PRIOR org_id = parent_id**；就是说**上一条记录的org_id 是本条记录的parent_id**，

给一张图 解释

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-06/60bb97a0-1123-42e8-85a4-2860b99d91f9.png?x-oss-process=style/template01)



修改字符集

```sql
su - oracle  
sqlplus /nolog  
connect /as sysdba  
 
--查看数据库字符编码集
select userenv('language') from dual;
 
shutdown immediate  
startup restrict  
alter database character set INTERNAL_USE ZHS16GBK;  
shutdown immediate  
startup
```





## 15.数据库操作练习

```sql
1. 取得每个部门中最高薪水的人员名称

SELECT
	f.DEPTNO,
	f.DNAME,
	f.ENAME,
	MAX(f.SAL) AS maxsal
FROM
	(
		SELECT
			d.DEPTNO,
			d.DNAME,
			e.ENAME,
			e.SAL
		FROM
			dept d
		JOIN emp e ON e.DEPTNO = d.DEPTNO
	) f
GROUP BY
	f.DNAME
	
+--------+------------+-------+---------+
| DEPTNO | DNAME      | ENAME | maxsal  |
+--------+------------+-------+---------+
|     10 | ACCOUNTING | KING  | 5000.00 |
|     20 | RESEARCH   | SMITH | 3000.00 |
|     30 | SALES      | ALLEN | 2850.00 |
+--------+------------+-------+---------+





2. 查询出哪些人的薪水在平均薪水之上
分析: 1.我们要先找出每个部门的平均薪水,然后跟我们的emp表进行连接诶,比较薪水大于平均薪水的

SELECT
	d.ename,
	d.sal,
	d.deptno,
	f.avgsal
FROM
	emp d
JOIN (
	SELECT
		e.DEPTNO,
		AVG(e.sal) AS avgsal
	FROM
		emp e
	GROUP BY
		e.DEPTNO
) f ON f.DEPTNO = d.deptno
WHERE
	d.sal > f.avgsal

+-------+---------+--------+-------------+
| ename | sal     | deptno | avgsal      |
+-------+---------+--------+-------------+
| ALLEN | 1600.00 |     30 | 1566.666667 |
| JONES | 2975.00 |     20 | 2220.833333 |
| BLAKE | 2850.00 |     30 | 1566.666667 |
| CLARK | 2450.00 |     20 | 2220.833333 |
| SCOTT | 3000.00 |     20 | 2220.833333 |
| KING  | 5000.00 |     10 | 3150.000000 |
| FORD  | 3000.00 |     20 | 2220.833333 |
+-------+---------+--------+-------------+


3. 取出部门中的人的平均薪水等级

思路先求出每个人的薪水等级,然后根据此表分组,计算出平均值, 然后根据dept表查询出部门



SELECT
	h.deptno,
	d.dname,
	h.avggra
FROM
	dept d
JOIN (
	SELECT
		f.deptno,
		AVG(f.grade) AS avggra
	FROM
		(
			SELECT
				e.ename,
				e.deptno,
				s.grade
			FROM
				emp e
			JOIN salgrade s ON e.sal BETWEEN s.losal AND s.hisal
		) f
	GROUP BY
		f.deptno
) h
ON
	h.deptno = d.deptno
		
或者

SELECT
	f.deptno,
	d.dname,
	AVG(f.grade) AS avggra	
FROM
	(
		SELECT
			e.ename,
			e.deptno,
			s.grade
		FROM
			emp e
		JOIN salgrade s ON e.sal BETWEEN s.losal AND s.hisal
	) f
JOIN dept d ON d.deptno = f.deptno
GROUP BY
	f.deptno
	
+--------+------------+--------+
| deptno | dname      | avggra |
+--------+------------+--------+
|     10 | ACCOUNTING | 3.5000 |
|     20 | RESEARCH   | 3.0000 |
|     30 | SALES      | 2.5000 |
+--------+------------+--------+	


或者 是 查询出平均薪水 然后根据 平均薪水找对应等级 

SELECT
	f.deptno,
	d.dname,
	s.grade,
	f.avgsal
FROM
	salgrade s
JOIN (
	SELECT
		e.ename,
		e.deptno,
		AVG(e.sal) AS avgsal
	FROM
		emp e
	GROUP BY
		e.deptno
) f ON f.avgsal BETWEEN s.losal AND s.hisal
JOIN dept d ON f.deptno = d.deptno
	
+--------+------------+-------+
| deptno | dname      | grade |
+--------+------------+-------+
|     30 | SALES      |     3 |
|     20 | RESEARCH   |     4 |
|     10 | ACCOUNTING |     5 |
+--------+------------+-------+	
	
	
	
	
4. 不准用max聚合函数,取得最高薪水等级 给出两种方案

SELECT
	sal
FROM
	emp
ORDER BY
	sal DESC
LIMIT 1

+---------+
| sal     |
+---------+
| 5000.00 |
+---------+

第二种方案 就是 两个一样的表比较  比如 (1,2,3,4),(1,2,3,4) 我要找出a表中小于b表中的数(1,2,3) 所以此时空掉的是 4 这个数字

SELECT
	sal
FROM
	emp
WHERE
	sal NOT IN (
		SELECT DISTINCT
			a.sal
		FROM
			emp a
		JOIN emp b ON a.sal < b.sal
	)
+---------+
| sal     |
+---------+
| 5000.00 |
+---------+


5.取出平均薪水最高的部门编号

SELECT
	deptno,
	avg(sal) AS avgsal
FROM
	emp
GROUP BY
	deptno
HAVING
	avgsal = (
		SELECT
			max(a.avgsal) AS maxavg
		FROM
			(
				SELECT
					deptno,
					avg(sal) AS avgsal
				FROM
					emp
				GROUP BY
					deptno
			) a
	)



6.找出平均薪水最高的部门所对应的部门名称

SELECT
	e.deptno,
	d.dname,
	avg(e.sal) AS avgsal
FROM
	emp e
JOIN dept d
ON e.deptno=d.deptno
GROUP BY
	e.deptno
HAVING
	avgsal = (
		SELECT
			max(a.avgsal) AS maxavg
		FROM
			(
				SELECT
					deptno,
					avg(sal) AS avgsal
				FROM
					emp
				GROUP BY
					deptno
			) a
	)

```























## 16. 建表语句

```sql

DROP TABLE IF EXISTS EMP;
DROP TABLE IF EXISTS DEPT;
DROP TABLE IF EXISTS SALGRADE;
 
CREATE TABLE DEPT
	(DEPTNO int(2) not null ,
	DNAME VARCHAR(14) ,
	LOC VARCHAR(13) ,
	primary key (DEPTNO)
	);
CREATE TABLE EMP
	(EMPNO int(4) not null ,
	ENAME VARCHAR(10) ,
	JOB VARCHAR(9) ,
	MGR INT(4) ,
	HIREDATE DATE DEFAULT NULL ,
	SAL DOUBLE(7,2) ,
	COMM DOUBLE(7,2) ,
	primary key (EMPNO) ,
	DEPTNO INT(2)
	);
CREATE TABLE SALGRADE
	(GRADE INT ,
	LOSAL INT ,
	HISAL INT 
	);
	
INSERT INTO DEPT ( DEPTNO, DNAME, LOC ) VALUES (
10, 'ACCOUNTING', 'NEW YOURK');
INSERT INTO DEPT ( DEPTNO, DNAME, LOC ) VALUES (
20, 'RESEARCH', 'DALLAS');
INSERT INTO DEPT ( DEPTNO, DNAME, LOC ) VALUES (
30, 'SALES', 'CHICAGO');
INSERT INTO DEPT( DEPTNO, DNAME, LOC ) VALUES (
40, 'OPERATIONS', 'BOSTON');
commit;
 
INSERT INTO EMP (EMPNO, ENAME, JOB, MGR, HIREDATE, SAL, COMM, DEPTNO) VALUES (
7396, 'SMITH', 'CLERK', 7902, '1980-12-17', 800, NULL, 20);
INSERT INTO EMP (EMPNO, ENAME, JOB, MGR, HIREDATE, SAL, COMM, DEPTNO) VALUES (
7499, 'ALLEN', 'SALESMAN', 7698, '1981-02-20', 1600, 300,30);
INSERT INTO EMP (EMPNO, ENAME, JOB, MGR, HIREDATE, SAL, COMM, DEPTNO) VALUES (
7521, 'WARD', 'SALESMAN', 7698, '1981-02-22', 1250, 500, 30);
INSERT INTO EMP (EMPNO, ENAME, JOB, MGR, HIREDATE, SAL, COMM, DEPTNO) VALUES (
7566, 'JONES', 'MANAGER', 7839, '1981-04-02', 2975, NULL, 20);
INSERT INTO EMP (EMPNO, ENAME, JOB, MGR, HIREDATE, SAL, COMM, DEPTNO) VALUES (
7654, 'MARTIN', 'SALESMAN', 7698, '1981-09-28', 1250, 1400, 30);
INSERT INTO EMP (EMPNO, ENAME, JOB, MGR, HIREDATE, SAL, COMM, DEPTNO) VALUES (
7698, 'BLAKE', 'MANAGER', 7839, '1981-05-01', 2850, NULL, 30);
INSERT INTO EMP (EMPNO, ENAME, JOB, MGR, HIREDATE, SAL, COMM, DEPTNO) VALUES (
7782, 'CLARK', 'MANAGER', 7839, '1981-06-09', 2450, NULL, 20);
INSERT INTO EMP (EMPNO, ENAME, JOB, MGR, HIREDATE, SAL, COMM, DEPTNO) VALUES (
7788, 'SCOTT', 'ANALYST', 7566, '1987-04-19', 3000, NULL, 20);
INSERT INTO EMP (EMPNO, ENAME, JOB, MGR, HIREDATE, SAL, COMM, DEPTNO) VALUES (
7839, 'KING', 'PRESIDENT', NULL, '1981-11-17', 5000, NULL, 10);
INSERT INTO EMP (EMPNO, ENAME, JOB, MGR, HIREDATE, SAL, COMM, DEPTNO) VALUES (
7844, 'TURNER', 'SALESMAN', 7698, '1981-09-08', 1500, 0, 30);
INSERT INTO EMP (EMPNO, ENAME, JOB, MGR, HIREDATE, SAL, COMM, DEPTNO) VALUES (
7876, 'ADAMS', 'CLERK', 7788, '1987-05-23', 1100, NULL, 20);
INSERT INTO EMP (EMPNO, ENAME, JOB, MGR, HIREDATE, SAL, COMM, DEPTNO) VALUES (
7900, 'JAMES', 'CLERK', 7698, '1981-12-03', 950, NULL, 30);
INSERT INTO EMP (EMPNO, ENAME, JOB, MGR, HIREDATE, SAL, COMM, DEPTNO) VALUES (
7902, 'FORD', 'ANALYST', 7566, '1981-12-03', 3000, NULL, 20);
INSERT INTO EMP (EMPNO, ENAME, JOB, MGR, HIREDATE, SAL, COMM, DEPTNO) VALUES (
7934, 'MILLER', 'CLERK', 7782, '1982-01-23', 1300, NULL, 10);
commit;
 
INSERT INTO SALGRADE (GRADE, LOSAL, HISAL) VALUES (
1, 700, 1200);
INSERT INTO SALGRADE (GRADE, LOSAL, HISAL) VALUES (
2, 1201, 1400);
INSERT INTO SALGRADE (GRADE, LOSAL, HISAL) VALUES (
3, 1401, 2000);
INSERT INTO SALGRADE (GRADE, LOSAL, HISAL) VALUES (
4, 2001, 3000);
INSERT INTO SALGRADE (GRADE, LOSAL, HISAL) VALUES (
5, 3001, 9999);
commit;
```

