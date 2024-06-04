#ifndef __SYLAR_IOMANAGER_H_
#define __SYLAR_IOMANAGER_H_

#include "scheduler.h"
#include "timer.h"

// 基于Epoll的IO协程调度器
namespace sylar
{
    class IOManager : public Scheduler, public TimerManager
    {
    public:
        std::shared_ptr<IOManager> ptr;
        typedef RWMutex RWMutexType;

        enum Event
        {
            NONE = 0x0, // 无事件
            READ = 0x1, // 读事件   //EPOLLIN
            WRITE = 0x4 // 写事件   //EPOLLOUT
        };

    private:
        // socket事件上下文类
        struct FdContext
        {
            typedef Mutex MutexType;
            // 事件上下文类
            struct EventContext
            {
                Scheduler *scheduler = nullptr; // 事件执行调度器
                Fiber::ptr fiber;               // 事件协程
                std::function<void()> cb;       // 事件回调函数
            };
            // 返回对应事件的上下文
            EventContext &getcontext(Event event);
            // 重置事件上下文
            void resetContext(EventContext &ctx);
            // 触发事件
            void triggerEvent(Event event);

            EventContext read;  // 读事件上下文
            EventContext write; // 写事件上下文

            int fd = 0;                 // 事件关联句柄
            Event events = Event::NONE; // 当前表示的事件  -- 枚举事件类型 -- NONE无事件
            MutexType mutex;            // 事件的mutex
        };

    public:
        /**
         * @brief 构造函数
         * @param[in] threads 线程数量
         * @param[in] use_caller 是否将调用线程包含进去
         * @param[in] name 调度器的名称
         */
        IOManager(size_t threads = 1, bool use_caller = true, const std::string &name = "");

        ~IOManager();

        // 添加事件  添加成功返回0,失败返回-1
        int addEvent(int fd, Event event, std::function<void()> cb = nullptr); // ==> 不会触发事件
        // 删除事件
        bool delEvent(int fd, Event event); //==> 不会触发事件
        // 取消事件 --> 取消单个
        bool cancelEvent(int fd, Event event); // ==> 如果事件存在则触发事件
        // 取消全部事件
        bool cancelAll(int fd);

        // 返回当前的IOManager
        static IOManager *GetThis();

    protected:
        void tickle() override;
        bool stopping() override;
        void idle() override;

        void onTimerInsertedtAtFront() override;
        // context容器初始化
        void ContextResize(size_t size);

        bool stopping(uint64_t &timeout);

    private:
        int m_epfd = 0; // epoll的fd句柄
        /// pipe 文件句柄  -- 一端读一端写
        int m_tickleFds[2];

        /// 当前等待执行的事件数量
        std::atomic<size_t> m_pendingEventCount = {0};

        RWMutexType m_mutex;
        /// socket事件上下文的容器
        std::vector<FdContext *> m_fdContexts;
    };
}

#endif