#include "iomanager.h"
#include "log.h"
#include "macro.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h> //文件控制头文件  ==> 操作文件描述符函数与宏
#include <string.h>

namespace sylar
{

    static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    // 返回对应事件的上下文
    IOManager::FdContext::EventContext &IOManager::FdContext::getcontext(Event event)
    {
        switch (event)
        {
        case Event::READ:
            return read;
        case Event::WRITE:
            return write;

        default:
            SYLAR_ASSERT2(false, "getContext");
        }
        throw std::invalid_argument("invaild event for the getContext().");
    }
    // 重置事件上下文
    void IOManager::FdContext::resetContext(EventContext &ctx)
    {
        ctx.scheduler = nullptr;
        ctx.fiber.reset();
        ctx.cb = nullptr;
    }
    // 触发事件
    void IOManager::FdContext::triggerEvent(Event event)
    {
        // SYLAR_LOG_INFO(g_logger) << "fd=" << fd
        //                          << " triggerEvent event=" << event
        //                          << " this->events=" << this->events;
        SYLAR_ASSERT(events & event);
        events = (Event)(events & ~event);
        EventContext &ctx = getcontext(event);
        if (ctx.cb)
        {
            ctx.scheduler->schedule(&ctx.cb);
        }
        else
        {
            ctx.scheduler->schedule(&ctx.fiber);
        }
        ctx.scheduler = nullptr;
        return;
    }

    IOManager::IOManager(size_t threads, bool use_caller, const std::string &name)
        : Scheduler(threads, use_caller, name)
    {
        m_epfd = epoll_create(5000);
        SYLAR_ASSERT2(m_epfd > 0, "IOManager(...){} epoll_create");
        int rt = pipe(m_tickleFds); // pipe成功返回0，失败返回-1
        SYLAR_ASSERT(!rt);          // SYLAR_ASSERT(rt != 0);

        epoll_event event;
        memset(&event, 0, sizeof(epoll_event));
        event.events = EPOLLIN | EPOLLET; //  EPOLLIN可读 + EPOLLET边缘触发（高效模式）
        event.data.fd = m_tickleFds[0];   // 0是读端， 1是写端

        rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK); // 将管道的读端（m_tickleFds[0]）设置为非阻塞模式。
        SYLAR_ASSERT(!rt);

        rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
        SYLAR_ASSERT2(!rt, "epoll ctl_for the IOManager(...){}");

        // m_fdContexts.resize(64); // 初始化socket上下文句柄容器
        ContextResize(32);

