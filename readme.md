# sylar

## 项目路径
bin -- 二进制  <br />
build -- 构建/中间文件路径 <br />
cmake -- cmake函数文件夹 <br />
CMakeLists.txt -- cmake定义文件 <br />
lib -- 库输出路径
sylar -- 源代码输出路径
tests -- 测试代码文件夹

## 日志系统
1）     Log4J

      LoggerManager --> (Singleton/SingletonPtr)
          Logger (日志记录器)                                 
          |                                                  
          +---- Appender (日志输出)                           
          |           |                                      
          |           +---- Formatter (日志格式)              
          |           |          |                           
          |           |          +---- FormatItem (格式项) --> (MessageItem/DataItem/ThreadIditem/FiberIdItem/FileItem/......)    
          |           |                                       
          |           +---- StdLogAppender (标准输出到控制台)  
          |           |                                       
          |           +---- FileLogAppender (输出到文件)       
          |                                                   
          +---- LogLevel (日志级别)                            
          |          |                                        
          |          +---- DEBUG                              
          |          +---- INFO                               
          |          +---- WARN                               
          |          +---- ERROR                              
          |          +---- FATAL                              
          |                                                   
          +---- LogEvent (日志事件)                            
## 配置系统
Config --> yaml<br>
yaml安装    <a href="https://github.com/jbeder/yaml-cpp.git" >github:yaml-cpp</a>
> -- git clone<br>
> -- cd yaml-cpp<br>
> -- mkdir build&& cd build<br>
> -- cmake ..  <br>
> -- make && make install<br>

yaml-cpp使用
```cpp
    YAML::Node node = YAML::LoadFile(filename);
    
    node.IsMap()
    for(aauto it = node.begin(); it != node.end(); ++it)
        it->first,it->second
    
    node.IsSequence()
    for(size_t i = 0; i<node.size(); ++i)

    node.IsScalar()
```

配置系统原则，约定优于配置
```cpp
template<T, FormStr, ToStr>
class ConfigVar;

template<F,T>
LexicalCast;

//容器偏特化   支持vector、list、set、map、unordered_set、unordered_map等
    template <class T> // 将string转vector
    class LexicalCast<std::string, std::vector<T>>

    template <class T>
    class LexicalCast<std::vector<T>, std::string>

    template <class T> // 将map转string
    class LexicalCast<std::map<std::string, T>, std::string>

    template <class T> // 将unordered_map转string
    class LexicalCast<std::unordered_map<std::string, T>, std::string>

    //map与unordered_map 支持key = std::string
    
    问题部分
    Config::Lookup(key) key相同 但是 类型不同的，不会有报错。需要处理！


    自定义类型，需要实现偏特化sylar::LexicalCast
    支持config解析自定义类型，以及stl容器与自定义类型结合使用
    
```

配置变更事件
    事件机制 -- 当一个配置发生修改的时候，可以反向通知对用代码回调
```cpp
public:
    void setValue(const T &v)
        {
            if (v == m_val)// 如果原值等于新值， 没有变化
            { return;}
            for (auto &i : m_cbs)// 不是，变化了，进行回调
            {
                i.second(m_val, v); // 这里 i 是m_cbs数组中的回调函数pair,[key, callback]， //通知回调函数当前值和新值
            }
            m_val = v; // 更新值
        }
        // 对回调函数 - 增加监听
        void addListener(u_int64_t key, on_change_cb cb)   --> 24/5/27  uint64_t addListener(on_channge_cb cb);
        // 删除
        void delListener(uint64_t key)
        // 返回cb
        on_change_cb getListener(u_int64_t key)
        //清空
        void clearListener()
private:
// 变更回调数组，uint64_t key要求唯一，一般使用hash
        std::map<uint64_t, on_change_cb> m_cbs; // 采用map是因为functional中没有比较函数，意味着无法判断是否是相同回调函数
```
#### 日志系统整合配置系统
```yaml
    Logs:
        - name: root (作为唯一标识)
          level: (debug、info、warn、error、fatal)
          formatter: '%d%T%p%T%t%T%m%n'
          appender:
            - type: StdoutLogAppender, FileLogAppender)
              level: (debug, ...)
              file: /logs/xxx.log, ...
```
```cpp
sylar::Logger g_logger = 
    sylar::LoggerMgr::GetInstance()->getLogger(name);

SYLAR_LOG_INFO(g_logger) << " ...  log ";
```

