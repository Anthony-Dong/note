# CGLIB的源码分析和深入理解

> ​	CGLIB 呢, 相对于Java的代理来说, 更加灵活和富有弹性, 他的功能提供的更加的强大, 依靠callback实现的功能的增强. 
>
> ​	通过这篇文章我相信你对于CGLIB的了解会提升一个档次的 .  本文开始从快速开始上手体验, 理解其核心模块,到源码分析其启动流程, 然后分析一些存在的问题. 
>
> ​	官网地址:  https://github.com/cglib/cglib

## 快速使用

需要被增强的类. 

```java
public class UserService {
    public String findUserNameById(int id) {
        return "name-" + id;
    }
}
```

启动类: 

```java
public class App {
    public static void main(String[] args) {
        // 设置字节码保存的地方.
        System.setProperty(DebuggingClassWriter.DEBUG_LOCATION_PROPERTY, "D:/test");
        // 1.创建一个增强器
        Enhancer enhancer = new Enhancer();
        // 2.设置父类
        enhancer.setSuperclass(UserService.class);
        // 3.设置回调接口
        enhancer.setCallback(new MethodInterceptor() {
            @Override
            public Object intercept(Object obj, Method method, Object[] args, MethodProxy proxy) throws Throwable {
                // invokeSuper是必须使用的.
                return "proxy" + proxy.invokeSuper(obj, args);
            }
        });
        // 4.启动
        UserService userService = (UserService) enhancer.create();
        String userNameById = userService.findUserNameById(1);
        System.out.println(userNameById);
    }
}
```

基本流程就是上面那么写的 , 那么 `CallBack`是什么呢 , 下文讲解

## CGLIB功能核心 - `CallBack`

> ​	这个玩意的功能的核心, 也就是CallBack , 依靠它来实现业务逻辑. 

### `MethodInterceptor`

> 顾名思义 拦截器. 所以就是一个环绕型的.

```java
public interface MethodInterceptor
extends Callback
{
    public Object intercept(Object obj, java.lang.reflect.Method method, Object[] args,
                               MethodProxy proxy) throws Throwable;
}
```

大多数都是使用的这个.  

注意一般是调用的话, 使用的是 `proxy.invokeSuper(obj, args)` , 不然会出现递归, 出不去的问题 . 

### `NoOp`

> ​	这个就是什么都不做. 就是代表这个方法不会做代理, 也就是不会重写父类的方法, 因此就直接走父类的实现. 

```java
public interface NoOp extends Callback
{
    public static final NoOp INSTANCE = new NoOp() { };
}
```

我们使用的使用只需要传入这个 `INSTANCE` 就行了.  实现逻辑很简单, 就是我不重写父类的方法不就行了,  哈哈哈哈. 就是这么走的. 

### `LazyLoader `

> ​	就是一个懒加载的机制. 说是懒加载, 其实也不是, 他指的是, 我们每次调用的时候吧, 我们不用我们`enhancer.create()`生成的这个对象, 这个确实是生成了, 而是使用的是`LazyLoader.loadObject()` 生成的对象去执行方法.  业务逻辑如下. 

```java
public interface LazyLoader extends Callback {
    Object loadObject() throws Exception;
}
```

实现原理的话 ,如下 . 很简单的其实是 . 

```java
public class App2 {
    public static void main(String[] args) {
        System.setProperty(DebuggingClassWriter.DEBUG_LOCATION_PROPERTY, "D:/test");
        Enhancer enhancer = new Enhancer();
        enhancer.setSuperclass(UserService.class);
        enhancer.setCallback((LazyLoader) UserService::new);
        UserService userService = (UserService) enhancer.create();
        String userNameById = userService.findUserNameById(1);
        System.out.println(userNameById);
    }
}
```

我们看看生成的字节码 , 就会发现其中的问题 , 原来就是这玩意哇. 哈哈哈哈. 

```java
public final String findUserNameById(int var1) {
    // 先去拿对象,但是每次都加锁.感觉效率好低.不如双重锁.
    return ((UserService)this.CGLIB$LOAD_PRIVATE_0()).findUserNameById(var1);
}
// 这里就是加了同步机制, 所以效率不高. 因为每次都要调用.
private final synchronized Object CGLIB$LOAD_PRIVATE_0() {
    // 这里判断一下对象是否是空. 空就创建一个.
    Object var10000 = this.CGLIB$LAZY_LOADER_0;
    if (var10000 == null) {
        LazyLoader var10001 = this.CGLIB$CALLBACK_0;
        if (var10001 == null) {
            CGLIB$BIND_CALLBACKS(this);
            var10001 = this.CGLIB$CALLBACK_0;
        }
        var10000 = this.CGLIB$LAZY_LOADER_0 = var10001.loadObject();
    }
    // 返回就行了.
    return var10000;
}
```

