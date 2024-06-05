#include "sylar/hook.h"
#include "sylar/log.h"
#include "sylar/iomanager.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_sleep()
{
    sylar::IOManager iom(1);
    iom.schedule([]()
                 { sleep(2);
                 SYLAR_LOG_INFO(g_logger) << "sylar sleep 2 seconds."; });
    iom.schedule([]()
                 { sleep(3);
                 SYLAR_LOG_INFO(g_logger) << "once aftre sleep 3 seconds."; });
    SYLAR_LOG_INFO(g_logger) << "test sleep.";
}

int main(int argc, char **argv)
{
    test_sleep();
    return 0;
}