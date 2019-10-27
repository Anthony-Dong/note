### spring-boot-maven-plugin 多模块打包实战

#### 1. 父工程类型的   父子关系型

1. 我们需要创建 父工程 

2. 创建三个子模块(web , commons , interface)

3. 写 三个测试方法

4. 最终结构

   - 目录结构

     ![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-01/8c76bb9b-41c1-4fab-90d0-617052f3f392.jpg?x-oss-process=style/template01)

   - 文件结构

     ​	![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-01/4b64a754-4f8a-44d6-82cf-3ee41c27dec1.jpg?x-oss-process=style/template01)

5. 需要在 web模块中导入 interface 和 comments依赖 ,代码不展示了

6. 然后 我们需要对其 进去 打包时,需要注意,

   1. 父工程  father的pom文件

      ```java
      <?xml version="1.0" encoding="UTF-8"?>
      <project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
               xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
          <modelVersion>4.0.0</modelVersion>
          <packaging>pom</packaging>
          <modules>
              <module>commons</module>
              <module>web</module>
              <module>interface</module>
          </modules>
          <parent>
              <groupId>org.springframework.boot</groupId>
              <artifactId>spring-boot-starter-parent</artifactId>
              <version>2.1.7.RELEASE</version>
              <relativePath/> <!-- lookup parent from repository -->
          </parent>
          <groupId>com.example</groupId>
          <artifactId>father</artifactId>
          <version>0.0.1-SNAPSHOT</version>
          <name>father</name>
          <description>Demo project for Spring Boot</description>
      
          <properties>
              <java.version>1.8</java.version>
              <spring-cloud.version>Greenwich.SR2</spring-cloud.version>
          </properties>
      
      
      
          <dependencyManagement>
              <dependencies>
                  <dependency>
                      <groupId>org.springframework.cloud</groupId>
                      <artifactId>spring-cloud-dependencies</artifactId>
                      <version>${spring-cloud.version}</version>
                      <type>pom</type>
                      <scope>import</scope>
                  </dependency>
              </dependencies>
          </dependencyManagement>
      
      
          <!-- 父 工程  -->
          <!-- compiler  编译器  当我们不写这个时,子模块 web,模块必须 packing =jar  ;不然没jar包生成 -->
          <!--除了 web模块 以外其他两个都不需要 build-->
          <build>
              <plugins>
                  <plugin>
                      <groupId>org.apache.maven.plugins</groupId>
                      <artifactId>maven-compiler-plugin</artifactId>
                      <configuration>
                          <source>${java.version}</source>
                          <target>${java.version}</target>
                      </configuration>
                  </plugin>
                  <plugin>
                      <groupId>org.apache.maven.plugins</groupId>
                      <artifactId>maven-surefire-plugin</artifactId>
                      <configuration>
                          <skipTests>true</skipTests>    <!--默认关掉单元测试 -->
                      </configuration>
                  </plugin>
              </plugins>
          </build>
      </project>
      
      ```

      

   2. 子模块

      1. web模块

         ```java
         <?xml version="1.0" encoding="UTF-8"?>
         <project xmlns="http://maven.apache.org/POM/4.0.0"
                  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 http://maven.apache.org/xsd/maven-4.0.0.xsd">
             <parent>
                 <artifactId>father</artifactId>
                 <groupId>com.example</groupId>
                 <version>0.0.1-SNAPSHOT</version>
             </parent>
             <modelVersion>4.0.0</modelVersion>
         
             <artifactId>web</artifactId>
              		<-- 父模块 无编译插件时;选择 jar包 ,必须加 ,所以 平时我们web项目还是加吧 -->  
               	  <packaging>jar</packaging>
                   <dependencies>
                       <dependency>
                           <groupId>org.springframework.boot</groupId>
                           <artifactId>spring-boot-starter-web</artifactId>
                       </dependency>
                       <dependency>
                           <groupId>com.example</groupId>
                           <artifactId>interface</artifactId>
                           <version>0.0.1-SNAPSHOT</version>
                       </dependency>
                       <dependency>
                           <groupId>com.example</groupId>
                           <artifactId>commons</artifactId>
                           <version>0.0.1-SNAPSHOT</version>
                       </dependency>
                   </dependencies>
               	<build>
            		 <plugins>
           			<plugin>
                    <groupId>org.springframework.boot</groupId>
                     <artifactId>spring-boot-maven-plugin</artifactId>
            <!-- 当我们只有一个模块时 ,可以 不用加这个 main入口函数 他会自己去找 -->         
                     <configuration>
                         <!-- 指定该Main Class为全局的唯一入口 -->
                         <mainClass>com.PackingApplication</mainClass>
                         
                     </configuration>
                     <executions>
                         <execution>
                             <goals>
                                 <goal>repackage</goal>
                                 <!--可以把依赖的包都打包到生成的Jar包中,当我们选择取消使用时;会发现;其他两个依赖的项目里是不会生成jar包的-->
                             </goals>
                         </execution>
                     </executions>
                 </plugin>
         	 </plugins>
         </build>
         </project>
         ```
         
         2. commons和 interface模块
      ```java
      <?xml version="1.0" encoding="UTF-8"?>
         <project xmlns="http://maven.apache.org/POM/4.0.0"
                  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 http://maven.apache.org/xsd/maven-4.0.0.xsd">
             <parent>
                 <artifactId>father</artifactId>
                 <groupId>com.example</groupId>
                 <version>0.0.1-SNAPSHOT</version>
             </parent>
             <modelVersion>4.0.0</modelVersion>
         
             <artifactId>commons</artifactId>
         
             <dependencies>
                 <dependency>
                     <groupId>org.apache.commons</groupId>
                     <artifactId>commons-lang3</artifactId>
                 </dependency>
             </dependencies>
         
         </project>
         
         
         <?xml version="1.0" encoding="UTF-8"?>
         <project xmlns="http://maven.apache.org/POM/4.0.0"
                  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 http://maven.apache.org/xsd/maven-4.0.0.xsd">
             <parent>
                 <artifactId>father</artifactId>
                 <groupId>com.example</groupId>
                 <version>0.0.1-SNAPSHOT</version>
             </parent>
             <modelVersion>4.0.0</modelVersion>
         
             <artifactId>interface</artifactId>
         
             <dependencies>
                 <dependency>
                     <groupId>org.projectlombok</groupId>
                     <artifactId>lombok</artifactId>
                 </dependency>
             </dependencies>
         </project>
       
      ```
   
