#include "util.h"

namespace sylar
{
    pid_t GetTheadId()
    {
        return syscall(SYS_gettid);
    }

    uint32_t GetFiberId()
    {
        return 0;
    }
}