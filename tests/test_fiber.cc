#include "sylar/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run_in_fiber()
{
    SYLAR_LOG_DEBUG(g_logger) << "====  run_in_fiber begin  ====";
    // sylar::Fiber::GetThis()->swapOut();  //这里不应该直接切换 //应该 先暂停 再唤醒 然后切换
    sylar::Fiber::GetThis()->YieldToHold();
    SYLAR_LOG_DEBUG(g_logger) << "==== run_in_fiber end ====";
    sylar::Fiber::GetThis()->YieldToHold();
}

int main(int argc, char **argv)
{
    SYLAR_LOG_INFO(g_logger) << "main begin.";

    // 创建主协程
    sylar::Fiber::GetThis();

    // 创建子协程
    sylar::Fiber::ptr fiber(new sylar::Fiber(run_in_fiber));
    fiber->swapIn();
    SYLAR_LOG_INFO(g_logger) << "main after swapIn.";
    fiber->swapIn();
    SYLAR_LOG_INFO(g_logger) << "main end.";
    return 0;
}