7. 当我们 再选择一个web新项目时 ,如果 我们不写入口函数会怎么样子呢

   **[ERROR] Failed to execute goal org.springframework.boot:spring-boot-maven-plugin:2.1.7.RELEASE:repackage (repackage) on project web02: Execution repackage of goal org.springframework.boot:spring-boot-maven-plugin:2.1.7.RELEASE:repackage failed: Unable to find main class -> [Help 1]**

   会发现找不到类,此时 当大于一个web 模块时,必须加上 ;就算是 子模块的启动类重名,依旧可以正常运行

   ```java
     <configuration>
                 <!-- 指定该Main Class为全局的唯一入口 -->
                <mainClass>com.PackingApplication</mainClass>
    </configuration>
   ```

   所以为了有个好的习惯 我们平时还要写这个

8.  当我们选择不写 

   ```java
    <executions>
                       <execution>
                           <goals>
                               <goal>repackage</goal>
                               <!--可以把依赖的包都打包到生成的Jar包中,
                               当我们选择取消使用时;会发现;其他两个依赖的项目里是不会生成jar包的-->
                           </goals>
                       </execution>
       </executions>
   ```

   这个时,当我们再一次打包 会发现 ,依旧在项目中生成了 jar包

9. 最后我们运行 :

   ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-01/530b7b98-0984-47eb-a542-58a62b30b616.jpg?x-oss-process=style/template01)

10. 最后我们可以去看看  生成的web 工程的jar包是什么

   1. 目录位置  :  就是web工程的 编译后的文件内
   2. 目录结构 
      
      1. ![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-01/3f353045-8162-4606-bfde-07b487234a04.jpg?x-oss-process=style/template01)

#### 2.模块化管理,无父类,只引入pom文件,子父继承关系 

1. 让我们看看传统的 parent关系 (是先创建一个父类工程,子类在父类里面)

   ```java
       <parent>
           <artifactId>father</artifactId>
           <groupId>com.example</groupId>
           <version>0.0.1-SNAPSHOT</version>
       </parent>
   ```

   

