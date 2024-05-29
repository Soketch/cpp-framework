#ifndef __SYLAR_FIBER_H_
#define __SYLAR_FIBER_H_

#include <ucontext.h>
#include <memory>
#include <functional>
#include "thread.h"
namespace sylar
{

    // 协程类
    class Fiber : public std::enable_shared_from_this<Fiber>
    {
    public:
        typedef std::shared_ptr<Fiber> ptr;

        enum State // 协程状态
        {
            INIT,  // 初始状态 init
            HOLD,  // 暂停状态 hold
            EXEC,  // 正在运行状态 exec
            TERM,  // 终止状态 term
            READY, // 准备就绪状态 ready
            EXCEPT // 异常状态
        };

    private:
        Fiber(); // 私有默认构造函数 -- 防止外部直接创建Fiber对象

    public:
        Fiber(std::function<void()> cb, size_t stacksize = 0);
        ~Fiber();

        // 重置协程函数,并重置状态
        void reset(std::function<void()> cb);

        // 切换到当前协程执行
        void swapIn();
        // 切换到后台执行
        void swapOut();

    public:
        // 返回当前协程
        static Fiber::ptr GetThis();
        // 协程切换到后台，并设置ready状态  --准备就绪
        static void YieldToReady();
        // 协程切换到后台，并设置hold状态  --暂停
        static void YieldToHold();
        // 获取总协程数
        static uint64_t TotalFibers();

        static void MainFunc();

    private:
        uint64_t m_id = 0;        // 协程id
        uint64_t m_stacksize = 0; // 栈大小
        State m_state = INIT;     // 状态
        ucontext_t m_ctx;         // 上下文变量
        void *m_stack = nullptr;  // 栈内存空间对象

        std::function<void()> m_cb; // 回调函数
    };
}

#endif
