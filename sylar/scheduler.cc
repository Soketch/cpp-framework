#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "hook.h"
namespace sylar
{
    static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    // 线程局部变量（协程调度器指针）
    static thread_local Scheduler *t_scheduler = nullptr;
    // 线程局部变量（当前协程）
    static thread_local Fiber *t_fiber = nullptr;

    Scheduler::Scheduler(size_t threads, bool use_caller, const std::string &name)
        : m_name(name)
    {
        SYLAR_ASSERT(threads > 0);

        if (use_caller)
        {                            // use_caller为true, 是主线程
            sylar::Fiber::GetThis(); // 初始化一个主协程
            --threads;               // 用主线程，没必要再重建一个线程，线程数减一
            SYLAR_ASSERT(GetThis() == nullptr);
            t_scheduler = this;
            m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
            sylar::Thread::setName(m_name);

            t_fiber = m_rootFiber.get();
            m_rootThread = sylar::GetTheadId();
            m_threadIds.push_back(m_rootThread);
        }
        else
        {
            m_rootThread = -1; // 没有线程id(无效线程id)
        }

        m_threadCount = threads;
    }
    Scheduler::~Scheduler()
    {
        SYLAR_ASSERT(m_stopping); // 判断是否执行 true停止、false启动
        if (GetThis() == this)
        {
            t_scheduler = nullptr;
        }
    }

    // 设置当前的协程调度器
    void Scheduler::setThis()
    {
        t_scheduler = this;
    }

    // 获取当前调度器
    Scheduler *Scheduler::GetThis()
    {
        return t_scheduler;
    }

    // 获取主协程
    Fiber *Scheduler::GetMainFiber()
    {
        return t_fiber;
    }

