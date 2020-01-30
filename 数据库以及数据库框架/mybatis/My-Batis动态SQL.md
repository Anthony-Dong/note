# My-Batis动态SQL

## if

​		动态 SQL 通常要做的事情是根据条件包含 where 子句的一部分。比如：注意一点, 你if 包含的字段传参时必须有,就算为null,不能用 java的多态,会报错例如

```java
List<Emp> selectEmpByNOAndSalary(@Param("deptno") Integer deptno, @Param("salary")Integer salary);

// 这个会发生异常
List<Emp> selectEmpByNOAndSalary(@Param("deptno") Integer deptno);
```



```xml
 <select id="selectEmpByNOAndSalary" resultMap="BaseResultMap">
        SELECT
            *
        FROM
            emp
        WHERE
            DEPTNO = #{deptno}
        <if test="null != salary">
            and  SAL > #{salary}
        </if>
</select>
```

```java
1.当不写 salary 时 ,记住 别用基本数据类型,用包装类
empMapper.selectEmpByNOAndSalary(10,null);
// 结果: 
SELECT * FROM emp WHERE DEPTNO = ? 
    
2. 写了以后 
empMapper.selectEmpByNOAndSalary(10, 2000);
// 结果
Preparing: SELECT * FROM emp WHERE DEPTNO = ? and SAL > ? 
```



## choose, when, otherwise

`choose `也就是 可选择的意思 ,类似与 switch ,`when `就是 case的意思 ,如果第一个条件符合 ,不会执行第二个 , 如果第一个第二个都不存在,就会走 `otherwise`  , 这个玩意还是上面那个问题,字段不能用方法重载,

```java
    <select id="selectEmpByChose" resultMap="BaseResultMap">
        SELECT
            *
        FROM
            emp
        WHERE
            DEPTNO = #{deptno}
        <choose>
            <when test="null != salary">
                AND SAL > #{salary}
            </when>
            <when test="null != manager">
                AND MGR = #{manager}
            </when>
            <otherwise>
                AND 0=0
            </otherwise>
        </choose>
    </select>
```

例如 

```java
方法 : 
List<Emp> selectEmpByChose(@Param("deptno") Integer deptno,@Param("salary")  Integer salary, @Param("manager") String manager);

1. 我们全部参数输入 : 
empMapper.selectEmpByChose(20, 2000,"7566");
输出 sql 是 :   SELECT * FROM emp WHERE DEPTNO = ? AND SAL > ?

2. 当我们只输入 salary 时 
empMapper.selectEmpByChose(20, 2000,null);
结果 :  SELECT * FROM emp WHERE DEPTNO = ? AND SAL > ? 

3. 当我们输入 manager 时 
empMapper.selectEmpByChose(20, null,"7566");
结果 :  SELECT * FROM emp WHERE DEPTNO = ? AND MGR = ? 
 
4. 当我们啥也不输入是
empMapper.selectEmpByChose(20, null,null);
结果 : SELECT * FROM emp WHERE DEPTNO = ? AND 0=0 
```



## trim, where, set

### where标签

where 问题 ,我们知道如果用if 第一个条件如果不满足, 那么 结果就成了 where and 显然语法错误 ,为了解决这样的问题 , 提供了` where 标签  `  , *where* 元素只会在至少有一个子元素的条件返回 SQL 子句的情况下才去插入“WHERE”子句。而且，若语句的开头为“AND”或“OR”，*where* 元素也会将它们去除。

```xml
    <select id="selectEmpByWhere"
            resultMap="BaseResultMap">
        SELECT
        *
        FROM
        emp
        <where>
            <if test="null != deptno">
                 DEPTNO = #{deptno}
            </if>
            <if test="null != salary">
                and  SAL > #{salary}
            </if>
            <if test="null != manager">
                and  MGR = #{manager}
            </if>
        </where>
    </select>
```

使用效果 : 

```java
empMapper.selectEmpByWhere(null, null, null);
empMapper.selectEmpByWhere(null, 3000, null);
empMapper.selectEmpByWhere(20, null, null);
empMapper.selectEmpByWhere(20, null, "7839");
empMapper.selectEmpByWhere(20, 2000, "7839");

sql 语句对应的
SELECT * FROM emp 
SELECT * FROM emp WHERE SAL > ? 
SELECT * FROM emp WHERE DEPTNO = ? 
SELECT * FROM emp WHERE DEPTNO = ? and MGR = ? 
SELECT * FROM emp WHERE DEPTNO = ? and SAL > ? and MGR = ?     
```



### trim 标签

如果 *where* 元素没有按正常套路出牌，我们可以通过自定义 trim 元素来定制 *where* 元素的功能。比如，和 *where* 元素等价的自定义 trim 元素为：

有几个字段含义  prefix  prefixOverrides , 是指前缀是where , 会删除多余的前缀 `AND |OR` ,

还有后缀  suffix suffixOverrides ,指后缀是啥  ?  , 会删除多余的后缀

```xml
这个等价于 上面的 where 查询
    <select id="selectEmpByWhere"
            resultMap="BaseResultMap">
        SELECT
        *
        FROM
        emp
        <trim prefix="where" prefixOverrides="AND |OR ">
            <if test="null != deptno">
                 DEPTNO = #{deptno}
            </if>
            <if test="null != salary">
                and  SAL > #{salary}
            </if>
            <if test="null != manager">
                and  MGR = #{manager}
            </if>
        </trim>
    </select>

```



### set

*set* 元素可以用于动态包含需要更新的列，而舍去其它的 

```xml
  <update id="updateRole">
    update role
    <set>
      <if test="role.id != null">id=#{role.id},</if>
      <if test="role.role != null">role=#{role.role},</if>
    </set>
    where id=#{id}
  </update>
```



```java
void updateRole(@Param("id") Integer id, @Param("role") Role role);
```

