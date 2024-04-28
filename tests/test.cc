#include <iostream>
#include "../sylar/log.h"

int main(int argc, char *argv[])
{
    sylar::Logger::ptr logger(new sylar::Logger);
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdLogAppender));

    // sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, 0, sylar::GetTheadId(), sylar::GetFiberId(), time(0)));
    // event->getSS() << "hello sylar log";
    // logger->log(sylar::LogLevel::DEBUG, event);
    SYLAR_LOG_INFO(logger) << "test marco!!!";

    SYLAR_LOG_DEBUG(logger);

    SYLAR_LOG_ERROR(logger);

    std::cout << "test -- debug1" << std::endl;

    return 0;
}
