#ifndef __SYLAR_UTIL_H__
#define __SYLAR_UTIL_H__
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <cstdint>
#include <vector>
#include <string>

namespace sylar
{
    pid_t GetTheadId(); // 获取当前线程id

    uint32_t GetFiberId(); // 获取当前协程（纤程）id

    // 生成和格式化调用堆栈
    void BackTrace(std::vector<std::string> &bt, int size, int skip = 1);
    std::string BackTraceToString(int size, int skip = 2, const std::string &prefix = "");
}
#endif