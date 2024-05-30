#include "fiber.h"
#include "config.h"
#include "macro.h"
#include "log.h"
#include <atomic>

// factory工厂模式   ？？？

namespace sylar
{
    sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    // 当前协程id
    static std::atomic<uint64_t> s_fiber_id{0};
    // 协程全局计数
    static std::atomic<uint64_t> s_fiber_count{0};

    // 线程局部变量（当前协程）
    static thread_local Fiber *t_fiber = nullptr;
    // 线程局部变量（主协程智能指针）
    static thread_local Fiber::ptr t_threadFiber = nullptr; // main协程

    // 配合配置系统指定协程的 栈大小 -- -- 通过config的Lookup方法查找yaml配置文件fiber的stack_size项指定栈大小
    static ConfigVar<uint32_t>::ptr g_fiber_stack_size =
        Config::Lookup<uint32_t>("fiber.stack_size", 1024 * 1024, "stack size for fiber.");

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

    using StackAllocator = MallocStackAllocator;

    // 将当前线程的上下文赋值给 协程类  >> 主协程
    Fiber::Fiber()
    {
        this->m_state = State::EXEC;
        this->setThis(this);
        if (getcontext(&m_ctx))
        {
            SYLAR_ASSERT2(false, "getcontext");
        }
        ++s_fiber_count; // 总协程数+1
    }

    // 真正的创建一个协程   >>  子协程 - 分配栈空间，每个协程都有独立栈运行空间
    Fiber::Fiber(std::function<void()> cb, size_t stacksize = 0) : m_id(++s_fiber_id),
                                                                   m_cb(cb)
    {
        ++s_fiber_count;                                                      // 协程总数+1
        m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue(); // 设置栈空间大小

        m_stack = StackAllocator::Alloc(m_stacksize); // 分配栈空间
        if (getcontext(&m_ctx))
        {
            SYLAR_ASSERT2(false, "Fiber getcontext");
        }
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;

        makecontext(&m_ctx, &Fiber::MainFunc, 0);
    }
    Fiber::~Fiber()
    {
        --s_fiber_count;
        if (m_stack)
        {
            SYLAR_ASSERT2(m_state == State::TERM ||
                              m_state == State::INIT ||
                              m_state == State::EXCEPT,
                          "~Fiber distory");

            StackAllocator::Dealloc(m_stack, m_stacksize); // 回收栈空间
        }
        else // 主协程不会有m_stack
        {
            // 判断是否是主协程
            SYLAR_ASSERT(!m_cb);
            SYLAR_ASSERT(m_state == State::EXEC);

            Fiber *cur = t_fiber;
            if (cur == this)
            {
                setThis(nullptr);
            }
        }
    }

    // 重置协程函数,并重置状态
    void Fiber::reset(std::function<void()> cb)
    {
        SYLAR_ASSERT(m_stack);
        SYLAR_ASSERT(m_state == State::TERM ||
                     m_state == State::EXCEPT ||
                     m_state == State::INIT); // 判断是否处于结束、异常或者初始状态

        m_cb = cb;

        // 重新初始化ucontext对象
        if (getcontext(&m_ctx))
        {
            SYLAR_ASSERT2(false, "fiber reset getcontext");
        }
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;

        makecontext(&m_ctx, &Fiber::MainFunc, 0);
        m_state = State::INIT;
    }

    // 切换到当前协程执行 -- (一般是由主协程切换到子协程)  //唤醒子协程
    void Fiber::swapIn()
    {
        setThis(this);
        SYLAR_ASSERT(m_state != State::EXEC); // 当前协程不能处于运行状态

        m_state = State::EXEC;
        // 将当前上下文保存到 t_threadFiber->m_ctx，并切换到当前协程的上下文 m_ctx
        if (swapcontext(&t_threadFiber->m_ctx, &m_ctx))
        {
            SYLAR_ASSERT2(false, "swapIn context");
        }
    }
    // 切换到后台执行 -- (一般是由子协程切换到主协程)  //唤醒main协程
    void Fiber::swapOut()
    {
        setThis(t_threadFiber.get()); // 返回对象的原始指针

        // 将当前上下文保存到 m_ctx，并切换到主协程的上下文 t_threadFiber->m_ctx
        if (swapcontext(&m_ctx, &t_threadFiber->m_ctx))
        {
            SYLAR_ASSERT2(false, "swapOut context");
        }
    }

    // 设置当前协程
    void Fiber::setThis(Fiber *f)
    {
        t_fiber = f;
    }

    // 返回当前协程
    Fiber::ptr Fiber::GetThis()
    {
        if (t_fiber)
        {
            return t_fiber->shared_from_this();
        }

        // 如果没有协程，就创建主协程
        Fiber::ptr main_fiber(new Fiber());
        SYLAR_ASSERT2(t_fiber == main_fiber.get(), "fiber getthis");

        t_threadFiber = main_fiber;
        return t_fiber->shared_from_this();
    }
    // 协程切换到后台，并设置ready状态  --准备就绪
    void Fiber::YieldToReady()
    {
        Fiber::ptr cur = GetThis();
        cur->m_state = State::READY;
        cur->swapOut();
    }

    // 协程切换到后台，并设置hold状态  --暂停
    void Fiber::YieldToHold()
    {
        Fiber::ptr cur = GetThis();
        cur->m_state = State::HOLD;
        cur->swapOut();
    }
    // 获取总协程数
    uint64_t Fiber::TotalFibers()
    {
        return s_fiber_count.load(); // load是std::atomic的一个原子加载操作，安全地获取当前原子变量的值
    }

    // 协程执行主函数
    void Fiber::MainFunc()
    {
        Fiber::ptr cur = GetThis();
        SYLAR_ASSERT(cur);
        try
        {
            cur->m_cb(); // 执行协程回调函数
            cur->m_cb = nullptr;
            cur->m_state = State::TERM; // 执行完成设置结束状态
        }
        catch (const std::exception &e)
        {
            cur->m_state = State::EXCEPT; // 设置异常状态
            SYLAR_LOG_ERROR(g_logger) << "Fiber Except: " << e.what() << '\n';
        }
        catch (...)
        {
            cur->m_state = EXCEPT;
            SYLAR_LOG_ERROR(g_logger) << "Fiber Except ...";
        }

        // 切回主协程
    }
}