### `Dispatcher`

> ​	和LazyLoad 一样, 但是它是每次方法调用的时候都会去调用`loadObject`. 

```java
public interface Dispatcher extends Callback {
    Object loadObject() throws Exception;
}
```

字节码我们看看

```java
public final String findUserNameById(int var1) {
    Dispatcher var10000 = this.CGLIB$CALLBACK_0;
    if (var10000 == null) {
        CGLIB$BIND_CALLBACKS(this);
        var10000 = this.CGLIB$CALLBACK_0;
    }
    // 每次都会调用Dispatcher.loadObject() , 所以就是每次方法调用都回去执行调用.
    return ((UserService)var10000.loadObject()).findUserNameById(var1);
}
```

### `InvocationHandler`

和Java的代理一样的.  基本逻辑是一样的. 

```java
public interface InvocationHandler
extends Callback
{
    public Object invoke(Object proxy, Method method, Object[] args) throws Throwable;
}
```

生成的代码如下

```java
public final String findUserNameById(int var1) {
    try {
        InvocationHandler var10000 = this.CGLIB$CALLBACK_0;
        if (var10000 == null) {
            CGLIB$BIND_CALLBACKS(this);
            var10000 = this.CGLIB$CALLBACK_0;
        }
        return (String)var10000.invoke(this, CGLIB$findUserNameById$0, new Object[]{new Integer(var1)});
    } catch (var2) {
        // 异常处理
    } 
}

// CGLIB$findUserNameById$0 = Class.forName("com.example.aop.UserService").getDeclaredMethod("findUserNameById", Integer.TYPE); 就是我们父类的方法.
```

### `FixedValue`

> ​	他呢就是每次方法调用都会调用这个玩意作为 `FixedValue.loadObject()` 作为方法的返回值.

```java
public interface FixedValue extends Callback {
    Object loadObject() throws Exception;
}
```

生成的代码如下

```java
public final String findUserNameById(int var1) {
    FixedValue var10000 = this.CGLIB$CALLBACK_0;
    if (var10000 == null) {
        CGLIB$BIND_CALLBACKS(this);
        var10000 = this.CGLIB$CALLBACK_0;
    }
	// 他呢就是调用`loadObject()` 作为调用findUserNameById方法的返回值. 跟equals\toString都是这个逻辑.所以就是这逻辑.
    return (String)var10000.loadObject();
}
```



## 啥时候用 `CallbackFilter`

这个很简单,  意思就是一个过滤器. 可以生成不同的策略.  比如有些时候我们有多个 `MethodInterceptor` , 那怎么办呢. 显然是不行的了.  所以此时需要一个. Filter进行对号入座. 懂了吧. 

> ​	注意当多个`CallBack`的同时使用的时候必须使用 `CallbackFilter` 进入对号入座 . 

```java
public interface CallbackFilter {

    // int值, 返回值是 CallBack数组的索引下标, 也就是不能超过CallBack数组的下标.
    int accept(Method method);
}
```

我们来个简单的例子. 

```jaa
public class UserService {
    public String findUserNameById1(int id) {
        return "name1-" + id;
    }
    public String findUserNameById2(int id) {
        return "name2-" + id;
    }
}
```

比如说 , 我们改造一下原来的`UserService`实现.  多加一个方法, 让我们的拦截器一拦截方法一, 然后拦截器二拦截方法二.  其他方法我们使用父类的方法, 不去重写. 

```java
public class App {

    public static void main(String[] args) {
        Enhancer enhancer = new Enhancer();
        enhancer.setSuperclass(UserService.class);

        Callback[] callback = new Callback[3];
        callback[0] = (MethodInterceptor) (obj, method, args1, proxy) -> "MethodInterceptor1 : "+proxy.invokeSuper(obj, args1);
        callback[1] = (MethodInterceptor) (obj, method, args1, proxy) -> "MethodInterceptor2 : "+proxy.invokeSuper(obj, args1);
        callback[2] = NoOp.INSTANCE;

        enhancer.setCallbacks(callback);
        enhancer.setCallbackFilter(method -> {
            if (method.getName().equals("findUserNameById1")) {
                return 0;
            }
            if (method.getName().equals("findUserNameById2")) {
                return 1;
            }
            return 2;
        });
        UserService userService = (UserService) enhancer.create();
        System.out.println(userService.findUserNameById1(1));
        System.out.println(userService.findUserNameById2(1));
    }
}
```

输出 : 

```java
MethodInterceptor1 : name1-1
MethodInterceptor2 : name2-1
```

## Encher代理的原理(源码分析)

