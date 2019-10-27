# java中System.getProperty()的作用及使用

## System.getProperties()

**public static Properties getProperties()确定当前的系统属性.** 

​		首先,如果有安全管理器,则不带参数直接调用其 checkPropertiesAccess 方法.这可能导致一个安全性异常. 
将 getProperty(String) 方法使用的当前系统属性集合作为 Properties 对象返回.如果没有当前系统属性集合,则先创建并初始化一个系统属性集合.这个系统属性集合总是包含以下键的值: 键 相关值的描述 

java.version Java 运行时环境版本 

java.vendor Java 运行时环境供应商 

java.vendor.url Java 供应商的 URL 

java.home Java 安装目录 

java.vm.specification.version Java 虚拟机规范版本 

java.vm.specification.vendor Java 虚拟机规范供应商 

java.vm.specification.name Java 虚拟机规范名称 

java.vm.version Java 虚拟机实现版本 

java.vm.vendor Java 虚拟机实现供应商 

java.vm.name Java 虚拟机实现名称 

java.specification.version Java 运行时环境规范版本 

java.specification.vendor Java 运行时环境规范供应商 

java.specification.name Java 运行时环境规范名称 

java.class.version Java 类格式版本号 

java.class.path Java 类路径 

java.library.path 加载库时搜索的路径列表 

java.io.tmpdir 默认的临时文件路径 

java.compiler 要使用的 JIT 编译器的名称 

java.ext.dirs 一个或多个扩展目录的路径 

os.name 操作系统的名称 

os.arch 操作系统的架构 

os.version 操作系统的版本 

file.separator 文件分隔符(在 UNIX 系统中是"/") 

path.separator 路径分隔符(在 UNIX 系统中是":") 

line.separator 行分隔符(在 UNIX 系统中是"/n") 

user.name 用户的账户名称 

user.home 用户的主目录 

user.dir 用户的当前工作目录 

系统属性值中的多个路径是用平台的路径分隔符分隔的. 

注意,即使安全管理器不允许执行 getProperties 操作,它可能也会选择允许执行 getProperty(String) 操作.



- 使用方式 : 

**如何添加自定义参数** 

java  -Dheart=12345 .........

idea呢 : 直接在下面写  -Dport=12345  -Dheart=aaa

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-10-02/4c5a0d3c-b7b2-4f65-a40b-bb39797d8240.png?x-oss-process=style/template01)



**获取的第一种方法**

```java
// 第一个参数  :  property name.
// 第二个参数 def :  default value.
System.getProperty("heart", "heart")
```



**获取的第二种方法**

```java
// 第一个参数 :  property name.
// 第二个参数 :  default value.
Integer.getInteger("heart", 10)  
```





