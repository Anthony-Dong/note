# ==,hashcode,equals三者关系

## 1.  "=="

> 	1. 8种基本数据类型 , 直接比较值
>  	2. 引用对象 :  比较的是对象的内存地址 ,



> ​	对于 `Integer` 类型 ,我们可以看一下他的源码 : 

```java
  /**
     * Cache to support the object identity semantics of autoboxing for values between
     * -128 and 127 (inclusive) as required by JLS.
     * 
     * The cache is initialized on first usage.  The size of the cache
     * may be controlled by the {@code -XX:AutoBoxCacheMax=<size>} option.
     * During VM initialization, java.lang.Integer.IntegerCache.high property
     * may be set and saved in the private system properties in the
     * sun.misc.VM class.
     */
默认是 [-128,127] 这个区间内的值 .
  其中说这个cache第一次使用的时候初始化,他的大小由 `-XX:AutoBoxCacheMax=<size>` 控制的,当vm初始化的是够 .IntegerCache.high 这个会被设置和保存为私有系统属性在sun.misc.VM class

    private static class IntegerCache {}

所以一下
        Integer integer = 127;
        Integer integer2 = 127;
        Integer integer3 = -129;
        Integer integer4 = -129;
        System.out.println(integer==integer2);    //true
        System.out.println(integer3==integer4);    //true
当我们设置了 -XX:AutoBoxCacheMax=500 ,那么500以内的也是可以返回true
```





## 2.  hashcode()

> ​	As much as is reasonably practical, the hashCode method defined by class {@code Object} does return distinct integers for distinct objects. (This is typically implemented by converting the internal address of the object into an integer, but this implementation technique is not required by the Java&trade; programming language.)
>
> 说的就是 大多数情况下是非常实用的,他返回一个integer类型的数值去区分不同对象. 
>
> 这个数值通常由将对象的内部地址转而成的一个数值 , 但是这种实现技术并不被java语言所需要.

所以默认实现的 `hashcode()` 返回的int数值,其实就是内存地址转换过来的.



## 3. equals()

```java
public boolean equals(Object obj) {
        return (this == obj);
}
```



> ​	object 源码中对equals方法的实现 ,发现它只是做了两个对象的 '==' 比较





> ​		对于以上 三种方法对于基本数据类型的包装类 基本上都重写了 ,目的就是为了某些实际需求.



## 4 . intern()

> String 源码对于 intern 的解答
>
> A pool of strings, initially empty, is maintained privately by the class {@code String}.
>
> 这个池子被string这个类所维护 .
>
> When the intern method is invoked, if the pool already contains a string equal to this {@code String} object as determined by the {@link #equals(Object)} method, then the string from the pool is returned. Otherwise, this {@code String} object is added to the
> pool and a reference to this {@code String} object is returned.
>
> 就是说改方法被调用,如果这个string池子已经有了和这个string对象相同的对象(通过 equals()方法比较),那么就从这个池子中返回池子中的这个对象. 否则就加到池子里 .



```java
所以 下面代码理解了吧 ! 
String str1 = "hello";
String str2 = new String("hello");
System.out.println(str1 == str2);   //false
System.out.println(str1 == str2.intern());   //true
```





## 5.  所以对于这方面的问题纠结吗?

那就继续思考吧.