```java
public class App {

    public static void main(String[] args) {
        System.setProperty(DebuggingClassWriter.DEBUG_LOCATION_PROPERTY, "D:/test");
        Enhancer enhancer = new Enhancer();
        enhancer.setSuperclass(UserService.class);
        enhancer.setCallback(new Dispatcher() {
            @Override
            public Object loadObject() throws Exception {
                return new UserService();
            }
        });
        // 核心流程在这里.因为创建最麻烦. 
        UserService userService = (UserService) enhancer.create();
        String userNameById = userService.findUserNameById(1);
        System.out.println(userNameById);
    }
}
```

其实有很大一部分是在 `Enhancer`这个类初始化的时候, 进行的.  所以我们不研究初始化做了什么, 无非就是初始化类加载器和缓存呗. 

第一步进去 , 我们直接进入`Enhancer#createHelper` 中 . 

```java
private Object createHelper() {
    preValidate();
    Object key = KEY_FACTORY.newInstance((superclass != null) ? superclass.getName() : null,
            ReflectUtils.getNames(interfaces),
            filter == ALL_ZERO ? null : new WeakCacheKey<CallbackFilter>(filter),
            callbackTypes,
            useFactory,
            interceptDuringConstruction,
            serialVersionUID);
    this.currentKey = key;
    Object result = super.create(key);
    return result;
}
```

再往下面走 `AbstractClassGenerator#create`

```java
protected Object create(Object key) {
    try {
        //这里会去调用生成类, 其实我们的类,在这个方法返回的时候就创建好了,有兴趣可以看看源码.
        Object obj = data.get(this, getUseCache());
        // 这里呢累死与一个递归的过程. 第一次初始化,会调用这里
        if (obj instanceof Class) {
            return firstInstance((Class) obj);
        }
        // 一般初始化完成就是直接走这里. 
        return nextInstance(obj);
    } catch (e) {
       // 异常.处理.
    }
}
```

具体逻辑就是这个 . 就是靠字节码生成器生成就好了. 逻辑很麻烦的.  . 不做解释. 

```java
Function<AbstractClassGenerator, Object> load =
        new Function<AbstractClassGenerator, Object>() {
            public Object apply(AbstractClassGenerator gen) {
                // AbstractClassGenerator 生成一个 class.
                Class klass = gen.generate(ClassLoaderData.this);
                // 返回.
                return gen.wrapCachedClass(klass);
            }
        };
```

再往下面走 `Encher#nextInstance` 方法了

```java
protected Object nextInstance(Object instance) {
    //这里还要涉及到`EnhancerFactoryData`
    EnhancerFactoryData data = (EnhancerFactoryData) instance;
    Class[] argumentTypes = this.argumentTypes;
    Object[] arguments = this.arguments;
    if (argumentTypes == null) {
        argumentTypes = Constants.EMPTY_CLASS_ARRAY;
        arguments = null;
    }
    // 这里初始化
    return data.newInstance(argumentTypes, arguments, callbacks);
}

```

`EnhancerFactoryData` 这就是类信息吧, 也就是存着`Class`  , `Method`  , `Constructor`   , `primaryConstructorArgTypes` 构造器. 就是一个元信息吧. 

```java
static class EnhancerFactoryData {
    public final Class generatedClass;
    private final Method setThreadCallbacks;
    private final Class[] primaryConstructorArgTypes;
    private final Constructor primaryConstructor;
}    
```

最后逻辑在

```java
public Object newInstance(Class[] argumentTypes, Object[] arguments, Callback[] callbacks) {
    // 设置threadLocalBack,通过反射.
    setThreadCallbacks(callbacks);
    try {
        if (primaryConstructorArgTypes == argumentTypes ||
                Arrays.equals(primaryConstructorArgTypes, argumentTypes)) {
            // 反射生成一个代理对象. 
            return ReflectUtils.newInstance(primaryConstructor, arguments);
        }
        return ReflectUtils.newInstance(generatedClass, argumentTypes, arguments);
    } finally {
        // 清空对象.也就是释放内存了.
        setThreadCallbacks(null);
    }
}
```

`setThreadCallbacks`   方法, 是依靠反射调用的. 

```java
private void setThreadCallbacks(Callback[] callbacks) {
    try {
        setThreadCallbacks.invoke(generatedClass, (Object) callbacks);
    } catch (IllegalAccessException e) {
        throw new CodeGenerationException(e);
    } catch (InvocationTargetException e) {
        throw new CodeGenerationException(e.getTargetException());
    }
}
```

他调用的是这个 , 就将`Callback` 放入到 ThreadLocal中.

```java
// 这里就是... 一个静态方法.往 CGLIB$THREAD_CALLBACKS 这个ThreadLocal里放入CallBack.
public static void CGLIB$SET_THREAD_CALLBACKS(Callback[] var0) {
    CGLIB$THREAD_CALLBACKS.set(var0);
}
```

