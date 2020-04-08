# Java- Agent技术 (踩坑篇)

比较给力的两种模式 ,  `premain`  和 `agentmain` 这是两种代理模式.  第一种就是运行前代理, 第二种是运行时代理. 

这两种代理模式 , 基于他们实现的工具 , 第一种模式有 `SkyWalking`这种链路追踪工具 和 基于第二种模式的是 `Arthas` 的线上服务工具. 都是很给力的开源项目. 有兴趣的可以看看. 这种模式比Java的硬编码的动态代理更加少编码侵入性, 指的是用户写的代码, 不用专门去修改, 而只用去启动一个代理就可以了.  我就来举例子吧.   

## 1. 运行前编译

> ​	这个其实最简单了, 因为是运行前编译 , 根本不怕运行中出现什么事情. 

这里有一个类, 在 `com.example.demo.Demo2` 这个目录下面. 

```java
public class Demo2 {

    public static void main(String[] args) throws InterruptedException {
        while (true) {
            TimeUnit.SECONDS.sleep(1);
            System.out.println("invoke - 1");
        }
    }
}
```

我们的目的是让他循环输出 `"invoke - 1"`  , 那么怎么办呢, 不使用动态代理. 

我们可以基于agent , 在他运行前代理一个.  我提前编译好了一个文件, `invoke-2`的class文件, 省的用字节码技术改了. 

```java
public class JavaPreMain {


    public static void premain(String arg, Instrumentation inst) {
        System.out.println("premain init");

        inst.addTransformer(new ClassFileTransformer() {
            @Override
            public byte[] transform(ClassLoader loader, String className, Class<?> classBeingRedefined, ProtectionDomain protectionDomain, byte[] classfileBuffer) throws IllegalClassFormatException {
                // 我们只去代理我们的代理类.
                if (className.equals("com/example/demo/Demo2")) {
                    try (FileInputStream stream = new FileInputStream("D:\\代码库\\aop\\agent\\Demo2.class")) {
                        int available = stream.available();
                        byte[] bytes = new byte[available];
                        stream.read(bytes);
                        return bytes;
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
                // 不执行任何转换, 返回null, 便可以 , 官方说的.
                return null;
            }
        });
        System.out.println("premain end");
    }
}
```

代码很简单就是如上的, 关于提前编译, 可以用idea 编译一份好的, 然后拿出来 . 

打包工具

```java
<plugin>
    <groupId>org.apache.maven.plugins</groupId>
    <artifactId>maven-jar-plugin</artifactId>
    <version>3.1.2</version>
    <configuration>
        <archive>
            <manifest>
                <addClasspath>true</addClasspath>
            </manifest>
            <manifestEntries>
                <!--这里就是你META-INF生成文件加的部分-->
                <Premain-Class>
                    com.agent.JavaPreMain
                </Premain-Class>
                <Can-Redefine-Classes>true</Can-Redefine-Classes>
                <Can-Retransform-Classes>true</Can-Retransform-Classes>
            </manifestEntries>
        </archive>
    </configuration>
</plugin>
```

直接maven打包一下就可以了  .  ` mvn clean package`



关于如何使用, 我们在我们的Demo2 上面加入 `-javaagent:D:\\代码库\\aop\\agent\\java-agent\\target\\java-agent-0.0.1.jar`  这个vm信息. 既可.  

启动后, 发现确实代理了.  会改成输出 2 . 



关于上面那个 args参数 如何传入, 可以通过 : `-javaagent:D:\代码库\aop\agent\java-agent\target\java-agent-0.0.1.jar=xiaoli` 传入, `=` 后面跟着的就是参数了 . 可以接收到.



## 运行时代理编译

这个过程吧, 有点像什么呢, 就是运行时, 我给你改一些动态加载的类 .   类似于arthas 的 mc .  这个是最麻烦那了, 很多的限制有. 主要是因为运行期么. 必然有一些限制. 

这个是我写的代理类. 也复用了上面很大一部分, 

```java
public class JavaAgentMain {

    public static void agentmain(String args, Instrumentation inst) {
        System.out.println("Agent - init");
        inst.addTransformer(new ClassFileTransformer() {
            @Override
            public byte[] transform(ClassLoader loader, String className, Class<?> classBeingRedefined,
                                    ProtectionDomain protectionDomain, byte[] classfileBuffer)
                    throws IllegalClassFormatException {
                System.out.println("className : " + className);
                if (className.equals("com/example/demo/Demo2")) {
                    try (FileInputStream stream = new FileInputStream("D:\\代码库\\aop\\agent\\Demo2.class")) {
                        int available = stream.available();
                        byte[] bytes = new byte[available];
                        stream.read(bytes);
                        System.out.println("代理成功 ........");
                        return bytes;
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
                return null;
            }
        }, true);
        // 这个必须设置为true , 就是接受重新转换的请求.false是不会走的.
        try {
            // 这个过程相当于, 重新转变这个类.
            Class<?> aClass = Class.forName("com.example.demo.Demo2");
            inst.retransformClasses(aClass);
        } catch (UnmodifiableClassException | ClassNotFoundException e) {
            e.printStackTrace();
        }
        System.out.println("agent - end");
    }
}
```

