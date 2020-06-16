# Java反射机制

> ​	java反射绝对是一个很强大的功能 ,他不需要使用 new 关键字显示的申明来实例化一个对象, 而是通过 `java.lang.reflect` 的一些包进行控制 
>
> ​	其实绝大多数框架实现都是拿到类对象首先, 然后可能是根据动态代理或者其他去实现一个真正的对象

` Class , Method ,  Field , Annotation ,Constructor, Unsafe  ,Classloader ` 基本就这几个类, 加上动态代理的 `Proxy `类 , 很简单的 , 其实就是玩字节码 , 懂了吧

## 1. Class.forName 与 ClassLoader().loadClass

```java
public class TestClass {
    static {
        System.out.println("调用了静态块");
    }

    public TestClass() {
        System.out.println("调用了构造方法");
    }
}
```

测试类一 : 

```java
public static void main(String[] args) throws Exception {
    System.out.println("========Class.forName=============");
    Class.forName("com.javase.unsafe.TestClass");
}
```

输出: 

```java
调用了静态块
```

测试类二 : 

```java
public static void main(String[] args) throws Exception {
    System.out.println("========ClassLoader().loadClass=============");
    Thread.currentThread().getContextClassLoader().loadClass("com.javase.unsafe.TestClass");
}
```

输出: 

```java
========ClassLoader().loadClass=============
```

我们发现 `java.lang.Class#forName(java.lang.String)`这个方法调用的时候会加载类的静态代码块 , 而 `java.lang.ClassLoader#loadClass(java.lang.String)` 在加载类的时候不会加载类的静态代码块, 所以需要注意一下. 

但是两个代码如果都执行`java.lang.Class#newInstance`,我们发现先出现的都会去加载静态代码块 , 

## 2. Unsafe 类

> ​	Unsafe 实例化对象是`不执行构造方法` , 注意这一点

```java
public static void main(String[] args) throws Exception {
    Unsafe unsafe = getUnsafe();
    TestClass o = (TestClass) unsafe.allocateInstance(TestClass.class);
    System.out.println("=====================================");
    new TestClass();
}
```

输出

```java
调用了静态块
==============
调用了构造方法
```



如何实例化Unsafe类 ,首先他是一个final, 而且构造方法私有化 , 显然我们无法实例化 , 但是有一个细节就是他又一个`私有`的静态变量 就是他自己,  如果我们调用 ` Unsafe.getUnsafe()` 方法的时候会出现因为类加载器的原因报错,

```java
public static Unsafe getUnsafe() throws Exception {
    Field filed = Unsafe.class.getDeclaredField("theUnsafe");
    // 私有成员变量,都需要设置这个
    filed.setAccessible(true);
    // 类变量不需要输入参数
    return (Unsafe) filed.get(null);
}
```

其实很简单获取类然后获取字段,注意私有的字段必须用`java.lang.Class#getDeclaredField` , 像`java.lang.Class#getField` 他是只能访问public的字段的, 所以我们获取私有后 ,然后就是设置可访问 , 因为私有吗, 所以成功获取了 , 所以获取私有的静态变量需要两部 一步就是获取字段, 然后设置访问权 , **非静态字段不行注意 , 因为他不属于类对象**



## 3. Field

java中的 `Field` 类 , 他可以获取字段所有的对象 , 使用很简单

#### 获取值

`public Object get(Object obj)`   , 参数是当前实例化对象

```java
public class TestField {

    private int num;

    public TestField(int num) {
        this.num = num;
    }

    public static void main(String[] args) throws Exception {
        // 1.实例化对象
        final TestField target = new TestField(10);
        // 2. 通过类的字节码拿到对应的Field对象
        final Field field = TestField.class.getDeclaredField("num");
        // 被private修饰需要设置可见性
        field.setAccessible(true);
        
        // 3. 通过get(target) 传入我们实例化的对象,拿到对应的对象
        Object o = field.get(target);
        
        // 
        System.out.println("num : "+o); // 输出 : 10
    }
}
```

#### 更改值 : 

`public void set(Object obj, Object value)`  , 参数一是当前对象, 参数二是修改的数值

```java
public class TestField {

    private int num;

    public TestField(int num) {
        this.num = num;
    }

    public int getNum() {
        return num;
    }

    public static void main(String[] args) throws Exception {
        // 1.实例化对象
        final TestField target = new TestField(10);
        // 2. 通过类的字节码拿到对应的Field对象
        final Field field = TestField.class.getDeclaredField("num");

        // 3. 通过get(target) 传入我们实例化的对象,拿到对应的对象
        Object o = field.get(target);
        System.out.println("获取当前 num : "+o); //输出 : 10


        // 基于 Unsafe类进行设置的
        field.set(target, 20);

        System.out.println("更改后的 num : "+target.num);  // 输出 : 20
    }
}

```



