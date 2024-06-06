#ifndef __SYLAR_ADDRESS_H_
#define __SYLAR_ADDRESS_H_

#include <memory>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/un.h>
#include <sstream>

/// @brief 网络地址的封装(IPv4,IPv6,Unix)
namespace sylar
{
    /**
     * @brief 网络地址的基类,抽象类
     */
    class Address
    {
    public:
        using ptr = std::shared_ptr<Address>;
        Address() {}
        virtual ~Address() {}

        // 返回协议簇
        int getFamily() const;
        /// 返回sockaddr指针,只读

        virtual const sockaddr *getAddr() const = 0;
        /// @brief 返回sockaddr指针,读写
        virtual sockaddr *getAddr() = 0;

        // 返回sockaddr的长度
        virtual socklen_t getAddrLen() const = 0;

        ///  可读性输出地址
        virtual std::ostream &insert(std::ostream &os) const;

        /// @brief 返回可读性字符串
        std::string toString();

        // 重载<  ，小于号比较函数
        bool operator<(const Address &rhs) const;
        ///
        bool operator==(const Address &rhs) const;
        ///
        bool operator!=(const Address &rhs) const;

    private:
    };

    // IP地址的基类
    class IPAddress : public Address
    {
    public:
        using ptr = std::shared_ptr<IPAddress>;

        /**
         * @brief 获取该地址的广播地址
         * @param[in] prefix_len 子网掩码位数
         * @return 调用成功返回IPAddress,失败返回nullptr
         */
        virtual IPAddress::ptr broadcastAAddress(uint32_t prefix_len) = 0;
        /**
         * @brief 获取该地址的网段
         * @param[in] prefix_len 子网掩码位数
         * @return 调用成功返回IPAddress,失败返回nullptr
         */
        virtual IPAddress::ptr networkAddress(uint32_t prefix_len) = 0;
        /**
         * @brief 获取子网掩码地址
         * @param[in] prefix_len 子网掩码位数
         * @return 调用成功返回IPAddress,失败返回nullptr
         */
        virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

        /// @brief 获取端口号
        virtual uint32_t getPort() const = 0;
        virtual void setPort(uint16_t v) = 0;

    private:
    };

    /// @brief IPv4地址
    class IPv4Address : public IPAddress
    {
    public:
        using ptr = std::shared_ptr<IPv4Address>;
        IPv4Address(uint32_t address = INADDR_ANY, uint32_t port = 0);

        /// 返回sockaddr指针,只读
        const sockaddr *getAddr() const override;
        /// 返回sockaddr的长度
        socklen_t getAddrLen() const override;
        ///  可读性输出地址
        std::ostream &insert(std::ostream &os) const override;

        /// @brief 获取该地址的广播地址
        IPAddress::ptr broadcastAAddress(uint32_t prefix_len) override;
        /// @brief 获取该地址的网段
        IPAddress::ptr networkAddress(uint32_t prefix_len) override;
        /// @brief 获取子网掩码地址
        IPAddress::ptr subnetMask(uint32_t prefix_len) override;
        /// @brief 获取端口号
        uint32_t getPort() const override;
        /// @brief 设置端口号
        void setPort(uint16_t v) override;

    private:
        sockaddr_in m_addr;
    };

    /// @brief IPv6地址
    class IPv6Address : public IPAddress
    {
    public:
        using ptr = std::shared_ptr<IPv6Address>;
        IPv6Address(uint32_t address = INADDR_ANY, uint32_t port = 0);

        /// 返回sockaddr指针,只读
        const sockaddr *getAddr() const override;
        /// 返回sockaddr的长度
        socklen_t getAddrLen() const override;
        ///  可读性输出地址
        std::ostream &insert(std::ostream &os) const override;

        /// @brief 获取该地址的广播地址
        IPAddress::ptr broadcastAAddress(uint32_t prefix_len) override;
        /// @brief 获取该地址的网段
        IPAddress::ptr networkAddress(uint32_t prefix_len) override;
        /// @brief 获取子网掩码地址
        IPAddress::ptr subnetMask(uint32_t prefix_len) override;
        /// @brief 获取端口号
        uint32_t getPort() const override;
        /// @brief 设置端口号
        void setPort(uint16_t v) override;

    private:
        sockaddr_in6 m_addr; // ipv6的addr
    };

    /// @brief UnixSocket地址
    class UnixAddress : public Address
    {
    public:
        using ptr = std::shared_ptr<UnixAddress>;
        UnixAddress();

        /**
         * @brief 通过路径构造UnixAddress
         * @param[in] path UnixSocket路径(长度小于UNIX_PATH_MAX)
         */
        UnixAddress(const std::string &path);

        const sockaddr *getAddr() const override;
        sockaddr *getAddr() override;
        socklen_t getAddrLen() const override;
        std::ostream &insert(std::ostream &os) const override;

        void setAddrLen(uint32_t v);
        std::string getPath() const;

    private:
        sockaddr_un m_addr;
        socklen_t m_length;
    };

    /// @brief 未知地址
    class UnknownAddress : public Address
    {
    public:
        using ptr = std::shared_ptr<UnknownAddress>;
        UnknownAddress();
        const sockaddr *getAddr() const override;
        sockaddr *getAddr() override;
        socklen_t getAddrLen() const override;
        std::ostream &insert(std::ostream &os) const override;

    private:
        sockaddr m_addr;
    };
}
#endif