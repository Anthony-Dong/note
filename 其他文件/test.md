# Test

c++

```c++
#include <iostream>

using namespace std;

int main(int argc, char const *argv[])
{
    int x = 1;
    int &y = x;
    cout << x << "," << y << endl;
    x = 10;
    cout << x << "," << y << endl;
    y = 100;
    cout << x << "," << y << endl;
    return 0;
}
```



c

```c
#include <iostream>

using namespace std;

int main(int argc, char const *argv[])
{
    int x = 1;
    int &y = x;
    cout << x << "," << y << endl;
    x = 10;
    cout << x << "," << y << endl;
    y = 100;
    cout << x << "," << y << endl;
    return 0;
}
```





go

```go
func main() {
    // 启动一个tcp端口
	listener, _ := net.Listen("tcp", "localhost:8888")
	for {
        //学过Java或者其他语言的都知道 , 一个新的连接过来入口就是一个accept
		conn, _ := listener.Accept()
        // 处理新连接
		handlers(&conn)
	}
}

func handlers(conn *net.Conn) {
    // 其实这个方法不执行, 因为就算客户端断了也会不断的写的,因为下面是个死循环..所以先不考虑这个,处理的话一般得做心跳处理,或者conn.SetDeadline().
	defer (*conn).Close()
	for {
		io.WriteString(*conn, time.Now().Format("Mon Jan 2 15:04:05 -0700 MST 2006\n\r"))
		time.Sleep(1 * time.Second)
	}
}
```



java

```java
public void add(long x) {
    Cell[] as; long b, v; int m; Cell a;
    // cells!=null , cas交换base是否成功  , 如果第一个false , 第二个为true , 其实就是atomic.
    if ((as = cells) != null || !casBase(b = base, b + x)) {
        // 当cas交换失败时 , 就需要标记为竞争失败
        boolean uncontended = true;
        // cells如果为空,反正一堆.判断
        if (as == null || (m = as.length - 1) < 0 ||
            (a = as[getProbe() & m]) == null ||
            !(uncontended = a.cas(v = a.value, v + x)))
            // 创建一个新的cell,然后操作他.累加
            longAccumulate(x, null, uncontended);
    }
}
```



sql

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



shell 不支持 , java

```java

netstat -nlp |grep LISTEN   

netstat -nlp |grep 80  

netstat -an | grep 3306   

netstat -apn | grep ssh  

netstat -an | grep ':22' 

netstat -ie

netstat -ano | findstr "80"
```



js

```js
const generateOverride = (params = {}) => {
  let result = ''

  if (params.customCss) {
    result += `
      ${params.customCss}
    `
  }


  console.log('result', result)

  return result
}

module.exports = generateOverride

```



```css
body
{
    background-color:#d0e4fe;
}
h1
{
    color:orange;
    text-align:center;
}
p
{
    font-family:"Times New Roman";
    font-size:20px;
}
```





properties

```properties
user.name=xiaoli
```



不支持

yaml

```scala
#在同级目录下创建config.yaml文件. 配置文件如下格式.
# 配置监听端口
listener:
  port: 8080

#  static , windows用文件/ 代替 \ , 比如访问 http://localhost:8080/static/image/a/a.jpg , 会被代理到 D:/user/media/image/a/a.jpg
static:
  static/image: D:/user/media/image
  static/wallpaper: D:/user/media/wallpaper
  static/test: D:/user/media/test
#  proxy , 比如访问 http://www.baidu.com/api/get -> 会被代理到 http://localhost:8888/get
proxy:
  api: http://localhost:8888
```



不支持

cmd

```cmd

netstat -nlp |grep LISTEN   //查看当前所有监听端口·(比较常用)

netstat -nlp |grep 80   //查看所有80端口使用情况·

netstat -an | grep 3306   //查看所有3306端口使用情况·

netstat -apn | grep ssh   并不是所有的进程都能找到，没有权限的会不显示，使用 root 权限查看所有的信息。

netstat -an | grep ':22'  找出运行在指定端口的进程

netstat -ie  跟  ifconfig 效果相似


windows的 查看进程 
netstat -ano | findstr "80"
```

