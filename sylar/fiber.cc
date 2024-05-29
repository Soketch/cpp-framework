#include "fiber.h"
#include "config.h"
#include <atomic>

namespace sylar
{
    // 当前协程id
    static std::atomic<uint64_t> s_fiber_id{0};
    // 协程全局计数
    static std::atomic<uint64_t> s_fiber_count{0};

    // 线程局部变量（当前协程）
    static thread_local Fiber *t_fiber = nullptr;
    // 线程局部变量（主协程智能指针）
    static thread_local std::shared_ptr<Fiber::ptr> t_threadFiber = nullptr; // main协程

    // 内存分配器
    class MallocStackAllocator
    {
    public:
        static void *Alloc(size_t size)
        {
            return malloc(size);
        }
        static void Dealloc(void *vp, size_t size)
        {
            return free(vp);
        }
    };

    Fiber::Fiber(std::function<void()> cb, size_t stacksize = 0)
    {
    }
    Fiber::~Fiber()
    {
    }

    // 重置协程函数,并重置状态
    void Fiber::reset(std::function<void()> cb)
    {
    }

    // 切换到当前协程执行
    void Fiber::swapIn()
    {
    }
    // 切换到后台执行
    void Fiber::swapOut()
    {
    }

    // 返回当前协程
    Fiber::ptr Fiber::GetThis()
    {
    }
    // 协程切换到后台，并设置ready状态  --准备就绪
    void Fiber::YieldToReady()
    {
    }
    // 协程切换到后台，并设置hold状态  --暂停
    void Fiber::YieldToHold()
    {
    }
    // 获取总协程数
    uint64_t Fiber::TotalFibers()
    {
    }

    void Fiber::MainFunc()
    {
    }

}