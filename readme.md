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

### 协程库封装
### socket函数库
### http协议开发
### 分布协议
### 推荐系统