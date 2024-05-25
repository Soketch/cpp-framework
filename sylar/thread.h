#ifndef __SYLAR_THREAD_H__
#define __SYLAR_THREAD_H__

// 1  pthread_xxx
// 2  std::thread   ==> 本质也是使用pthread实现

#include <thread>
#include <functional>
#include <memory>
#include <string>
#include <pthread.h>
namespace sylar
{
    class Thread
    {
    public:
        typedef std::shared_ptr<Thread> ptr;

        Thread(std::function<void()> cb, const std::string &name);
        const std::string getName() const { return this->m_name; }
        pid_t getId() const { return this->m_id; }
        void join();

        static Thread *GetThis();            // 获取当前线程
        static const std::string &GetName(); // 获取当前线程名称  -- 配合日志系统
        static void setName(const std::string &name);
        ~Thread();

    private:
        // 禁止拷贝
        Thread(const Thread &) = delete;
        Thread(const Thread &&) = delete;
        Thread &operator=(const Thread &) = delete;
        Thread &operator=(Thread &&) = delete;

    private:
        static void *run(void *arg); // 线程执行函数
    private:
        pid_t m_id = -1;            // 线程id
        pthread_t m_thread = 0;     // 线程结构     //线程实际表示
        std::function<void()> m_cb; // 线程执行函数
        std::string m_name;         // 线程名称
    };
}

#endif