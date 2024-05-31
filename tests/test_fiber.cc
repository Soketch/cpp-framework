#include "sylar/sylar.h"
#include <string.h>
sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run_in_fiber()
{
    SYLAR_LOG_DEBUG(g_logger) << "====  run_in_fiber begin  ====";
    // sylar::Fiber::GetThis()->swapOut();  //这里不应该直接切换 //应该 先暂停 再唤醒 然后切换
    sylar::Fiber::YieldToHold();
    SYLAR_LOG_DEBUG(g_logger) << "==== run_in_fiber end ====";
    sylar::Fiber::YieldToHold();
}

void test_fiber()
{
    SYLAR_LOG_INFO(g_logger) << "main begin.";
    {
        // 创建主协程
        auto main_fiber = sylar::Fiber::GetThis();
        // 创建子协程
        sylar::Fiber::ptr fiber(new sylar::Fiber(run_in_fiber));
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "main after swapIn ==1==.";
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "main after swapIn ==2==.";
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "main after swapIn ==3==.";
        SYLAR_LOG_INFO(g_logger) << "main end.";
    }

    // 检查主协程在程序结束前是否正确析构
    SYLAR_LOG_INFO(g_logger) << "Total fibers: " << sylar::Fiber::TotalFibers();
}

int main(int argc, char **argv)
{
    sylar::Thread::setName("main_thread"); // 线程命名

    std::vector<sylar::Thread::ptr> thrs;
    for (int i = 0; i < 3; ++i)
    {
        thrs.push_back(sylar::Thread::ptr(
            new sylar::Thread(&test_fiber, "name_" + std::to_string(i))));
    }
    for (auto &i : thrs)
    {
        i->join();
    }
    return 0;
}