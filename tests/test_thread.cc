#include "sylar/sylar.h"
sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

int g_count = 0;

sylar::RWMutex s_mutex;

void fun1()
{
    SYLAR_LOG_INFO(g_logger) << "thread_name:" << sylar::Thread::GetName()
                             << "  this.name:" << sylar::Thread::GetThis()->getName()
                             << "  thread_id:" << sylar::GetTheadId()
                             << "  this.id:" << sylar::Thread::GetThis()->getId();
    for (int i = 0; i < 1000000; ++i)
    {
        sylar::RWMutex::WriteLock lock(s_mutex);
        ++g_count;
    }
}

void fun2()
{
}

int main()
{
    SYLAR_LOG_INFO(g_logger) << "thread test begin";
    std::vector<sylar::Thread::ptr> thrs;

    for (int i = 0; i < 5; ++i)
    {
        sylar::Thread::ptr thr(new sylar::Thread(&fun1, "name_" + std::to_string(i)));
        thrs.push_back(thr);
    }

    for (int i = 0; i < 5; ++i)
    {
        thrs[i]->join();
    }

    SYLAR_LOG_INFO(g_logger) << "thread test end";

    SYLAR_LOG_INFO(g_logger) << "g_count=" << g_count;
    return 0;
}