2. 让我们看看这个类型的模块关系(父类,子类是平行关系,子类和父类在一个文件夹下的平行目录下,只是依靠下面这个依赖保持关系)

   ```java
       <parent>
           <artifactId>demo</artifactId>
           <groupId>com.example</groupId>
           <version>0.0.1-SNAPSHOT</version>
           <relativePath>../demo/pom.xml</relativePath>
       </parent>
   ```

   1. 我们会发现 多了个 relativePath

      当我们 选择去掉这个时候 ,会报错

      **[ERROR]     Non-resolvable parent POM for com.example:commons:0.0.1-SNAPSHOT: Could not find artifact com.example:demo:pom:0.0.1-SNAPSHOT and 'parent.relativePath' points at wrong local POM @ line 5, column 13 -> [Help 2]**

3. 当我们使用这种方式时,需要注意的是什么

4. 流程 创建项目流程

   1. ,先新建一个空文件夹,然后选择 idea打开这个文件夹,此时 这个文件夹就是你的工作空间,

   2. 然后选择一个 framework项目,是你的核心包,其中默认选择作为pom文件,packing方式为pom,作为项目的版本控制

   3. 创建子类,选择创建项目时 选择 

      ![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-01/5ec150b5-ba5f-49a3-8dad-c908a4e2e202.jpg?x-oss-process=style/template01       )

   4. 然后创建三个项目,

      ![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-01/1723f9f6-2ef0-49c4-b0f6-b7acc618af35.jpg?x-oss-process=style/template01)

   5. 如果我们选择 父类的方式 ,就是需要改动 一下 module添加到 demo 模块下 ,就是 我们的第一种模式

      ​	![image](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-01/3ef05d71-1220-45bb-94bb-1753ed4cf40c.jpg?x-oss-process=style/template01)

   6. 然后 目录结构

