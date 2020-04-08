# Maven使用的一些技巧

## 1. 依赖

### 1. properties

> ​	其实就是全局的属性,  其他地方引用可以通过 `${java.version}` 引入 , 比如下面的标签. 

```xml
<properties>
    <project.reporting.outputEncoding>UTF-8</project.reporting.outputEncoding>
    <java.version>1.8</java.version>
</properties>
```

### 2. dependencies

> ​	用的最多的, 就是项目直接导入的依赖. 

```xml
<dependencies>
    <dependency>
        <groupId>org.apache.commons</groupId>
        <artifactId>commons-lang3</artifactId>
    </dependency>
</dependencies>
```

这里还有个 scope . 

> - compile，缺省值，适用于所有阶段，会随着项目一起发布。  **(这个是强依赖)**
>
> * provided，类似compile，期望JDK、容器或使用者会提供这个依赖。如servlet.jar。 
> * runtime，只在运行时使用，如JDBC驱动，适用运行和测试阶段。  **(这个是运行时依赖) **
> * test，只在测试时使用，用于编译和运行测试代码。不会随项目发布。 
> * system，需要显式提供包含依赖的jar，Maven不会在Repository中查找它。,需要指定`<systempath>${basedir}/WebContent/WEB-INF/lib/hamcrest-core-1.3.jar</systempath>`
> * import , 这个是maven2.0.9版本后出的属性，import只能在dependencyManagement的中使用，能解决maven单继承问题，import依赖关系实际上并不参与限制依赖关系的传递性。

### dependencyManagement

> ​	帮助我们管理 dependencies  ,这个的作用就是全局的一个包管理. 比如直接依赖中没有找到, 则会来这里找 . 

```xml
<dependencyManagement>
    <dependencies>
        <dependency>
            <groupId>org.springframework.cloud</groupId>
            <artifactId>spring-cloud-dependencies</artifactId>
            <version>Finchley.RELEASE</version>
            <type>pom</type>
            <scope>import</scope>
        </dependency>
    </dependencies>
</dependencyManagement>
```

## 2. 插件 

### JDK编译插件

```xml
<build>
    <plugins>
        <plugin>
            <groupId>org.apache.maven.plugins</groupId>
            <artifactId>maven-compiler-plugin</artifactId>
            <version>3.8.0</version>
            <configuration>
                <source>1.8</source>
                <target>1.8</target>
                <encoding>UTF-8</encoding>
            </configuration>
        </plugin>
    </plugins>
</build>
```



### 可执行文件插件

> ​	在 MainClass 这里放入你的主类的全限定名称. 

```xml
<plugin>
    <groupId>org.apache.maven.plugins</groupId>
    <artifactId>maven-shade-plugin</artifactId>
    <version>3.1.1</version>
    <executions>
        <execution>
            <phase>package</phase>
            <goals>
                <goal>shade</goal>
            </goals>
            <configuration>
                <transformers>
                    <transformer implementation="org.apache.maven.plugins.shade.resource.ManifestResourceTransformer">
                        <mainClass>com.test.Demo</mainClass>
                    </transformer>
                </transformers>
                <artifactSet>
                </artifactSet>
            </configuration>
        </execution>
    </executions>
</plugin>
```



### 编译RT包

> ​	我们的编译插件额外加入编译目标.

```xml
<build>
    <plugins>
        <plugin>
            <groupId>org.apache.maven.plugins</groupId>
            <artifactId>maven-compiler-plugin</artifactId>
            <version>3.8.0</version>
            <configuration>
                <source>1.8</source>
                <target>1.8</target>
                <encoding>UTF-8</encoding>
                <compilerArguments>
                    <bootclasspath>${JAVA_HOME}/jre/lib/rt.jar</bootclasspath>
                </compilerArguments>
            </configuration>
        </plugin>
    </plugins>
</build>
```



## 3. 多环境

下面配置会说明一下当前的 环境. 默认是基于dev的 . 

