#include "timer.h"
#include "util.h"

namespace sylar
{
    // 比较两个智能指针，首先比较下次执行时间（m_next）判断 得出顺序
    bool Timer::Comparator::operator()(const Timer::ptr &lhs, const Timer::ptr &rhs)
    {
        if (!lhs && !rhs)
        { // 两个都为空
            return false;
        }

        if (!lhs)
        { // 左为空
            return true;
        }
        if (!rhs)
        { // 右为空
            return false;
        }

        if (lhs->m_next < rhs->m_next)
        { // 左边执行时间小于右边执行时间
            return true;
        }
        if (rhs->m_next < lhs->m_next)
        { // 右边执行时间小于左边执行时间
            return false;
        }

        // 上述都两边相同（时间都一样），最后比地址大小
        return lhs.get() < rhs.get();
    }

    // 毫秒级别
    Timer::Timer(uint64_t ms, std::function<void()> cb, bool recurring, TimerManager *manager) // recurring : 是否是循环定时器
        : m_recurring(recurring), m_ms(ms), m_cb(cb), m_manager(manager)
    {
        m_next = sylar::GetCurrentMS() + m_ms;
    }
    Timer::Timer(uint64_t next) : m_next(next)
    {
    }

    // 取消定时器
    bool Timer::cancel()
    {
        TimerManager::RWMutexType::WriteLock wrlock(m_manager->m_mutex);
        if (m_cb)
        {
            m_cb = nullptr;
            auto it = m_manager->m_timers.find(shared_from_this());
            m_manager->m_timers.erase(it);
            return true;
        }
    }
    // 刷新设置定时器的执行时间
    bool Timer::refresh()
    { // 按照当前时间开始算
        // 这里需要注意，set容器不能直接改时间，会影响整个set的元素位置。
        // 应先移除该元素，然后重新设置再插入元素，set容器会重新排序。
        TimerManager::RWMutexType::WriteLock wrlock(m_manager->m_mutex);
        if (!m_cb)
        {
            return false;
        }
        auto it = m_manager->m_timers.find(shared_from_this());
        if (it == m_manager->m_timers.end())
        {
            return false;
        }
        m_manager->m_timers.erase(it);
        m_next = sylar::GetCurrentMS() + m_ms;
        m_manager->m_timers.insert(shared_from_this());
        return true;
    }

    // 重置定时器时间
    bool Timer::reset(uint64_t ms, bool from_now)
    {
        if (ms == m_ms && !from_now)
        {
            return true;
        }
        TimerManager::RWMutexType::WriteLock wrlock(m_manager->m_mutex);
        if (!m_cb)
        {
            return false;
        }

        auto it = m_manager->m_timers.find(shared_from_this());
        if (it == m_manager->m_timers.end())
        {
            return false;
        }
        m_manager->m_timers.erase(it);

        uint64_t start = 0;
        if (from_now)
        {
            start = sylar::GetCurrentMS();
        }
        else
        {
            start = m_next - m_ms;
        }
        m_ms = ms;
        m_next = start + m_ms;
        m_manager->addTimer(shared_from_this(), wrlock);

        return true;
    }

    TimerManager::TimerManager()
    {
    }
    TimerManager::~TimerManager()
    {
    }

    Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recurring)
    {
        Timer::ptr timer(new Timer(ms, cb, recurring, this));
        RWMutexType::WriteLock wrlock(m_mutex);
        /*
        // 将新定时器插入到定时器集合中，并获取插入位置的迭代器
        // std::set 会根据 Comparator 自动保持定时器按执行时间排序
        auto it = m_timers.insert(timer).first;

        // 检查新插入的定时器是否位于集合的第一个位置
        // 如果是第一个位置，意味着这是当前所有定时器中最早触发的一个
        bool at_front = (it == m_timers.begin());
        wrlock.unlock();

        if (at_front)
        {
            onTimerInsertedtAtFront();
        }
        */
        addTimer(timer, wrlock);
        return timer;
    }

    // 条件定时器的 辅助函数OnTimer
    static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb)
    {
        std::shared_ptr<void> tmp = weak_cond.lock();
        if (tmp)
        {
            cb();
        }
    }
    // 条件定时器
    Timer::ptr TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recurring)
    {
        return addTimer(ms, std::bind(&OnTimer, weak_cond, cb), recurring);
    }

    uint64_t TimerManager::getNextTimer()
    {
        RWMutexType::ReadLock rdlock(m_mutex);
        m_tickled = false;
        if (m_timers.empty())
        {
            return ~0ull; // 0取反（unsigned long long）
        }
        const Timer::ptr &next = *m_timers.begin();
        uint64_t now_ms = sylar::GetCurrentMS();
        if (now_ms >= next->m_next)
        {
            return 0;
        }
        else
        {
            return next->m_next - now_ms;
        }
    }

    // 获取需要执行的定时器的回调函数列表
    void TimerManager::listExpiredCb(std::vector<std::function<void()>> &cbs)
    {
        uint64_t now_ms = sylar::GetCurrentMS();
        std::vector<Timer::ptr> expried;
        {
            RWMutexType::ReadLock rdlock(m_mutex);
            if (m_timers.empty())
            {
                return;
            }
        }
        RWMutexType::WriteLock wrlock(m_mutex);
        Timer::ptr now_timer(new Timer(now_ms));
        auto it = m_timers.lower_bound(now_timer);
        while (it != m_timers.end() && (*it)->m_next == now_ms)
        {
            ++it;
        }
        expried.insert(expried.begin(), m_timers.begin(), it);
        m_timers.erase(m_timers.begin(), it);
        cbs.reserve(expried.size());

        for (auto &timer : expried)
        {
            cbs.push_back(timer->m_cb);
            if (timer->m_recurring)
            {
                timer->m_next = now_ms + timer->m_ms;
                m_timers.insert(timer);
            }
            else
            {
                timer->m_cb = nullptr;
            }
        }
    }

    // 将定时器添加到管理器中
    void TimerManager::addTimer(Timer::ptr val, RWMutexType::WriteLock &lock)
    {
        auto it = m_timers.insert(val).first;
        bool at_front = (it == m_timers.begin()) && !m_tickled;
        if (at_front)
        {
            m_tickled = true;
        }
        lock.unlock();

        if (at_front)
        {
            onTimerInsertedtAtFront();
        }
    }

}