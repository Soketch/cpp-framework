#include "log.h"
#define RESET "\033[0m"
#define BLACK "\033[30m"   /* Black */
#define RED "\033[31m"     /* Red */
#define GREEN "\033[32m"   /* Green */
#define YELLOW "\033[33m"  /* Yellow */
#define BLUE "\033[34m"    /* Blue */
#define MAGENTA "\033[35m" /* Magenta */
#define CYAN "\033[36m"    /* Cyan */
#define WHITE "\033[37m"   /* White */

// 定义日志级别和颜色的映射
#define LOG_COLOR(level)                                                     \
    ((level) == LogLevel::DEBUG ? BLUE : (level) == LogLevel::INFO ? GREEN   \
                                     : (level) == LogLevel::WARN   ? YELLOW  \
                                     : (level) == LogLevel::ERROR  ? RED     \
                                     : (level) == LogLevel::FATAL  ? MAGENTA \
                                                                   : RESET)

namespace sylar
{
    const char *LogLevel::ToString(LogLevel::Level level)
    {
        switch (level)
        {
#define XX(name)         \
    case LogLevel::name: \
        return #name;    \
        break;
            XX(DEBUG);
            XX(INFO);
            XX(WARN);
            XX(ERROR);
            XX(FATAL);
#undef XX
        default:
            return "UNKNOW";
        }
        return "UNKNOW";
    }

    LogEventWrap::LogEventWrap(LogEvent::ptr e) : m_event(e)
    {
    }
    LogEventWrap::~LogEventWrap()
    {
        if (m_event)
        {
            m_event->getLogger()->log(m_event->getLevel(), m_event);
        }
    }
    std::stringstream &LogEventWrap::getSS()
    {
        return m_event->getSS();
    }

    void LogEvent::format(const char *fmt, ...)
    {
        va_list al;
        va_start(al, fmt);
        format(fmt, al);
        va_end(al);
    }

    void LogEvent::format(const char *fmt, va_list al)
    {
        char *buf = nullptr;
        int len = vasprintf(&buf, fmt, al);
        if (len != -1)
        {
            this->m_ss << std::string(buf, len);
            free(buf);
        }
    }

    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        MessageFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        LevelFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << LogLevel::ToString(level);
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem
    {
    public:
        ElapseFormatItem(const std::string &str) {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getElapse();
        }
    };

    class NameFormatItem : public LogFormatter::FormatItem
    {
    public:
        NameFormatItem(const std::string &str) {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << logger->getName();
        }
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadIdFormatItem(const std::string &str) {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getThreadId();
        }
    };

