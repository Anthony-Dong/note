# Golang的 J(G)DBC

> ​	Java很简单, 第一步当然驱动 , 第二步获取连接 , 第三步 新建statement查询语句, 第四步执行statement . 
>
> ​	其实Golang也是这四步



下载MySQL 数据库驱动

```go
go get github.com/go-sql-driver/mysql
```

然后项目 import

```go
_ "github.com/go-sql-driver/mysql"
```

建立连接

```go
const (
	userName = "root"
	password = "123456"
	ip       = "localhost"
	port     = "3306"
	dbName   = "go_db"
)

//Db数据库连接池
var DB *sql.DB

//注意方法名大写，就是public
func InitDB() {

	//URL："用户名:密码@tcp(IP:端口)/数据库?charset=utf8"
	path := strings.Join([]string{userName, ":", password, "@tcp(", ip, ":", port, ")/", dbName, "?charset=utf8"}, "")

	//打开数据库,前者是驱动名，所以要导入： _ "github.com/go-sql-driver/mysql"
	DB, _ = sql.Open("mysql", path)
    
    //可以设置一些数据库连接池的其他参数
    
	//验证连接
	if err := DB.Ping(); err != nil {
        // 抛出异常,强制退出
		panic("opon database fail")
	}
	fmt.Println("connnect success")
}
```

拿到db当然要做一些事情 CRUD的事情了

```sql
CREATE TABLE `go_user` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(20)  NOT NULL,
  `password` varchar(50)  NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=10 DEFAULT CHARSET=utf8;
```

Pojo对象

```go
type User struct {
	ID       int
	UserName string
	Password string
}
```

增

```go
func InsertUser(user *User) (int64, error) {
	//开启事务
	tx, err := DB.Begin()
	if err != nil {
		tx.Rollback()
		return 0, err
	}
	//准备sql语句
	stmt, err := tx.Prepare("INSERT INTO go_user (`name`, `password`) VALUES (?, ?)")
	if err != nil {
		tx.Rollback()
		return 0, err
	}
	//将参数传递到sql语句中并且执行
	res, err := stmt.Exec(user.UserName, user.Password)
	if err != nil {
		tx.Rollback()
		return 0, err
	}
	//将事务提交
	tx.Commit()

	i, _ := res.LastInsertId()
	return i + 1, nil
}
```

删除 

```go
func DeleteUser(id int64) (bool, error) {
	//开启事务
	tx, err := DB.Begin()
	if err != nil {
		tx.Rollback()
		return false, err
	}
	//准备sql语句
	stmt, err := tx.Prepare("DELETE FROM go_user WHERE id=?")
	if err != nil {
		tx.Rollback()
		return false, err
	}

	r, err := stmt.Exec(id)
	//将参数传递到sql语句中并且执行
	if err != nil {
		tx.Rollback()
		return false, err
	}
	i, _ := r.RowsAffected()
	//将事务提交
	tx.Commit()
	if i == 0 {
		return false, errors.New("ID不存在")
	} else {
		return true, nil
	}
}
```

查询

```go
func SelectUserById(id int) User {
	var user User
	err := DB.QueryRow("SELECT * FROM go_user WHERE id = ?", id).Scan(&user.ID, &user.UserName, &user.Password)
	if err != nil {
		fmt.Println("查询出错了")
	}
	return user
}
```



