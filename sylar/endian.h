#ifndef __SYLAR_ENDIAN_H_
#define __SYLAR_ENDIAN_H_

/// @brief 字节序操作函数(大端/小端)

/**
 大端序（Big-endian）：高字节存储在低地址处，低字节存储在高地址处。
 小端序（Little-endian）：低字节存储在低地址处，高字节存储在高地址处。
 */
#define SYLAR_LITTLE_ENDIAN 1
#define SYLAR_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>
#include <iostream>

namespace sylar
{

    /**
     * @brief 8字节类型的字节序转化
     */
    template <class T>
    typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
    byteswap(T value)
    {
        return (T)bswap_64((uint64_t)value);
    }

    /**
     * @brief 4字节类型的字节序转化
     */
    template <class T>
    typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
    byteswap(T value)
    {
        return (T)bswap_32((uint32_t)value);
    }

    /**
     * @brief 2字节类型的字节序转化
     */
    template <class T>
    typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
    byteswap(T value)
    {
        return (T)bswap_16((uint16_t)value);
    }

#if BYTE_ORDER == BIG_ENDIAN
#define SYLAR_BYTE_ORDER SYLAR_BIG_ENDIAN
#else
#define SYLAR_BYTE_ORDER SYLAR_LITTLE_ENDIAN
#endif

#if SYLAR_BYTE_ORDER == SYLAR_BIG_ENDIAN

    /**
     * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
     */
    template <class T>
    T byteswapOnLittleEndian(T t)
    {
        return t;
    }

    /**
     * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
     */
    template <class T>
    T byteswapOnBigEndian(T t)
    {
        return byteswap(t);
    }
#else

    /**
     * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
     */
    template <class T>
    T byteswapOnLittleEndian(T t)
    {
        return byteswap(t);
    }

    /**
     * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
     */
    template <class T>
    T byteswapOnBigEndian(T t)
    {
        return t;
    }
#endif

}
#endif