        Start(); // Scheduler::Start()  启动调度器
    }

    IOManager::~IOManager()
    {
        // 关闭句柄fd, 关闭调度器Scheduler  ==> Scheduler::Stop()
        Stop();
        close(m_epfd);
        close(m_tickleFds[0]);
        close(m_tickleFds[1]);

        // 释放内存
        for (size_t i = 0; i < m_fdContexts.size(); ++i)
        {
            if (m_fdContexts[i])
            {
                delete (m_fdContexts[i]);
            }
        }
    }

    void IOManager::ContextResize(size_t size)
    {
        m_fdContexts.resize(size);
        for (size_t i = 0; i < m_fdContexts.size(); ++i)
        {
            if (!m_fdContexts[i])
            {
                m_fdContexts[i] = new FdContext;
                m_fdContexts[i]->fd = i;
            }
        }
    }

    // 添加事件
    int IOManager::addEvent(int fd, Event event, std::function<void()> cb)
    {
        FdContext *fd_ctx = nullptr;

        RWMutexType::ReadLock rdlock(m_mutex); // 加读锁
        if ((int)m_fdContexts.size() > fd)
        {
            fd_ctx = m_fdContexts[fd];
            rdlock.unlock();
        }
        else
        {
            rdlock.unlock();
            RWMutexType::WriteLock wrlock(m_mutex);
            ContextResize(fd * 1.5);
            fd_ctx = m_fdContexts[fd]; //***
        }

        FdContext::MutexType::Lock lock(fd_ctx->mutex);
        if (fd_ctx->events & event)
        {
            SYLAR_LOG_ERROR(g_logger) << "addEvent assert fd=" << fd
                                      << "Event=" << event
                                      << "fd_ctx.event=" << fd_ctx->events;
            SYLAR_ASSERT(!(fd_ctx->events & event));
        }

        int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
        epoll_event epevent;
        epevent.events = EPOLLET | fd_ctx->events | event;
        epevent.data.ptr = fd_ctx;

        int ret = epoll_ctl(m_epfd, op, fd, &epevent);
        if (ret)
        {
            SYLAR_LOG_ERROR(g_logger) << "addevent  epoll_ctl(" << m_epfd << "," << op << "," << fd << "," << epevent.events << "):"
                                      << ret << "(" << errno << ")" << strerror(errno) << ")";
            return -1;
        }
        ++m_pendingEventCount; // 当前等待执行的事件数量+1
        fd_ctx->events = (Event)(fd_ctx->events | event);
        FdContext::EventContext &event_ctx = fd_ctx->getcontext(event);
        SYLAR_ASSERT(!event_ctx.scheduler &&
                     !event_ctx.fiber &&
                     !event_ctx.cb);
        event_ctx.scheduler = Scheduler::GetThis();
        if (cb)
        {
            event_ctx.cb.swap(cb);
        }
        else
        {
            event_ctx.fiber = Fiber::GetThis();
            SYLAR_ASSERT2(event_ctx.fiber->getState() == Fiber::EXEC, "state=" << event_ctx.fiber->getState());
        }
        return 0;
    }

    bool IOManager::delEvent(int fd, Event event)
    {
        RWMutexType::ReadLock rdlock(m_mutex);
        if ((int)m_fdContexts.size() <= fd)
        {
            return false;
        }
        FdContext *fd_ctx = m_fdContexts[fd];
        rdlock.unlock();

        FdContext::MutexType::Lock lock(fd_ctx->mutex);
        if (!(fd_ctx->events & event))
        {
            return false;
        }

        Event new_events = (Event)(fd_ctx->events & ~event);
        int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = EPOLLET | new_events;
        epevent.data.ptr = fd_ctx;

        int ret = epoll_ctl(m_epfd, op, fd, &epevent);
        if (ret)
        {
            SYLAR_LOG_ERROR(g_logger) << "delevent  epoll_ctl(" << m_epfd << "," << op << "," << fd << "," << epevent.events << "):"
                                      << ret << "(" << errno << ")" << strerror(errno) << ")";
            return false;
        }
        --m_pendingEventCount;
        fd_ctx->events = new_events;
        FdContext::EventContext &event_ctx = fd_ctx->getcontext(event);
        fd_ctx->resetContext(event_ctx);
        return true;
    }

    bool IOManager::cancelEvent(int fd, Event event)
    {
        RWMutexType::ReadLock rdlock(m_mutex);
        if ((int)m_fdContexts.size() <= fd)
        {
            return false;
        }
        FdContext *fd_ctx = m_fdContexts[fd];
        rdlock.unlock();

        FdContext::MutexType::Lock lock(fd_ctx->mutex);
        if (!(fd_ctx->events & event))
        {
            return false;
        }

        Event new_events = (Event)(fd_ctx->events & ~event);
        int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = EPOLLET | new_events;
        epevent.data.ptr = fd_ctx;

        int ret = epoll_ctl(m_epfd, op, fd, &epevent);
        if (ret)
        {
            SYLAR_LOG_ERROR(g_logger) << "delevent  epoll_ctl(" << m_epfd << "," << op << "," << fd << "," << epevent.events << "):"
                                      << ret << "(" << errno << ")" << strerror(errno) << ")";
            return false;
        }

        fd_ctx->triggerEvent(event); // 触发事件
        --m_pendingEventCount;       // 事件数-1
        return true;
    }

    bool IOManager::cancelAll(int fd)
    {
        RWMutexType::ReadLock rdlock(m_mutex);
        if ((int)m_fdContexts.size() <= fd)
        {
            return false;
        }
        FdContext *fd_ctx = m_fdContexts[fd];
        rdlock.unlock();

        FdContext::MutexType::Lock lock(fd_ctx->mutex);
        if (!fd_ctx->events)
        {
            return false;
        }

        int op = EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = 0;
        epevent.data.ptr = fd_ctx;

        int ret = epoll_ctl(m_epfd, op, fd, &epevent);
        if (ret)
        {
            SYLAR_LOG_ERROR(g_logger) << "delevent  epoll_ctl(" << m_epfd << "," << op << "," << fd << "," << epevent.events << "):"
                                      << ret << "(" << errno << ")" << strerror(errno) << ")";
        }

        if (fd_ctx->events & READ) // 读事件
        {
            fd_ctx->triggerEvent(READ);
            --m_pendingEventCount; // 事件数-1
        }
        if (fd_ctx->events & WRITE) // 写事件
        {
            fd_ctx->triggerEvent(WRITE);
            --m_pendingEventCount;
        }

        SYLAR_ASSERT(fd_ctx->events == 0);
        return true;
    }

    IOManager *IOManager::GetThis()
    {
        return dynamic_cast<IOManager *>(Scheduler::GetThis());
    }

    void IOManager::tickle()
    {
        // 是否有空闲线程，用空闲线程处理 : idleTheads
        if (!hasIdleThreads())
        {
            return;
        }
        int rt = write(m_tickleFds[1], "T", 1);
        SYLAR_ASSERT(rt == 1);
    }

    bool IOManager::stopping(uint64_t &timeout)
    {
        timeout = getNextTimer();
        return timeout == ~0ull && m_pendingEventCount == 0 && Scheduler::stopping();
    }

    bool IOManager::stopping()
    {
        uint64_t timeout = 0;
        return stopping(timeout);
    }

    // 核心点 ==> 处理epoll相关事务
    void IOManager::idle()
    {
        SYLAR_LOG_DEBUG(g_logger) << "IOManager::idle()";
        const uint64_t MAX_EVNETS = 256;
        epoll_event *events = new epoll_event[MAX_EVNETS]();

        std::shared_ptr<epoll_event> shared_events(events, [](epoll_event *ptr)
                                                   { delete[] ptr; });
        while (true)
        {
            uint64_t next_timeout = 0;
            if (stopping(next_timeout))
            {
                SYLAR_LOG_INFO(g_logger) << "name=" << getName() << " idle stopping exit.";
                break;
            }

            int ret = 0;
            do
            {
                static const int MAX_TIMEOUT = 3000; // epoll定时是采用毫秒级 --> 3秒
                if (next_timeout != ~0ull)
                {
                    next_timeout = (int)next_timeout > MAX_TIMEOUT
                                       ? MAX_TIMEOUT
                                       : next_timeout;
                }
                else
                {
                    next_timeout = MAX_TIMEOUT;
                }
                ret = epoll_wait(m_epfd, events, MAX_EVNETS, MAX_TIMEOUT);
                if (ret < 0 && errno == EINTR)
                { // 重新epoll_wait一次
                }
                else
                {
                    break;
                }
            } while (true);

            std::vector<std::function<void()>> cbs;
            listExpiredCb(cbs);
            if (!cbs.empty())
            {
                // SYLAR_LOG_DEBUG(g_logger) << "on timer cbs.size=" << cbs.size();
                schedule(cbs.begin(), cbs.end());
                cbs.clear();
            }

            for (int i = 0; i < ret; ++i)
            {
                epoll_event &event = events[i];
                if (event.data.fd == m_tickleFds[0])
                {
                    uint8_t dummy;
                    while (read(m_tickleFds[0], &dummy, 1) == 1)
                        ;
                    continue;
                }

                FdContext *fd_ctx = (FdContext *)event.data.ptr;
                FdContext::MutexType::Lock lock(fd_ctx->mutex);

                if (event.events & (EPOLLERR | EPOLLHUP))
                {
                    // event.events |= EPOLLIN | EPOLLOUT;
                    event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->events;
                }

                int real_events = Event::NONE;
                if (event.events & EPOLLIN)
                {
                    real_events |= Event::READ;
                }
                if (event.events & EPOLLOUT)
                {
                    real_events |= Event::WRITE;
                }
                if ((fd_ctx->events & real_events) == Event::NONE)
                {
                    continue;
                }
                int left_events = (fd_ctx->events & ~real_events); // 剩余事件

                int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
                event.events = EPOLLET | left_events;

                int ret2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
                if (ret2)
                {
                    SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                                              << op << ", " << fd_ctx->fd << ", " << (EPOLL_EVENTS)event.events << "):"
                                              << ret2 << " (" << errno << ") (" << strerror(errno) << ")";
                    continue;
                }
                if (real_events & Event::READ)
                {
                    fd_ctx->triggerEvent(Event::READ);
                    --m_pendingEventCount; // 事件数-1
                }
                if (real_events & Event::WRITE)
                {
                    fd_ctx->triggerEvent(Event::WRITE);
                    --m_pendingEventCount;
                }
            }

            // 处理完一个idle, 让出控制权到外部协程框架
            // ==> 回到主协程 MainFiber
            Fiber::ptr cur = Fiber::GetThis();
            auto raw_ptr = cur.get();
            cur.reset();
            raw_ptr->swapOut();
        }
    }

    void IOManager::onTimerInsertedtAtFront()
    {
        tickle();
    }
}