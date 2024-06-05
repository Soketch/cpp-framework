#include "hook.h"
#include <dlfcn.h>
#include "config.h"
#include "log.h"
#include "fiber.h"
#include "iomanager.h"
#include "macro.h"

namespace sylar
{
    static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX) \
    XX(sleep)        \
    XX(usleep)       \
    XX(nanosleep)

    void hook_init()
    {
        static bool is_inited = false;
        if (is_inited)
        {
            return;
        }
#define XX(name) name##_f = (name##_fun)dlsym(RTLD_NEXT, #name);
        HOOK_FUN(XX)
#undef XX
    }

    // 引入 ==> 怎么在main函数前执行一个方法？？？？
    struct _HookIniter
    {
        _HookIniter()
        {
            hook_init();
        }
    };
    // 利用静态对象的构造函数在 main 函数之前运行机制，
    //  ====>   静态对象在程序启动时初始化，而它们的构造函数会在 main 函数之前执行
    //  ====>   在构造函数中执行hook初始化操作
    static _HookIniter s_hook_initer;

    bool is_hook_enable()
    {
        return t_hook_enable;
    }

    void set_hook_enable(bool flag)
    {
        t_hook_enable = flag;
    }

}

extern "C"
{
#define XX(name) name##_fun name##_f = nullptr;
    HOOK_FUN(XX);
#undef XX

    unsigned int sleep(unsigned int seconds)
    {
        if (!sylar::t_hook_enable)
        {
            return sleep_f(seconds);
        }

        sylar::Fiber::ptr fiber = sylar::Fiber::GetThis();
        sylar::IOManager *iom = sylar::IOManager::GetThis();

        // iom->addTimer(seconds * 1000, std::bind(&sylar::IOManager::schedule, iom, fiber, -1)); // 参数类型错误

        // iom->addTimer(seconds * 1000, [iom, fiber]()
        //               { iom->schedule(fiber); });
        iom->addTimer(seconds * 1000,
                      std::bind(
                          (void(sylar::Scheduler::*)(sylar::Fiber::ptr, int thread)) & sylar::IOManager::schedule,
                          iom, fiber, -1));

        sylar::Fiber::YieldToHold();
        return 0;
    }

    int usleep(useconds_t usec)
    {
        if (!sylar::t_hook_enable)
        {
            return usleep_f(usec);
        }
        sylar::Fiber::ptr fiber = sylar::Fiber::GetThis();
        sylar::IOManager *iom = sylar::IOManager::GetThis();
        iom->addTimer(usec / 1000, std::bind((void(sylar::Scheduler::*)(sylar::Fiber::ptr, int thread)) & sylar::IOManager::schedule, iom, fiber, -1));
        sylar::Fiber::YieldToHold();
        return 0;
    }

    int nanosleep(const struct timespec *req, struct timespec *rem)
    {
        if (!sylar::t_hook_enable)
        {
            return nanosleep_f(req, rem);
        }
        int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 / 1000;

        sylar::Fiber::ptr fiber = sylar::Fiber::GetThis();
        sylar::IOManager *iom = sylar::IOManager::GetThis();
        iom->addTimer(timeout_ms, std::bind((void(sylar::Scheduler::*)(sylar::Fiber::ptr, int thread)) & sylar::IOManager::schedule, iom, fiber, -1));
        sylar::Fiber::YieldToHold();
        return 0;
    }
}