####  获取类对象的静态字段

测试对象

```java
public class TestField {

    private static int snum = 100;
}
```

测试用例

```java
public static void main(String[] args) throws Exception{

    // 获取静态字段
    final Field field = TestField.class.getDeclaredField("snum");

    // 必须设置可见性, 因为是private修饰
    field.setAccessible(true);

    // 他属于类对象, 所以设置为null
    Object o = field.get(null);

    System.out.println(o);  // 输出 : 100
}
```





## 4. Method对象

`method. invoke(obj,arg) `基本就是他的核心精髓了 

## 5. 简单的字节码扫描

```java
public class Main {

    public static void main(String[] args) throws Exception {
        Predicate<Class<?>> filter = new Predicate<Class<?>>() {
            @Override
            public boolean test(Class<?> aClass) {
                return !aClass.isAnnotation();
            }
        };

		// 存放类对象
        Set<Class<?>> classSet = new HashSet<>();

        String packName = "com.javase.spring";

        String packUrl = packName.replace('.', '/');
        
        // spring的ClassPathResource , 他将简单的封装了都,下面我的方法也是差不多实现了找个
        ClassPathResource resource = new ClassPathResource(packUrl);
	
        // 获取文件
        File file = resource.getFile();

        // 是否是文件夹
        if (file.isDirectory()) {
            // 获取文件 , 可能需要递归 .... 真麻烦我简单除暴
            File[] files = file.listFiles();
            for (File file1 : files) {
                String name = file1.getName();
                String className = packName + "." + name.substring(0, name.length() - 6);
                // 获取className ,最终目的
                Class<?> aClass = Class.forName(className);
                // 过滤
                if (filter.test(aClass)) {
                    classSet.add(Class.forName(className));
                }
            }
        }

        classSet.forEach((e) -> {
                System.out.println("Class : " + e);
        });
    }

    private static void test() throws IOException {
        String packName = "com.javase.spring";

        String packUrl = packName.replace('.', '/');

        Enumeration<URL> resources = Thread.currentThread().getContextClassLoader().getResources(packUrl);

        while (resources.hasMoreElements()) {
            URL url = resources.nextElement();

            Assert.isTrue(url.getProtocol().equalsIgnoreCase("file"), "必须是文件系统");

            String filePath = URLDecoder.decode(url.getFile(), "UTF-8");

            File file = new File(filePath);
            
            /// .......就和spring的一样了
        }
    }
}
```



## 6. 字节码扫描工具类

