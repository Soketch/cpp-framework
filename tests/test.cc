#include <iostream>
#include "../sylar/log.h"

int main(int argc, char *argv[])
{
    sylar::Logger::ptr logger(new sylar::Logger);
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdLogAppender));

    sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("../txtlog/log.txt"));
    sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%T%p%T%t%T%m%n")); // 时间-级别-线程号-内容-换行
    file_appender->setFormatter(fmt);

    file_appender->setLevel(sylar::LogLevel::ERROR);

    logger->addAppender(file_appender);
    // sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, 0, sylar::GetTheadId(), sylar::GetFiberId(), time(0)));
    // event->getSS() << "hello sylar log";
    // logger->log(sylar::LogLevel::DEBUG, event);

    SYLAR_LOG_INFO(logger) << "test marco!!!";
    SYLAR_LOG_DEBUG(logger);
    SYLAR_LOG_ERROR(logger) << "error";

    std::cout << "test -- log debug1" << std::endl;

    auto l = sylar::LoggerMgr::GetInstance()->getLogger("xx");
    SYLAR_LOG_INFO(l) << " xx";

    std::cout << "test -- log debug2" << std::endl;
    return 0;
}
