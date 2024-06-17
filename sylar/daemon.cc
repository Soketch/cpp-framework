#include "daemon.h"
#include "log.h"
#include "config.h"
#include <time.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace sylar
{
    static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    // 设置重启时间间隔 --> 防止资源没有完全释放
    static ConfigVar<uint32_t>::ptr g_daemon_restart_interval =
        Config::Lookup("daemon.restart_interval", (uint32_t)5, "daemon restart interval");

    /**
     * @brief 正常启动函数，调用main_cb回调函数
     */
    static int real_start(int argc, char **argv,
                          std::function<int(int argc, char **argv)> main_cb)
    {
        ProcessInfoMgr::GetInstance()->main_id = getpid();
        ProcessInfoMgr::GetInstance()->main_start_time = time(0);
        return main_cb(argc, argv);
    }

    /**
     * @brief 守护进程启动函数
     */
    static int real_daemon(int argc, char **argv,
                           std::function<int(int argc, char **argv)> main_cb)
    {
        daemon(1, 0);
        // 父进程id
        ProcessInfoMgr::GetInstance()->parent_id = getpid();
        // 父进程启动时间
        ProcessInfoMgr::GetInstance()->parent_start_time = time(0);

        while (true)
        {
            // 创建子进程
            pid_t pid = fork();

            if (pid == 0)
            { // 是子进程  ==> 返回子进程
                ProcessInfoMgr::GetInstance()->main_id = getpid();
                ProcessInfoMgr::GetInstance()->main_start_time = time(0);
                // 打印日志
                SYLAR_LOG_INFO(g_logger) << "daemon process start pid=" << getpid();
                return real_start(argc, argv, main_cb);
            }
            else if (pid < 0)
            { // 创建失败有问题
                SYLAR_LOG_ERROR(g_logger) << "fork fail return=" << pid
                                          << ",  errno=" << errno
                                          << ",  str(errno):" << strerror(errno);
                return -1;
            }
            else
            { // 是父进程  ==>   返回父进程

                int status = 0; // 用于存储进程退出状态

                waitpid(pid, &status, 0); // 等待父进程，直到进程结束
                if (status)
                {
                    if (status == 9) // SIGKILL
                    {
                        SYLAR_LOG_INFO(g_logger) << " killd (被终止)";
                        break;
                    }
                    else
                    {
                        SYLAR_LOG_ERROR(g_logger) << "child crash pid=" << pid
                                                  << ",  status=" << status;
                    }
                }
                else
                {
                    // 正常退出，父进程不用重启
                    SYLAR_LOG_INFO(g_logger) << "child finished pid=" << pid;
                    break;
                }
            }
            // 记录子进程重启次数
            ProcessInfoMgr::GetInstance()->restart_count += 1;

            sleep(g_daemon_restart_interval->getValue());
        }
        return 0;
    }

    //

    int start_daemon(int argc, char **argv,
                     std::function<int(int argc, char **argv)> main_cb,
                     bool is_daemon)
    {
        if (!is_daemon)
        {
            ProcessInfoMgr::GetInstance()->parent_id = getpid();
            ProcessInfoMgr::GetInstance()->parent_start_time = time(0);
            return real_start(argc, argv, main_cb);
        }
        return real_daemon(argc, argv, main_cb);
    }

    // 转string序列化输出
    std::string ProcessInfo::toString() const
    {
        std::stringstream ss;
        ss << "[ProcessInfo parent_id=" << parent_id
           << " main_id=" << main_id
           << " parent_start_time=" << sylar::Time2Str(parent_start_time)
           << " main_start_time=" << sylar::Time2Str(main_start_time)
           << " restart_count=" << restart_count << "]";
        return ss.str();
    }
}