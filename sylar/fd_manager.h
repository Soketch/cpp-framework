#ifndef __FD_MANAGER_H_
#define __FD_MANAGER_H_

#include <memory>
#include <vector>
#include "thread.h"
#include "log.h"
// #include "iomanager.h"
#include "singleton.h"

// 文件句柄管理类
namespace sylar
{
    class FdCtx : public std::enable_shared_from_this<FdCtx>
    {
    public:
        using ptr = std::shared_ptr<FdCtx>;

        FdCtx(int fd);
        ~FdCtx();

        bool init();
        bool isInit() const { return m_isInit; }
        bool isSocket() const { return m_isSocket; }
        bool isClose() const { return m_isClosed; }

        void setUserNonblock(bool v) { this->m_userNonblock = v; }
        bool getUserNonblock() const { return m_userNonblock; }

        /**
         * @brief 设置系统非阻塞
         * @param[in] v 是否阻塞
         */
        void setSysNonblock(bool v) { m_sysNonblock = v; }

        /**
         * @brief 获取系统非阻塞
         */
        bool getSysNonblock() const { return m_sysNonblock; }
        void setTimeout(int type, uint64_t v);
        uint64_t getTimeout(int type);

    private:
        bool m_isInit : 1; // 位域（Bit-field）语法   --> 只分配一位存储空间
        bool m_isSocket : 1;
        /// 是否hook非阻塞
        bool m_sysNonblock : 1;
        /// 是否用户主动设置非阻塞
        bool m_userNonblock : 1;
        bool m_isClosed : 1;

        int m_fd;
        uint64_t m_recvTimeout;
        uint64_t m_sendTimeout;
        // IOManager *m_iomanager;
    };

    class FdManager
    {
    public:
        using RWMutexType = RWMutex;
        FdManager();
        ~FdManager() {}

        // 获取句柄，如果不存在设置参数true可以创建再获取
        FdCtx::ptr get(int fd, bool auto_create = false);

        void delFd(int fd);

    private:
        RWMutexType m_mutex;
        std::vector<FdCtx::ptr> m_datas;
    };

    // 文件句柄 单例模式
    using FdMgr = Singleton<FdManager>;
    // typedef Singleton<FdManager> FdMgr;
}
#endif