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
macro ：定义自己的宏  sylar/macro.h
### socket函数库
### http协议开发
### 分布协议
### 推荐系统