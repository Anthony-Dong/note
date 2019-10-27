# Shell

## 1.入门 shell 实例

```shell
ping -c1 www.baidu.com &>/home/user/null && echo "www.baidu.com is up" || echo "www.baidu.com is down"
```

1. 在这里面我们 -c1 的意思是

- -c1 是 -c<完成次数> 设置完成要求回应的次数。
- 其中 && 是 逻辑与 当我前面的数据正确时,他可以输出 || 是 要么要么,如果前面对就执行前面的不对级执行我
- 输出重定向
  - "&>"是 将指令正确和错误信息都不输出在屏幕上直接输到你重定向的一个文件内。
  - ">" 代表将数据全部定向输入到一个文件内同时覆盖文件内容。
  - ">>" 是把输出追加到文件中, 如果文件存在, 追加到末尾, 不存在则创建文件.

2. 执行 shell 脚本

- bash test.sh
- 然后 bash 和 sh 的关系

```shell
[root@iz2zegua78a74kqn8clokfz script]# which bash
 /usr/bin/bash
[root@iz2zegua78a74kqn8clokfz script]# ll /usr/bin/bash
-rwxr-xr-x 1 root root 960472 Dec  7  2016 /usr/bin/bash

[root@iz2zegua78a74kqn8clokfz script]# ll /usr/bin/sh
lrwxrwxrwx 1 root root 4 Aug 18  2017 /usr/bin/sh -> bash

```

- 我们发现 bash 是执行文件,当你使用 sh 他只是一个软连接


3. 再次修改 加权限

   > Linux文件的三种身份和四种权限，三种身份分别为：
   >
   > 1. u: 文件拥有者
   > 2. g: 文件所属的群主
   > 3. o : 其他用户
   > 4. a 表示“所有（all）用户”。它是系统默认值。
   >
   > 对于每个身份，又有四种权限，分别为：
   >
   > 1. r：读取文件的权限（read）
   > 2. w：写入文件的权限（write）
   > 3. x：执行的权限（execute）
   > 4. s：特殊权限

   此时执行 : chmod 新的权限 文件列表

   ```shell
   chmod +x test.sh
   chmod 777 test.sh
   ```

   此时去执行  相对路径下 或绝对 路径吓得 的 test.sh 就可以直接执行

   ```shell
   [root@iz2zegua78a74kqn8clokfz script]# ./test01.sh
   www.baidu.com is up
   ```

4. shebang : 就是可以 选择默认的执行文件 

   ```shell
   #!/usr/bin/bash
   ping -c1 www.baidu.com &>/home/user/null && echo "www.baidu.com is up" || echo "www.baidu.com is down"
   ```

   当我们么有明确指定 解释器时,会选择 我们的shebang

5. 当我们选择 python 2.X时  linux自带的懒得换

   ```python
   #!/usr/bin/python
   print "hello"
   ```

6. 我们将 他也 进行chmod -x python.py  就可以直接 输入文件执行了

7. cat pyhton.py >> test.sh  可以将py文件添加到sh文件后面
   
8. 输入 重定向

   ```shell
   - "<<"  命令<<标识符  ......  标识符   命令把标准输入中读入内容，直到遇到“标识符”分解符为止
   - "<"     命令<文件1   命令把文件1的内容作为标准输入设备
   - "命令< 文件1 >文件2"  命令把文件1的内容作为标准输入，把文件2作为标准输出。
   
   cat < pyhton.py         其实和 cat python.py 效果一样
   
   cat << eof
   > hhh
   > yyy
   > eof
   hhh
   yyy
   ```

9. 修改我们原来合并的文件 

   ```shell
   #!/usr/bin/bash
   ping -c1 www.baidu.com &>/home/user/null && echo "www.baidu.com is up" || echo "www.baidu.com is down"
   /usr/bin/python <<-eof
   print "hello"
   print "hello 2"
   eof
   
   echo "hello world"                    
   ```

10. 此时 就可以同时在 shell中 执行 python和shell 脚本了 

11. ```shell
    test02.sh 脚本如下
    #!/usr/bin/bash
    cd /home
    ls
    
    1.   赋予权限 chmod +x test02.sh
    2.   ./test02.sh   目录不变  (子shell中执行)
    3.  . test02.sh   目录会切换到home路径下 (当前shell中执行)
    4.  source test02.sh  同样目录也换切换到 home路径下
    ```

## 2.shell插件

~~~shell
$ yum install -y epel-release  
$ yum clean all
$ yum makecache

$ yum install -y bash-completion