然后就是

```java
ReflectUtils.newInstance(primaryConstructor, arguments)
```

这里也很简单

```java
public static Object newInstance(final Constructor cstruct, final Object[] args) {
    boolean flag = cstruct.isAccessible();
    try {
        if (!flag) {
            cstruct.setAccessible(true);
        }
        // 这里调用了 newInstance . 
        Object result = cstruct.newInstance(args);
        return result;
    } catch ( e) {
        // 异常....
    } finally {
        if (!flag) {
            // 重置,属性.
            cstruct.setAccessible(flag);
        }
    }
}
```

然后就是调用了 构造方法

```java
public UserService$$EnhancerByCGLIB$$896a9058() {
    CGLIB$BIND_CALLBACKS(this);
}
```

```java
private static final void CGLIB$BIND_CALLBACKS(Object var0) {
    UserService$$EnhancerByCGLIB$$896a9058 var1 = (UserService$$EnhancerByCGLIB$$896a9058)var0;
    // 一开始绝对是var1.CGLIB$BOUND为false.
    if (!var1.CGLIB$BOUND) {
        // 然后设置成true.
        var1.CGLIB$BOUND = true;
        // 从ThreadLocal中拿出我们的callback.
        Object var10000 = CGLIB$THREAD_CALLBACKS.get();
        if (var10000 == null) {
            // 设置.
            var10000 = CGLIB$STATIC_CALLBACKS;
            if (var10000 == null) {
                return;
            }
        }
        // 创建一个新的.防止空指针.异常.
        var1.CGLIB$CALLBACK_0 = (Dispatcher)((Callback[])var10000)[0];
    }
}
```



APP的代码如下 

```java
public class App {
    public static void main(String[] args) {
        System.setProperty(DebuggingClassWriter.DEBUG_LOCATION_PROPERTY, "D:/test");
        Enhancer enhancer = new Enhancer();
        enhancer.setSuperclass(UserService.class);
        enhancer.setCallback((MethodInterceptor) (obj, method, args1, proxy) -> "proxy" + proxy.invokeSuper(obj, args1));
        UserService userService = (UserService) enhancer.create();
        String userNameById = userService.findUserNameById(1);
        System.out.println(userNameById);
    }
}
```

生成的代码如下 : 

所以同时也实现了`Factory` 接口, 我们可以根绝结构的参数进行实例化对象 . 也是很方便的. 

