# xorm学习

首先得 ：下载依赖  `go get github.com/xormplus/xorm` , 这个会自动加载一个压缩框架snappy ， 其次就是mysql的驱动 ：` go get github.com/go-sql-driver/mysql` , 基本环境就是这些 ， 

关于xorm 的文档在[https://www.kancloud.cn/xormplus/xorm](https://www.kancloud.cn/xormplus/xorm)



### 初始化 ： 

```go
func init()  {
	var err error
	dns := fillDns(map[string]string{
		"user":     "root",
		"password": "123456",
		"host":     "localhost",
		"port":     "3307",
		"dbname":   "xorm",
		"charset":  "utf8",
	})
	engine, err = xorm.NewEngine("mysql", dns)
	if err!=nil {
		log.Fatal(err)
	}
  // 显示日志
	engine.ShowSQL(true)
  // 开启字段的驼峰命名法
	engine.SetMapper(names.GonicMapper{})
}

func fillDns(mysqlConfig map[string]string) string {
	return fmt.Sprintf("%s:%s@tcp(%s:%s)/%s?charset=%s&parseTime=True&loc=Local",
		mysqlConfig["user"],
		mysqlConfig["password"],
		mysqlConfig["host"],
		mysqlConfig["port"],
		mysqlConfig["dbname"],
		mysqlConfig["charset"])
}
```

### 元信息获取

```go
tables, _ := engine.DBMetas()
for _,table := range tables {
  fmt.Printf("%#v\n",table)
}
// &schemas.Table{Name:"users", Type:reflect.Type(nil), columnsSeq:[]string{"id", "name", "age", "date"}, columnsMap:map[string][]*schemas.Column{"age":[]*schemas.Column{(*schemas.Column)(0xc00019c410)}, "date":[]*schemas.Column{(*schemas.Column)(0xc00019c4e0)}, "id":[]*schemas.Column{(*schemas.Column)(0xc00019c1a0)}, "name":[]*schemas.Column{(*schemas.Column)(0xc00019c340)}}, columns:[]*schemas.Column{(*schemas.Column)(0xc00019c1a0), (*schemas.Column)(0xc00019c340), (*schemas.Column)(0xc00019c410), (*schemas.Column)(0xc00019c4e0)}, Indexes:map[string]*schemas.Index{}, PrimaryKeys:[]string{"id"}, AutoIncrement:"id", Created:map[string]bool{}, Updated:"", Deleted:"", Version:"", StoreEngine:"InnoDB", Charset:"", Comment:""}
```



## sql 查询

简单实用一下。 

```go
strings, _ := engine.QueryString("select * from users")
fmt.Println(strings) // [map[age:1 date:2020-04-26T10:47:30+08:00 id:1 name:tom]]
```

其内部原理 , 项目中都是 new session， 然后一次session 执行一次sql ， 和Java的mybatis很相似，一样的抽象层次。

```go
func (engine *Engine) QueryString(sqlOrArgs ...interface{}) ([]map[string]string, error) {
	session := engine.NewSession()
	defer session.Close()
	return session.QueryString(sqlOrArgs...)
}
```



查询集合

```sql
// 1.新建一个回话
session := engine.NewSession()
defer session.Close()
// 2. 创建一个数组
users := make([]model.Users, 0)
// 3. 查询，这里可以进行orm映射，但是我不懂切片已经是一个指针了，为啥还要取地址
err := session.Table("users").Where("age = ?", 1).Find(&users)
if err!=nil {
  log.Fatal(err)
}
fmt.Println(users)
```

这里我感觉，有必要将对象改成，指针，也就是`make([]*model.Users, 0)`  , 减少几次数据拷贝.



查询单列

```sql
var name string
engine.Table("users").Where("id=?",1).Cols("name").Get(&id)
fmt.Println(name)
```



查询多列结果 ，查询多条数据使用Find方法，Find方法的第一个参数为slice的指针或Map指针 ， 其次就是可以利用 Iterate 进行回掉代替find

```sql
strings := make([]string, 3)
engine.Table("users").Where("id=?",1).Cols("id","name","date").Get(&strings)
fmt.Println(strings)
```



可以根据当前的对象进行条件查询 ,如果想关闭需要执行NoAutoCondition()

```sql
users := model.Users{Name:"tom"}
engine.Get(&users)
fmt.Printf("%+v\n",users)
// [xorm] [info]  2020/04/26 14:36:51.635047 [SQL] SELECT `id`, `name`, `age`, `date` FROM `users` WHERE `name`=? LIMIT 1 [tom] - 15.497857ms
```



#### join 查询

```go
type Result struct {
	User model.Users `xorm:"extends"`
	ClassName string `xorm:"name"`
}


func testJoin()  {
	session := engine.NewSession()
	results := make([]Result, 0)
	_ = session.Table("users").Alias("u").
		Join("left", []string{"class", "c"}, "u.class_id=c.id").
		Where("u.class_id=?", 1).
		Cols("u.*","c.name").
		Find(&results)
	fmt.Printf("%+v",results)
}
```

其中 这个比如 join的链接属于什么样的链接，需要我们手动写出来，有inner，left，right等



### Row \ Iterator 遍历

```go
rows, err := engine.Where("id >?", 1).Rows(user)
if err != nil {
}
defer rows.Close()
for rows.Next() {
    err = rows.Scan(user)
    //...
} 
```





### 聚合操作

> ​	https://www.kancloud.cn/xormplus/xorm/175313  sum操作

> ​	https://www.kancloud.cn/xormplus/xorm/167104  , count





### 更多的条件查询请看

[https://www.kancloud.cn/xormplus/xorm/167098](https://www.kancloud.cn/xormplus/xorm/167098)



### json/ xml 结果模型

```go
s, _ := engine.SQL("select * from users where id=?", 1).Query().Json()
fmt.Println(s)
```

参数匹配

```go
paramMap := map[string]interface{}{"id": 1, "name": "tom"}
s, _ := engine.SQL("select * from users where id=?id and name=?name", &paramMap).Query().Json()
fmt.Println(s)
```







### 简单事务模型

```go
func insetModel()  {
	// 1. 开启回话
	session := engine.NewSession()
	defer session.Close()
	// 2。开启事物
	session.Begin()
	users := model.Users{
		Name:"anthony",
		Age:10,
		Date:time.Now(),
	}
	i, e := session.Insert(users)
	if e!=nil {
		// 异常，rollback
		session.Rollback()
		log.Fatal(e)
	}
	// 最后提交 ， commit或rollback是事物的结束事件
	session.Commit()
	fmt.Println("success : ",i)
	fmt.Printf("%+v\n",users)
}
```



### 嵌套事务模型

> 这个我有次面试的时候，问过我Spring的事物事物传播类型有几种，好像我记得是7种 ，一般就是嵌套生成，嵌套复用，嵌套有异常/无异常，嵌套有则用无则生成，基本就是这几种。

在这个框架里，一共有8种传播类型， [https://www.kancloud.cn/xormplus/xorm/235728](https://www.kancloud.cn/xormplus/xorm/235728)  ， 

```go
func addModel() {
	session := engine.NewSession()
	defer session.Close()
	tx1(session)
}

func tx1(session *xorm.Session) {
	tx, e := session.BeginTrans(xorm.PROPAGATION_SUPPORTS)
	if e != nil {
		log.Fatal(e)
	}
	result, e := tx.Session().Exec("select * from users")
	if e != nil {
		log.Fatal(e)
	}
	tx.CommitTrans()
	fmt.Println(result)
}
```



## 导出表格

```go
err := engine.SQL("select * from users").Query().SaveAsXLSX("1.xlsx", []string{"id", "name", "date","age","class_id"}, 0777)

if err != nil {
  log.Fatal(err)
}
```



