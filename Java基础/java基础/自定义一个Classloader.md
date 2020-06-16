# Java - 自定义一个Classloader

> ​	类加载器, 其实是一个很重要的东西 ,Java的类 其实就是元信息, 拿到元信息啥都可以做,  所以最后它放到了元信息的内存中 . 	

其实有些时候需要自定义类加载器. 如果不同容器之间, 比如Tomcat这个服务器, 可以存在多个容器. 所以他就需要自定义个类加载器. 

## 1. 自己写一个类加载器.

类加载器, 第一我们必须拿到字节流流, 也就是编译后的Class文件, 如何拿到 , 

Classloader提供了 `getClass().getResourceAsStream(class_name)` 方法可以拿到与当前class对象同级目录下的代码.  于此同时还有一个 `getClassLoader().getResourceAsStream(class_name)`他是拿到path路径下的同级目录下的字节码.

上面说的是依赖于Java生成的字节码. 其实很多工具可以生成字节码的. 比如 javassit , asm , cjlib哇. 

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

> ​	以上例子就是一个简单的自定义类加载器的过程 .  



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



## 2. 类加载器 如何卸载类 , 防止内存泄漏

> ​	文章主要是这一篇. [https://www.jianshu.com/p/de971579fcac](https://www.jianshu.com/p/de971579fcac)

其中关于类的卸载  , 主要条件有一下几点

1.该类所有实例对象不可达
2.该类的Class对象不可达
3.该类的ClassLoader不可达

也就是满足一下三点, 可以满足卸载这个类. 

我们可以通过一下几个参数观察 : 

-verbose:class 用于同时跟踪类的加载和卸载
-XX:+TraceClassLoading 单独跟踪类的加载
-XX:+TraceClassUnloading 单独跟踪类的卸载



其实满足一下三点很简单. 就是我们将我们的classloader 直接 被GC掉就行了. 

```java
public class JavaLoad {

    public static void main(String[] args) throws Exception {
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
        };



        Class<?> aClass = classLoader.loadClass(Demo.class.getCanonicalName());
        Object obj =  aClass.getConstructor().newInstance();
        // 1. 对象不可达
        obj = null;
        
        // 2. 类不可达
        aClass = null;
        
        // 3. load不可达
        classLoader = null;

        // 这个类被回收
        System.gc();
    }
}
```



最终打印

```java
// 系统的加载器
[Loaded com.javassit.Demo from file:/D:/%e4%bb%a3%e7%a0%81%e5%ba%93/aop/agent/java-agent/target/classes/]

// 自定义加载器 加载的.
[Loaded com.javassit.Demo from __JVM_DefineClass__]
[Unloading class com.javassit.Demo 0x00000007c0061028]
```



## 不同的类加载器加载的类

不同的类加载器加载的类, 是不能互通访问的. 

比如上面的 . 为啥Demo被加载两次. 

比如我们这么写 

```java
Demo obj = (Demo) aClass.getConstructor().newInstance();
```

一定会报错的.

那么如何解决这个问题呢, 那么就依赖于Java的继承(父类 或者 多接口)模式了.  

一下我依靠于Java的第三方库 JavaSSIT , 给大家展示一下如何实现单基础. 

加入这个是我们的父类 

```java
public class Parent {

    public void getName() {
        System.out.println("classname : " + this.getClass().getName());
    }
}
```



最终是一个这样的代码

```java
import javassist.*;

import java.util.HashMap;
import java.util.Map;
/**
 * <dependency>
 * <groupId>org.javassist</groupId>
 * <artifactId>javassist</artifactId>
 * <version>3.25.0-GA</version>
 * </dependency>
 *
 * @date:2020/4/3 22:25
 * @author: <a href='mailto:fanhaodong516@qq.com'>Anthony</a>
 */
public class CreatePerson {

    /**
     * 创建一个Person 对象
     *
     * @throws Exception
     */
    public static byte[] createPseson() throws Exception {
        ClassPool pool = ClassPool.getDefault();

        // 1. 创建一个空类
        CtClass cc = pool.makeClass("com.javassist.Person");

        
        // 2. 创建一个父类
        CtClass makeClass = pool.makeClass(CreatePerson.class.getClassLoader().getResourceAsStream("com/javassit/Parent.class"));


        // 3. 空类继承父类
        cc.setSuperclass(makeClass);

        // 4. 新增一个字段 private String name;
        // 字段名为name
        CtField param = new CtField(pool.get("java.lang.String"), "name", cc);
        // 访问级别是 private
        param.setModifiers(Modifier.PRIVATE);
        // 初始值是 "xiaoming"
        cc.addField(param, CtField.Initializer.constant("xiaoming"));

        // 5. 生成 getter、setter 方法
        cc.addMethod(CtNewMethod.setter("setName", param));
        cc.addMethod(CtNewMethod.getter("getName", param));

        // 6. 添加无参的构造函数
        CtConstructor cons = new CtConstructor(new CtClass[]{}, cc);
        cons.setBody("{name = \"xiaohong\";}");
        cc.addConstructor(cons);

        // 7. 添加有参的构造函数
        cons = new CtConstructor(new CtClass[]{pool.get("java.lang.String")}, cc);
        // $0=this / $1,$2,$3... 代表方法参数
        cons.setBody("{$0.name = $1;}");
        cc.addConstructor(cons);

        // 8. 创建一个名为printName方法，无参数，无返回值，输出name值
        CtMethod ctMethod = new CtMethod(CtClass.voidType, "printName", new CtClass[]{}, cc);
        ctMethod.setModifiers(Modifier.PUBLIC);
        ctMethod.setBody("{System.out.println(name);}");
        cc.addMethod(ctMethod);

        return cc.toBytecode();
    }


    public static void main(String[] args) {
        Map<String, Class<?>> cache = new HashMap<>();
        try {
            ClassLoader loader = new ClassLoader() {
                @Override
                public Class<?> loadClass(String name) throws ClassNotFoundException {
                    if (name.equals("com.javassist.Person")) {
                        synchronized (cache) {
                            if (cache.get(name) == null) {
                                try {
                                    byte[] pseson = createPseson();
                                    Class<?> defineClass = defineClass(name, pseson, 0, pseson.length);
                                    cache.put(name, defineClass);
                                    return defineClass;
                                } catch (Exception e) {
                                    e.printStackTrace();
                                }
                            }
                        }
                    }
                    return super.loadClass(name);
                }
            };


            Class<?> name = loader.loadClass("com.javassist.Person");
            Parent parent = (Parent) name.getConstructor().newInstance();
            System.out.println(parent);
            parent.getName();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
```

最终输出 

```java
com.javassist.Person@3b95a09c
classname : com.javassist.Person
```

所以只能依赖于这个. 