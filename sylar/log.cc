#include "log.h"
namespace sylar
{
    Logger::Logger(const std::string &name = "root")
        : m_name(name)
    {
    }

    void Logger::addAppender(LogAppender::ptr appender) // 添加appender
    {
        m_appenders.push_back(appender);
    }
    void Logger::delAppender(LogAppender::ptr appender) // 删除appender
    {
        for (auto it = m_appenders.begin();
             it != m_appenders.end();
             ++it)
        {
            if (*it == appender)
            {
                m_appenders.erase(it);
                break;
            }
        }
    }

    void Logger::log(LogLevel::Level level, const LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            for (auto &i : m_appenders)
            {
                i->log(level, event);
            }
        }
    }

    void Logger::info(LogEvent::ptr event)
    {
        log(LogLevel::INFO, event);
    }

    void Logger::debug(LogEvent::ptr event)
    {
        log(LogLevel::DEBUG, event);
    }

    void Logger::warn(LogEvent::ptr event)
    {
        log(LogLevel::WARN, event);
    }

    void Logger::error(LogEvent::ptr event)
    {
        log(LogLevel::ERROR, event);
    }

    void Logger::fatal(LogEvent::ptr event)
    {
        log(LogLevel::FATAL, event);
    }

    // StdLogAppender重写基类LogAppender的log
    void StdLogAppender::log(LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            std::cout << m_formatter->format(event) << std::endl;
        }
    }

    FileLogAppender::FileLogAppender(const std::string &filename) : m_filename(filename)
    {
    }

    bool FileLogAppender::reopen() // 文件重新打开
    {
        if (m_filestream) // 如果文件已经打开过就关闭，然后重新打开
        {
            m_filestream.close();
        }
        m_filestream.open(m_filename);
        // return !!m_filestream; //!!m_filestream 这里的双重否定 (!!) 将 m_filestream 转换为布尔值。这个表达式等价于 m_filestream.good(), 如果流对象处于良好状态，则返回 true，否则返回 false。

        // 更好的写法
        return m_filestream.is_open(); // 检查文件流是否成功打开
    }

    // FileLogAppender重写基类LogAppender的log
    void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            m_filestream << m_formatter->format(event);
        }
    }

}
