#include "thread.h"
#include "log.h"
#include "util.h"
namespace sylar
{
    // 线程局部变量
    static thread_local Thread *t_thread = nullptr;
    static thread_local std::string t_thread_name = "UNKNOWN";

    static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    Thread *Thread::GetThis()
    {
        return t_thread;
    }

    const std::string &Thread::GetName()
    {
        return t_thread_name;
    }
    void Thread::setName(const std::string &name)
    {
        if (name.empty())
        {
            return;
        }
        if (t_thread)
        {
            t_thread->m_name = name;
        }
        t_thread_name = name;
    }

    void *Thread::run(void *arg) // 线程执行函数
    {
        Thread *thread = (Thread *)arg;
        t_thread = thread;
        t_thread_name = thread->m_name;

        thread->m_id = sylar::GetTheadId();                                       // 获取当前线程id并设置
        pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str()); // 给线程命名：接受最大16字符数

        std::function<void()> cb;
        cb.swap(thread->m_cb);

        thread->m_semaphore.notify(); // 如果有线程等待，其中的一个会被唤醒， sem_post

        cb();
        return 0;
    }

    //
    // pthread_create 和 pthread_join 返回一个整数值。
    // 如果返回值不为 0，则表示出现错误。
    // 错误原因可以通过 perror 或 strerror 函数获取并打印。
    //

    Thread::Thread(std::function<void()> cb, const std::string &name) : m_cb(cb), m_name(name)
    {
        if (name.empty())
        {
            m_name = "unknow";
        }
        int ret = pthread_create(&m_thread, nullptr, &Thread::run, this);
        if (ret != 0)
        {
            SYLAR_LOG_ERROR(g_logger) << "pthread_create thread fail, result=" << ret
                                      << "  name=" << m_name;
            throw std::logic_error("pthread_create error");
        }

        m_semaphore.wait();
    }

    void Thread::join()
    {
        if (m_thread)
        {
            int ret = pthread_join(m_thread, nullptr);
            if (ret != 0)
            {
                SYLAR_LOG_ERROR(g_logger) << "pthread_join thread fail, result=" << ret
                                          << "  name=" << m_name;
                throw std::logic_error("pthread_join error");
            }
            m_thread = 0;
        }
    }

    Thread::~Thread()
    {
        // 在析构中 detach
        if (m_thread)
        {
            pthread_detach(m_thread);
        }
    }

}