# PHP

> php 最大的特点就是项目大了，如何维护起来，就需要包管理和类加载器了。 同时php也提供了它的便利性，突出点其实也是体现了它是脚本语言的特性，比如一个脚本就是一个对象。

## Include 和 include_once

> ​	这个主要是负责加载php文件的，类似于Java的import，等等。

### 1.Include

demo.php

```php
<?php
echo "demo.php"."\n";
```

load.php 

```php
<?php
include __DIR__.'/demo.php';
include __DIR__.'/demo.php';

echo "finish";
```

​	输出

```php
demo.php
demo.php
ok
```



### 2.Include_once

修改上面的load.php文件

```php
<?php

include_once __DIR__.'/demo.php';
include_once __DIR__.'/demo.php';

echo "finish";
```

输出 

```php
demo.php
finish
```



`$rest=include_once __DIR__.'/demo.php';` 这个返回值是是个没有用的返回值



## Require 和 Request_once

> ​	上面说include以及存在了一些的问题，php既然是脚本语言，我可以不可以拿一个脚本作为对象，对就是这个

```php
// demo.php 文件
echo "loading ...\n";
return "hello world";

// load.php文件
$result=require __DIR__."/demo.php";
echo $result;
// loading ...
// hello world

$result=require_once __DIR__."/demo.php";
echo $result;
// 1
```

为啥呢，是因为require的时候内存中已经保存了一份这个php文件，当我们第二次的时候就不会去加载了，和上诉的incloude_once一样，但是它可以提供返回值



## 3. 数组操作

添加数组，语法糖

```php
$arr = [1,2,3]; // 或者 $arr = array(1,2,3);
$arr[]=4; // append 
$arr[]=5; 
$arr[]=6;
var_export($arr);  //  1,2,3,4,5,6
```

map

```php
$arr = ["k1"=>"v1"];
$arr["k2"]="v2";
$arr["k3"]["k1"]="v1";
var_export($arr);
```

输出

```php
$arr = ["k1"=>"v1"];
$arr["k2"]="v2";
$arr["k3"]["k1"]="v1";

var_export($arr);
```

随心所欲