```java
public class UserService$$EnhancerByCGLIB$$74d5b933 extends UserService implements Factory {
    private boolean CGLIB$BOUND;
    public static Object CGLIB$FACTORY_DATA;
    private static final ThreadLocal CGLIB$THREAD_CALLBACKS;
    private static final Callback[] CGLIB$STATIC_CALLBACKS;
    private MethodInterceptor CGLIB$CALLBACK_0;
    private static Object CGLIB$CALLBACK_FILTER;
    private static final Method CGLIB$findUserNameById$0$Method;
    private static final MethodProxy CGLIB$findUserNameById$0$Proxy;
    private static final Object[] CGLIB$emptyArgs;
    private static final Method CGLIB$equals$1$Method;
    private static final MethodProxy CGLIB$equals$1$Proxy;
    private static final Method CGLIB$toString$2$Method;
    private static final MethodProxy CGLIB$toString$2$Proxy;
    private static final Method CGLIB$hashCode$3$Method;
    private static final MethodProxy CGLIB$hashCode$3$Proxy;
    private static final Method CGLIB$clone$4$Method;
    private static final MethodProxy CGLIB$clone$4$Proxy;

    static void CGLIB$STATICHOOK1() {
        CGLIB$THREAD_CALLBACKS = new ThreadLocal();
        CGLIB$emptyArgs = new Object[0];
        Class var0 = Class.forName("com.example.aop.UserService$$EnhancerByCGLIB$$74d5b933");
        Class var1;
        Method[] var10000 = ReflectUtils.findMethods(new String[]{"equals", "(Ljava/lang/Object;)Z", "toString", "()Ljava/lang/String;", "hashCode", "()I", "clone", "()Ljava/lang/Object;"}, (var1 = Class.forName("java.lang.Object")).getDeclaredMethods());
        CGLIB$equals$1$Method = var10000[0];
        CGLIB$equals$1$Proxy = MethodProxy.create(var1, var0, "(Ljava/lang/Object;)Z", "equals", "CGLIB$equals$1");
        CGLIB$toString$2$Method = var10000[1];
        CGLIB$toString$2$Proxy = MethodProxy.create(var1, var0, "()Ljava/lang/String;", "toString", "CGLIB$toString$2");
        CGLIB$hashCode$3$Method = var10000[2];
        CGLIB$hashCode$3$Proxy = MethodProxy.create(var1, var0, "()I", "hashCode", "CGLIB$hashCode$3");
        CGLIB$clone$4$Method = var10000[3];
        CGLIB$clone$4$Proxy = MethodProxy.create(var1, var0, "()Ljava/lang/Object;", "clone", "CGLIB$clone$4");
        CGLIB$findUserNameById$0$Method = ReflectUtils.findMethods(new String[]{"findUserNameById", "(I)Ljava/lang/String;"}, (var1 = Class.forName("com.example.aop.UserService")).getDeclaredMethods())[0];
        CGLIB$findUserNameById$0$Proxy = MethodProxy.create(var1, var0, "(I)Ljava/lang/String;", "findUserNameById", "CGLIB$findUserNameById$0");
    }

    final String CGLIB$findUserNameById$0(int var1) {
        return super.findUserNameById(var1);
    }

    // 主要看这里
    public final String findUserNameById(int var1) {
        MethodInterceptor var10000 = this.CGLIB$CALLBACK_0;
        if (var10000 == null) {
            CGLIB$BIND_CALLBACKS(this);
            var10000 = this.CGLIB$CALLBACK_0;
        }

        // 调用了. 当前对象 , CGLIB$findUserNameById$0$Method方法 , 参数, CGLIB$findUserNameById$0$Proxy这个代理对象. 
        // 这里也就是为什么不能使用调用当前对象(this)的findUserNameById,因为他是个递归过程. 所以这就是原因.
        return var10000 != null ? (String)var10000.intercept(this, CGLIB$findUserNameById$0$Method, new Object[]{new Integer(var1)}, CGLIB$findUserNameById$0$Proxy) : super.findUserNameById(var1);
    }

    final boolean CGLIB$equals$1(Object var1) {
        return super.equals(var1);
    }

    public final boolean equals(Object var1) {
        MethodInterceptor var10000 = this.CGLIB$CALLBACK_0;
        if (var10000 == null) {
            CGLIB$BIND_CALLBACKS(this);
            var10000 = this.CGLIB$CALLBACK_0;
        }

        if (var10000 != null) {
            Object var2 = var10000.intercept(this, CGLIB$equals$1$Method, new Object[]{var1}, CGLIB$equals$1$Proxy);
            return var2 == null ? false : (Boolean)var2;
        } else {
            return super.equals(var1);
        }
    }

    final String CGLIB$toString$2() {
        return super.toString();
    }

    public final String toString() {
        MethodInterceptor var10000 = this.CGLIB$CALLBACK_0;
        if (var10000 == null) {
            CGLIB$BIND_CALLBACKS(this);
            var10000 = this.CGLIB$CALLBACK_0;
        }

        return var10000 != null ? (String)var10000.intercept(this, CGLIB$toString$2$Method, CGLIB$emptyArgs, CGLIB$toString$2$Proxy) : super.toString();
    }

    final int CGLIB$hashCode$3() {
        return super.hashCode();
    }

    public final int hashCode() {
        MethodInterceptor var10000 = this.CGLIB$CALLBACK_0;
        if (var10000 == null) {
            CGLIB$BIND_CALLBACKS(this);
            var10000 = this.CGLIB$CALLBACK_0;
        }

        if (var10000 != null) {
            Object var1 = var10000.intercept(this, CGLIB$hashCode$3$Method, CGLIB$emptyArgs, CGLIB$hashCode$3$Proxy);
            return var1 == null ? 0 : ((Number)var1).intValue();
        } else {
            return super.hashCode();
        }
    }

    final Object CGLIB$clone$4() throws CloneNotSupportedException {
        return super.clone();
    }

    protected final Object clone() throws CloneNotSupportedException {
        MethodInterceptor var10000 = this.CGLIB$CALLBACK_0;
        if (var10000 == null) {
            CGLIB$BIND_CALLBACKS(this);
            var10000 = this.CGLIB$CALLBACK_0;
        }

        return var10000 != null ? var10000.intercept(this, CGLIB$clone$4$Method, CGLIB$emptyArgs, CGLIB$clone$4$Proxy) : super.clone();
    }

    public static MethodProxy CGLIB$findMethodProxy(Signature var0) {
        String var10000 = var0.toString();
        switch(var10000.hashCode()) {
        case -508378822:
            if (var10000.equals("clone()Ljava/lang/Object;")) {
                return CGLIB$clone$4$Proxy;
            }
            break;
        case 1012028561:
            if (var10000.equals("findUserNameById(I)Ljava/lang/String;")) {
                return CGLIB$findUserNameById$0$Proxy;
            }
            break;
        case 1826985398:
            if (var10000.equals("equals(Ljava/lang/Object;)Z")) {
                return CGLIB$equals$1$Proxy;
            }
            break;
        case 1913648695:
            if (var10000.equals("toString()Ljava/lang/String;")) {
                return CGLIB$toString$2$Proxy;
            }
            break;
        case 1984935277:
            if (var10000.equals("hashCode()I")) {
                return CGLIB$hashCode$3$Proxy;
            }
        }

        return null;
    }

    public UserService$$EnhancerByCGLIB$$74d5b933() {
        CGLIB$BIND_CALLBACKS(this);
    }

    public static void CGLIB$SET_THREAD_CALLBACKS(Callback[] var0) {
        CGLIB$THREAD_CALLBACKS.set(var0);
    }

    public static void CGLIB$SET_STATIC_CALLBACKS(Callback[] var0) {
        CGLIB$STATIC_CALLBACKS = var0;
    }

    private static final void CGLIB$BIND_CALLBACKS(Object var0) {
        UserService$$EnhancerByCGLIB$$74d5b933 var1 = (UserService$$EnhancerByCGLIB$$74d5b933)var0;
        if (!var1.CGLIB$BOUND) {
            var1.CGLIB$BOUND = true;
            Object var10000 = CGLIB$THREAD_CALLBACKS.get();
            if (var10000 == null) {
                var10000 = CGLIB$STATIC_CALLBACKS;
                if (var10000 == null) {
                    return;
                }
            }

            var1.CGLIB$CALLBACK_0 = (MethodInterceptor)((Callback[])var10000)[0];
        }

    }

    public Object newInstance(Callback[] var1) {
        CGLIB$SET_THREAD_CALLBACKS(var1);
        UserService$$EnhancerByCGLIB$$74d5b933 var10000 = new UserService$$EnhancerByCGLIB$$74d5b933();
        CGLIB$SET_THREAD_CALLBACKS((Callback[])null);
        return var10000;
    }

    public Object newInstance(Callback var1) {
        CGLIB$SET_THREAD_CALLBACKS(new Callback[]{var1});
        UserService$$EnhancerByCGLIB$$74d5b933 var10000 = new UserService$$EnhancerByCGLIB$$74d5b933();
        CGLIB$SET_THREAD_CALLBACKS((Callback[])null);
        return var10000;
    }

    public Object newInstance(Class[] var1, Object[] var2, Callback[] var3) {
        CGLIB$SET_THREAD_CALLBACKS(var3);
        UserService$$EnhancerByCGLIB$$74d5b933 var10000 = new UserService$$EnhancerByCGLIB$$74d5b933;
        switch(var1.length) {
        case 0:
            var10000.<init>();
            CGLIB$SET_THREAD_CALLBACKS((Callback[])null);
            return var10000;
        default:
            throw new IllegalArgumentException("Constructor not found");
        }
    }

    public Callback getCallback(int var1) {
        CGLIB$BIND_CALLBACKS(this);
        MethodInterceptor var10000;
        switch(var1) {
        case 0:
            var10000 = this.CGLIB$CALLBACK_0;
            break;
        default:
            var10000 = null;
        }

        return var10000;
    }

    public void setCallback(int var1, Callback var2) {
        switch(var1) {
        case 0:
            this.CGLIB$CALLBACK_0 = (MethodInterceptor)var2;
        default:
        }
    }

    public Callback[] getCallbacks() {
        CGLIB$BIND_CALLBACKS(this);
        return new Callback[]{this.CGLIB$CALLBACK_0};
    }

    public void setCallbacks(Callback[] var1) {
        this.CGLIB$CALLBACK_0 = (MethodInterceptor)var1[0];
    }

    static {
        CGLIB$STATICHOOK1();
    }
}
```