```cpp
保证静态 log 的唯一性
static Logger::ptr g_log = SYLAR_LOG_NAME("system");   //log 静态化

当logger的appenders为空，使用m_root写logger
```

```cpp
//定义LogDefine和LogAppenderDefine, 偏特化LexicalCast
struct LogAppenderDefine
    {
        int type = 0; // 1 file, 2 stdout
        LogLevel::Level level = LogLevel::UNKNOW;
        std::string formatter;
        std::string file;
        bool operator==(const LogAppenderDefine &oth) const
    };
 struct LogDefine
    {
        std::string name;
        LogLevel::Level level = LogLevel::UNKNOW;
        std::string formatter;
        std::vector<LogAppenderDefine> appenders;
        bool operator==(const LogDefine &oth) const

        bool operator<(const LogDefine &oth) const
    };

 template <>
    class LexicalCast<std::string, std::set<LogDefine>>

 template <>
    class LexicalCast<std::set<LogDefine>, std::string>

sylar::ConfigVar<std::set<LogDefine>>::ptr g_log_defines =
    sylar::Config::Lookup("logs", std::set<LogDefine>(), "logs config");
//实现日志配置解析
```

## 线程库

实现Thread类、Mutex互斥量、读写锁

//  禁用拷贝原则

**1.读写分离**
```cpp
    
    . Semaphore 信号量
    . ScopedLockImpl  局部锁模板实现 
        - ReadScopedLockImpl 局部读锁模板
        - WriteScopedLockImpl 局部写锁模板
    . Mutex 互斥量
    . RWMutex 读写互斥量
```
**2.整合日志系统--实现线程安全**

>整合Logger、LogAppender、LogManager

**3.保证线程安全情况下，尽量不损失原有性能**

    -- Spinlock 自旋锁    
    -- CASLock 原子锁
    -- 改进log写文件  ==> 周期性,reopen
    >> 最终在log.h中使用SpinLock代替Mutex
**4.整合配置系统**
```cpp
    typedef RWMutex RWMutexType; 
    RWMutexType m_mutex;             //读写锁互斥量

    static RWMutexType& GetMutex()   //封装配置项互斥量的静态方法
    {
        static RWMutexType s_mutex;
        return s_mutex;
    }
```
## 协程库封装
    
    -- Fiber
    -- Coroutine
定义协程接口 <br>
基于ucontext_t <br>
macro ：定义自己的宏  sylar/macro.h <br>
>    -- 实现SYLAR_ASSERT宏 打印调试栈信息<br>
>    -- 通过backtrace、backtrace_symbols实现

协程构建
```cpp
Fiber::GetThis()  //主协程
Thread -> main_fiber <------> sub_fiber
            ^
            |
            v
         sub_fiber
/* 
    5/30当前模型实现  -- 基于ucontext
    这里没有任意切换协程, 每个调度线程都有一个主协程
    主协程（Main fiber）可以创建子协程（sub fiber）,并调度子协程
    子协程只能执行完任务回到主协程，或者让出执行时间回到主协程
*/
```
```cpp
    // 重置协程函数,并重置状态
    void reset(std::function<void()> cb, size_t stacksize = 0);
    // 切换到当前协程执行
    void swapIn();
    // 切换到后台执行
    void swapOut();
    // 设置当前协程
    static void setThis(Fiber *f);
    // 返回当前协程
    static Fiber::ptr GetThis();
    // 协程切换到后台，并设置ready状态  --准备就绪
    static void YieldToReady();
    // 协程切换到后台，并设置hold状态  --暂停
    static void YieldToHold();
    // 获取总协程数
    static uint64_t TotalFibers();
    // 协程执行主函数
    static void MainFunc();
    // 获取协程id
    static uint64_t GetFiberId();

```
构建Scheduler协程调度模块 -- 实现协程调度器
```cpp
    +- Scheduler --> thread pool （Scheduler类似于一个线程池概念。）
    |        1 - N  一对多
    |
    +- Thread   -->  Fiber
    |        1 - M  线程与协程也是一个一对多的关系
    |
    |
    |
    V
  N : M    ==> 最终可以推导为 N对M 的关系
```
```cpp
Scheduler 1.是一个线程池，可以分配一组线程
          2.是一个协程调度器，实现将协程指定到相应的线程中去执行任务（多种调度策略）
```
优雅的实现start()、stop()、run()方法
```cpp
    // 启动调度器
    void Scheduler::Start()
    {
    }
    // 停止调度器   
    void Scheduler::Stop()
    {
        // 1.等待所有任务完成才退出而不是直接而退出
        // 2.分两种情况，使用了use_caller和没有使用use_caller
    }
     // 协程调度运行函数
    void Scheduler::run()
    {
    }
```
```
1.设置当前线程的scheduler
2.设置当前线程的run、fiber
3.协程调度循环while(true)
    > 1.协程消息队列里面是否有任务
    > 2.无任务执行idle
```

