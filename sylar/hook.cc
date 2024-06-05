#include "hook.h"
#include <dlfcn.h>

#include "config.h"
#include "log.h"
#include "fiber.h"
#include "iomanager.h"
#include "macro.h"
#include "fd_manager.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");
namespace sylar
{
    static sylar::ConfigVar<int>::ptr g_tcp_connect_timeout =
        sylar::Config::Lookup("tcp.connect.timeout", 5000, "tcp connect timeout");

    static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX) \
    XX(sleep)        \
    XX(usleep)       \
    XX(nanosleep)    \
    XX(socket)       \
    XX(connect)      \
    XX(accept)       \
    XX(read)         \
    XX(readv)        \
    XX(recv)         \
    XX(recvfrom)     \
    XX(recvmsg)      \
    XX(write)        \
    XX(writev)       \
    XX(send)         \
    XX(sendto)       \
    XX(sendmsg)      \
    XX(close)        \
    XX(fcntl)        \
    XX(ioctl)        \
    XX(getsockopt)   \
    XX(setsockopt)

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

    // connect 超时时间
    static uint64_t s_connect_timeout = -1;

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

// 条件 超时条件结构体  //条件定时器使用
struct timer_info
{
    int cancelled = 0;
};

// 对 I/O 操作进行 Hook，通过协程调度和超时管理实现异步 I/O 操作
template <typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char *hook_fun_name,
                     uint32_t event, int timeout_so, Args &&...args)
{
    // 是否启用HOOK
    if (!sylar::t_hook_enable)
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    // 获取fd上下文：
    sylar::FdCtx::ptr ctx = sylar::FdMgr::GetInstance()->get(fd);
    if (!ctx)
    {
        return fun(fd, std::forward<Args>(args)...); // 不是fd，直接调用原io函数
    }

    if (ctx->isClose()) // 检测状态
    {
        errno = EBADF;
        return -1;
    }

    if (!ctx->isSocket() || ctx->getUserNonblock()) // 是否是套接字，或者是否人为设置为非阻塞。不满足，直接调用原始的 I/O 函数。
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    uint64_t to = ctx->getTimeout(timeout_so); // 获取超时时间，并创建一个timer_info对象 tinfo
    std::shared_ptr<timer_info> tinfo(new timer_info);

    //  ----------  进行 I/O 操作并处理重试逻辑
retry:
    ssize_t n = fun(fd, std::forward<Args>(args)...); // 先直接执行一遍，返回值有效 直接return n返回

    while (n == -1 && errno == EINTR) // 执行一次后返回值无效，如果返回 EINTR 错误，继续重试操作。
    {
        n = fun(fd, std::forward<Args>(args)...);
    }
    if (n == -1 && errno == EAGAIN) // 如果返回 EAGAIN 错误，表示资源暂时不可用，需要等待。
    {
        // 设置IOManager定时器和事件处理
        sylar::IOManager *iom = sylar::IOManager::GetThis();
        sylar::Timer::ptr timer;
        std::weak_ptr<timer_info> winfo(tinfo);

        if (to != (uint64_t)-1) // 判断超时时间   timeout != -1
        {
            // 添加条件的超时的定时器
            timer = iom->addConditionTimer(to, [winfo, fd, iom, event]()
                                           {
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, (sylar::IOManager::Event)(event)); }, winfo); // 取消，唤醒回来
        }

        int rt = iom->addEvent(fd, (sylar::IOManager::Event)(event)); // 这里没有传递cb参数，是以当前写出作为唤醒对象
        // if (SYLAR_UNLIKELY(rt))
        if (rt)
        { // 添加失败打印日志
            SYLAR_LOG_ERROR(g_logger) << hook_fun_name << " addEvent("
                                      << fd << ", " << event << ")";
            if (timer)
            {
                timer->cancel();
            }
            return -1;
        }
        else // 将当前协程挂起等待事件完成。定时器存在取消，cancel
        {
            sylar::Fiber::YieldToHold(); // 等待唤醒 --> 两种情况唤醒（1读取到事件唤醒）（2超时唤醒）
            if (timer)                   // 检查是否有定时器，有就取消掉
            {
                timer->cancel();
            }
            if (tinfo->cancelled) // 如果定时条件状态存在，就设置errno,直接返回
            {
                errno = tinfo->cancelled;
                return -1;
            }
            goto retry; // 有IO事件回来 --> 重新读 --> goto
        }
    }

    return n;
}

extern "C"
{
#define XX(name) name##_fun name##_f = nullptr;
    HOOK_FUN(XX);
#undef XX

    // sleep相关函数的hook
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

    int socket(int domain, int type, int protocol)
    {
        if (!sylar::t_hook_enable)
        {
            return socket_f(domain, type, protocol);
        }
        int fd = socket_f(domain, type, protocol);
        if (fd == -1)
        {
            return fd;
        }
        sylar::FdMgr::GetInstance()->get(fd, true);
        return fd;
    }

    int connect_with_timeout(int fd, const struct sockaddr *addr, socklen_t addrlen, uint64_t timeout_ms)
    {
        if (!sylar::t_hook_enable)
        {
            return connect_f(fd, addr, addrlen);
        }
        sylar::FdCtx::ptr ctx = sylar::FdMgr::GetInstance()->get(fd);
        if (!ctx || ctx->isClose())
        {
            errno = EBADF;
            return -1;
        }

        if (!ctx->isSocket())
        {
            return connect_f(fd, addr, addrlen);
        }

        if (ctx->getUserNonblock())
        {
            return connect_f(fd, addr, addrlen);
        }

        int n = connect_f(fd, addr, addrlen);
        if (n == 0)
        {
            return 0;
        }
        else if (n != -1 || errno != EINPROGRESS)
        {
            return n;
        }

        sylar::IOManager *iom = sylar::IOManager::GetThis();
        sylar::Timer::ptr timer;
        std::shared_ptr<timer_info> tinfo(new timer_info);
        std::weak_ptr<timer_info> winfo(tinfo);

        if (timeout_ms != (uint64_t)-1)
        {
            timer = iom->addConditionTimer(timeout_ms, [winfo, fd, iom]()
                                           {
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, sylar::IOManager::WRITE); }, winfo);
        }

        int rt = iom->addEvent(fd, sylar::IOManager::WRITE);
        if (rt == 0)
        {
            sylar::Fiber::YieldToHold();
            if (timer)
            {
                timer->cancel();
            }
            if (tinfo->cancelled)
            {
                errno = tinfo->cancelled;
                return -1;
            }
        }
        else
        {
            if (timer)
            {
                timer->cancel();
            }
            SYLAR_LOG_ERROR(g_logger) << "connect addEvent(" << fd << ", WRITE) error";
        }

        int error = 0;
        socklen_t len = sizeof(int);
        if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len))
        {
            return -1;
        }
        if (!error)
        {
            return 0;
        }
        else
        {
            errno = error;
            return -1;
        }
    }

    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    {
        return connect_with_timeout(sockfd, addr, addrlen, sylar::s_connect_timeout);
    }

    int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
    {
        int fd = do_io(s, accept_f, "accept", sylar::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
        if (fd >= 0)
        {
            sylar::FdMgr::GetInstance()->get(fd, true);
        }
        return fd;
    }

    //
}
