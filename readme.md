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
Config --> yaml
yaml安装    <a href="https://github.com/jbeder/yaml-cpp.git" >github:yaml-cpp</a>
> -- git clone
> -- cd yaml-cpp
> -- mkdir build&& cd build
> -- cmake ..  
> -- make && make install
### 协程库封装
### socket函数库
### http协议开发
### 分布协议
### 推荐系统