最后 
[root@iz2zegua78a74kqn8clokfz home]# rpm -qa | grep bash-completion
bash-completion-2.1-6.el7.noarch
~~~


## 3.shell变量

- ```shell
  #!/usr/bin/bash
  #author fanhaodong
  
  name="fanhaodong"
  
  echo "my name is $name"
  ```

- 注意 :

  - shell的变量 中间的 等于号 必须  跟前后两个字母 无空格
  - 引用只需要 加一个 $name  就可以 

- 常用的变量

  - "$?"  代表执行结果

  - "$0"  代表当前sh文件名+后缀

  - "$pwd"  当前sh文件所在位置

  - "$SHELL"  当前执行的shell

  - "$n" 代表第几个变量

    - ```shell
      #!/usr/bin/bash
      #author fanhaodong
      
      name="fanhaodong"
      
      echo "my name is $name"
      
      echo $0
      
      echo $1
      
      echo $2
      
      
      然后
      [root@iz2zegua78a74kqn8clokfz script]# ./variables.sh 1 2
      my name is fanhaodong
      ./variables.sh
      1
      2
      ```

  - "$*"  代表全部变量  但是 会当做一个整体 比如说 

    ```shell  
    $*="aaa  bbb  ccc" 输出 就是 aaa  bbb  ccc
    ```

  - "$@"  也是代表全部变量  ,但是不管怎么写  他都是不会把他当做一个整体,

  - "$#"  表示 参数的个数

- 常用指令

  - ```shell
    1. 赋值变量操作
    [user@iz2zegua78a74kqn8clokfz script]$ a=5
    [user@iz2zegua78a74kqn8clokfz script]$ echo $a   	--->5
    2. unset 取消赋值变量
    [user@iz2zegua78a74kqn8clokfz script]$ unset a
    [user@iz2zegua78a74kqn8clokfz script]$ echo $a
    
    3. readonly  只可以读的变量
    [user@iz2zegua78a74kqn8clokfz script]$ readonly a=6
    [user@iz2zegua78a74kqn8clokfz script]$ echo $a    ---> 6
    [user@iz2zegua78a74kqn8clokfz script]$ a=7
    -bash: a: readonly variable
    [user@iz2zegua78a74kqn8clokfz script]$ unset a
    -bash: unset: a: cannot unset: readonly variable
    
    当我们去修改 a的值时会报错,而且 也不可以车削
    ```

  - 

- 注意事项 

  - shell 命令的区分参数  是以空格为分界线 ,所以不要随意的写空格 ,

## 4.shell运算符

1. 基本语法

   ```shell
   $((运算式))  或者 $[运算式]
   expr + - * / % 中间参数之间用空格隔开
   
   exp:
   [user@iz2zegua78a74kqn8clokfz script]$ expr 1 + 2          ------->3
   
   [user@iz2zegua78a74kqn8clokfz script]$ x=6
   [user@iz2zegua78a74kqn8clokfz script]$ y=7
   //挨着也没事 分开隔着空格也么事 (这里空格不需要那么讲究)
   [user@iz2zegua78a74kqn8clokfz script]$ echo $[$x + $y]     -------->13
   [user@iz2zegua78a74kqn8clokfz script]$ echo $[ $x + $y ]     -------->13
   [user@iz2zegua78a74kqn8clokfz script]$ echo $[$x+$y]     -------->13
   [user@iz2zegua78a74kqn8clokfz script]$ echo $(($x + $y))   -------->13
   [user@iz2zegua78a74kqn8clokfz script]$ expr $x + $y       ----------->13
   
   ```

   



## 5.条件判断式

- 基本语法
  
- [条件判断式  参数之间用 空格 隔开,首位空格隔开]
  
- 数值间比较

  - "="  比较数值相等

    ```shell
    [user@iz2zegua78a74kqn8clokfz script]$ [ $x = $y ]
    [user@iz2zegua78a74kqn8clokfz script]$ echo $?         -->1(状态码 0位true,1为false)
    
    *注意
    1. 空格 一个不能少 [空格+变量(/数值)+空格+判断词+空格+变量(/数值)+空格]  一共需要四个空格
    2. $?  是 上此次执行的结果
    ```

    

  - "-lt"   less than   <
  - "-le"  less equal    <=
  - "-eq"  equal    =
  - "-gt"  greater than   =
  - "-ge"  greater equal   >=
  - "-ne"  not equal   !=

- 文件权限

  - "-x"  查看是否有执行权限

    - ```shell
      [user@iz2zegua78a74kqn8clokfz script]$ [ -x test02.sh ]
      [user@iz2zegua78a74kqn8clokfz script]$ echo $?      --->0
      ```

  - "-w"  写

  - "-r"  读

