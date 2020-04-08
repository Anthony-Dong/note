# 入门Javassit - 字节码操作

> ​	`Javassit` 是通过直接书写`java代码`, 通过它提供了一套系统, 给你编译成字节码的操作 . `ASM` 是直接书写`字节码`, 所以难度上来很大的, 不是一般水平, 根本难以使用, 所以CGLIB基于ASM提出了一种代理的模板 , 为什么说是模板呢, 因为真正的字节码操作, 不仅仅局限在代理.  所以``



它如何区分JDK版本的  , 这个就告诉我们了, JDK版本每次变迁多了些设么. 

```java
public static final int MAJOR_VERSION;

static {
    int ver = JAVA_3;
    try {
        Class.forName("java.lang.StringBuilder");
        ver = JAVA_5;
        Class.forName("java.util.zip.DeflaterInputStream");
        ver = JAVA_6;
        Class.forName("java.lang.invoke.CallSite", false, ClassLoader.getSystemClassLoader());
        ver = JAVA_7;
        Class.forName("java.util.function.Function");
        ver = JAVA_8;
        Class.forName("java.lang.Module");
        ver = JAVA_9;
        List.class.getMethod("copyOf", Collection.class);
        ver = JAVA_10;
        Class.forName("java.util.Optional").getMethod("isEmpty");
        ver = JAVA_11;
    }
    catch (Throwable t) {}
    MAJOR_VERSION = ver;
}
```



## 1. 简单入门

> ​	**由于我们开发使用的话, 绝对不可能修改当前已有的类, 除非agent 或者自定义ClassLoader. 因为Java不支持卸载系统类加载器加载的类, 你说恶不恶心, 除非agent , **   因此我们这个例子只是生产一个新的类, 去继承父类的方法. 

```java
public class Parent {

    public void test() {
        System.out.println(this + " : echo");
    }

    public static void main(String[] args) throws Exception {
        // 1. 默认生成一个 ClassPool , 其实很简单就是加入了Java的基本包装类型9种和加入了Java的Object类.
        ClassPool pool = ClassPool.getDefault();

        // 2. 去拿到我们com.javassit.demo.Parent类 , 由于Javassit 中最基本的类单元是CTClass .
        CtClass parent = pool.getCtClass("com.javassit.demo.Parent");
        // 3. 创建一个 子对象, 去继承父对象
        CtClass child = pool.makeClass("com.javassit.demo.Child", parent);
        // 4. 我们直接可以拿到该类对象了.
        Class<?> cClass = child.toClass();
        // 5. 直接基本构造器实例化就行了.
        Parent ins = (Parent) cClass.getConstructor().newInstance();
        // 6. invoke test
        ins.test();
    }
}
```

上诉代码流程很简单, 最后会调用成功, 就是一个简单的例子.   下面我们要添加东西了. 



## 2. 添加构造器

> ​	构造器 , 是类基本结构 , 所以提高了`CtConstructor` 定义一个构造器. 

```java
public class Parent {
    private Long id;

    // 最好使用包装类型. 基本类型好像失败.
    public Parent(Long id) {
        this.id = id;
    }

    public long getId() {
        return id;
    }

    public static void main(String[] args) throws Exception {
        ClassPool pool = ClassPool.getDefault();
        CtClass parent = pool.getCtClass("com.javassit.demo2.Parent");
        CtClass child = pool.makeClass("com.javassit.demo2.Child", parent);

        // 添加构造器, 只用写出构造器如何书写就行了. 纯JAVA代码
        child.addConstructor(CtNewConstructor.make("public Child(Long id){super(id);}", child));
        
        Class<?> cClass = child.toClass();
        Parent ins = (Parent) cClass.getConstructor(Long.class).newInstance(100L);
        long id = ins.getId();
        System.out.println(id);
    }
}
```

其中 `CtNewConstructor.make("public Child(Long id){super(id);}", child)`  这个make操作会去编译你的Java代码, 编译成字节码, 由于我字节码学的不好, 不展开解释了. .

## 3. 为什么反射必须使用包装类型

为什么非要使用包装类型呢, 是因为Java的反射的缺陷, 这也是Java的弊端.  你不信可以自己反射去实例化一个对象. 基本类型, 根本无法实例化 , 不管是不是基础, 原因是Java会自动拆箱 , 装箱. 不是你手动执行的.  

当构造器 是一个基本数据类型, 不会有拆箱操作的. 如果我们传入Long类型, 而且反射只能传入包装类型, 此时注定会失败的. 

```java
 0: aload_0
 1: invokespecial #1                  // Method java/lang/Object."<init>":()V
 4: aload_0
 5: lload_1
 6: putfield      #2                  // Field id:J
 9: return
```

如同下属代码 : 

```java
Long x = 100L;
long y = x;
```

