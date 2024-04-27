#ifndef __SYLAR_H__
#define __SYLAR_H__
#include <string>
#include <stdint.h>
#include <memory>
#include <sstream>
#include <fstream>
#include <list>
#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>
namespace sylar
{
    class Logger;
    // 日志事件
    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent();
        const char *getFile() const { return m_file; }
        int32_t getLine() const { return m_line; }
        int32_t getThreadId() const { return m_threadId; }
        uint32_t getFiberId() const { return m_fiberId; }
        std::string getContent() const { return m_content; }
        uint64_t getTime() const { return m_time; }
        uint32_t getElapse() const { return m_elapse; }

    private:
        const char *m_file = nullptr; // 文件名
        int32_t m_line = 0;           // 行号
        int32_t m_threadId = 0;       // 线程ID
        uint32_t m_fiberId = 0;       // 协程ID
        std::string m_content;        // 日志内容？？？
        uint64_t m_time;              // 时间戳
        uint32_t m_elapse = 0;        // 程序启动开始到现在的的毫秒
    };

    // 日志级别
    class LogLevel
    {
    public:
        enum Level // 日志级别
        {
            UNKNOW = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };

        static const char *ToString(LogLevel::Level level);
    };

    // 日志格式器
    class LogFormatter
    {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
        LogFormatter(const std::string &pattern); // pattern在复制时会根据pattern格式解析出Item信息

    public:
        // 日志解析子模块
        class FormatItem
        {
        public:
            FormatItem(const std::string &fmt = "") {}
            typedef std::shared_ptr<FormatItem> ptr;
            virtual ~FormatItem(){};
            virtual void format(std::ostream os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };

    private:
        std::string m_pattern;                // format输出结构
        std::vector<FormatItem::ptr> m_items; // formatItem列表定义输出多少个项

        void init(); // 初始化pattern解析
    };

    // 日志输出地
    class LogAppender
    {
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        virtual ~LogAppender(){};

        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0; // 定义成纯虚函数
        void setFormatter(LogFormatter::ptr formatter) { m_formatter = formatter; }                       // 设置输出格式
        LogFormatter::ptr getFormatter() const { return m_formatter; }                                    // 获取输出格式     -->常量成员函数:表示保证不修改对象状态，适用于常量对象
    protected:
        LogLevel::Level m_level;
        LogFormatter::ptr m_formatter; // 定义LogAppender的输出格式
    };

    // 日志输出器
    class Logger
    {
    public:
        typedef std::shared_ptr<Logger> ptr;

        Logger(const std::string &name = "root");
        void log(LogLevel::Level level, const LogEvent::ptr event);

        void info(LogEvent::ptr event);
        void debug(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);
        void addAppender(LogAppender::ptr appender);          // 添加appender
        void delAppender(LogAppender::ptr appender);          // 删除appender
        LogLevel::Level getLevel() const { return m_level; }  // 获取日志级别
        void setLevel(LogLevel::Level val) { m_level = val; } // 设置日志级别

        const std::string getName() { return m_name; }

    private:
        LogLevel::Level m_level;                 // 日志级别
        std::string m_name;                      // 日志名称
        std::list<LogAppender::ptr> m_appenders; // appender日志集合
    };

    // 输出到控制台的appender
    class StdLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<StdLogAppender> ptr;
        void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override; // 使用override描述子类的log()的确是从基类中继承而来
    };

    // 定义输出到文件的appender
    class FileLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string &filename);
        void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;
        bool reopen(); // 重新打开文件， true打开成功

    private:
        std::string m_filename;
        std::ofstream m_filestream;
    };
}
#endif