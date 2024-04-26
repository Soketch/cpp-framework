#ifndef __SYLAR_H__
#define __SYLAR_H__
#include <string>
#include <stdint.h>
#include <memory>
namespace sylar
{
    // 日志事件
    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent();

    private:
        const char *m_file = nullptr;
        int32_t m_line = 0;     // 行号
        int32_t m_threadId = 0; // 线程ID
        uint32_t m_fiberId = 0; // 协程ID
        std::string m_content;
        uint64_t m_time;
    };

    // 日志级别
    class LogLevel
    {
    public:
        enum Level // 日志级别
        {
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };
    };

    // 日志输出地
    class LogAppender
    {
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        virtual ~LogAppender(){};

        void log(LogLevel::Level level, LogEvent::ptr event);

    private:
        LogLevel::Level m_level;
    };

    // 日志格式器
    class LogFormatter
    {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        std::string format(LogEvent::ptr event);

    private:
    };

    // 日志输出器
    class Logger
    {
    public:
        typedef std::shared_ptr<Logger> ptr;

        Logger(const std::string &name = "root");
        void log(LogLevel::Level, const LogEvent &event);

    private:
        LogLevel::Level m_level;
    };

    // 输出到控制台的appender
    class StdLogAppender : public LogAppender
    {
    };
}
#endif