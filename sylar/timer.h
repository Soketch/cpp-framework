#ifndef __SYLAR_TIMER_H_
#define __SYLAR_TIMER_H_

#include <memory>
#include <set>
#include <vector>
#include "thread.h"

// 定时器封装

namespace sylar
{
    // 定时器管理类
    class TimerManager;

    class Timer : public std::enable_shared_from_this<Timer>
    {
        friend class TimerManager; // 声明友元类，用TimerManager操作Timer私有成员

    public:
        typedef std::shared_ptr<Timer> ptr;

        // 取消定时器
        bool cancel();
        // 刷新设置定时器的执行时间
        bool refresh();

        /**
         * @brief 重置定时器时间
         * @param ms  定时器执行间隔时间(毫秒)
         * @param from_now 是否从当前开始计算
         */
        bool reset(uint64_t ms, bool from_now);

    private:
        /**
         * @brief 构造函数
         * @param[in] ms 定时器执行间隔时间
         * @param[in] cb 回调函数
         * @param[in] recurring 是否循环
         * @param[in] manager 定时器管理器
         */
        Timer(uint64_t ms, std::function<void()> cb, bool recurring, TimerManager *manager); // recurring : 是否是循环定时器

        Timer(uint64_t next);

    private:
        bool m_recurring = false; // 是否循环定时器
        uint64_t m_ms = 0;        // 执行时间周期（即多长时间执行一次这个任务）
        uint64_t m_next = 0;      // 精确执行时间（时间点）
        std::function<void()> m_cb;
        TimerManager *m_manager = nullptr;

    private:
        struct Comparator // 比较结构体 - 比较顺序
        {
            bool operator()(const Timer::ptr &lhs, const Timer::ptr &rhs);
        };
    };

    class TimerManager
    {
        friend class Timer;

    public:
        /// @brief 读写锁类型， 读写分离
        typedef RWMutex RWMutexType;
        TimerManager();
        virtual ~TimerManager();

        /**
         * @brief  添加一个定时器
         */
        Timer::ptr addTimer(uint64_t ms, std::function<void()>, bool recurring = false);

        // 条件定时器  ==> 用智能指针weak_ptr做条件,利用引用计数机制
        Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recurring = false); // weak_cond条件

        // 获取下一个定时器的执行时间
        uint64_t getNextTimer();
        // 获取需要执行的定时器的回调函数列表
        void listExpiredCb(std::vector<std::function<void()>> &cbs);

        bool hasTimer();

    protected:
        // 当有新的定时器插入到定时器的首部,执行该函数
        virtual void onTimerInsertedtAtFront() = 0;

        // 将定时器添加到管理器中
        void addTimer(Timer::ptr val, RWMutexType::WriteLock &lock);

    private:
        // 检测服务器时间是否被调后了
        bool detectClockRollover(uint64_t now_ms);

    private:
        RWMutexType m_mutex;
        // 定时器集合
        std::set<Timer::ptr, Timer::Comparator> m_timers;
        // 是否触发onTimerInsertedAtFront
        bool m_tickled = false;
        // 上次执行时间
        uint64_t m_previouseTime = 0;
    };
}

#endif