- 文件类型判断并且判断是否存在

  - "-f" 是否是个常规文件且存在

    - ```shell
      [user@iz2zegua78a74kqn8clokfz script]$ [ -f test02.sh ]
      [user@iz2zegua78a74kqn8clokfz script]$ echo $? 	------>0
      ```

  - "-d"  是否是个目录且存在

  - "-e" 是否存在



## 6. if判断式

- 1. 基本语法

  - ```shell
    if[条件判断式]
    then
    	程序
    fi
    
    或者
    if[条件判断式]
    	then
    		程序
    elif
    	then
    		程序
    else 
    	程序
    fi
    ```

- 2.使用

  - ```shell
    #!/bin/bash
    if [ $1 -eq 1 ]
    then
            echo 1111
    fi
    
    [user@iz2zegua78a74kqn8clokfz script]$ ./test04.sh 1      ---->1111
    ```

  - ```shell
    #!/bin/bash
    if [ -d /home/user/logs ]
    then
            rm -rf /home/user/logs
    elif [ -f /home/user/logs ]
    then
            rm -rf /home/user/logs
    else
            mkdir /home/user/logs
    fi
    ```



## 7.switch case 语句

- 1.基本语法

  - ```shell
    case $变量名 in
    	"值1")
    		程序
    	;;
    	"值2")
    		程序
    	;;
    	*)
    		程序(这个相当于default)
    	;;
    esac
    ```

- 2.使用

  - ```shell
    #!/bin/bash
    case $1 in
      "1")
            echo this is 1
       ;;
     "2")
        echo this is 2
      ;;
      *)
       echo i do not konw
      ;;
    esac
    
    
    执行
    [user@iz2zegua78a74kqn8clokfz script]$ ./test04.sh 1
    this is 1
    [user@iz2zegua78a74kqn8clokfz script]$ ./test04.sh 2
    this is 2
    [user@iz2zegua78a74kqn8clokfz script]$ ./test04.sh 3
    i do not konw
    [user@iz2zegua78a74kqn8clokfz script]$ ./test04.sh 4
    i do not konw
    ```

## 8. for循环

- 1.基本语法

  - ```shell
    for((变量初始值;循环条件;变量变化))
    do 
    	程序
    done
    ```

- 2.使用

  - ```shell
    #!/bin/bash
    #实现累加
    sum=0
    for((i=1;i<=100;i++))
    do
            sum=$[$sum + $i]
    done
    
    echo $sum
    
    ```

- 3.注意事项

  - 1. 赋值  直接 sum=0  ;sum=1   就是等于多少
    2. 引用   比如说我要取值 我要加减  就是引用变量 必须用  $sum

- 4.for in 语句

  - 1.语法

    - ```shell
      for 变量  in  数值1 数值2 数值3 (数值之间用空格隔开)
      do
      	程序
      done
      ```

  - 2.使用

    - ```shell
      #!/bin/bash
      for i in "$*"
      do
              echo $i
      done
      
      echo '----------------'
      for i in "$@"
      do
              echo $i
      done
      
      echo '----------------'
      for i in $*
      do
              echo $i
      done
      
      echo '----------------'
      for i in $@
      do
              echo $i
      done
      
      
      
      [user@iz2zegua78a74kqn8clokfz script]$ ./test05.sh 111 222 333
      111 222 333
      ----------------
      111
      222
      333
      ----------------
      111
      222
      333
      ----------------
      111
      222
      333
      ```

## 9.while循环

- 1.语法

  - ```shell
    while [条件判断式]  (while和[] 必须有空格)
    do
    	程序
    done
    ```

- 2.实例

  - ```shell
    #!/bin/bash
    # 实现 1-100 累加
    sum=0
    num=1
    while [ $num -le 100 ]
    do
      sum=$[$sum + $num]
      num=$[$num + 1]
    done
    
    echo $sum
    
    
    
    [user@iz2zegua78a74kqn8clokfz script]$ ./while.sh 
    5050
    
    ```



## 10. read 用法

```shell
1. 用法
read i  (无条件参数时;程序运行无提示)
echo $i


read -p "输入数值:" i
echo $i

#!/bin/bash
read -p '请在5S内输入数值:' -t 5 i
echo $i
	
```

## 11.function用法

