#include "mutex.h"
#include <iostream>
namespace sylar
{
    // sem_init:  pshared: 指定信号量是在进程间共享还是仅在线程间共享。
    // 如果 pshared = 0，则信号量在进程内的线程间共享。如果 pshared != 0，则信号量在进程间共享。
    Semaphore::Semaphore(uint count)
    {
        if (sem_init(&m_semaphore, 0, count)) // 线程共享信号量
        {
            throw std::logic_error("sem_init error");
        }
    }

    Semaphore::~Semaphore()
    {
        sem_destroy(&m_semaphore); // 释放信号量
    }

    void Semaphore::wait() // 获取信号量
    {

        if (sem_wait(&m_semaphore))
        {
            throw std::logic_error("sem_wait error");
        }
    }
    void Semaphore::notify() // 释放信号量  sem_post
    {
        if (sem_post(&m_semaphore))
        {
            throw std::logic_error("sem_post error");
        }
    }

}