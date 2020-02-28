# Java - 自定义一个Classloader

> ​	类加载器, 其实是一个很重要的东西 ,Java的类 其实就是元信息, 拿到元信息啥都可以做,  所以最后它放到了元信息的内存中 . 	

其实有些时候需要自定义类加载器. 如果不同容器之间, 比如Tomcat这个服务器, 可以存在多个容器. 所以他就需要自定义个类加载器. 



类加载器, 第一我们必须拿到文件流, 也就是编译后的文件在哪, 如何拿到 , 

Classloader提供了 `getClass().getResourceAsStream(class_name)` 方法可以拿到与当前class对象同级目录下的代码.  于此同时还有一个 `getClassLoader().getResourceAsStream(class_name)`他是拿到path路径下的同级目录. 

拿到字节流, (class文件的字节流) , 那么如何定义一个类呢 , 需要 `defineClass()` , 来定义类. 



下面是一个简单的例子. 

```java
ClassLoader classLoader = new ClassLoader() {
    @Override
    public Class<?> loadClass(String name) throws ClassNotFoundException {
        // 输入 ->com.example.Bean
        // Bean.class
        String class_name = name.substring(name.lastIndexOf(".") + 1) + ".class";
        try {
            InputStream stream = getClass().getResourceAsStream(class_name);
            // 如果我们拿不到对象, 就走父类加载(这就是双亲委派模型)
            if (stream == null) {
                return super.loadClass(name);
            }
            // 拿到字节流
            byte[] bytes = new byte[stream.available()];
            stream.read(bytes);
            // 最后定义(native接口)
            return defineClass(name, bytes, 0, bytes.length);
        } catch (IOException e) {
            throw new ClassNotFoundException(name);
        }
    }
}
```

> ​	以上例子就是一个类加载器的过程 . 





一下是一个类加载器的关系图 , 一般面试喜欢问, 什么是双亲委派模型. 

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-20-22/f7f363ba-bda9-4716-9c93-2d33c8a83c91.png?x-oss-process=style/template01)



这个关系图 , 其实对于自定义来说, 是可以不遵循,双亲委派模型的. 但是对于系统自带的确实属于. 

```java
protected Class<?> loadClass(String name, boolean resolve)
    throws ClassNotFoundException
{
    synchronized (getClassLoadingLock(name)) {
        // 1. 检查是否已经加载过了, 这里其实对于我们来说,平时使用LRU应该清楚.
        Class<?> c = findLoadedClass(name);
        if (c == null) {
            long t0 = System.nanoTime();
            try {
            	// 2. 父类加载器有没有.有就递归指向父类的.(ext)
                if (parent != null) {
                    c = parent.loadClass(name, false);
                } else {
                // 3.如果到了跟,那么就走系统定义的加载器.(比如rt包)
                    c = findBootstrapClassOrNull(name);
                }
            } catch (ClassNotFoundException e) {
                // ClassNotFoundException thrown if class not found
                // from the non-null parent class loader
            }

            if (c == null) {
                // If still not found, then invoke findClass in order
                // to find the class.
                long t1 = System.nanoTime();
                c = findClass(name);

                // this is the defining class loader; record the stats
                sun.misc.PerfCounter.getParentDelegationTime().addTime(t1 - t0);
                sun.misc.PerfCounter.getFindClassTime().addElapsedTimeFrom(t1);
                sun.misc.PerfCounter.getFindClasses().increment();
            }
        }
        if (resolve) {
            resolveClass(c);
        }
        return c;
    }
}
```



> ​	双亲委派模型解决的问题就是一个进程中只会存在一个类 , 不存在覆盖的可能性 .  父类的优先级高于子类, 也就是我们永远无法覆盖 , Java定义的核心包中的类.





其次就是我们自定义的class. 如果你使用 `-XX:+TraceClassLoading` 参数启动, 是我们定义的类加载器加载的话, 会打印这个 , form jvm define , (还有不能重复让JVM加载, 我们上诉代码的话是, 我们可以使用map存起来)

```java
[Loaded com.common.time.test.Bean from __JVM_DefineClass__]
```









