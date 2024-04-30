#ifndef __SYLAR_UTIL_H__
#define __SYLAR_UTIL_H__
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <cstdint>
namespace sylar
{
    pid_t GetTheadId();

    uint32_t GetFiberId();
}
#endif