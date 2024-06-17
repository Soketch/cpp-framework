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
安装net-tools
> apt install net-tools  或者 yum install net-tools

#### 2.压测服务器
目录文件：samples/my_http_server.cc<br>
压测命令
```cpp
ab -n 1000000 -c 200 -t 5 "http://60.204.235.160:8020/sylar"
```
查看程序
```cpp
netstat -ntlap | grep my_http_server
```
查看进行线程信息
```cpp
top -p PID
top -H -p PID

```
压测命令得到的压测结果
```cppclear
## 单线程测试
并发数200，超时时间5秒
root@hcss-ecs-42c0:/home/coding/cpp/sylar/build# ab -n 1000000 -c 200 -t 5 "http://60.204.235.160:8020/sylar"
This is ApacheBench, Version 2.3 <$Revision: 1879490 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 60.204.235.160 (be patient)
Finished 247 requests


Server Software:        sylar/1.0.0
Server Hostname:        60.204.235.160
Server Port:            8020

Document Path:          /sylar
Document Length:        24440 bytes

Concurrency Level:      200
Time taken for tests:   5.041 seconds
Complete requests:      247
Failed requests:        246
   (Connect: 0, Receive: 0, Length: 246, Exceptions: 0)
Non-2xx responses:      395
Total transferred:      64408744 bytes
HTML transferred:       64364504 bytes
Requests per second:    49.00 [#/sec] (mean)
Time per request:       4081.397 [ms] (mean)
Time per request:       20.407 [ms] (mean, across all concurrent requests)
Transfer rate:          12478.69 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        2    4   1.2      5       6
Processing:    10  172 461.8     62    5029
Waiting:        2   63  48.1     54     203
Total:         12  176 461.5     68    5034

Percentage of the requests served within a certain time (ms)
  50%     67
  66%     90
  75%    107
  80%    117
  90%    502
  95%    565
  98%    753
  99%    905
 100%   5034 (longest request)
```
```cpp
双线程测试
并发数200 超时时间10秒
root@hcss-ecs-42c0:/home/coding/cpp/sylar/build# ab -n 1000000 -c 200 -t 10  "http
://60.204.235.160:8020/sylar"
This is ApacheBench, Version 2.3 <$Revision: 1879490 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 60.204.235.160 (be patient)
Finished 121 requests


Server Software:        sylar/1.0.0
Server Hostname:        60.204.235.160
Server Port:            8020

Document Path:          /sylar
Document Length:        122152 bytes

Concurrency Level:      200
Time taken for tests:   10.861 seconds
Complete requests:      121
Failed requests:        88
   (Connect: 0, Receive: 0, Length: 88, Exceptions: 0)
Non-2xx responses:      225
Total transferred:      12555272 bytes
HTML transferred:       12530072 bytes
Requests per second:    11.14 [#/sec] (mean)
Time per request:       17952.491 [ms] (mean)
Time per request:       89.762 [ms] (mean, across all concurrent requests)
Transfer rate:          1128.88 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        2  181 1094.5      4    8207
Processing:     5  619 1845.5     12    8246
Waiting:        2  296 1298.8      6    6425
Total:          9  800 2107.5     14    8248

Percentage of the requests served within a certain time (ms)
  50%     14
  66%     23
  75%     24
  80%     33
  90%   3388
  95%   6457
  98%   8196
  99%   8218
 100%   8248 (longest request)
```
```cpp
4线程执行， 200并发数， 超时时间10秒
root@hcss-ecs-42c0:/home/coding/cpp/sylar/build# ab -n 1000000 -c 200 -t 10  "http
://60.204.235.160:8020/sylar"
This is ApacheBench, Version 2.3 <$Revision: 1879490 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 60.204.235.160 (be patient)
Finished 676 requests


Server Software:        sylar/1.0.0
Server Hostname:        60.204.235.160
Server Port:            8020

Document Path:          /sylar
Document Length:        23200 bytes

Concurrency Level:      200
Time taken for tests:   10.374 seconds
Complete requests:      676
Failed requests:        458
   (Connect: 0, Receive: 0, Length: 458, Exceptions: 0)
Non-2xx responses:      818
Total transferred:      59609144 bytes
HTML transferred:       59517528 bytes
Requests per second:    65.17 [#/sec] (mean)
Time per request:       3069.102 [ms] (mean)
Time per request:       15.346 [ms] (mean, across all concurrent requests)
Transfer rate:          5611.58 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        2   17 322.3      3    8321
Processing:     7  116 432.9     41    6560
Waiting:        2   39 115.1     35    2975
Total:         10  133 586.1     44    9832

Percentage of the requests served within a certain time (ms)
  50%     44
  66%     48
  75%     51
  80%     54
  90%    247
  95%    381
  98%    685
  99%   1873
 100%   9832 (longest request)
```


#### nginx
安装nginx
```
apt install nginx
```
启动nginx
```
systemctl start nginx

systemctl status nginx
```
压测nginx
```cpp
压测命令： ab -n 1000000 -c 200 -t -10 "http://60.204.235.160:80/sylar" 

压测结果：
root@hcss-ecs-42c0:/home/coding/cpp/sylar/build# ab -n 1000000 -c 200 -t 10  "http
://60.204.235.160:80/sylar"
This is ApacheBench, Version 2.3 <$Revision: 1879490 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 60.204.235.160 (be patient)
Completed 5000 requests
Finished 7189 requests


Server Software:        nginx/1.18.0
Server Hostname:        60.204.235.160
Server Port:            80

Document Path:          /sylar
Document Length:        162 bytes

Concurrency Level:      200
Time taken for tests:   10.000 seconds
Complete requests:      7189
Failed requests:        0
Non-2xx responses:      7228
Total transferred:      2320188 bytes
HTML transferred:       1170936 bytes
Requests per second:    718.89 [#/sec] (mean)
Time per request:       278.206 [ms] (mean)
Time per request:       1.391 [ms] (mean, across all concurrent requests)
Transfer rate:          226.58 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        2   15 190.9      2    7247
Processing:     2   42 351.5      2    6488
Waiting:        2   22 234.7      2    6488
Total:          4   57 408.9      4    8076

Percentage of the requests served within a certain time (ms)
  50%      4
  66%      5
  75%      9
  80%     10
  90%     11
  95%     11
  98%   1026
  99%   1684
 100%   8076 (longest request)

```

### 下载Libevent
```
下载：
apt install libevent-dev

验证安装：
ls /usr/include/event2

安装Libtool:
apt install libtool
```

