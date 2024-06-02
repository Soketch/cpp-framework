#ifndef __SYLAR_SCHEDULER_H_
#define __SYLAR_SCHEDULER_H_

// 协程调度器 scheduler
#include <memory>
#include "fiber.h"
#include "mutex.h"
#include "thread.h"
#include <vector>
#include <list>

// 有栈非对称协程 ？？？

namespace sylar
{
    class Scheduler
    {
    public:
        typedef std::shared_ptr<Scheduler> ptr;
        typedef Mutex MutexType;

        /* threads 调度器线程数
           use_caller  是否纳入调度器，表示是否使用调用者线程
           name  调度器名称
        */
        Scheduler(size_t threads = 1, bool use_caller = true, const std::string &name = "");
        virtual ~Scheduler();

        std::string getName() const { return m_name; }

        // 获取当前调度器
        static Scheduler *GetThis();
        // 获取主协程
        static Fiber *GetMainFiber();

        // 启动调度器
        void Start();
        // 停止调度器
        void Stop();

        // 模板函数 - 带锁的schedule调度函数, 这里thread = -1 => 表示任意线程
        // 单个地放入m_fibers
        template <class FiberOrCb>
        void schedule(FiberOrCb fc, int thread = -1)
        {
            bool need_tickle = false; // 是否需要通知
            {
                MutexType::Lock lock(m_mutex); // 加锁
                need_tickle = scheduleNoLock(fc, thread);
            }
            if (need_tickle)
            {
                tickle();
            }
        }

        // 批量地放入m_fibers  : 锁一次可以把想要放的都放到m_fibers中
        template <class InputIterator>
        void schedule(InputIterator begin, InputIterator end)
        {
            bool need_tickle = false;
            {
                MutexType::Lock lock(m_mutex);
                while (begin != end)
                {
                    need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;
                    ++begin;
                }
            }
            if (need_tickle)
            {
                tickle();
            }
        }

    protected:
        // 通知协程调度器有任务了
        virtual void tickle();

        // 协程调度运行函数
        void run();

        // 返回是否可以停止
        virtual bool stopping();

        // 协程无任务可调度时执行idle协程  // idle空闲线程
        virtual void idle();

        // 设置当前的协程调度器
        void setThis();

        // 是否有空闲线程
        bool hasIdleThreads() { return m_idleThreadCount > 0; }

    private:
        // 无锁schedule调度函数
        template <class FiberOrCb>
        bool scheduleNoLock(FiberOrCb fc, int thread)
        {
            bool need_tickle = m_fibers.empty(); // 是否通知
            FiberAndThread ft(fc, thread);
            if (ft.fiber || ft.cb)
            {
                m_fibers.push_back(ft);
            }
            return need_tickle;
        }

    private:
        // 内部结构体，用于关联协程、线程和function方法
        struct FiberAndThread
        {
            Fiber::ptr fiber;
            std::function<void()> cb;
            int thread;
            FiberAndThread(Fiber::ptr f, int thr) : fiber(f), thread(thr) {}

            FiberAndThread(Fiber::ptr *f, int thr) : thread(thr)
            {
                fiber.swap(*f);
            }

            FiberAndThread(std::function<void()> f, int thr) : cb(f), thread(thr) {}

            FiberAndThread(std::function<void()> *f, int thr) : thread(thr)
            {
                cb.swap(*f);
            }

            // 采用默认构造函数如果无参可能无法初始化，这里用初始化列表: thread(-1)
            // 主要是做一个无效线程 ID 标记，不执行一些任务
            FiberAndThread() : thread(-1) {}

            void reset()
            {
                fiber = nullptr;
                cb = nullptr;
                thread = -1;
            }
        };

    private:
        MutexType m_mutex;                  // 互斥量
        std::vector<Thread::ptr> m_threads; // 线程池
        std::list<FiberAndThread> m_fibers; // 协程组 - 存放将要执行或者计划、准备要执行的协程
        /// use_caller为true时有效, 调度协程
        Fiber::ptr m_rootFiber; // 主协程
        std::string m_name;     // 调度器名称
    protected:
        std::vector<int> m_threadIds;               // 存放线程id的数组
        size_t m_threadCount = 0;                   // 线程数
        std::atomic<size_t> m_activeThreadCount{0}; // 活跃线程数量
        std::atomic<size_t> m_idleThreadCount{0};   // 空闲线程数量
        bool m_stopping = true;                     // 执行状态    m_stopping=false说明启动了
        bool m_autoStop = false;                    // 是否自动停止
        // 主线程id(use_caller)
        int m_rootThread = 0;
    };
}
#endif