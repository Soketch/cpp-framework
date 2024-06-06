# sylar

## 项目路径
bin -- 二进制  <br />
build -- 构建/中间文件路径 <br />
cmake -- cmake函数文件夹 <br />
CMakeLists.txt -- cmake定义文件 <br />
lib -- 库输出路径
sylar -- 源代码输出路径
tests -- 测试代码文件夹

### 日志系统
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
### 配置系统
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

### 线程库

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
### 协程库封装
    
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
    |        1 - N  线程与协程也是一个一对多的关系
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
### socket函数库
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

// 抽象出Address 实现在到配置文件中可以配置多种"地址"
```

### http协议开发
### 分布协议
### 推荐系统