#ifndef __SYLAR_UTIL_H__
#define __SYLAR_UTIL_H__
#include <cxxabi.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <cstdint>
#include <vector>
#include <string>
#include <iomanip>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <boost/lexical_cast.hpp>

namespace sylar
{
    pid_t GetTheadId(); // 获取当前线程id

    uint32_t GetFiberId(); // 获取当前协程（纤程）id

    // 生成和格式化调用堆栈
    void BackTrace(std::vector<std::string> &bt, int size = 64, int skip = 1);
    std::string BackTraceToString(int size = 64, int skip = 2, const std::string &prefix = "");

    // 时间ms  ==> 获取当前时间的毫秒数和微秒数
    uint64_t GetCurrentMS(); //  ==> 毫秒
    uint64_t GetCurrentUS(); //  ==> 微秒

    // time结构转string字符串
    std::string Time2Str(time_t ts = time(0), const std::string &format = "%Y-%m-%d %H:%M:%S");
    // string字符串转 time
    time_t Str2Time(const char *str, const char *format = "%Y-%m-%d %H:%M:%S");

    class FSUtil
    {
    public:
        static void ListAllFile(std::vector<std::string> &files, const std::string &path, const std::string &subfix);
        static bool Mkdir(const std::string &dirname);
        static bool IsRunningPidfile(const std::string &pidfile);
        static bool Rm(const std::string &path);
        static bool Mv(const std::string &from, const std::string &to);
        static bool Realpath(const std::string &path, std::string &rpath);
        static bool Symlink(const std::string &frm, const std::string &to);
        static bool Unlink(const std::string &filename, bool exist = false);
        static std::string Dirname(const std::string &filename);
        static std::string Basename(const std::string &filename);
        static bool OpenForRead(std::ifstream &ifs, const std::string &filename, std::ios_base::openmode mode);
        static bool OpenForWrite(std::ofstream &ofs, const std::string &filename, std::ios_base::openmode mode);
    };

}
#endif