# mysql关系映射

官网文档  地址很详细,可以不看我写的  https://mybatis.org/mybatis-3/zh/sqlmap-xml.html 

​		MyBatis 的真正强大在于它的映射语句，这是它的魔力所在。由于它的异常强大，映射器的 XML 文件就显得相对简单。如果拿它跟具有相同功能的 JDBC 代码进行对比，你会立即发现省掉了将近 95% 的代码。MyBatis 为聚焦于 SQL 而构建，以尽可能地为你减少麻烦。

SQL 映射文件只有很少的几个顶级元素（按照应被定义的顺序列出）：

- `cache` – 对给定命名空间的缓存配置。
- `cache-ref` – 对其他命名空间缓存配置的引用。
- `resultMap` – 是最复杂也是最强大的元素，用来描述如何从数据库结果集中来加载对象。
- `parameterMap` – 已被废弃！老式风格的参数映射。更好的办法是使用内联参数，此元素可能在将来被移除。文档中不会介绍此元素。
- `sql` – 可被其他语句引用的可重用语句块。
- `insert` – 映射插入语句
- `update` – 映射更新语句
- `delete` – 映射删除语句
- `select` – 映射查询语句



## 认识一下 映射语句

文件绝对有一个   <mapper namespace="com.mybatis.mapper.DeptMapper"> 标签开始,对应着你的mapper接口

### resultMap

`		resultMap` 元素是 MyBatis 中最重要最强大的元素。它可以让你从 90% 的 JDBC `ResultSets` 数据提取代码中解放出来，并在一些情形下允许你进行一些 JDBC 不支持的操作。实际上，在为一些比如连接的复杂语句编写映射代码的时候，一份 `resultMap` 能够代替实现同等功能的长达数千行的代码。ResultMap 的设计思想是，对于简单的语句根本不需要配置显式的结果映射，而对于复杂一点的语句只需要描述它们的关系就行了。

```xml
<resultMap id="BasePlusResultMap" type="com.mybatis.pojo.Dept">    <id column="DEPTNO" jdbcType="INTEGER" property="deptno"/>    <result column="DNAME" jdbcType="VARCHAR" property="dname"/>    <result column="LOC" jdbcType="VARCHAR" property="loc"/>    <!-- oftype  emps 是一个存储 Emp 的 ArrayList 集合 -->    <collection property="emps" ofType="com.mybatis.pojo.Emp" javaType="java.util.ArrayList" resultMap="EmpResultMap">    </collection></resultMap>
```



对应的实体bean 是 

```java
@Data
public class Dept implements Serializable {

    private Integer deptno;

    private String dname;

    private String loc;

}
```

查询结果是

```java
==>  Preparing: SELECT d.*, e.* FROM dept d JOIN emp e ON d.DEPTNO = e.DEPTNO WHERE d.DEPTNO = ? 
==> Parameters: 20(Integer)
<==    Columns: DEPTNO, DNAME, LOC, EMPNO, ENAME, JOB, MGR, HIREDATE, SAL, COMM, DEPTNO
<==        Row: 20, RESEARCH, DALLAS, 7396, SMITH, CLERK, 7902, 1980-12-17, 800.00, null, 20
<==        Row: 20, RESEARCH, DALLAS, 7566, JONES, MANAGER, 7839, 1981-04-02, 2975.00, null, 20
<==        Row: 20, RESEARCH, DALLAS, 7782, CLARK, MANAGER, 7839, 1981-06-09, 2450.00, null, 20
<==        Row: 20, RESEARCH, DALLAS, 7788, SCOTT, ANALYST, 7566, 1987-04-19, 3000.00, null, 20
<==        Row: 20, RESEARCH, DALLAS, 7876, ADAMS, CLERK, 7788, 1987-05-23, 1100.00, null, 20
<==        Row: 20, RESEARCH, DALLAS, 7902, FORD, ANALYST, 7566, 1981-12-03, 3000.00, null, 20
<==      Total: 6
```



实际获取的结果是

```java
{
"deptno": 20,
"dname": "RESEARCH",
"loc": "DALLAS",
"emps": [...省略不写了]
}
```





## association

上面的**collection** 是集合 , 那么 **association** 就是对象了 

关联（association）元素处理“有一个”类型的关系。 比如，在我们的示例中，一个博客有一个用户。关联结果映射和其它类型的映射工作方式差不多。 你需要指定目标属性名以及属性的`javaType`（很多时候 MyBatis 可以自己推断出来），在必要的情况下你还可以设置 JDBC 类型，如果你想覆盖获取结果值的过程，还可以设置类型处理器。

关联的不同之处是，你需要告诉 MyBatis 如何加载关联。MyBatis 有两种不同的方式加载关联：

