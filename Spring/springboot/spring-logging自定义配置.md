# Spring-Logging 学习

>参考版本 [spring-boot 2.0.4 RELEASE](https://docs.spring.io/spring-boot/docs/2.0.4.RELEASE/reference/htmlsingle/#boot-features-custom-log-levels)   , 把握住一点学习spring框架最好别随意换版本 .



> ​	spring-logging  默认使用的是 logback
>
> ​	By default, if you use the “Starters”, Logback is used for logging.   



配置源信息位置 spring-boot:2.0.4 包下面的 /META-INF/spring-configuration-metadata.json目录下自己找去吧,

YML不推荐用来学习



### 日志信息: 

```
2014-03-05 10:57:51.112  INFO 45469 --- [           main] org.apache.catalina.core.StandardEngine  : Starting Servlet Engine: Apache Tomcat/7.0.52
2014-03-05 10:57:51.253  INFO 45469 --- [ost-startStop-1] o.a.c.c.C.[Tomcat].[localhost].[/]       : Initializing Spring embedded WebApplicationContext
2014-03-05 10:57:51.253  INFO 45469 --- [ost-startStop-1] o.s.web.context.ContextLoader            : Root WebApplicationContext: initialization completed in 1358 ms
2014-03-05 10:57:51.698  INFO 45469 --- [ost-startStop-1] o.s.b.c.e.ServletRegistrationBean        : Mapping servlet: 'dispatcherServlet' to [/]
2014-03-05 10:57:51.702  INFO 45469 --- [ost-startStop-1] o.s.b.c.embedded.FilterRegistrationBean  : Mapping filter: 'hiddenHttpMethodFilter' to: [/*]
```

The following items are output:

- 日期: Millisecond precision and easily sortable.
- 日志级别: `ERROR`, `WARN`, `INFO`, `DEBUG`, or `TRACE`.
- 进程ID.
- 分隔符  `---`  
- 线程名称: Enclosed in square brackets (may be truncated for console output).
- 源类名: This is usually the source class name (often abbreviated).
- 日志信息 
- .



### **Logging properties**

| `logging.file` | `logging.path`     | Example    | Description                                                  |
| -------------- | ------------------ | ---------- | ------------------------------------------------------------ |
| *(none)*       | *(none)*           |            | Console only logging.                                        |
| Specific file  | *(none)*           | `my.log`   | Writes to the specified log file. Names can be an exact location or relative to the current directory. |
| *(none)*       | Specific directory | `/var/log` | Writes `spring.log` to the specified directory. Names can be an exact location or relative to the current directory. |

日志文件位置 file 和 path  二者选一即刻

注意 : The logging system is initialized early in the application lifecycle. 日志系统启动比spring应用快







