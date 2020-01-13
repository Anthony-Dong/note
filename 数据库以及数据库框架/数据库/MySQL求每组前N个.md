# MySQL 找每组前N个

题目来自于 [LeetCode中](https://leetcode-cn.com/problems/department-top-three-salaries/submissions/) 第185道题目 ,属于困难难度

> ​	一般我们通常的思路就是 先分组, 然后组内排序 , 用 limit , 这种对吗? , 很显然limit是在 group以后执行的 ..... 
>
> ​	所以需要换一种思维去想

## 1. 前期准备

```sql
Create table If Not Exists Employee (Id int, Name varchar(20), Salary int, DepartmentId int);
Create table If Not Exists Department (Id int, Name varchar(20));
Truncate table Employee;
insert into Employee (Id, Name, Salary, DepartmentId) values ('1', 'Joe', '85000', '1');
insert into Employee (Id, Name, Salary, DepartmentId) values ('2', 'Henry', '80000', '2');
insert into Employee (Id, Name, Salary, DepartmentId) values ('3', 'Sam', '60000', '2');
insert into Employee (Id, Name, Salary, DepartmentId) values ('4', 'Max', '90000', '1');
insert into Employee (Id, Name, Salary, DepartmentId) values ('5', 'Janet', '69000', '1');
insert into Employee (Id, Name, Salary, DepartmentId) values ('6', 'Randy', '85000', '1');
insert into Employee (Id, Name, Salary, DepartmentId) values ('7', 'Will', '70000', '1');
Truncate table Department;
insert into Department (Id, Name) values ('1', 'IT');
insert into Department (Id, Name) values ('2', 'Sales');
```

## 2. 如何查找前N个

### 1. 查找最大值不用max函数

如果两个表连接, 是不是A 表中的最大值是只有一个,就是A表中这个最大值比B表所有数据都要大(A.MAX>B.all)的个数是0 , 或者最简单(A.MAX>=B.all)是 1 , 所以就是这个思路

```sql
SELECT
	e1.Salary ,e1.DepartmentId
FROM
	employee e1 
WHERE
	(
	SELECT
		COUNT( DISTINCT e2.Salary ) 
	FROM
		employee e2 
	WHERE
		e2.Salary > e1.Salary 
		AND e2.DepartmentId = e1.DepartmentId 
	)=0
```



### 2. 同样的前N个就是 ...

```sql
SELECT
	e1.Salary,
	e1.DepartmentId 
FROM
	employee e1 
WHERE
	(
	SELECT
		COUNT( DISTINCT e2.Salary ) 
	FROM
		employee e2 
	WHERE
		e2.Salary > e1.Salary 
		AND e1.DepartmentId = e2.DepartmentId 
	) <n
```



### 4. 所以这个题就是 

````sql
SELECT
	d1.`Name` as Department , 
	e1.`Name` as Employee,
	e1.Salary
FROM
	employee e1
	JOIN department d1 ON d1.id = e1.DepartmentId 
WHERE
	(
	SELECT COUNT( DISTINCT e2.Salary ) 
	FROM
		employee e2 
	WHERE
		e2.Salary > e1.Salary 
		AND e1.DepartmentId = e2.DepartmentId 
	) <3
````



