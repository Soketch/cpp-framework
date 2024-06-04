#include "util.h"
#include "log.h"
#include "fiber.h"
#include <execinfo.h>

namespace sylar
{
    sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    pid_t GetTheadId()
    {
        return syscall(SYS_gettid);
    }

    uint32_t GetFiberId()
    {
        return sylar::Fiber::GetFiberId();
    }

    void BackTrace(std::vector<std::string> &bt, int size, int skip)
    {
        void **array = (void **)malloc((sizeof(void *) * size));
        size_t s = ::backtrace(array, size);

        char **strings = backtrace_symbols(array, s);
        if (strings == NULL)
        {
            SYLAR_LOG_ERROR(g_logger) << "backtrace_symbols error";
            return;
        }
        for (size_t i = skip; i < s; ++i) // 跳过前 skip 个堆栈帧，将剩余的帧添加到 bt 向量中。
        {
            bt.push_back(strings[i]);
        }

        free(strings);
        free(array);
    }

    std::string BackTraceToString(int size, int skip, const std::string &prefix)
    {
        std::vector<std::string> bt;
        BackTrace(bt, size, skip);
        std::stringstream ss;
        for (size_t i = 0; i < bt.size(); ++i)
        {
            ss << prefix << bt[i] << std::endl;
        }
        return ss.str();
    }

    uint64_t GetCurrentMS() //  ==> 毫秒
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        return tv.tv_sec * 1000ul + tv.tv_usec / 1000; // 注意：这里的ul 指的是无符号长整型（unsigned long）
    }
    uint64_t GetCurrentUS() //  ==> 微秒
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
    }

    //
}