```shell
#!/bin/bash

function aaa(){
  echo '我是个函数'
}

aaa

bbb(){
        echo '我是个函数bbb'
}
bbb


ccc(){

  s=0
  s=$[ $1 + $2 ]
  echo $s
}

read -p "请输入参数1: " i
read -p "请输入参数2: " j
ccc $i $j



[user@iz2zegua78a74kqn8clokfz script]$ ./function.sh
我是个函数
我是个函数bbb
请输入参数1: 12
请输入参数2: 12
24

注意函数调用 必须在申明函数之后




```



## 12.cut 命令

- "-d"  切割的字符  记住是字符

- "-f"  切割后的数组的第几个位置 从 1开始

- "-c"  切割字符  从1开始

- ```shell
  [root@iz2zegua78a74kqn8clokfz ~]# ifconfig | grep  broadcast 
  inet 172.24.22.95  netmask 255.255.192.0  broadcast 172.24.63.255
  
  
  [root@iz2zegua78a74kqn8clokfz ~]# ifconfig | grep  broadcast | cut -d "i" -f 2
  net 172.24.22.95  netmask 255.255.192.0  broadcast 172.24.63.255
  
  [root@iz2zegua78a74kqn8clokfz ~]# ifconfig | grep  broadcast | cut -d "i" -f 2 | cut -c 1,2,3
  net
  [root@iz2zegua78a74kqn8clokfz ~]# ifconfig | grep  broadcast | cut -d "i" -f 2 | cut -c 1-3
  net
  
  
  ```



## 13.sed命令

- ```shell
  "a"  代表 匹配/inet/的行 然后在匹配行后面 添加一行 xxx
  ifconfig | sed '/inet/axxx'
  ```

- ```shell
  "d"  删除
  ifconfig | sed '1,2d'  删除第 1-2行
  ```

- ```shell
  "s" 替换  1-3行中 将 /172/的地方 替换为 /sss/  全局
  ifconfig | sed '1,3s/172/sss/g'
  ```

- ```shell
  "-e"  添加多条指令  
  ifconfig | sed -e '1,3s/172/sss/g' -e '4,5s/RX/RR/g'
  ```

- ```shell
  "-i"  直接修改数据源  将 1-2行删除 并且 直接给数据源修改
  sed -i '1,2d' tt
  ```

- ```shell
  格式:
  
  set -e '{范围}操作'   范围:"/start/,/end/"  以start开始end结尾  "1,2" 第一行到第二行
  
  "s"操作:  s/匹配条件/修改内容/g
  "a"操作:  axxx   xxx内容为添加的
  "d" 直接 d    就删除了
  ```



## 14. awk命令

- 默认分词器是 空格 所以是空格分开的可以不指定 分隔符

- ```shell
   "-F" 指定 分隔符 这里是 :   后面是指定输出内容  
   "/^a/"  指定匹配的模式  以a开头
   cat tt | awk -F: '/^a/{print}'      匹配/^a/ 开头的 然后打印此行
  ```

- ```shell
  cat tt | awk -F: '/^a/{print $2}'   匹配/^a/ 开头的   然后打印此行第二个参数 
  ```

- ```shell
  awk '$1>2 && $2=="Are" {print $1,$2,$3}' log.txt   过滤提哦啊见可以写
  ```

- ```shell
  cat tt | awk -F: 'BEGIN{print "BEGIN"}{print '6666'}END{print "END"}'
  
  "BEGIN" 开始函数
  中间执行流程
  "END" 函数做什么
  ```

- ```shell
  cat tt | awk -F: 'BEGIN{print "begin";sum=0}{sum+=$2}END{print "end:" sum}'
  ```

- ```shell
  cat tt | awk -F: -v sum=3 '{sum+=$2}END{print "sum:" sum}'
  
  "-v" 指定参数
  ```

- ```shell
  split("2012/08/23",da,"/")  内置的函数
  awk 'BEGIN{split("2012/08/23",da,"/");print da[2],da[3],da[1]}'
  ```

- ```shell
  [root@iz2zegua78a74kqn8clokfz test]# awk -F: '{print FILENAME, NF ,NR}' tt
  tt 2 1
  tt 2 2
  tt 2 3
  tt 2 4
  tt 2 5
  
  FILENAME : 文件名
  NF :  参数个数
  NR : 行数
  
  ```

- 变量名称

- ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-02/b13102fd-f112-4f28-881d-9af6279ba746.jpg?x-oss-process=style/template01)



## 15.sort 命令

- 默认分词器是 空格 所以是空格分开的可以不指定 分隔符

