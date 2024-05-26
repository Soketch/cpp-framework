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
#include <map>
#include <stdarg.h>
#include <string.h>
#include <cctype>
#include "util.h"
#include "singleton.h"
#include <time.h>
#include "thread.h"

#define SYLAR_LOG_LEVEL(logger, level)                                                                       \
    if (logger->getLevel() <= level)                                                                         \
    sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level,                              \
                                                                 __FILE__, __LINE__, 0, sylar::GetTheadId(), \
                                                                 sylar::GetFiberId(), time(0))))             \
        .getSS()

#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)

#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, ...)                                                          \
    if (logger->getLevel() <= level)                                                                          \
    sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level,                               \
                                                                 __FILE__, __LINE__, 0, sylar::GetThreadId(), \
                                                                 sylar::GetFiberId(), time(0))))              \
        .getEvent()                                                                                           \
        ->format(fmt, __VA_ARGS__)

#define SYLAR_LOG_FMT_DEBUG(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_INFO(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::INFO, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_WARN(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::WARN, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_ERROR(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::ERROR, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_FATAL(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::FATAL, fmt, __VA_ARGS__)

// 通过宏获取logger  m_root
#define SYLAR_LOG_ROOT() sylar::LoggerMgr::GetInstance()->getRoot()

// find log
#define SYLAR_LOG_NAME(name) sylar::LoggerMgr::GetInstance()->getLogger(name)

namespace sylar
{
    class Logger;
    class LoggerManager;
    // 日志事件

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
        static LogLevel::Level FromString(const std::string &str);
    };

    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int32_t line, uint32_t elapse, uint32_t threadId, uint32_t fiberId, uint64_t time);
        const char *getFile() const { return m_file; }
        int32_t getLine() const { return m_line; }
        int32_t getThreadId() const { return m_threadId; }
        uint32_t getFiberId() const { return m_fiberId; }
        std::string getContent() const { return m_ss.str(); }
        uint64_t getTime() const { return m_time; }
        uint32_t getElapse() const { return m_elapse; }
        std::shared_ptr<Logger> getLogger() const { return m_logger; }
        LogLevel::Level getLevel() const { return m_level; }
        std::stringstream &getSS() { return m_ss; }

        void format(const char *fmt, ...);
        void format(const char *fmt, va_list al);

    private:
        const char *m_file = nullptr; // 文件名
        int32_t m_line = 0;           // 行号
        uint32_t m_elapse = 0;        // 程序启动开始到现在的的毫秒
        int32_t m_threadId = 0;       // 线程ID
        uint32_t m_fiberId = 0;       // 协程ID
        uint64_t m_time;              // 时间戳
        std::stringstream m_ss;       //

        std::shared_ptr<Logger> m_logger;
        LogLevel::Level m_level;
    };

    // 存放event的wrap
    class LogEventWrap
    {
    public:
        LogEventWrap(LogEvent::ptr e);
        ~LogEventWrap();
        std::stringstream &getSS();

    private:
        LogEvent::ptr m_event;
    };

    // 日志格式器
    class LogFormatter
    {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
        // std::ostream &format(std::ostream &ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
        LogFormatter(const std::string &pattern); // pattern在复制时会根据pattern格式解析出Item信息

    public:
        // 日志解析子模块
        class FormatItem
        {
        public:
            // FormatItem(const std::string &fmt = "") {}
            typedef std::shared_ptr<FormatItem> ptr;
            virtual ~FormatItem(){};
            virtual void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };

        void init(); // 初始化pattern解析

        bool isError() const { return m_error; }
        std::string getPattern() const { return m_pattern; }

    private:
        std::string m_pattern;                // format输出结构
        std::vector<FormatItem::ptr> m_items; // formatItem列表定义输出多少个项
        bool m_error = false;
    };

    // 日志输出地
    class LogAppender
    {
        friend class Logger;

    public:
        typedef SpinLock MutexType;
        typedef std::shared_ptr<LogAppender> ptr;
        virtual ~LogAppender(){};
        virtual std::string toYamlString() = 0;

        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0; // 定义成纯虚函数
        void setFormatter(LogFormatter::ptr formatter);                                                   // 设置输出格式
        LogFormatter::ptr getFormatter();                                                                 // 获取输出格式

        void setLevel(LogLevel::Level level) { this->m_level = level; }
        LogLevel::Level getLevel() const { return m_level; }

    protected:
        LogLevel::Level m_level = LogLevel::DEBUG;
        bool hasFormatter = false;
        MutexType m_mutex;             // 互斥量
        LogFormatter::ptr m_formatter; // 定义LogAppender的输出格式
    };

    // 日志输出器
    class Logger : public std::enable_shared_from_this<Logger>
    {
        friend class LoggerManager;

    public:
        typedef SpinLock MutexType;
        typedef std::shared_ptr<Logger> ptr;

        Logger(const std::string &name = "root");
        void log(LogLevel::Level level, const LogEvent::ptr event);

        void info(LogEvent::ptr event);
        void debug(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);
        void addAppender(LogAppender::ptr appender); // 添加appender
        void delAppender(LogAppender::ptr appender); // 删除appender
        void clearAppenders();
        LogLevel::Level getLevel() const { return m_level; }  // 获取日志级别
        void setLevel(LogLevel::Level val) { m_level = val; } // 设置日志级别

        const std::string getName() { return m_name; }

        void setFormatter(LogFormatter::ptr val);
        void setFormatter(const std::string &val);
        LogFormatter::ptr getFormatter();

        std::string toYamlString();

    private:
        std::string m_name;                      // 日志名称
        LogLevel::Level m_level;                 // 日志级别
        MutexType m_mutex;                       // 互斥量
        std::list<LogAppender::ptr> m_appenders; // appender日志集合
        LogFormatter::ptr m_formatter;

        Logger::ptr m_root;
    };

    // 输出到控制台的appender
    class StdLogAppender : public LogAppender
    {
        friend class Logger;

    public:
        typedef std::shared_ptr<StdLogAppender> ptr;
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override; // 使用override描述子类的log()的确是从基类中继承而来
        std::string toYamlString() override;
    };

    // 定义输出到文件的appender
    class FileLogAppender : public LogAppender
    {
        friend class Logger;

    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string &filename);
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
        bool reopen(); // 重新打开文件， true打开成功
        std::string toYamlString() override;

    private:
        std::string m_filename;
        std::ofstream m_filestream;
        /// 上次重新打开时间
        uint64_t m_lastTime = 0;
    };

    // 日志管理器
    class LoggerManager
    {
    public:
        typedef SpinLock MutexType;
        LoggerManager();
        Logger::ptr getLogger(const std::string &name);
        void init();
        Logger::ptr getRoot() const { return m_root; }

        std::string toYamlString();

    private:
        MutexType m_mutex; // 互斥量
        std::map<std::string, Logger::ptr> m_loggers;
        // 默认Logger  --> name: root
        Logger::ptr m_root;
    };

    // 使用单例模式
    typedef sylar::Singleton<LoggerManager> LoggerMgr;
}
#endif