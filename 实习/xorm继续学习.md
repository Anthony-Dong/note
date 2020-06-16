# xorm 继续学习

## 项目目录结构 

```java
sgcx015@172-15-70-185 com.example.xorm % ls -al
total 0
drwxr-xr-x   8 sgcx015  staff  256 May  6 11:57 .
drwxr-xr-x  10 sgcx015  staff  320 Apr 27 16:27 ..
drwxr-xr-x   8 sgcx015  staff  256 May  6 12:00 cmd   // 主函数
drwxr-xr-x   2 sgcx015  staff   64 Apr 26 10:31 dao		// orm层
drwxr-xr-x   3 sgcx015  staff   96 May  6 11:55 db      // 数据库连接
drwxr-xr-x   3 sgcx015  staff   96 May  6 11:59 logger  // 日志
drwxr-xr-x   4 sgcx015  staff  128 May  6 11:52 model  // 放入数据库的实体层
```

model ， 放入数据库的实体层

dao ， 数据库操作层，orm

logger ，日志

db， 数据库连接

service ， 调用dao

controller，调用service



## 创建表

```sql
CREATE TABLE `student` (
  `id` bigint(11) NOT NULL AUTO_INCREMENT COMMENT '主键',
  `name` varchar(50) NOT NULL COMMENT '名字',
  `age` int(11) NOT NULL DEFAULT '0' COMMENT '年龄',
  `class_id` int(11) NOT NULL DEFAULT '0' COMMENT '班级id',
  `create_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00' COMMENT '创建时间',
  `update_time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00' COMMENT '更新时间',
  PRIMARY KEY (`id`),
	KEY `idx_class_id` (`class_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

一般是不允许为空 ，对于一些要求差的需要设置默认值，全部必须有注释， 数据库引擎使用innodb ， 字符集为utf8mb4

这里在mysql5.7的时候，运行失败，因为严格遵守sql92的规范，具体做法是[修改配置](https://blog.csdn.net/qq_30770095/article/details/81458931?utm_medium=distribute.pc_relevant.none-task-blog-BlogCommendFromBaidu-2&depth_1-utm_source=distribute.pc_relevant.none-task-blog-BlogCommendFromBaidu-2)  ， 就是设置一下全局变量修改一下。

## Module 层

```go
package model

import "time"

type Student struct {
	Id         int64     `xorm:"pk autoincr"  json:"-"`
	Name       string    `xorm:"name" json:"name"`
	Age        int8      `xorm:"age" json:"age"`
	ClassId    int64     `xorm:"class_id" json:"class_id"`
	CreateTime time.Time `xorm:"Created" json:"create_time"`
	UpdateTime time.Time `xorm:"updated" json:"update_time"`
}
func (table Student) TableName() string {
	return "student"
}
```

> ​	需要注意，主键必须是加`pk autoincr`  , 创建时间和修改时间必须加入`Created` , `updated` , json必须都有

## CRUD操作

### 插入操作

```go
func main() {
	_=insertStudent()
	student, _ := findById()
	fmt.Printf("%+v\n",student)
	bytes, _ := json.Marshal(&student)
	fmt.Printf("json: %s\n",bytes)
}
// 查询
func findById() (*model.Student, error) {
	session := db.XormDb().NewSession()
	defer session.Close()
	student := &model.Student{}
	_, e := session.Where("id=?", 2).Get(student)
	if e != nil {
		return nil, e
	}
	return student, nil
}

// 插入
func insertStudent() error {
	session := db.XormDb().NewSession()
	defer session.Close()
	if e := session.Begin(); e != nil {
		return e
	}
	student := model.Student{
		Name: "tony",
		Age:  10,
	}
	_, e := session.Insert(&student)
	if e != nil {
		session.Rollback()
		return e
	}
	session.Commit()
	return nil
}
```

打印：

```go
[xorm] [info]  2020/05/06 16:11:06.260275 [SQL] BEGIN TRANSACTION [] - 35.943346ms
[xorm] [info]  2020/05/06 16:11:06.276075 [SQL] INSERT INTO `student` (`name`,`age`,`class_id`,`create_time`,`update_time`) VALUES (?,?,?,?,?) [tony 10 0 2020-05-06 16:11:06 2020-05-06 16:11:06] - 14.741485ms
[xorm] [info]  2020/05/06 16:11:06.285972 [SQL] COMMIT [] - 9.763191ms
[xorm] [info]  2020/05/06 16:11:06.301095 [SQL] SELECT `id`, `name`, `age`, `class_id`, `create_time`, `update_time` FROM `student` WHERE (id=?) LIMIT 1 [2] - 14.968628ms
&{Id:2 Name:tony Age:10 ClassId:0 CreateTime:2020-05-06 16:11:06 +0800 CST UpdateTime:2020-05-06 16:11:06 +0800 CST}
json: {"name":"tony","age":10,"class_id":0,"create_time":"2020-05-06T16:11:06+08:00","update_time":"2020-05-06T16:11:06+08:00"}
```

### 插入多条记录

```go
func insertStudents() error {
	session := db.XormDb().NewSession()
	defer session.Close()
	if e := session.Begin(); e != nil {
		return e
	}
	// 设置一个数组
	students := make([]*model.Student, 2)
	students[0] = &model.Student{
		Name: "tony1",
		Age:  10,
	}
	students[1] = &model.Student{
		Name: "tony2",
		Age:  10,
	}
	// 插入进去
	_, e := session.Insert(&students)
	if e != nil {
		session.Rollback()
		return e
	}
	session.Commit()
	return nil
}
```

### 更新操作

```go
func update() error {
	session := db.XormDb().NewSession()
	defer session.Close()
	if e := session.Begin(); e != nil {
		return e
	}
	student := model.Student{
		Age: 20,
	}
	_, e := session.Where("id=?", 2).Update(&student)
	if e != nil {
		session.Rollback()
		return e
	}
	session.Commit()
	return nil
}
```



