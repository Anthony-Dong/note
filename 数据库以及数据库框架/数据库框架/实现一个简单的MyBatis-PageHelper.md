# 简单实现一个PageHelper

## 1. [PageHelper](https://github.com/pagehelper/Mybatis-PageHelper)

可以去GitHub上看一下

原理就是基于 Mybatis 的 Interceptor接口实现的 , 其实很简单 ,Mybatis这个接口可以拓展第三方插件

## 1. 直接粘代码

注释很详细 ....

```java
import com.mybatis.mapper.DeptMapper;
import com.mybatis.pojo.Dept;
import org.apache.ibatis.cache.CacheKey;
import org.apache.ibatis.datasource.pooled.PooledDataSource;
import org.apache.ibatis.executor.Executor;
import org.apache.ibatis.mapping.*;
import org.apache.ibatis.plugin.*;
import org.apache.ibatis.session.*;
import org.apache.ibatis.transaction.jdbc.JdbcTransactionFactory;

import java.util.List;
import java.util.Properties;

/**
 * @date:2019/11/30 16:18
 * @author: <a href='mailto:fanhaodong516@qq.com'>Anthony</a>
 */

public class MybatisConf {

    public static void main(String[] args) {

        // 1. 创建configuration 对象
        Configuration configuration = new Configuration(
                new Environment("env",
                        new JdbcTransactionFactory(),
                        new PooledDataSource("com.mysql.jdbc.Driver",
                                "jdbc:mysql://127.0.0.1:3306/tyut?useSSL=false",
                                "root", "root")));

        configuration.addMappers("com.mybatis.mapper");

        // 设置一级缓存级别
        configuration.setLocalCacheScope(LocalCacheScope.SESSION);

		// 添加我们的拦截器对象
        configuration.addInterceptor(new MyIntercepts());


        // 2. 拿到配置创建一个工厂
        SqlSessionFactory sqlSessionFactory = new SqlSessionFactoryBuilder().build(configuration);

        // 3. 通过工厂创建一个 会话对象 SqlSession
        SqlSession sqlSession = sqlSessionFactory.openSession();

        // 4. 会话建立 执行SQL
        DeptMapper mapper = sqlSession.getMapper(DeptMapper.class);


        System.out.println("============第一次查询==============");
        /**
         * 第一次查询
         */
        PageHelper.limit(2, 4);
        List<Dept> depts = mapper.selectAll();
        depts.forEach(e -> {
            System.out.println("查询到 : " + e);
        });

        System.out.println("============第二次查询==============");
        /**
         * 第二次查询
         */
        PageHelper.limit(2, 4);
        List<Dept> deptss = mapper.selectAll();

        for (Dept dept : deptss) {
            System.out.println("查询到 : " + dept);
        }

    }


    /**
     * 拦截器 ,
     */
    @Intercepts(@Signature(type = Executor.class, method = "query", args = {MappedStatement.class, Object.class, RowBounds.class, ResultHandler.class}))
    static class MyIntercepts implements Interceptor {

        private static final String PLACE_HOLDER = " ";

        private ThreadLocal<PageHelper> helpers = PageHelper.helpers;

        /**
         * 整个拦截链路 ,其实就是 创建一个新的 BoundSql .......
         *
         * @param invocation
         * @return
         * @throws Throwable
         */
        @Override
        public Object intercept(Invocation invocation) throws Throwable {
            Object[] args = invocation.getArgs();
            //  mappedStatement
            MappedStatement ms = (MappedStatement) args[0];

            // parameter
            Object parameterObject = args[1];

            /**
             * 我们唯一要做的就是 修改 SQL ,其他都不用管理  其实就是 BoundSql
             */
            BoundSql boundSql = ms.getBoundSql(parameterObject);
            String sql = ms.getBoundSql(parameterObject).getSql();

            // 获取 pageHelper
            PageHelper pageHelper = helpers.get();

            String afterSql = sql + PLACE_HOLDER + "limit" + PLACE_HOLDER + pageHelper.getStartNum() + "," + pageHelper.getPageSize();

            // 用完移除 , 防止堆溢出
            helpers.remove();

            // rowBound
            RowBounds rowBounds = (RowBounds) args[2];


            //resultHandler
            ResultHandler resultHandler = (ResultHandler) args[3];


            // executor
            Executor executor = (Executor) invocation.getTarget();


            // 修改后的 BoundSql
            BoundSql modifyBoundSql = new BoundSql(ms.getConfiguration(), afterSql, boundSql.getParameterMappings(), parameterObject);


            //构建一个CacheKey , 其实这里 的话 MyBatis已经帮我们做了缓存Key的生成,内部维护了缓存 
            CacheKey cacheKey = executor.createCacheKey(ms, parameterObject, rowBounds, modifyBoundSql);

			// 执行返回结果
            return executor.query(ms, parameterObject, rowBounds, resultHandler, cacheKey, modifyBoundSql);
        }

        @Override
        public Object plugin(Object target) {
            return Plugin.wrap(target, this);
        }

        @Override
        public void setProperties(Properties properties) {
            //...
        }
    }


    /**
     * page Helper 对象
     */
    static class PageHelper {
        /**
         * 维护了一个 ThreadLocal 对象去维护他
         */
        final static ThreadLocal<PageHelper> helpers = new ThreadLocal<>();
        private int startNum;
        private int pageSize;

        public int getStartNum() {
            return startNum;
        }

        public int getPageSize() {
            return pageSize;
        }


        public PageHelper(int startNum, int pageSize) {
            this.startNum = startNum;
            this.pageSize = pageSize;
        }

        /**
         * 执行一般多线程 , 所以不会出现啥意外事故 ... 如果怕出现, 就需要维护更复杂的对象了 , 比如设置 校验值
         *
         * 单线程容易出现, 我第一个查询还没拿到第一个查询设置的对象 , 你第二个查询执行到这里把我刚刚设置的对象给覆盖了
         * @param startNum
         * @param pageSize
         */
        public static void limit(int startNum, int pageSize) {
            // 设置对象
            helpers.set(new PageHelper(startNum, pageSize));
        }
    }
}

```