    class FiberIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        FiberIdFormatItem(const std::string &str) {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getFiberId();
        }
    };

    class DateTimeFormatItem : public LogFormatter::FormatItem
    {
    public:
        DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S")
            : m_format(format)
        {
            if (m_format.empty())
            {
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        }

        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            struct tm tm;
            time_t time = event->getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            os << buf;
        }

    private:
        std::string m_format;
    };

    class FileNameFormatItem : public LogFormatter::FormatItem
    {
    public:
        FileNameFormatItem(const std::string &str) {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem
    {
    public:
        LineFormatItem(const std::string &str) {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem
    {
    public:
        NewLineFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << std::endl;
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem
    {
    public:
        StringFormatItem(const std::string &str) : m_string(str) {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << m_string;
        }

    private:
        std::string m_string;
    };

    class TabFormatItem : public LogFormatter::FormatItem
    {
    public:
        TabFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << "\t";
        }

    private:
        std::string m_string;
    };

    LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int32_t line, uint32_t elapse, uint32_t threadId, uint32_t fiberId, uint64_t time)
        : m_file(file), m_line(line), m_elapse(elapse), m_threadId(threadId), m_fiberId(fiberId), m_time(time), m_logger(logger), m_level(level)
    {
    }

    Logger::Logger(const std::string &name) : m_name(name), m_level(LogLevel::DEBUG)
    {
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    }

    void Logger::addAppender(LogAppender::ptr appender) // 添加appender
    {
        if (!appender->getFormatter())
        {
            appender->setFormatter(m_formatter);
        }
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
            auto self = shared_from_this();
            for (auto &i : m_appenders)
            {
                i->log(self, level, event);
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

    LogFormatter::LogFormatter(const std::string &pattern) : m_pattern(pattern)
    {
        this->init();
    }

    std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        std::stringstream ss;
        for (auto &i : m_items)
        {
            i->format(ss, logger, level, event);
        }
        return ss.str();
    }

    // StdLogAppender重写基类LogAppender的log
    void StdLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            // std::cout << m_formatter->format(logger, level, event); // std::endl;
            const char *color = LOG_COLOR(level);
            std::cout << color << m_formatter->format(logger, level, event) << RESET << std::endl;
        }
    }

    FileLogAppender::FileLogAppender(const std::string &filename) : m_filename(filename)
    {
        this->reopen();
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
    void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            m_filestream << m_formatter->format(logger, level, event);
        }
    }

    void LogFormatter::init()
    {
        // str, format, type
        std::vector<std::tuple<std::string, std::string, int>> vec;
        std::string nstr; // normal_string
        for (size_t i = 0; i < m_pattern.size(); ++i)
        {
            if (m_pattern[i] != '%')
            {
                nstr.append(1, m_pattern[i]);
                continue;
            }
            if ((i + 1) < m_pattern.size())
            {
                if (m_pattern[i + 1] == '%')
                {
                    nstr.append(1, '%');
                    continue;
                }
            }

            size_t n = i + 1;
            int fmt_status = 0;
            size_t fmt_begin = 0;

            std::string str;
            std::string fmt;
            // while (++n < m_pattern.size())
            while (n < m_pattern.size())
            {
                // if (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}')
                if (!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}'))
                {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    break;
                }
                if (fmt_status == 0)
                {
                    if (m_pattern[n] == '{')
                    {
                        // str = m_pattern.substr(i + 1, n - i); // i代表%，是不需要被记录的
                        str = m_pattern.substr(i + 1, n - i - 1);
                        // std::cout << "*" << str << std::endl;
                        fmt_status = 1; // 解析格式
                        fmt_begin = n;
                        ++n;
                        continue;
                    }
                }
                else if (fmt_status == 1)
                {
                    if (m_pattern[n] == '}')
                    { // 说明格式已经结束
                        // fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin);
                        fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                        // std::cout << "#" << fmt << std::endl;
                        fmt_status = 0;
                        ++n;
                        break;
                    }
                }
                ++n;
                if (n == m_pattern.size())
                {
                    if (str.empty())
                    {
                        str = m_pattern.substr(i + 1);
                    }
                }
            }

            if (fmt_status == 0)
            {
                if (!nstr.empty())
                {
                    vec.push_back(std::make_tuple(nstr, std::string(), 0));
                    nstr.clear();
                }
                // str = m_pattern.substr(i + 1, n - i - 1);
                vec.push_back(std::make_tuple(str, fmt, 1));
                // i = n;   //这里应该是 i = n - 1;
                i = n - 1;
            }
            else if (fmt_status == 1)
            {
                std::cout << "pattern parse error:" << m_pattern << "-" << m_pattern.substr(i) << std::endl;
                vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
            }
            // else if (fmt_status == 2)
            // {
            //     if (!nstr.empty())
            //     {
            //         vec.push_back(std::make_tuple(nstr, "", 0));
            //     }
            //     vec.push_back(std::make_tuple(str, fmt, 1));
            //     i = n - 1;
            // }
        }
        if (!nstr.empty())
        {
            vec.push_back(std::make_tuple(nstr, "", 0));
        }

        static std::map<std::string, std::function<FormatItem::ptr(const std::string &str)>> s_format_items = {
#define XX(str, C)                                                               \
    {                                                                            \
        #str, [](const std::string &fmt) { return FormatItem::ptr(new C(fmt)); } \
    }
            XX(m, MessageFormatItem),
            XX(p, LevelFormatItem),
            XX(r, ElapseFormatItem),
            XX(c, NameFormatItem),
            XX(t, ThreadIdFormatItem),
            XX(n, NewLineFormatItem),
            XX(d, DateTimeFormatItem),
            XX(f, FileNameFormatItem),
            XX(l, LineFormatItem),
            XX(F, FiberIdFormatItem),
            XX(T, TabFormatItem),
#undef XX
        };

        for (auto &i : vec)
        {
            if (std::get<2>(i) == 0)
            {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            }
            else
            {
                auto it = s_format_items.find(std::get<0>(i));
                if (it == s_format_items.end())
                {
                    m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                }
                else
                {
                    m_items.push_back(it->second(std::get<1>(i)));
                }
            }

            // std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
        }

        // std::cout << m_items.size() << std::endl;
    }

    LoggerManager::LoggerManager()
    {
        this->m_root.reset(new Logger());
        this->m_root->addAppender(LogAppender::ptr(new StdLogAppender));
    }
    Logger::ptr LoggerManager::getLogger(const std::string &name)
    {
        auto it = m_loggers.find(name);
        return it == m_loggers.end() ? m_root : it->second;
    }
    void LoggerManager::init()
    {
    }

}