##### IO协程调度模块
实现方式
```
    IOManager(epoll)  ------------>  Scheduler
        |
        |
        V
      idle(epoll_wait)

      Semaphore(信号量)
      PutMessage(msg, ...) => semaphore+1 single()
    Message_queue(消息队列)
        |
        |----Thread
        |----Thread
      RecvMessage(msg, ...) => wait() semaphore-1

    应该实现异步IO，等待数据返回。在epoll_wait等待，没有消息回来会阻塞在epoll_wait中
                                                            |----->处理方式: socket pair? pipe?
    两种唤醒方式
```
```
epoll使用  <sys/epoll.h>
    |----epoll_create
    |----epoll_ctl
    |----epoll_wait
```

##### 定时器设计（根据IO协程调度器、epoll的毫秒级响应）
    Timer --> addTimer() --> cancel()  --> refresh()  -->reset() 
     |
     |---- 获取当前的定时器触发距离到现在的时间差  ==> 返回当前需要触发的定时器

```
 utils中设置获取当前时间毫秒数和微秒数
 |
 +-------------  uint64_t GetCurrentMS();  // ==> ms毫秒
 |
 +-------------  uint64_t GetCurrentUS();  // ==> us微秒
```    
```
定时器Timer和定时器管理类TimerManager
```                                

```
                     [Fiber]                [Timer]
                        ^  N                   ^
                        |                      |
                        |  1                   |
                     [Thread]            [TimerManager]
                        ^  M                   ^
                        |                      |
                        |  1                   |
                   [Scheduler]  <----  [IOManager(epoll)]
                                               |
                                               |
                                            主要是iomanager负责io事件和定时器
```
## socket函数库
##### 1.socket IO Hook
用同步IO写法也能达到实现异步IO功能

socket相关（socket, connect, accept）<br>
io相关 （read, write, recv, send ...）<br>
fd相关 （fcntl, ioctl ...）<br>

引入 ==> 怎么在main函数前执行一个方法？？？？
```
hook   +---- sleep
       |---- usleep

    // 利用静态对象的构造函数在 main 函数之前运行机制，
    //  ====>   静态对象在程序启动时初始化，而它们的构造函数会在 main 函数之前执行
    //  ====>   在构造函数中执行hook初始化操作

    struct _HookIniter
    {
        _HookIniter()
        {
            hook_init();
        }
    };
    
    static _HookIniter s_hook_initer;
```