## 分析`invoke` 和 `invokeSuper` 做了些啥 ?

我们再看看这个代码 `MethodProxy`是如何生成的. ? 

```java
CGLIB$findUserNameById$0$Proxy = MethodProxy.create(var1, var0, "(I)Ljava/lang/String;", "findUserNameById", "CGLIB$findUserNameById$0");
```

`findUserNameById` 就是一个重写父类的的方法.  

`CGLIB$findUserNameById$0` 的过程是直接调用父类方法. 

```java
final String CGLIB$findUserNameById$0(int var1) {
    return super.findUserNameById(var1);
}
```

`var0` 是指的是未增强的类, 也就是默认是父类. (Object/父类/父类.... 看继承关系)

`var1` 指的是增强的类 ,就时我们重写的子类.

```java
public static MethodProxy create(Class c1, Class c2, String desc, String name1, String name2) {
    MethodProxy proxy = new MethodProxy();
    proxy.sig1 = new Signature(name1, desc);
    proxy.sig2 = new Signature(name2, desc);
    proxy.createInfo = new CreateInfo(c1, c2);
    return proxy;
}
```

```java
private static class CreateInfo
{
    // c1指的是父类/超类.
    Class c1;
    // C2指的是代理类.
    Class c2;
    NamingPolicy namingPolicy;
    GeneratorStrategy strategy;
    boolean attemptLoad;

    public CreateInfo(Class c1, Class c2)
    {
        this.c1 = c1;
        this.c2 = c2;
        AbstractClassGenerator fromEnhancer = AbstractClassGenerator.getCurrent();
        if (fromEnhancer != null) {
            namingPolicy = fromEnhancer.getNamingPolicy();
            strategy = fromEnhancer.getStrategy();
            attemptLoad = fromEnhancer.getAttemptLoad();
        }
    }
}
```