还是需要在配置文件中加入这个 就是多了个 agent-class . 

```java
<plugin>
    <groupId>org.apache.maven.plugins</groupId>
    <artifactId>maven-jar-plugin</artifactId>
    <version>3.1.2</version>
    <configuration>
        <archive>
            <manifest>
                <addClasspath>true</addClasspath>
            </manifest>
            <manifestEntries>
                <!--这里就是你META-INF生成文件加的部分-->
                <Premain-Class>
                    com.agent.JavaPreMain
                </Premain-Class>
                <Agent-Class>
                    com.agent.JavaAgentMain
                </Agent-Class>
                <Can-Redefine-Classes>true</Can-Redefine-Classes>
                <Can-Retransform-Classes>true</Can-Retransform-Classes>
            </manifestEntries>
        </archive>
    </configuration>
</plugin>
```



启动Demo2 这个类 . 不用加VM参数  , 加一个 `-verbose:class` 这个参数

我们写一个 attache的类 

```java
public class Demo {

    public static void main(String[] args) throws Exception {
        // 这个是PID. 每个JVM进程都有
        VirtualMachine attach = VirtualMachine.attach("4804");
        attach.loadAgent("D:\\代码库\\aop\\agent\\java-agent\\target\\java-agent-0.0.1.jar");
    }
}
```

就是我们这个了, 让他去告诉 demo2 这个程序, 

我们在demo2的控制台发现

```java
[Loaded com.agent.JavaAgentMain$1 from file:/D:/%e4%bb%a3%e7%a0%81%e5%ba%93/aop/agent/java-agent/target/java-agent-0.0.1.jar]
className : com/example/demo/Demo2
代理成功 ........
[Loaded com.example.demo.Demo2 from __VM_RedefineClasses__]
agent - end
```

趋势重新装载了 `com.example.demo.Demo2`  ,但是不一样的是, 是不同的类加载器加载的. 

那为什么 , 不会改变方法的调用输出呢. 

是因为 ,**`retransformClasses`**：**对于已经加载的类重新进行转换处理，即会触发重新加载类定义，需要注意的是，新加载的类不能修改旧有的类声明，譬如不能增加属性、不能修改方法声明；**

这也就是为什么 , 他不能修改调用方法了 .  其实还有一个特别的地方就是 他可以修改属性字段值. 我试过是可以成立的.   比如我们第一份的 id是500  , 第二份是1000 , 将第一份修改成第二份是满足的. 

```java
public class Demo2 {

    private int id = 500;

    @Override
    public String toString() {
        return "Demo2{" +
                "id=" + id +
                '}';
    }

    public static void main(String[] args) throws InterruptedException {
        while (true) {
            TimeUnit.SECONDS.sleep(1);
            System.out.println(new Demo2().toString());
        }
    }
}
```



**那么我们可以通过 `redefineClasses`  方法直接将字节码告诉 JVM 加载.   而不是走`addTransformer` 的回调了. 他是直接返回字节码数组的.**   但是这种也是一个问题, 他不能代理 Main方法的,  我们上诉代码, 还需要改动. 



这个是Java的代理类. 

```java
public class JavaAgentMain {

    public static void agentmain(String args, Instrumentation inst) {
        System.out.println("Agent - init");
        try {
            Class<?> demo2 = Class.forName("com.example.demo.Demo2");
            inst.redefineClasses(new ClassDefinition(demo2, getBytes()));
        } catch (ClassNotFoundException | UnmodifiableClassException e) {
            e.printStackTrace();
        }

        System.out.println("agent - end");
    }
    private static byte[] getBytes() {
        try (FileInputStream stream = new FileInputStream("D:\\代码库\\aop\\agent\\Demo2.class")) {
            int available = stream.available();
            byte[] bytes = new byte[available];
            stream.read(bytes);
            System.out.println(Thread.currentThread().getContextClassLoader() + " : 代理成功 ........");
            return bytes;
        } catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }
}
```

然后我们的Demo2也得改改  一个改成输出 `"invoke -1"`  一个输出 `"invoke -2"`

```java
public class Demo2 {

    public static void main(String[] args) throws InterruptedException {
        while (true) {
            TimeUnit.SECONDS.sleep(1);
            test();
        }
    }

    public static void test(){
        System.out.println("invoke -1");
    }
}
```

这么就可以动态修改了  我下面 是我的日志信息

```java
invoke -1
invoke -1
[Loaded java.lang.CharacterData00 from C:\Program Files\Java\jdk1.8.0_221\jre\lib\rt.jar]
Agent - init
[Loaded java.lang.instrument.ClassDefinition from C:\Program Files\Java\jdk1.8.0_221\jre\lib\rt.jar]
sun.misc.Launcher$AppClassLoader@18b4aac2 : 代理成功 ........
[Loaded com.example.demo.Demo2 from __VM_RedefineClasses__]
agent - end
invoke -2
invoke -2
invoke -2
```



## 总结

基本上就是这些了, 其实我只是简单的告诉你功能, 但是如何继续封装, 如何继续使用剩下的都是你的事情了, 坑我已经帮你踩完了 . 我测试这些的时候, 无数次尝试. 啊哈哈哈. 太难了. 不用开发工具还是很快的 . 