针对IO操作部分开发一个模板do_io，hook这个di_io模板函数,(对 I/O 操作进行 Hook，通过协程调度和超时管理实现异步 I/O 操作)
```cpp
template <typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char *hook_fun_name,
                     uint32_t event, int timeout_so, Args &&...args)
{
    // 是否启用HOOK
    if (!sylar::t_hook_enable)
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    SYLAR_LOG_DEBUG(g_logger) << "do_io" << hook_fun_name;

    // 获取fd上下文：
    sylar::FdCtx::ptr ctx = sylar::FdMgr::GetInstance()->get(fd);
    if (!ctx)
    {
        return fun(fd, std::forward<Args>(args)...); // 不是fd，直接调用原io函数
    }

    if (ctx->isClose()) // 检测状态
    {
        errno = EBADF;
        return -1;
    }

    if (!ctx->isSocket() || ctx->getUserNonblock()) // 是否是套接字，或者是否人为设置为非阻塞。不满足，直接调用原始的 I/O 函数。
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    uint64_t to = ctx->getTimeout(timeout_so); // 获取超时时间，并创建一个timer_info对象 tinfo
    std::shared_ptr<timer_info> tinfo(new timer_info);

    //  ----------  进行 I/O 操作并处理重试逻辑
retry:
    ssize_t n = fun(fd, std::forward<Args>(args)...); // 先直接执行一遍，返回值有效 直接return n返回

    while (n == -1 && errno == EINTR) // 执行一次后返回值无效，如果返回 EINTR 错误，继续重试操作。
    {
        n = fun(fd, std::forward<Args>(args)...);
    }
    if (n == -1 && errno == EAGAIN) // 如果返回 EAGAIN 错误，表示资源暂时不可用，需要等待。
    {
        // 设置IOManager定时器和事件处理
        sylar::IOManager *iom = sylar::IOManager::GetThis();
        sylar::Timer::ptr timer;
        std::weak_ptr<timer_info> winfo(tinfo);

        if (to != (uint64_t)-1) // 判断超时时间   timeout != -1
        {
            // 添加条件的超时的定时器
            timer = iom->addConditionTimer(to, [winfo, fd, iom, event]()
                                           {
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, (sylar::IOManager::Event)(event)); }, winfo); // 取消，唤醒回来
        }

        int rt = iom->addEvent(fd, (sylar::IOManager::Event)(event)); // 这里没有传递cb参数，是以当前写出作为唤醒对象
        // if (SYLAR_UNLIKELY(rt))
        if (rt)
        { // 添加失败打印日志
            SYLAR_LOG_ERROR(g_logger) << hook_fun_name << " addEvent("
                                      << fd << ", " << event << ")";
            if (timer)
            {
                timer->cancel();
            }
            return -1;
        }
        else // 将当前协程挂起等待事件完成。定时器存在取消，cancel
        {
            sylar::Fiber::YieldToHold(); // 等待唤醒 --> 两种情况唤醒（1读取到事件唤醒）（2超时唤醒）
            if (timer)                   // 检查是否有定时器，有就取消掉
            {
                timer->cancel();
            }
            if (tinfo->cancelled) // 如果定时条件状态存在，就设置errno,直接返回
            {
                errno = tinfo->cancelled;
                return -1;
            }
            goto retry; // 有IO事件回来 --> 重新读 --> goto
        }
    }

    return n;
}
// 这里后期应该考虑不使用goto,采用while可能更安全
```

##### 2.socket 网络模块开发
```
           [UnixAddress]
                |
            +--------+                            +-----[IPv4Address]
            | Address| ------- [IPAddress] -------|
            +--------+                            +-----[IPv6Address]
                |
             [Socket]

// 通过address网络地址的封装(IPv4,IPv6,Unix)
// 抽象出Address 实现在到配置文件中可以配置多种"地址"
```

```
封装Socket
    ==> create 、socket、 bind、 listen、 accept、 connect、 send、 recv等
```
##### 序列化ByteArray
```
write(int 、float、int64_t ....)
read(int 、float、int64_t ....)
```
```cpp
// 关于TLV协议压缩整型
//  ==>  7bit压缩法

// Google Protocol Buffers（protobuf）就使用了Varint编码来压缩整数值

// 有符号转无符号
    // 格式转化
    static uint32_t EncodeZigzag32(const int32_t &v)
    {
        if (v < 0)
        {
            return ((uint32_t)(-v)) * 2 - 1;
        }
        else
        {
            return v * 2;
        }
    }

    // 可变长度的int/uint类型的数据 ：： 压缩
    // 压缩算法
    void ByteArray::writeInt32(int32_t value)
    {
        writeUint32(EncodeZigzag32(value));
    }
    void ByteArray::writeUint32(uint32_t value)
    {
        uint8_t tmp[5];
        uint8_t i = 0;
        while (value >= 0x80)
        {
            tmp[i++] = (value & 0x7f) | 0x80;
            value >>= 7;
        }
        tmp[i++] = value;
        write(tmp, i);
    }

// 格式转化  有符号 转 无符号

/*
原始有符号整数的二进制表示：
-5： 11111011
10： 00001010
-3： 11111101
25： 00011001
50： 00110010
80： 01010000
-2： 11111110
0： 00000000
15： 00001111

经过Zigzag编码后的无符号整数的二进制表示：
9： 00001001
20： 00010100
5： 00000101
50： 00110010
100： 01100100
160： 10100000
3： 00000011
0： 00000000
30： 00011110

Zigzag编码将负数映射为大的奇数，而正数映射为大的偶数，这样可以更有效地利用比特位，减少所需的存储空间。

解码中只需判断是奇数还是偶数，简单解码即可。
Zigzag编码的解码过程相对简单，因为只需要检查无符号整数的最低位（最后一位）即可。
*/
```