实际上走了两次转换, 因为Java的字面量全是基本数据类型 , 所以100L - > Long , 需要valueOf,  而第二次 Long -> long 需要拆箱, 也就是langValue . 

```java
 0: ldc2_w        #3                  // long 100l
 3: invokestatic  #5                  // Method java/lang/Long.valueOf:(J)Ljava/lang/Long;
 6: astore_1
 7: aload_1
 8: invokevirtual #6                  // Method java/lang/Long.longValue:()J
```

## 4. 修改方法体

> ​	我们这个例子, 直接在 Parent上修改 他的类构造器, 因此需要定义一个ClassLoader. 

```java
public class Parent {
    private long id;

    // 最好使用包装类型. 基本类型好像失败.
    public Parent(Long id) {
        this.id = id;
    }


    public static void main(String[] args) throws Exception {
        ClassPool pool = ClassPool.getDefault();
        // 1. 加载目前的字节码
        CtClass parent = pool.getCtClass("com.javassit.demo3.Parent");


        // 2. 就是插入输出
        for (CtConstructor constructor : parent.getConstructors()) {
            // 插入到前面, 插入到后面. 
            // 第一个参数是$1
            constructor.insertAfter("System.out.println(\"id=\"+$1);");
        }

        // 3. 输出字节码
        byte[] bytes = parent.toBytecode();

        // 4. 遵循双亲委派模型, 去加载类 , 破坏会发现一堆问题 , 一堆类找不到
        Map<String, Class<?>> cache = new ConcurrentHashMap<>();
        ClassLoader classLoader = new ClassLoader(Thread.currentThread().getContextClassLoader()) {
            @Override
            public Class<?> loadClass(String name) throws ClassNotFoundException {
                // 如果使我们需要的类,直接放到catch中. 这种容易出现内存泄漏, 但是重复定义会报错的. 只能这么做了.
                if (name.equals("com.javassit.demo3.Parent")) {
                    if (cache.get(name) == null) {
                        Class<?> aClass = defineClass(name, bytes, 0, bytes.length);
                        cache.put(name, aClass);
                    }
                    return cache.get(name);
                }
                // 委派给父类加载
                return super.loadClass(name);
            }
        };

        Class<?> aClass = classLoader.loadClass(Parent.class.getName());
        // 这里不能类型转换, 不然会失败的. 因为会有俩不同的类存在
        Object o = aClass.getConstructor(Long.class).newInstance(1L);
        System.out.println("对象的类加载器是 : "+o.getClass().getClassLoader());
    }
}
// 输出 : 
// id=1
// 对象的类加载器是 : com.javassit.demo3.Parent$1@2038ae61
```

最后生成的代码如下

```java
public Parent(Long id) {
    this.id = id;
    // 这个是方法的返回值. 所以构造器. 默认是空的.
    Object var3 = null;
    System.out.println("id=" + id);
}
```



## 5. 创建字段

```java
public class JavaBean {

    public static void main(String[] args) throws Exception {
        ClassPool pool = ClassPool.getDefault();
        String className = "com.javassit.demo5.GavaBean";
        CtClass cc = pool.makeClass(className);

        // 创建字段, 需要申明类型 . 名字, 和什么插入的类
        CtField field = new CtField(pool.get(String.class.getName()), "name", cc);
        // 设置访问类型
        field.setModifiers(Modifier.PRIVATE);
        // 添加字段
        cc.addField(field);
        // 直接可以创建get set方法.
        cc.addMethod(CtNewMethod.setter("setName", field));
        cc.addMethod(CtNewMethod.getter("getName", field));

        // 创建无参构造器.
        CtConstructor constructor = CtNewConstructor.make(null, null, cc);
        constructor.setBody("{System.out.println(\"instance\");}");
        cc.addConstructor(constructor);

        CtConstructor pc = new CtConstructor(new CtClass[]{pool.get(String.class.getName())}, cc);
        // $0=this , $1,$2,$3... 代表方法参数 , 因为非静态方法默认就是带一个this指针,默认就是第0个参数
        pc.setBody("{$0.name = $1;System.out.println(\"instance\");}");
        cc.addConstructor(pc);

        Class<?> GavaBeanClass = cc.toClass();
        Object tom = GavaBeanClass.getConstructor(String.class).newInstance("tom");
        Method method = GavaBeanClass.getMethod("getName");

        Object invoke = method.invoke(tom);
        System.out.println(invoke);

        cc.writeFile("D:\\代码库\\aop");
    }
}
```





## AOP

> ​	由于前面插入的 insert before 和 after 两者之间还不能有共通的变量 , 所以很麻烦,  , 还取不到方法体.  等我修炼. 



## 参考文章

[https://www.jianshu.com/p/b9b3ff0e1bf8](https://www.jianshu.com/p/b9b3ff0e1bf8)



