# PHP 学习

## 1、 变量

```php
$a = 1 + 2;
echo $a;
```

和js一样, 就是个var , $的意思可以理解为 var



全局变量 , 类似于闭包.  方法内部访问外部的变量. 需要申明 global

```php
$a = 1;
$b = 2;
function add(){
    global $a, $b;
    return $a + $b;
}
echo add();
```



static 的话, 类似于c语言的效果

```php
$a = 1;
$b = 2;
function add(){
    static  $sun=0;
    global $a, $b;
    $sun += $a + $b;
    return $sun;
}

echo add()."\n";
echo add()."\n";
```

输出 

```java
3
6
```

### 2. 变量的释放

> ​	类似于c语言的释放内存, 可以手动释放, 比java强点

```php
$value = "111";

// 判断是否释放
var_dump(isset($value));
// 执行释放
unset($value);
var_dump(isset($value));
```



## 2.数据类型

> ​	和 js 一样, 赋值就是定义类型

```php
function echoType(...$a)
{
    foreach ($a as $value){
        echo var_dump($value);
    }
}
echoType(1, "hello", true,100,100.1);
```

输出 : 

```php
int(1)
string(5) "hello"
bool(true)
int(100)
float(100.1)
```



## 3.数据结构

### 1. 普通数组

> ​	类似于Java的数组

```php
$arr = array(1, 23, 4, 1, "1");
echo $arr[1]."\n";
echo count($arr);
```

输出

```java
23
5
```

### 2. 关联数组 (字典)

> ​	类似于python中的字典

```php
$arr = array("Tom" => 25, "tony" => 15);
echo $arr["Tom"];
```

输出

```php
25
```



由于是弱类型么, 所以可以添加方法

```php
$arr=array("f1"=>function($v){echo $v;});
$arr["f1"]("1");
```

输出

```php
1
```



> ​	对应的排序方法 , 其实默认使用的全局域, 所以可以直接调用, 不需要 域名::方法, `sort` , `rsort`  升序降序 ,  `asort`值排序, `ksort()`  键排序

### 3. SPL拓展

#### 1. SplStack  栈

#### 2. SplQueue 队列

#### 3.SplMinHeap  小顶堆  / SplMaxHeap 大顶堆

#### 4. SplFixedArray 固定大小的数组







## 3. 函数

> ​	php 的函数不用显示的指定返回值, 类似于js一样. 

```php
function _echo($str){
    echo $str;
}

_echo("11111111");
```



同时支持默认值 

```php
function test($s = "默认值")
{
    echo $s;
}
test();  // 输出 : 默认值
```



## PHP的元信息变量

```php
function println($const){
    echo "$const\n";
}
// 当前代码处于第几行
println(__LINE__);
// 当前php文件
println(__FILE__);
// 当前的目录
println(__DIR__);

class User{
    static function fF1()
    {
        // 类名 ,如果使用了 namespace 则是 namespace+类型 ,以下同理
        echo __CLASS__ . "\n";
        echo __FUNCTION__ . "\n";
        // 这个区分静态与非静态方法,也就是这个返回User::fF1 , 上面返回fF1
        echo __METHOD__ . "\n";

    }
}
User::fF1();
echo __NAMESPACE__ . "\n";
```



## namespace

php 的 autoload机制 [https://www.cnblogs.com/-simon/p/5875063.html](https://www.cnblogs.com/-simon/p/5875063.html)

