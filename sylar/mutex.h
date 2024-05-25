#ifndef __SYLAR_MUTEX_H_
#define __SYLAR_MUTEX_H_

#include <thread>
#include <functional>
#include <memory>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

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

#endif