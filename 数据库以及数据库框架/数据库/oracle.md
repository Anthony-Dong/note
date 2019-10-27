# oracle 学习

```shell
1. 拉去镜像
docker pull registry.cn-hangzhou.aliyuncs.com/helowin/oracle_11g

2.创建容器：
docker run -d –p 1521:1521 --name oracle11g registry.cn-hangzhou.aliyuncs.com/helowin/oracle_11g

3.启动容器
docker start oracle11g

4.进入镜像进行配置
- 1. docker exec -it oracle11g bash
- 2. su root
	密码是 helowin
- 3. 编辑profile文件配置ORACLE环境变量
	export ORACLE_HOME=/home/oracle/app/oracle/product/11.2.0/dbhome_2
	export ORACLE_SID=helowin
	export PATH=$ORACLE_HOME/bin:$PATH

5. 创建软连接
ln -s $ORACLE_HOME/bin/sqlplus /usr/bin

6. 切换到oracle 用户
su -u oracle 

7. 登录oracle
sqlplus /nolog
conn / as sysdba;

8.修改密码(system 和 sys 用户 的密码)
alter user system identified by system;
alter user sys identified by sys;
ALTER PROFILE DEFAULT LIMIT PASSWORD_LIFE_TIME UNLIMITED;

9. 创建用户
create user anthony identified by anthony

授权: 
grant dba to anthony;

连接
conn anthony/anthony

select * from user_sys_privs;

10. 退出来
quit

11. 检查状态
lsnrctl status
```





用户切换 

```shell

　　sqlplus / as sysdba;//登陆sys帐户

　　sqlplus sys as sysdba;//同上

　　sqlplus anthony/anthony;//登陆普通用户scott
　　
```





表空间 : 

```shell
- 创建表空间
create tablespace tablespacename datafile '/home/oracle/tablespacename' size 150m;
 
- 给用户分配空间 
 alert user username default tablespace tablespacename;
```







