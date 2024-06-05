#ifndef __SYLAR_HOOK_H_
#define __SYLAR_HOOK_H_

#include <unistd.h>
#include <iostream>

namespace sylar
{
    /**
     * @brief 检查钩子功能是否启用
     *
     * @return 返回当前钩子是否启用的状态
     */
    bool is_hook_enable();

    /**
     * @brief 设置钩子功能的启用状态
     *
     * @param flag 如果为 true，则启用钩子功能；如果为 false，则禁用钩子功能
     */
    void set_hook_enable(bool flag);
    // hook 初始化
    void hook_init();
}

extern "C"
{
    typedef unsigned int (*sleep_fun)(unsigned int seconds);
    extern sleep_fun sleep_f;

    typedef int (*usleep_fun)(useconds_t usec);
    extern usleep_fun usleep_f;
}
#endif