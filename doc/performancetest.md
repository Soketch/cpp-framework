## <center> sylar-性能测试 </center>
#### 1. Linux中下载 并发测试工具
> yum install httpd-tools<br>

Ubuntu下没有这个工具包，这里选择**apache2-utils**,获取 htpasswd 等工具
> apt install apache2-utils

检测是否安装成功命令：
```
dpkg -l | grep apache2-utils
```
成功显示如下：
```
ii  apache2-utils                 2.4.52-1ubuntu4.9              amd64        Apache HTTP Server (utility programs for web servers)
```

#### 2.压测服务器
目录文件：samples/my_http_server.cc
```
ab -n 1000000 -c 200 "http://60.204.235.160:8020/sylar"
```