- 嵌套 Select 查询：通过执行另外一个 SQL 映射语句来加载期望的复杂类型。
- 嵌套结果映射：使用嵌套的结果映射来处理连接结果的重复子集。



**property**  : bean属性名称

**jdbcType** : JDBC 类型，所支持的 JDBC 类型参见这个表格之前的“支持的 JDBC 类型”。

**javaType** : 一个 Java 类的完全限定名，或一个类型别名（关于内置的类型别名，可以参考上面的表格）。





#### 关联的嵌套 Select 查询

**column**  : 数据库中的列名，或者是列的别名。一般情况下，这和传递给 `resultSet.getString(columnName)` 方法的参数一样。注意：在使用复合主键的时候，你可以使用 `column="{prop1=col1,prop2=col2}"` 这样的语法来指定多个传递给嵌套 Select 查询语句的列名。

**select** : 用于加载复杂类型属性的映射语句的 ID，它会从 column 属性指定的列中检索数据，作为参数传递给目标 select 语句。

**fetchType** :  可选的。有效值为 `lazy` 和 `eager`。 指定属性后，将在映射中忽略全局配置参数 `lazyLoadingEnabled`，使用属性的值。	 

使用 : 

```java
  <resultMap id="UserMapJoinRole" type="User">
    <association property="role" column="id" javaType="com.mybatis.pojo.Role" select="selectRoleById" />
  </resultMap>

  <select id="selectUserById" resultMap="UserMapJoinRole">
    SELECT * FROM `user` WHERE id=#{id}
  </select>

  <select id="selectRoleById" resultMap="RoleMap">
    SELECT * FROM role WHERE id=#{id}
  </select>
```



#### 关联的多结果集（ResultSet）

```xml
 <resultMap id="UserMapJoinRole2" type="User">
    <result column="id" jdbcType="INTEGER" property="id" />
    <result column="username" jdbcType="VARCHAR" property="username" />
    <result column="hashedPassword" jdbcType="VARCHAR" property="hashedpassword" />
    <association property="role" javaType="com.mybatis.pojo.Role" resultMap="RoleMap" />
  </resultMap>


  <select id="selectUserByI2" resultMap="UserMapJoinRole2">
          SELECT u.*,r.*
      FROM `user` u
      JOIN  role  r
      ON u.id=r.id
      WHERE u.id=#{id}
  </select>
```



## collection 

属性 

“ofType” 属性。这个属性非常重要，它用来将 JavaBean（或字段）属性的类型和集合存储的类型区分开来。 所以你可以按照下面这样来阅读映射

```xml
  <resultMap id="BasePlusResultMap" type="com.mybatis.pojo.Dept">
        <id column="DEPTNO" jdbcType="INTEGER" property="deptno"/>
        <result column="DNAME" jdbcType="VARCHAR" property="dname"/>
        <result column="LOC" jdbcType="VARCHAR" property="loc"/>
        <!-- oftype  emps 是一个存储 Emp 的 ArrayList 集合 -->
        <collection property="emps" ofType="com.mybatis.pojo.Emp" javaType="java.util.ArrayList" resultMap="EmpResultMap">
        </collection>
    </resultMap>
```

使用 

```java

JDBC Connection [HikariProxyConnection@339252666 wrapping com.mysql.jdbc.JDBC4Connection@4ec77824] will not be managed by Spring
==>  Preparing: SELECT d.*, e.* FROM dept d JOIN emp e ON d.DEPTNO = e.DEPTNO WHERE d.DEPTNO = ? 
==> Parameters: 20(Integer)
<==    Columns: DEPTNO, DNAME, LOC, EMPNO, ENAME, JOB, MGR, HIREDATE, SAL, COMM, DEPTNO
<==        Row: 20, RESEARCH, DALLAS, 7396, SMITH, CLERK, 7902, 1980-12-17, 800.00, null, 20
<==        Row: 20, RESEARCH, DALLAS, 7566, JONES, MANAGER, 7839, 1981-04-02, 2975.00, null, 20
<==        Row: 20, RESEARCH, DALLAS, 7782, CLARK, MANAGER, 7839, 1981-06-09, 2450.00, null, 20
<==        Row: 20, RESEARCH, DALLAS, 7788, SCOTT, ANALYST, 7566, 1987-04-19, 3000.00, null, 20
<==        Row: 20, RESEARCH, DALLAS, 7876, ADAMS, CLERK, 7788, 1987-05-23, 1100.00, null, 20
<==        Row: 20, RESEARCH, DALLAS, 7902, FORD, ANALYST, 7566, 1981-12-03, 3000.00, null, 20
<==      Total: 6
Closing non transactional SqlSession [org.apache.ibatis.session.defaults.DefaultSqlSession@5b76503f]

```



结果

```json
{
"deptno": 20,
"dname": "RESEARCH",
"loc": "DALLAS",
"emps": [...............]
}
```

