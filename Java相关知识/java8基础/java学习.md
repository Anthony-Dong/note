### Class

#### Class.newInstance()和Constructor.newInstance()
	Class.forName("HelloWorld").newInstance();
	HelloWorld.class.getConstructor().newInstance();
	Class.newInstance()只能反射无参的构造器；
	Constructor.newInstance()可以反任何构造器；

	Class.newInstance()需要构造器可见(visible)；
	Constructor.newInstance()可以反私有构造器；

	Class.newInstance()对于捕获或者未捕获的异常均由构造器抛出;
	Constructor.newInstance()通常会把抛出的异常封装成InvocationTargetException抛出；

#### SPL
	SPI全称Service Provider Interface，是Java提供的一套用来被第三方实现或者扩展的API，它可以用来启用框架扩展和替换组件。
![image](https://upload-images.jianshu.io/upload_images/5618238-5d8948367cb9b18e.png?imageMogr2/auto-orient/)
	
		Java SPI 实际上是“基于接口的编程＋策略模式＋配置文件”组合实现的动态加载机制。
	系统设计的各个抽象，往往有很多不同的实现方案，在面向的对象的设计里，一般推荐模块之间基于接口编程，模块之间不对实现类进行硬编码。一旦代码里涉及具体的实现类，就违反了可拔插的原则，如果需要替换一种实现，就需要修改代码。为了实现在模块装配的时候能不在程序里动态指明，这就需要一种服务发现机制。
	Java SPI就是提供这样的一个机制：为某个接口寻找服务实现的机制。有点类似IOC的思想，就是将装配的控制权移到程序之外，在模块化设计中这个机制尤其重要。所以SPI的核心思想就是解耦。
	概括地说，适用于：调用者根据实际使用需要，启用、扩展、或者替换框架的实现策略
##### 比较常见的例子：
	数据库驱动加载接口实现类的加载
	JDBC加载不同类型数据库的驱动
	日志门面接口实现类加载
	SLF4J加载不同提供商的日志实现类
	Spring
	Spring中大量使用了SPI,比如：对servlet3.0规范对ServletContainerInitializer的实现、自动类型转换Type Conversion SPI(Converter SPI、Formatter SPI)等
	Dubbo
	Dubbo中也大量使用SPI的方式实现框架的扩展, 不过它对Java提供的原生SPI做了封装，允许用户扩展实现Filter接口
#####优点：
	使用Java SPI机制的优势是实现解耦，使得第三方服务模块的装配控制的逻辑与调用者的业务代码分离，而不是耦合在一起。应用程序可以根据实际业务情况启用框架扩展或替换框架组件。
#####缺点：
	虽然ServiceLoader也算是使用的延迟加载，但是基本只能通过遍历全部获取，也就是接口的实现类全部加载并实例化一遍。如果你并不想用某些实现类，它也被加载并实例化了，这就造成了浪费。获取某个实现类的方式不够灵活，只能通过Iterator形式获取，不能根据某个参数来获取对应的实现类。
	多个并发多线程使用ServiceLoader类的实例是不安全的。
##### 实现方式
![image](image\Snipaste_2019-08-06_22-04-19.jpg)
![image](image\Snipaste_2019-08-06_22-20-48.jpg)