5. 打包流程

   1. 我们选择 非 web项目,就是么有 main方法 启动类的项目,选择 打包,可以直接生成一个war包

   2. 其中我们不需要在framework工程中进行 编译那个插件

      ```java
      <?xml version="1.0" encoding="UTF-8"?>
      <project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
               xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
          <modelVersion>4.0.0</modelVersion>
      
          <parent>
              <groupId>org.springframework.boot</groupId>
              <artifactId>spring-boot-starter-parent</artifactId>
              <version>2.1.7.RELEASE</version>
              <relativePath/> <!-- lookup parent from repository -->
          </parent>
          <groupId>com.example</groupId>
          <artifactId>demo</artifactId>
          <version>0.0.1-SNAPSHOT</version>
          <name>demo</name>
          <description>Demo project for Spring Boot</description>
          <packaging>pom</packaging>
          <properties>
              <java.version>1.8</java.version>
          </properties>
      
      <--这个就是个版本 控制工具,可以选择 对于的 编码方式,jdk版本 , 可以不选择 添加,-->
          <build>
              <plugins>
                  <plugin>
                      <groupId>org.apache.maven.plugins</groupId>
                      <artifactId>maven-compiler-plugin</artifactId>
                      <configuration>
                          <source>${java.version}</source>
                          <target>${java.version}</target>
                      </configuration>
                  </plugin>
                  <plugin>
                      <groupId>org.apache.maven.plugins</groupId>
                      <artifactId>maven-surefire-plugin</artifactId>
                      <configuration>
                          <skipTests>true</skipTests>    <!--默认关掉单元测试 -->
                      </configuration>
                  </plugin>
              </plugins>
          </build>
      </project>
      
      ```
      
   3. 我们 直接 把commons和interface工程可以直接打包成功 ,但是 关键来了,我们的web模块才是关键,

      ```java
   interface和common 包 我就不全写了  其中 并无差异 
      
      
      <?xml version="1.0" encoding="UTF-8"?>
      <project xmlns="http://maven.apache.org/POM/4.0.0"
               xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
               xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 http://maven.apache.org/xsd/maven-4.0.0.xsd">
          <parent>
              <artifactId>demo</artifactId>
              <groupId>com.example</groupId>
              <version>0.0.1-SNAPSHOT</version>
              <relativePath>../demo/pom.xml</relativePath>
          </parent>
          <modelVersion>4.0.0</modelVersion>
      
          <artifactId>commons</artifactId>
      
      
      </project>
      ```
   
      我们看看 common打包后的 文件目录

      ​	

   4. 此时我们需要怎么办的???????

   5. 当我们尝试去打包时 会发现 报错 ,

      ```java
   <?xml version="1.0" encoding="UTF-8"?>
      <project xmlns="http://maven.apache.org/POM/4.0.0"
               xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
               xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 http://maven.apache.org/xsd/maven-4.0.0.xsd">
          <parent>
              <artifactId>demo</artifactId>
              <groupId>com.example</groupId>
              <version>0.0.1-SNAPSHOT</version>
              <relativePath>../demo/pom.xml</relativePath>
          </parent>
          <modelVersion>4.0.0</modelVersion>
      
          <packaging>jar</packaging>
          <artifactId>web</artifactId>
      
          <dependencies>
              <dependency>
                  <groupId>org.springframework.boot</groupId>
                  <artifactId>spring-boot-starter-web</artifactId>
              </dependency>
              <dependency>
                  <groupId>com.example</groupId>
                  <artifactId>commons</artifactId>
                  <version>0.0.1-SNAPSHOT</version>
              </dependency>
              <dependency>
                  <groupId>com.example</groupId>
                  <artifactId>interface</artifactId>
                  <version>0.0.1-SNAPSHOT</version>
              </dependency>
          </dependencies>
      
          <build>
              <plugins>
                  <plugin>
                      <groupId>org.springframework.boot</groupId>
                      <artifactId>spring-boot-maven-plugin</artifactId>
                      <configuration>
                          <mainClass>com.PackingApplication</mainClass>
                      </configuration>
                  </plugin>
              </plugins>
          </build>
      </project>
      ```
   
   6. 会报什么错误呢 会发现找不着 jar包 ,此时 尴尬了  ,找不到 是因为啥 ,是的 使我们的  maven仓库没有,

   7. 此时 我选择了  把我刚刚的  那两个jar包 commons 和 interface 两个包都丢到maven仓库中去

      **mvn install:install-file -Dfile=D:\软件\正则表达式\yinxiang\model\commons\target\commons-0.0.1-SNAPSHOT.jar  -DgroupId=com.example -DartifactId=commons  -Dversion=0.0.1-SNAPSHOT -Dpackaging=jar**

      语法 很简单就是   将jar包 放到我们的maven仓库中 ,

   8. 然后 我们在 packing 我们发现 打包成功 ,nice 

      1. 然后 我选择了  既然 我已经选择了 jar包打包的方式,我用啥 plugin呢 ,我直接 衣蛾 packing不就好了

      2. 于是我依旧在尝试中

         ```java
          <build>
                 <plugins>
                     <plugin>
                         <groupId>org.springframework.boot</groupId>
                         <artifactId>spring-boot-maven-plugin</artifactId>
                         <configuration>
                             <mainClass>com.PackingApplication</mainClass>
                         </configuration>
                     </plugin>
                 </plugins>
             </build>
             
                 
         ```
   
      3. 结果 确实可以打包成功但是不是一个web项目 ,里面和普通目录 一样

         **web-0.0.1-SNAPSHOT.jar中没有主清单属性**

         还报了个这错误

      4.  不搞了,默认是jar包打包,哈哈哈哈,上面common和interface都可以看出来(当我在 common包填上这个时;依旧可以打包成功)

         ```java
          <packaging>jar</packaging>
         ```
   
         

   9. 但是 这么做的问题是什么 , 我们需要一个maven仓库 (自己的私服,可以直接提交到那里,每次就不用 我们去 手动提交了,感觉好麻烦,然而 其他解决这种模式的 方式我没有想到,)

   10. 打包插件  maven-plugin 只是对 web工程有效 ,只需要在 我们需要的web模块上加上 这个  plugin就可以了 ;第二种 方式我没有想到解决方法 哎,求大佬带带 更简单的.

​	QA 还有一个是  看俩截图 

- ​	这个是common包下的

![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-01/c31cccc3-3806-4f9d-9669-44d11a9b1dcb.jpg?x-oss-process=style/template01)

- web 模块呢 是这个 

  ![](https://tyut.oss-cn-beijing.aliyuncs.com/image/2019-09-01/df21f68e-9fb1-4d79-98c8-edf142098419.jpg?x-oss-process=style/template01)

可以看到 打包成功 ,结果是实现了 但是挺麻烦的;哎 ;我看看怎么修改一哈





#### 总结

两种模式都是 只用在 web工程下 有main入口函数的文件下, 放入 maven-pluging就可以了;只是第二种需要把放入maven仓库

maven 也是一门学问 ,如果你写框架 必须学,哎,我也不会玩