    // 启动调度器
    void Scheduler::Start()
    {
        MutexType::Lock lock(m_mutex);

        // 启动线程
        if (!m_stopping) // m_stopping=false直接返回，m_stopping=true就重置状态为false,再执行后续操作
        {
            return;
        }
        m_stopping = false;
        SYLAR_ASSERT(m_threads.empty());

        m_threads.resize(m_threadCount);
        for (size_t i = 0; i < m_threadCount; ++i)
        {
            m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this),
                                          m_name + "_" + std::to_string(i))); // callback, thread_name
            m_threadIds.push_back(m_threads[i]->getId());
        }
        lock.unlock(); // 这里必须解锁， 进入call会重新锁产生死锁
        // if (m_rootFiber)
        // {
        //     // m_rootFiber->swapIn();
        //     m_rootFiber->call();
        //     SYLAR_LOG_INFO(g_logger) << "root_fiber call out" << m_rootFiber->getState();
        // }
    }
    // 停止调度器   --
    void Scheduler::Stop()
    {
        // 1.等待所有任务完成才退出而不是直接而退出
        // 2.分两种情况，使用了use_caller和没有使用use_caller

        m_autoStop = true;
        if (m_rootFiber &&
            m_threadCount == 0 &&
            (m_rootFiber->getState() == Fiber::TERM || m_rootFiber->getState() == Fiber::INIT))
        {
            SYLAR_LOG_INFO(g_logger) << this << " stopped.";
            m_stopping = true;

            if (stopping())
            {
                return;
            }
        }

        // bool exit_on_this_fiber = false; // 在当前线程退出
        if (m_rootThread != -1)
        { // use_caller
            SYLAR_ASSERT(GetThis() == this);
        }
        else
        {
            SYLAR_ASSERT(GetThis() != this);
        }

        m_stopping = true;
        for (size_t i = 0; i < m_threadCount; ++i)
        {
            tickle(); // 唤醒线程自行结束
        }

        if (m_rootFiber)
        {
            tickle();
        }
        if (m_rootFiber)
        {
            // while (!stopping())
            // {
            //     if (m_rootFiber->getState() == Fiber::EXCEPT || m_rootFiber->getState() == Fiber::TERM)
            //     {
            //         m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
            //         SYLAR_LOG_INFO(g_logger) << "root fiber is term, reset";
            //         t_fiber = m_rootFiber.get();
            //     }
            //     m_rootFiber->call();
            // }
            if (!stopping())
            {
                m_rootFiber->call();
            }
        }
        // if(exit_on_this_fiber){
        // }
        std::vector<Thread::ptr> thrs;
        {
            MutexType::Lock lock(m_mutex);
            thrs.swap(m_threads);
        }

        for (auto &i : thrs)
        {
            i->join();
        }
    }

    // 协程调度运行函数
    void Scheduler::run()
    {
        SYLAR_LOG_DEBUG(g_logger) << m_name << " run";
        set_hook_enable(true);
        setThis();
        if (sylar::GetTheadId() != m_rootThread) // 当前线程id != 主线程id
        {
            t_fiber = Fiber::GetThis().get();
        }

        Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
        Fiber::ptr cb_fiber; // 回调函数的协程

        FiberAndThread ft;
        while (true)
        {
            ft.reset();
            bool tickle_me = false;
            bool is_active = false;
            {
                MutexType::Lock locl(m_mutex);
                auto it = m_fibers.begin();
                while (it != m_fibers.end())
                {
                    if (it->thread != -1 && it->thread != sylar::GetTheadId())
                    {
                        ++it;
                        tickle_me = true;
                        continue;
                    }
                    SYLAR_ASSERT(it->fiber || it->cb);
                    if (it->fiber &&
                        it->fiber->getState() == Fiber::EXEC)
                    {
                        ++it;
                        continue;
                    }

                    ft = *it;
                    m_fibers.erase(it++);
                    ++m_activeThreadCount;
                    is_active = true;
                    break;
                }
                tickle_me |= it != m_fibers.end();
            }
            if (tickle_me)
            {
                tickle();
            }

            if (ft.fiber &&
                (ft.fiber->getState() != Fiber::TERM || ft.fiber->getState() != Fiber::EXCEPT))
            {
                ft.fiber->swapIn();
                --m_activeThreadCount;

                if (ft.fiber->getState() == Fiber::READY)
                {
                    schedule(ft.fiber);
                }
                else if (ft.fiber->getState() != Fiber::TERM &&
                         ft.fiber->getState() != Fiber::EXCEPT)
                {
                    ft.fiber->m_state = Fiber::HOLD;
                }
                ft.reset();
            }
            else if (ft.cb)
            {
                if (cb_fiber)
                {
                    cb_fiber->reset(ft.cb);
                }
                else
                {
                    cb_fiber.reset(new Fiber(ft.cb));
                }
                ft.reset();
                cb_fiber->swapIn();
                --m_activeThreadCount;
                if (cb_fiber->getState() == Fiber::READY)
                {
                    schedule(cb_fiber);
                    cb_fiber.reset();
                }
                else if (cb_fiber->getState() == Fiber::EXCEPT ||
                         cb_fiber->getState() == Fiber::TERM)
                {
                    cb_fiber->reset(nullptr);
                }
                else // if(cb_fiber->getState() != Fiber::TERM){
                {
                    cb_fiber->m_state = Fiber::HOLD;
                    cb_fiber.reset();
                }
            }
            else
            {
                if (is_active)
                {
                    --m_activeThreadCount;
                    continue;
                }
                if (idle_fiber->getState() == Fiber::TERM)
                {
                    SYLAR_LOG_INFO(g_logger) << "idle fiber term";
                    break;
                }
                ++m_idleThreadCount;
                idle_fiber->swapIn();
                --m_idleThreadCount;
                if (idle_fiber->getState() != Fiber::EXCEPT &&
                    idle_fiber->getState() != Fiber::TERM)
                {
                    idle_fiber->m_state = Fiber::HOLD;
                }
            }
        }
    }

    void Scheduler::tickle()
    {
        SYLAR_LOG_INFO(g_logger) << "tickle";
    }

    bool Scheduler::stopping()
    {
        MutexType::Lock lock(m_mutex);
        return m_autoStop &&
               m_stopping &&
               m_fibers.empty() &&
               m_activeThreadCount == 0;
    }

    void Scheduler::idle()
    {
        SYLAR_LOG_INFO(g_logger) << "idle";
        while (!stopping())
        {
            sylar::Fiber::YieldToHold();
        }
    }
}