以上就是一个初始化过程  .  会在生成类信息的时候生成这些  `MethodProxy`

第二步就是执行我们的调用的方法了. 

`invokeSuper` 操作如下. 会调用`init `方法. 

```java
public Object invokeSuper(Object obj, Object[] args) throws Throwable {
    try {
        init();
        FastClassInfo fci = fastClassInfo;
        // F2的话就指的是代理的类方法. 
        // F1是指的是父类.
        return fci.f2.invoke(fci.i2, obj, args);
    } catch (InvocationTargetException e) {
        throw e.getTargetException();
    }
}
```

`init` 方法是一个双重监测锁的实现. 进行初始化过程. 

```java
private void init()
{
    // 就是一个双重监测锁. 初始化fastClassInfo.
    if (fastClassInfo == null)
    {
        synchronized (initLock)
        {
            if (fastClassInfo == null)
            {
                CreateInfo ci = createInfo;
                FastClassInfo fci = new FastClassInfo();
                // 初始化一些元信息.
                fci.f1 = helper(ci, ci.c1);
                fci.f2 = helper(ci, ci.c2);
                fci.i1 = fci.f1.getIndex(sig1);
                fci.i2 = fci.f2.getIndex(sig2);
                fastClassInfo = fci;
                createInfo = null;
            }
        }
    }
}
```

所以这里`invokeSuper`调用的是  : 

```java
fci.f2.invoke(fci.i2, obj, args);
// 代理的类对象. 调用的CGLIB$findUserNameById$0()方法. 
```

也就是

```java
final String CGLIB$findUserNameById$0(int var1) {
    return super.findUserNameById(var1);
}
```

**当我们调用 `invoke` 的时候. 我们作为子类, 子类重写了父类的方法. 所以我们调用父类的方法的时候,会走我们重写的方法 , 会出现一个递归的问题.  就是这段代码.** 

```java
// 主要看这里
public final String findUserNameById(int var1) {
    MethodInterceptor var10000 = this.CGLIB$CALLBACK_0;
    if (var10000 == null) {
        CGLIB$BIND_CALLBACKS(this);
        var10000 = this.CGLIB$CALLBACK_0;
    }
    // 调用了. 当前对象 , CGLIB$findUserNameById$0$Method方法 , 参数, CGLIB$findUserNameById$0$Proxy这个代理对象. 
    // 这也是为啥不能使用invoke方法了.
    return var10000 != null ? (String)var10000.intercept(this, CGLIB$findUserNameById$0$Method, new Object[]{new Integer(var1)}, CGLIB$findUserNameById$0$Proxy) : super.findUserNameById(var1);
}
```

说到这里其实没玩呢. 为啥呢, 因为`FastClass` 是一个抽象类, 那么我们还要重写一些方法. 我们去点击. `abstract public Object invoke(int index, Object obj, Object[] args) throws InvocationTargetException;`  这个方法是一个抽象方法, 显然不成立

怎么办呢.  显然是生成了一个. 哈哈哈哈.  正常情况下啊 , 会生成三个类. 核心的类上面写的就是 `UserService$$EnhancerByCGLIB$$74d5b933`

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-30-33/eac8b038-1910-4000-8cba-721b4efe8cfe.png?x-oss-process=style/template01)

下面真正调用的时候, 使用的是 `UserService$$EnhancerByCGLIB$$74d5b933$$FastClassByCGLIB$$406726de` 这个.  他呢继承了`FastClass` , 所以就直接走的它.  

