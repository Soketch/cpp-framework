#ifndef __SYLAR_MUTEX_H_
#define __SYLAR_MUTEX_H_

#include <thread>
#include <functional>
#include <memory>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
namespace sylar
{
    // 信号量
    class Semaphore
    {
    public:
        Semaphore(uint32_t count = 0); // count 信号量值的大小
        ~Semaphore();
        void wait();   // 获取信号量
        void notify(); // 释放信号量

    private:
        // 禁用拷贝
        Semaphore(const Semaphore &) = delete;
        Semaphore(const Semaphore &&) = delete;
        Semaphore operator=(const Semaphore &) = delete;
        Semaphore operator=(const Semaphore &&) = delete;

    private:
        sem_t m_semaphore;
    };

    // 局部锁模板 具体实现 scoped  lock  Implementation
    template <class T>
    class ScopedLockImpl
    {
    public:
        ScopedLockImpl(T &mutex) : m_mutex(mutex)
        {
            m_mutex.lock();  // 加锁
            m_locked = true; // 设置加锁状态
        }
        ~ScopedLockImpl()
        {
            unlock(); // 解锁
        }
        void lock()
        {
            // 先判断锁状态，防止死锁
            if (!m_locked)
            {
                m_mutex.lock();
                m_locked = true;
            }
        }
        void unlock()
        {
            // 同样先判断锁状态
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T &m_mutex;
        bool m_locked;
    };

    // READ 局部 读锁 模板实现
    template <class T>
    class ReadScopedLockImpl
    {
    public:
        ReadScopedLockImpl(T &mutex) : m_mutex(mutex)
        {
            m_mutex.rdlock(); // 加锁
            m_locked = true;  // 设置加锁状态
        }
        ~ReadScopedLockImpl()
        {
            unlock(); // 解锁
        }
        void rdlock()
        {
            // 先判断锁状态，防止死锁
            if (!m_locked)
            {
                m_mutex.rdlock();
                m_locked = true;
            }
        }
        void unlock()
        {
            // 同样先判断锁状态
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T &m_mutex;
        bool m_locked;
    };

    // WRITE 局部 写锁 模板实现
    template <class T>
    class WriteScopedLockImpl
    {
    public:
        WriteScopedLockImpl(T &mutex) : m_mutex(mutex)
        {
            m_mutex.wrlock(); // 加锁
            m_locked = true;  // 设置加锁状态
        }
        ~WriteScopedLockImpl()
        {
            unlock(); // 解锁
        }
        void wrlock()
        {
            // 先判断锁状态，防止死锁
            if (!m_locked)
            {
                m_mutex.wrlock();
                m_locked = true;
            }
        }
        void unlock()
        {
            // 同样先判断锁状态
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T &m_mutex;
        bool m_locked;
    };

    // 互斥量
    class Mutex
    {
    public:
        typedef ScopedLockImpl<Mutex> Lock;
        Mutex()
        {
            pthread_mutex_init(&m_mutex, nullptr);
        }
        ~Mutex()
        {
            pthread_mutex_destroy(&m_mutex);
        }
        void lock()
        {
            pthread_mutex_lock(&m_mutex);
        }
        void unlock()
        {
            pthread_mutex_unlock(&m_mutex);
        }

    private:
        pthread_mutex_t m_mutex;
    };

    // 空锁 -- 仅用于调试验证工程
    class NullMutex
    {
    public:
        typedef ScopedLockImpl<NullMutex> Lock;
        NullMutex() {}
        ~NullMutex() {}
        void lock() {}
        void unlock() {}

    private:
    };

    // 读写锁互斥量
    class RWMutex
    {
    public:
        typedef ReadScopedLockImpl<RWMutex> ReadLock;
        typedef WriteScopedLockImpl<RWMutex> WriteLock;

        RWMutex()
        {
            pthread_rwlock_init(&m_lock, nullptr);
        }
        ~RWMutex()
        {
            pthread_rwlock_destroy(&m_lock);
        }

        // 读锁
        void rdlock()
        {
            pthread_rwlock_rdlock(&m_lock);
        }
        // 写锁
        void wrlock()
        {
            pthread_rwlock_wrlock(&m_lock);
        }
        // 解锁
        void unlock()
        {
            pthread_rwlock_unlock(&m_lock);
        }

    private:
        pthread_rwlock_t m_lock;
    };

    // 空读写锁 -- 用于调试
    class NullRWMutex
    {
    public:
        typedef ReadScopedLockImpl<NullMutex> ReadLock;
        typedef WriteScopedLockImpl<NullMutex> WriteLock;

        NullRWMutex()
        {
        }
        ~NullRWMutex()
        {
        }
        void rdlock()
        {
        }
        void wrlock()
        {
        }
        void unlock()
        {
        }
    };

    // 自旋锁：
    // -- 等待锁可用时不会使线程进入睡眠状态，而是不断地循环检查锁的状态。这种机制被称为“自旋”
    class SpinLock
    {
    public:
        typedef ScopedLockImpl<SpinLock> Lock;
        SpinLock()
        {
            pthread_spin_init(&m_mutex, 0);
        }
        ~SpinLock()
        {
            pthread_spin_destroy(&m_mutex);
        }
        void lock()
        {
            pthread_spin_lock(&m_mutex);
        }
        void unlock()
        {
            pthread_spin_unlock(&m_mutex);
        }

    private:
        pthread_spinlock_t m_mutex;
    };
}
#endif