## 3.环境

**pojo**   `com.mybatis.pojo.Dept`

```java
public class Dept {

    private Integer deptno;

    private String dname;

    private String loc;

}
```

mapper.xml  `com.mybatis.mapper.DeptMapper.xml`

```java
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE mapper PUBLIC "-//mybatis.org//DTD Mapper 3.0//EN" "http://mybatis.org/dtd/mybatis-3-mapper.dtd">
<mapper namespace="com.mybatis.mapper.DeptMapper">

    <resultMap id="DeptResultMap" type="com.mybatis.pojo.Dept">
        <id column="DEPTNO" jdbcType="INTEGER" property="deptno"/>
        <result column="DNAME" jdbcType="VARCHAR" property="dname"/>
        <result column="LOC" jdbcType="VARCHAR" property="loc"/>
    </resultMap>

    <select id="selectAll" resultType="com.mybatis.pojo.Dept">
        SELECT * from  Dept
    </select>

</mapper>
```

mapper :  `com.mybatis.mapper.DeptMapper`

```java
@Repository
public interface DeptMapper {

    List<Dept> selectAll();

}
```

SQL语句

```java
-- ----------------------------
-- DATABASE structure for dept
-- ----------------------------

CREATE DATABASE tyut

USE tyut

-- ----------------------------
-- Table structure for dept
-- ----------------------------
DROP TABLE IF EXISTS `dept`;
CREATE TABLE `dept` (
  `DEPTNO` int(2) NOT NULL,
  `DNAME` varchar(14) DEFAULT NULL,
  `LOC` varchar(13) DEFAULT NULL,
  PRIMARY KEY (`DEPTNO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- Records of dept
-- ----------------------------
INSERT INTO `dept` VALUES ('10', 'ACCOUNTING', 'NEW YOURK');
INSERT INTO `dept` VALUES ('20', 'PostOffice', 'DALLAS');
INSERT INTO `dept` VALUES ('30', 'government', 'CHICAGO');
INSERT INTO `dept` VALUES ('40', 'OPERATIONS', 'BOSTON');
```