```java
//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

package com.example.aop;

import com.example.aop.UserService..EnhancerByCGLIB..74d5b933;
import java.lang.reflect.InvocationTargetException;
import net.sf.cglib.core.Signature;
import net.sf.cglib.proxy.Callback;
import net.sf.cglib.reflect.FastClass;

public class UserService$$EnhancerByCGLIB$$74d5b933$$FastClassByCGLIB$$406726de extends FastClass {

        // 这个就是invoke的重载方法. 
   public Object invoke(int var1, Object var2, Object[] var3) throws InvocationTargetException {
        74d5b933 var10000 = (74d5b933)var2;
        int var10001 = var1;
       // 反正吧, 就是找到对应的方法直接调用,根据坐标, 这个坐标是生成这个类的时候生成的, 不是固定不变的.
        try {
            switch(var10001) {
            case 0:
                return new Boolean(var10000.equals(var3[0]));
            case 1:
                return var10000.toString();
            case 2:
                return new Integer(var10000.hashCode());
            case 3:
                return var10000.clone();
            case 4:
                return var10000.newInstance((Callback[])var3[0]);
            case 5:
                return var10000.newInstance((Callback)var3[0]);
            case 6:
                return var10000.newInstance((Class[])var3[0], (Object[])var3[1], (Callback[])var3[2]);
            case 7:
                74d5b933.CGLIB$SET_THREAD_CALLBACKS((Callback[])var3[0]);
                return null;
            case 8:
                74d5b933.CGLIB$SET_STATIC_CALLBACKS((Callback[])var3[0]);
                return null;
            case 9:
                var10000.setCallback(((Number)var3[0]).intValue(), (Callback)var3[1]);
                return null;
            case 10:
                var10000.setCallbacks((Callback[])var3[0]);
                return null;
            case 11:
                return var10000.getCallback(((Number)var3[0]).intValue());
            case 12:
                return var10000.getCallbacks();
            case 13:
                return var10000.CGLIB$clone$4();
            case 14:
                return new Integer(var10000.CGLIB$hashCode$3());
            case 15:
                return new Boolean(var10000.CGLIB$equals$1(var3[0]));
            case 16:
                return var10000.CGLIB$toString$2();
            case 17:
                return 74d5b933.CGLIB$findMethodProxy((Signature)var3[0]);
            case 18:
                return var10000.findUserNameById(((Number)var3[0]).intValue());
            case 19:
                74d5b933.CGLIB$STATICHOOK1();
                return null;
			// 下面是一个var1 , 指的是索引坐标(index). 找到后直接用对象去调用,所以没有使用反射机制.而是硬编码的方式.
            case 20:
                return var10000.CGLIB$findUserNameById$0(((Number)var3[0]).intValue());
            }
        } catch (Throwable var4) {
            throw new InvocationTargetException(var4);
        }

        throw new IllegalArgumentException("Cannot find matching method/constructor");
    }    
}
```



那么还有一个类, 是干啥的呢, 显然当然有用, 那就是 当我们调用, `proxy.invoke(obj, args1)`  就走的另外一个. `UserService$$FastClassByCGLIB$$417646d`  这个没有写增强, 顾名思义, 也就是父类的FastClass.  是吧. 所以他会调用 `var10000.findUserNameById(((Number)var3[0]).intValue());`  , 然后呢, 就递归卡死了. 懂了吧.

所以这里就是这个意思  , 他的流程和上面死一样的. 

````java
public class UserService$$FastClassByCGLIB$$417646d extends FastClass {
    // 省略一堆代码. 
    
    // 下面就是一个调用流程. 跟上诉一样. 
    public Object invoke(int var1, Object var2, Object[] var3) throws InvocationTargetException {
        UserService var10000 = (UserService)var2;
        int var10001 = var1;
        try {
            switch(var10001) {
            case 0:
                return var10000.findUserNameById(((Number)var3[0]).intValue());
            case 1:
                return new Boolean(var10000.equals(var3[0]));
            case 2:
                return var10000.toString();
            case 3:
                return new Integer(var10000.hashCode());
            }
        } catch (Throwable var4) {
            throw new InvocationTargetException(var4);
        }

        throw new IllegalArgumentException("Cannot find matching method/constructor");
    }
}
````

## CGLIB的核心

### `Enhancer `

这个就是增强器. 我们的一个生成代码的工具类 ,

### `ClassGenerator`

这个是生成代码的类, 也就是生成字节码

### `MethodProxy`

拦截器的实现方式, 就时通过这个, 他会生成两个代理类,  继承了 `FastClass` , 所以就是快的字节码.  因为直接调用未使用反射, 但是多生成两个类  , 会占用内存的. 

### `Callback`

> ​	这个讲过了, 也就是定义一些我们的业务逻辑. 

### `CallbackFilter`

这个就是一个 过滤器.  对应着 `Callback`



> ​	缺点就是, 我们上面说的, 占用内存, 没了. 因为Java的代理 , 也就生成一个类, 而CGLIB代理会生成一个代理类和两个辅助类.  

