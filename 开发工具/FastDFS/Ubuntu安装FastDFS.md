# Ubuntu安装FastDFS

## 1 安装依赖

### 1.1 安装libevent

```shell
防火墙
ufw enable
ufw disable

自启动管理：
apt-get install sysv-rc-conf



apt-get install make

apt-get install unzip

apt-get install gcc

apt-get install libevent-dev

```

  



### 1.2 安装libfastcommon

通过git下载即可： [https://github.com/happyfish100/libfastcommon.git](http://blog.csdn.net/u014230881/article/details/78537708) 

apt install unzip

apt install gcc





### nginx依赖

```shell
安装gcc g++的依赖库
sudo apt-get install build-essential
sudo apt-get install libtool

安装pcre依赖库（http://www.pcre.org/）
sudo apt-get update
sudo apt-get install libpcre3 libpcre3-dev

安装zlib依赖库（http://www.zlib.net）
sudo apt-get install zlib1g-dev

安装SSL依赖库（16.04默认已经安装了）
sudo apt-get install openssl
```

