# package-info的作用

## 1. 聊聊为什么会有他

1. 我们可能看`ElementType`会发现他有一个`PACKAGE`的类型,那我们怎么给一个包申明注解呢?

2. 这个时候就会发现我们怎么给一个包申明注解

3. 那么这时候就引出了它的作用

   ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-15/a0ac3fb6-cca5-4574-b8a1-4df831b0e23a.png?x-oss-process=style/template01)

4. 这个是什么呢,官方给的解释

   ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-15/9e2df005-c5e3-4f12-823e-243565e36ae7.png?x-oss-process=style/template01)

5. 那么它的作用范围就是在他申明的包的内部,出了就没有用了,这个作用需要知道

6. 还有就是我们知道类的作用范围是无法申明作用域的,都只能public,这个package-info内部写的类,就很好的限制了作用范围



## 2.尝试去使用它

```java
1. 我们尝试去写一个注解,他的目标在package上
package com.javase.package_info;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.PACKAGE)
public @interface MyAnnotation {
 	String value();
}


2. 写一个 package-info的类

@MyAnnotation("测试一哈")
package com.javase.package_info;

class PackageInfo{
    public void common(){
        System.out.println("我在package-info内部");
    }
}

interface Constant{
    String value = "测试";
}

enum Enumeration{
    CONSTANT("CONSTANT");


    private String value;

    Enumeration(String value) {
        this.value = value;
    }
}



3. 写一个测试类
package com.javase.package_info;

/**
 * package_info的作用
 *
 * @date:2019/9/15 12:29
 * @author: <a href='mailto:fanhaodong516@qq.com'>Anthony</a>
 */

public class Demo {

    public static void main(String[] args) {
        Package p = Package.getPackage("com.javase.package_info");
        MyAnnotation annotation = p.getAnnotation(MyAnnotation.class);
        String value = annotation.value();
        System.out.println("value = " + value);
        System.out.println("p.getName() = " + p.getName());


        PackageInfo packageInfo = new PackageInfo();
        packageInfo.common();

        System.out.println("Constant.value = " + Constant.value);

        System.out.println("Enumeration.CONSTANT = " + Enumeration.CONSTANT);
    }
}

```



- 我的目录结构

  ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-15/d63ef26d-8790-4a6c-9ea6-b7580b2b2e91.png?x-oss-process=style/template01)