#ifndef __SYLAR_MACRO_H_
#define __SYLAR_MACRO_H_

#include <string.h>
#include <assert.h> //断言
#include "util.h"

#if defined __GNUC__ || defined __llvm__
#define SYLAR_LIKELY(x) __builtin_expect(!!(x), 1)
#define SYLAR_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define SYLAR_LIKELY(x) (x)
#define SYLAR_UNLIKELY(X) (X)
#endif

#define SYLAR_ASSERT(x)                                                                \
    if (!(x))                                                                          \
    {                                                                                  \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION: " #x                          \
                                          << "\nbacktrace:\n"                          \
                                          << sylar::BackTraceToString(100, 2, "    "); \
        assert(x);                                                                     \
    }

#define SYLAR_ASSERT2(x, w)                                                            \
    if (!(x))                                                                          \
    {                                                                                  \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION: " #x                          \
                                          << "\n"                                      \
                                          << w                                         \
                                          << "\nbacktrace:\n"                          \
                                          << sylar::BackTraceToString(100, 2, "    "); \
        assert(x);                                                                     \
    }

#endif