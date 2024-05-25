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
    private:
    };

    // 读写互斥量
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
        void rdLock()
        {
            pthread_rwlock_rdlock(&m_lock);
        }
        // 写锁
        void wrLock()
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
}
#endif