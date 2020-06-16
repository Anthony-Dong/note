# XORM

### xorm.NewEngine("mysql", dns)

JDBC/GDBC哇，其实都有一个基础库

首先看看基础库如何连接的

```go
func main() {
	//URL："用户名:密码@tcp(IP:端口)/数据库?charset=utf8"
	path := strings.Join([]string{"root", ":", "123456", "@tcp(", "localhost", ":", "3306", ")/", "xorm", "?charset=utf8"}, "")
	//打开数据库,前者是驱动名，所以要导入： _ "github.com/go-sql-driver/mysql"
	DB, _ := sql.Open("mysql", path)
	defer DB.Close()
	// 查询
	rows, _ := DB.Query("select * from users where id=?", 1)
	defer rows.Close()

	// 字段查询类型
	types, _ := rows.ColumnTypes()
	for _, _type := range types {
		fmt.Printf("%+v\n", *_type)
	}

	// 字段名称
	columns, _ := rows.Columns()
	for _, name := range columns {
		fmt.Println(name)
	}

	for rows.Next() {
		student := model.Student{}
		e := rows.Scan(&student.Id, &student.Name, &student.Age, &student.CreateTime, &student.ClassId)
		if e != nil {
			fmt.Println(e)
		}
		fmt.Printf("%+v\n", student)
	}
}
```



结构：`sql.Rows`

```go
type Rows struct {
  // 连接
	dc          *driverConn 
  // 释放
	releaseConn func(error)
  // 这个是一个接口，类似于Java的ResultSet
	rowsi       driver.Rows
	cancel      func()     
	closeStmt   *driverStmt // if non-nil, statement to Close on close
	closemu sync.RWMutex
	closed  bool
	lasterr error // non-nil only if closed is true

	// lastcols is only used in Scan, Next, and NextResultSet which are expected
	// not to be called concurrently.
	lastcols []driver.Value
}
```

接口: `driver.Rows`  是给驱动，使用的，对外拓展，可以发现一个是close方法，一个是获取columns，一个是迭代器。

```go
type Rows interface {

	Columns() []string

	Close() error

	Next(dest []Value) error
}
```



其次就是scan 的原理 ， 其实就是一些类型判断进行的一系列的赋值，如果失败就使用反射进行。





