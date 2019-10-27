##jdbc-study

### 1.datesource

### 2. 什么是sql注入 ?
	Statement statement = connection.createStatement();
    ResultSet resultSet = statement.executeQuery("select * from student where id =" + id);
    这句话中 可以拼接子字符串  SELECT * FROM student WHERE id=1 OR 1=1
    这样子 就查询了全部的信息