```java
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.FileFilter;
import java.io.IOException;
import java.lang.annotation.Annotation;
import java.lang.reflect.Modifier;
import java.net.JarURLConnection;
import java.net.URL;
import java.net.URLDecoder;
import java.util.Enumeration;
import java.util.LinkedHashSet;
import java.util.Set;
import java.util.function.Predicate;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

/**
 * 类扫描器
 * 
 * @author kingston
 */
public class ClassScanner {

	private static Logger logger = LoggerFactory.getLogger(ClassScanner.class);

	/**
	 * 默认过滤器（无实现）
	 */
	private final static Predicate<Class<?>> EMPTY_FILTER = clazz -> true;

	/**
	 * 扫描目录下的所有class文件
	 * 
	 * @param scanPackage 搜索的包根路径
	 * @return
	 */
	public static Set<Class<?>> getClasses(String scanPackage) {
		return getClasses(scanPackage, EMPTY_FILTER);
	}

	/**
	 * 返回所有的子类（不包括抽象类）
	 * 
	 * @param scanPackage 搜索的包根路径
	 * @param parent
	 * @return
	 */
	public static Set<Class<?>> listAllSubclasses(String scanPackage, Class<?> parent) {
		return getClasses(scanPackage, (clazz) -> {
			return parent.isAssignableFrom(clazz) && !Modifier.isAbstract(clazz.getModifiers());
		});
	}

	/**
	 * 返回所有带制定注解的class列表
	 * 
	 * @param scanPackage 搜索的包根路径
	 * @param annotation
	 * @return
	 */
	public static <A extends Annotation> Set<Class<?>> listClassesWithAnnotation(String scanPackage,
			Class<A> annotation) {
		return getClasses(scanPackage, (clazz) -> {
			return clazz.getAnnotation(annotation) != null;
		});
	}

	/**
	 * 扫描目录下的所有class文件
	 * 
	 * @param pack   包路径
	 * @param filter 自定义类过滤器
	 * @return
	 */
	public static Set<Class<?>> getClasses(String pack, Predicate<Class<?>> filter) {
		Set<Class<?>> result = new LinkedHashSet<Class<?>>();
		// 是否循环迭代
		boolean recursive = true;
		// 获取包的名字 并进行替换
		String packageName = pack;
		String packageDirName = packageName.replace('.', '/');
		// 定义一个枚举的集合 并进行循环来处理这个目录下的things
		Enumeration<URL> dirs;
		try {
			dirs = Thread.currentThread().getContextClassLoader().getResources(packageDirName);
			// 循环迭代下去
			while (dirs.hasMoreElements()) {
				// 获取下一个元素
				URL url = dirs.nextElement();
				// 得到协议的名称
				String protocol = url.getProtocol();
				// 如果是以文件的形式保存在服务器上
				if ("file".equals(protocol)) {
					// 获取包的物理路径
					String filePath = URLDecoder.decode(url.getFile(), "UTF-8");
					// 以文件的方式扫描整个包下的文件 并添加到集合中
					findAndAddClassesInPackageByFile(packageName, filePath, recursive, result, filter);
				} else if ("jar".equals(protocol)) {
					// 如果是jar包文件
					Set<Class<?>> jarClasses = findClassFromJar(url, packageName, packageDirName, recursive, filter);
					result.addAll(jarClasses);
				}
			}
		} catch (IOException e) {
			logger.error("", e);
		}

		return result;
	}

	private static Set<Class<?>> findClassFromJar(URL url, String packageName, String packageDirName, boolean recursive,
			Predicate<Class<?>> filter) {
		Set<Class<?>> result = new LinkedHashSet<Class<?>>();
		try {
			// 获取jar
			JarFile jar = ((JarURLConnection) url.openConnection()).getJarFile();
			// 从此jar包 得到一个枚举类
			Enumeration<JarEntry> entries = jar.entries();
			// 同样的进行循环迭代
			while (entries.hasMoreElements()) {
				// 获取jar里的一个实体 可以是目录 和一些jar包里的其他文件 如META-INF等文件
				JarEntry entry = entries.nextElement();
				String name = entry.getName();
				// 如果是以/开头的
				if (name.charAt(0) == '/') {
					// 获取后面的字符串
					name = name.substring(1);
				}
				// 如果前半部分和定义的包名相同
				if (name.startsWith(packageDirName)) {
					int idx = name.lastIndexOf('/');
					// 如果以"/"结尾 是一个包
					if (idx != -1) {
						// 获取包名 把"/"替换成"."
						packageName = name.substring(0, idx).replace('/', '.');
					}
					// 如果可以迭代下去 并且是一个包
					if ((idx != -1) || recursive) {
						// 如果是一个.class文件 而且不是目录
						if (name.endsWith(".class") && !entry.isDirectory()) {
							// 去掉后面的".class" 获取真正的类名
							String className = name.substring(packageName.length() + 1, name.length() - 6);
							try {
								// 添加到classes
								Class<?> c = Class.forName(packageName + '.' + className);
								if (filter.test(c)) {
									result.add(c);
								}
							} catch (ClassNotFoundException e) {
								logger.error("", e);  
							}
						}
					}
				}
			}
		} catch (IOException e) {
			logger.error("", e);
		}
		return result;
	}

	private static void findAndAddClassesInPackageByFile(String packageName, String packagePath,
			final boolean recursive, Set<Class<?>> classes, Predicate<Class<?>> filter) {
		// 获取此包的目录 建立一个File
		File dir = new File(packagePath);
		// 如果不存在或者 也不是目录就直接返回
		if (!dir.exists() || !dir.isDirectory()) {
			// log.warn("用户定义包名 " + packageName + " 下没有任何文件");
			return;
		}
		// 如果存在 就获取包下的所有文件 包括目录
		File[] dirfiles = dir.listFiles(new FileFilter() {
			// 自定义过滤规则 如果可以循环(包含子目录) 或则是以.class结尾的文件(编译好的java类文件)
			@Override
			public boolean accept(File file) {
				return (recursive && file.isDirectory()) || (file.getName().endsWith(".class"));
			}
		});
		// 循环所有文件
		for (File file : dirfiles) {
			// 如果是目录 则继续扫描
			if (file.isDirectory()) {
				findAndAddClassesInPackageByFile(packageName + "." + file.getName(), file.getAbsolutePath(), recursive,
						classes, filter);
			} else {
				// 如果是java类文件 去掉后面的.class 只留下类名
				String className = file.getName().substring(0, file.getName().length() - 6);
				try {
					// 添加到集合中去
					Class<?> clazz = Thread.currentThread().getContextClassLoader()
							.loadClass(packageName + '.' + className);
					if (filter.test(clazz)) {
						classes.add(clazz);
					}
				} catch (ClassNotFoundException e) {
					logger.error("", e);  
				}
			}
		}
	}

}
```

写的还不错奥 , 基本功能如下

![](https://tyut.oss-accelerate.aliyuncs.com/image/2019-12-15/af5d7c56-8db3-4d7a-a89e-ee457b742c7d.jpg?x-oss-process=style/template01)