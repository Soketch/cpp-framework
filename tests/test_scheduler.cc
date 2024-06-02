#include "sylar/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_fiber()
{
    static int s_count = 5;
    SYLAR_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    sleep(1);
    if (--s_count >= 0)
    {
        sylar::Scheduler::GetThis()->schedule(&test_fiber, sylar::GetTheadId());
    }
}

int main(int argc, char **argv)
{
    SYLAR_LOG_INFO(g_logger) << "main begin";
    // sylar::Scheduler sc;
    sylar::Scheduler sc(3, true, "test");
    sc.Start();
    sleep(2);
    SYLAR_LOG_INFO(g_logger) << "schedule";
    sc.schedule(&test_fiber);
    sc.Stop();
    SYLAR_LOG_INFO(g_logger) << "main over";
    return 0;
}
