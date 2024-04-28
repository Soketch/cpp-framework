#include <iostream>
#include "../sylar/log.h"
#include "../sylar/util.h"
int main(int argc, char *argv[])
{
    sylar::Logger::ptr logger(new sylar::Logger);
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdLogAppender));

    sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, 0, sylar::GetTheadId(), sylar::GetFiberId(), time(0)));
    event->getSS() << "hello sylar log";
    logger->log(sylar::LogLevel::DEBUG, event);

    std::cout << "test -- debug1" << std::endl;

    return 0;
}
