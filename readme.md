# sylar

## 项目路径
bin -- 二进制  <br/>
build -- 构建/中间文件路径 <br/>
cmake -- cmake函数文件夹 <br/>
CMakeLists.txt -- cmake定义文件 <br/>
lib -- 库输出路径
sylar -- 源代码输出路径
tests -- 测试代码文件夹

### 日志系统
1）     Log4J

Logger (日志记录器) </br>
  |                 </br>
  +---- Appender (日志输出)</br>
  |           |     </br>
  |           +---- Formatter (日志格式)    </br>
  |           |          |      </br>
  |           |          +---- FormatItem (格式项)    </br>
  |           |     </br>
  |           +---- StdLogAppender (标准输出到控制台)     </br>
  |           |     </br>
  |           +---- FileLogAppender (输出到文件)
  |
  +---- LogLevel (日志级别)     </br>
  |          |      </br>
  |          +---- DEBUG    </br>
  |          |      </br>
  |          +---- INFO     </br>
  |          |        </br>
  |          +---- WARN     </br>
  |          |      </br>
  |          +---- ERROR      </br>
  |          |      </br>
  |          +---- FATAL      </br>
  |     </br>
  +---- LogEvent (日志事件)     </br>
            
### 协程库封装
### socket函数库
### http协议开发
### 分布协议
### 推荐系统