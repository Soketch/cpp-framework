/// 不可拷贝对象封装

#ifndef __SYLAR_NONCOPYABLE_H_
#define __SYLAR_NONCOPYABLE_H_

namespace sylar
{
    class Noncopyable
    {
    public:
        Noncopyable() = default;
        ~Noncopyable() = default;

        Noncopyable(const Noncopyable &) = delete;

        Noncopyable &operator=(const Noncopyable &) = delete;

    private:
    };
}

#endif