## http协议开发
```
协议封装 -- http协议封装
HTTP/1.1 针对API
        |
        +-------- 解析URL, 解析参数，结构体数据

封装两个主要结构体  1：HttpRequest
                  2: HttpResponse
```
```cpp
GET / HTTP/1.1
Host: www.skgfweb.top

HTTP/1.0 200 OK
Pragma: no-cache
Content-Type: text/html
Content-Length: 14988
Connection: close

<-----------------------------分割-------------------------------->

解析一个url:如下

url: http://www.skgfweb.top:80/page/xxx?id=10&v=20#fr
            http  ==> scheme 协议
 www.skgfweb.top  ==> host 主机
              80  ==> port 端口
       /page/xxx  ==> path 路径 
      id=10&v=20  ==> query/param 查询参数
              fr  ==> fragment 片段

协议（scheme）: 指定使用的协议类型，如 http, https, ftp 等。
主机（host）: 通常是域名或IP地址，表示服务器的地址。
端口（port）: 服务器的端口号，用于指定网络端口；默认的HTTP端口是80，如果是HTTPS，则默认端口是443。
路径（path）: 服务器上的特定资源的路径。
查询参数（query/parameter）: 在路径后面，用 ? 开始，包含参数键值对，用 & 分隔多个参数。
片段（fragment）: 在 # 之后的部分，用于指向资源内的某个部分，通常用于网页中的锚点。
```



<b>http协议报文格式： 
<font color="skyBlue"><br>
请求行 - 通用信息头 - 响应头 - 实体头 - 报文主体
</font>
</b>

<b>
http响应-状态消息：
<font color="skyBlue"><br>
1XX:信息（100 continue, 101服务器转换协议）<br>
2XX:成功（200 ok ,  201 Created ,  202 Accepted）<br>
3XX:重定向<br>
4XX:客户端错误<br>
5XX:服务器错误<br>
</font>
</b>
<br>

```
封装 States codes
封装 Methods

```

#### <font color="#18C29E" size="3px">
实现HttpRequestParser和HtttpResponseParser
</font>

http_parser  ==> http解析

>>通过有限状态机的方式生成源码<br>
>>通过有限状态机 => 实现状态解析 =>  http解析

 http_parser是通过有限状态机实现的， 当parser解析到method就会回调到method部分去处理这个method，解析到uri就会回调到uri部分然后处理uri，解析到path就回调到path部分去处理这个path。
解析到什么地方就处理到什么地方，把相应地方处理完。
处理完之后，通过isFinished判断是否结束，如果isFinished是结束状态说明已经解析完了。



 **安装 regel**  
 ```yaml
apt install ragel

git clone mongrel2
github:  https://github.com/mongrel2/mongrel2.git

/mongrel2/src/http11/http*    此目录下共7个文件
使用三个头文件（.h）,两个ragel文件（.rl）
在上面目录中两个c文件就是ragel生成的（可以不必要复制使用）  
```


**ragel生成文件执行命令**
```cpp
ragel -G2 -C http11_parser.rl -o http11_parser.rl.cc
ragel -G2 -C httpclient_parser.rl -o httpclient_parser.rl.cc

ragel帮助命令 ragel --help
```

```cpp
http没有规定每个字段有多长，为了规避恶意发包行为（不属于http协议包、超出长度包、混乱包等）。
这里可以认定所有外部的请求，任何的协议包都是危险的。服务器应该把除它以外外面的所有请求都是恶意的

识别包是否出错：
        ==> 规定请求头最大是多少 ： 4k  超出4k认定这个包有问题
        ==> 规定请求消息体最大是多少 :  64M 同理，超出64M认定这个包有问题
```


#### <font color="#18C29E" size="3px">
封装TCPServer
</font>
```cpp
echo_server测试
···
```