```xml
<profiles>
    <profile>
        <id>dev</id>
        <activation>
            <activeByDefault>true</activeByDefault>
        </activation>
        <properties>
            <!--这个是我们的定义的标签.下面这个deploy.type-->
            <deploy.type>dev</deploy.type>
        </properties>
    </profile>

    <profile>
        <id>test</id>
        <properties>
            <deploy.type>test</deploy.type>
        </properties>
    </profile>
</profiles>
```

其次就是配置 resource , 这里代表你的resource目录位置 .  同时还可以添加多个resource. 

```xml
<build>
        <resources>
            <resource>
                <!--这个其实就是你的 classpath 到 src/main/resources/dev 下面了-->
                <directory>src/main/resources/${deploy.type}</directory>
                <!--关闭过滤功能, 开启后可以进行过滤.比如 <includes> 或者  <excludes>  -->
                <filtering>false</filtering>
            </resource>
        </resources>
</build>
```

这个有啥用呢. 可以通过 `mvn clean package -P test`  可以通过 `-P` 指定一下环境. 这个命令是test .

这个命令是 `mvn clean package -DprofileActive=prod` 的缩写. 

## 4. build

```xml
<build>
    <finalName>${project.artifactId}</finalName>
</build>
```

可以指定你的 build后面文件名

mvn的命令 很简单.  clean 就是我们说的.  

## 5.命令

#### 1. 添加配置属性

首先 `mvn -Dxxx=xxx` , 其中`xxx` 代表的就是properties , 也就是属性, 这个可以设置多个属性. 

#### 2. 跳过测试

第二就是跳过测试 `mvn clean package -Dmaven.test.skip=true `  , 这个并没并不会去编译test目录下的文件.  而 `-DskipTests` 会编译测试文件, 所以很坑. 

#### package 和 install 的区别

第一个 , 只是打包, 在你的target目录下.  但是install会发布到你本地的仓库.   如果使用命令打包 , 可以通过`mvn install:install-file -Dfile=D:\commons\target\commons-0.0.1-SNAPSHOT.jar -DgroupId=com.example -DartifactId=commons -Dversion=0.0.1-SNAPSHOT -Dpackaging=jar`  这个命令还是很简单的. 



所以通常打包的命令就是 `mvn clean  package -Dmaven.test.skip=true` 

## 6. mvn镜像设置

普通镜像. 

```xml
<repositories>
    <repository>
        <id>public</id>
        <name>aliyun nexus</name>
        <url>http://maven.aliyun.com/nexus/content/groups/public/</url>
        <releases>
            <enabled>true</enabled>
        </releases>
    </repository>
</repositories>
```



设置pluging 的source

```xml
<pluginRepositories>
    <pluginRepository>
        <id>public</id>
        <name>aliyun nexus</name>
        <url>http://maven.aliyun.com/nexus/content/groups/public/</url>
        <releases>
            <enabled>true</enabled>
        </releases>
        <snapshots>
            <enabled>false</enabled>
        </snapshots>
    </pluginRepository>
</pluginRepositories>
```



比如maven仓库的设置镜像如下.  在setting.xml设置.

```xml
<mirror>
    <id>nexus-aliyun</id>
    <mirrorOf>central</mirrorOf>
    <name>Nexus aliyun</name>
    <url>http://maven.aliyun.com/nexus/content/groups/public</url>
</mirror>/
```

## 7. SpringBoot的多环境配置

SpringBoot配置文件, 如果不通过maven配置的话, 其实只要通过一个参数配置就行了. 

`spring.profiles.active=dev`

我们可以通过.  命令行启动命令来设置, 区别于mvn的环境, 他那个是打包环境, 这个是运行时环境. 

这里可以通过三种方式配置 : 

第一种在classpath下面的`application.properties` 文件中加入你需要激活的环境

![](https://tyut.oss-accelerate.aliyuncs.com/image/2020-40-44/936c5210-2350-4281-9394-3e6ef06d3b78.png?x-oss-process=style/template01)



第二种就是靠evn环境 , 可以通过 `System.setProperty("spring.profiles.active" ,"dev");`

第三站 : ` java -jar -Dspring.profiles.active=dev`  

第四种 : `java -jar --spring.profiles.active=dev`



