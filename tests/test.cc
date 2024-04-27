#include <iostream>
#include "../sylar/log.h"

int main(int argc, char *argv[])
{
    sylar::Logger::ptr logger(new sylar::Logger);
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdLogAppender));

    sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, 0, 1, 2, time(0)));
    logger->log(sylar::LogLevel::DEBUG, event);

    std::cout << "test -- debug1" << std::endl;
    return 0;
}