#### Stream 针对文件/socket封装
read/write/readFixSize/writeFixSize<br>
相当于**粘包处理**
```cpp
实现 stream抽象类 - 流接口
实现固定buffer的读写操作
readFixSize(void* buffer)/writeFixSize(void* buffer)
readFixSize(ByteArray::ptr ba)/writeFixSize(ByteArray::ptr ba)
```
```cpp
通过stream 实现 socket_stream封装
重写实现 read/write/close
```

#### HttpSession封装
**<font color="#97829E">HttpSession和HttpConnection区别</font>**：
Server端accept操作产出的socket   ===>(命名) Session
Client端connect操作产生的socket  ===>(命名) Connetion

#### HttpServer封装
HttpServer : TcpServer  (继承与TcpServer)


#### 实现Servlet接口
Servlet 的主要功能是接收 HTTP 请求、处理请求并生成 HTTP 响应。<br>
设置适当的缓冲区大小<br>
检测和限制<br>
流式处理<br>
异常处理<br>
```js
Servlet 的工作原理
客户端请求:   
     客户端（通常是 Web 浏览器）向 Web 服务器发送 HTTP 请求。
Web 服务器接收请求:   
     Web 服务器接收客户端的请求，并将其转发给 Servlet 容器。
Servlet 容器处理请求: 
     Servlet 容器负责查找并调用适当的 Servlet 来处理请求。Servlet 容器会根据请求的 URL 映射找到相应的 Servlet 类。
Servlet 生成响应: 
     Servlet 处理请求，通常会访问数据库、调用其他服务或执行业务逻辑，然后生成响应内容。
发送响应: 
     Servlet 将生成的响应内容返回给 Web 服务器，再由 Web 服务器发送回客户端。
Servlet 的生命周期
Servlet 的生命周期由 Servlet 容器管理，主要包括以下几个阶段：

加载和实例化: 
     当 Servlet 容器启动或收到对某个 Servlet 的第一次请求时，它会加载并实例化该 Servlet。
初始化: 
     Servlet 容器调用 Servlet 的 init 方法进行初始化。这通常用于初始化资源，如数据库连接等。
处理请求: 
     每次请求到达时，Servlet 容器调用 Servlet 的 service 方法，service 方法会根据请求类型（GET、POST 等）调用相应的 doGet、doPost 等方法来处理请求。
销毁: 
    当 Servlet 容器决定销毁一个 Servlet 实例时，会调用 Servlet 的 destroy 方法进行清理工作。这通常用于释放资源，如关闭数据库连接等。
```

        (控制器)            （方法控制器）
        Servlet  <-------- FunctionServlet
           |        继承
           |
           | 管理
           |
           V
        ServletDispatch
        (前端控制器)   它负责将传入的请求分派（dispatch）到不同的 Servlet 或处理器（handler）。

Servlet 是一个抽象基类，定义了处理 HTTP 请求的接口。所有具体的 Servlet 类都必须继承自 Servlet 并实现 handle 方法,Servlet启动，会同时存在多个Servlet<br>
FunctionServlet继承与Servlet, 可以专门用于处理某种类型的请求。通过回调函数实现具体的请求处理逻辑。这使得可以很方便地使用 Lambda 表达式或函数指针来定义处理逻辑。<br>
ServletDispatch会统一管理Servlet,  当用户传递一个uri, 管理对象会通知 现在应该命中的那个Servlet去处理。<br>

#### HttpConnection封装

1.使用Postman模式调试<br>
2.实现httpConnection封装<br>

>  ==> ----- HttpResults封装<br>

3.实现URI,使用uri解析<br>
4.实现一个Http连接池 || 实现keep-alive 长连接的方式<br>
    HttpConnectionPool<br>

## 分布协议
## 推荐系统

2024/6/16   sylar bilibili 72节 有一个问题
http11_parser.rl  
这个文件中  exec处
parser->nread = 0;
parser->mark = 0;
parser->field_len = 0;
parser->field_start = 0;
初始化问题，
重新生成.cc 文件，
ragel -G2 -C http11_parser.rl -o http11_parser.rl.cc


另外修改了http_parser.cc文件中两处parser->setError(1002);
注释掉了parser->setError(1002);
这里后续可以不修改，github中sylar也注释了


另外，弹幕提供一个方法应该也可用
在http11_parser.rl  
这个文件中exec处，将parser->field_len = 0; 修改。
修改为parser->field_len = len;
当天测试是没问题，后续可以重试。