```shell
[root@iz2zegua78a74kqn8clokfz test]# cat tt | grep ^a
a:1
aa:2
aaa:3
aaaa:4
aaaaa:5
[root@iz2zegua78a74kqn8clokfz test]# vim tt
[root@iz2zegua78a74kqn8clokfz test]# cat tt | grep ^a
a:1
aaa:32
aaaaa:15
[root@iz2zegua78a74kqn8clokfz test]# cat tt | grep ^a | sort -t : -k 2 -n
a:1
aaaaa:15
aaa:32

cat tt | grep ^a | sort -t : -k 2 -n

1."-t"  以什么切割
2."-k"  取切割后的第几个数字
3."-n"  以数值排序  默认 ASC
4."-u"  去重
5."-o"  重定向到一个文件中取
6:"-r"  倒叙排列  DESC
7."-f"  脚本文件

exp  awk -f a.awk  a.txt
```

## 16.wc

- 1.语法

  - ```shell
    wc [选项] 文件名　　#统计
    ```

- 2.功能

  - ```shell
    "-l" 显示行数
    [user@iz2zegua78a74kqn8clokfz script]$ wc -l score.txt 
    5 score.txt
    
    "-w" 只统计单词数
    [user@iz2zegua78a74kqn8clokfz script]$ wc -w score.txt
    25 score.txt
    
    "-m"　　只统计字符数
    [user@iz2zegua78a74kqn8clokfz script]$ wc -m score.txt
    110 score.txt
    ```

  - 



## 17.综合练习

1. printf 用法

   1. 输出类型

      > 1. %s　　输出字符串，n是数字，指代输出几个字符  比如 %8s 8位 
      > 2. %d　　输出整数。n是数字，指代输出几个数字   比如 %8d  8位 
      > 3. %m.nf　　位数和小数位数。例如：%8.2f 代表输出8位数，其中2位是小数，6位是整数

   2. 输出格式

      > \a　　输出警告声音 
      > \b　　输出退格键，也就是BackSpace键 
      > \f　　消除屏幕 
      > \n　　换行 
      > \r　　回车
      > \t　　水平制表符 
      > \v　　垂直制表符

2. 练习

   ```shell
   [user@iz2zegua78a74kqn8clokfz script]$ cat score.txt 
   Marry   2143 78 84 77
   Jack    2321 66 78 45
   Tom     2122 48 77 71
   Mike    2537 87 97 95
   Bob     2415 40 57 62
   
   
   
   #!/bin/bash
   # 按学号顺序排列
   
   sort -k 2 score.txt |
   awk '
   BEGIN{
   printf "NAME    NO.   MATH  ENGLISH  COMPUTER   TOTAL\n"
   printf "----------------------------------------------\n"
   sum=0;
   sum1=0;
   sum2=0;
   sum3=0;
   }
   {
   sum=sum+1
   sum1=sum1+$3
   sum2=sum2+$4
   sum3=sum3+$5
   printf "%s\t%s\t%d\t%d\t%d\t%d\n", $1, $2, $3,$4,$5, $3+$4+$5
   }
   END{      
   printf "TOTAL:\t\t%d\t%d\t%d\n",sum1,sum2,sum3
   printf "AVERAGE:%10.2f %8.2f %8.2f\n", sum1/NR, sum2/NR, sum3/NR
   }
   '
   
   
   [user@iz2zegua78a74kqn8clokfz script]$ ./score.sh 
   NAME    NO.   MATH  ENGLISH  COMPUTER   TOTAL
   ----------------------------------------------
   Tom		2122	48	77	71	196
   Marry	2143	78	84	77	239
   Jack	2321	66	78	45	189
   Bob		2415	40	57	62	159
   Mike	2537	87	97	95	279
   TOTAL:			319	393	350
   AVERAGE:     	63.80   78.60 70.00
   
   
   
   
   
   awk文件的
   
   #!/bin/awk -f
   #运行前
   BEGIN {
   math = 0
   english = 0
   computer = 0
   
   printf "NAME    NO.   MATH  ENGLISH  COMPUTER   TOTAL\n"
   printf "---------------------------------------------\n"
   }
   #运行中
   {
   math+=$3
   english+=$4
   computer+=$5
   printf "%-6s %-6s %4d %8d %8d %8d\n", $1, $2, $3,$4,$5, $3+$4+$5
   }
   #运行后
   END {
   printf "---------------------------------------------\n"
   printf "  TOTAL:%10d %8d %8d \n", math, english, computer
   printf "AVERAGE:%10.2f %8.2f %8.2f\n", math/NR, english/NR, computer/NR
   }
   
   怎么运行 awk -f score.awk  score.txt
   ```

   