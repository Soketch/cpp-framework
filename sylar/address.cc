#include "address.h"
#include <sstream>
#include "endian.h"

namespace sylar
{
    // 创建不同类型位掩码模板函数 : 非常巧妙
    // 使用例如：  CreateMask<uint32_t>(3);
    //                   ==>  return (1 << ( sizeof(uint32_t) * 8 - 3)) - 1
    //                   ==>  return (1 << 4*8-3) -1  即 (1<<29) -1  ==> 0x20000000 - 1  = 0x1FFFFFFF
    template <class T>
    static T CreateMask(uint32_t bits)
    {
        return (1 << (sizeof(T) * 8 - bits)) - 1;
    }

    int Address::getFamily() const
    {
        return getAddr()->sa_family;
    }

    std::string Address::toString()
    {
        std::stringstream ss;
        insert(ss);
        return ss.str();
    }

    bool Address::operator<(const Address &rhs) const
    {
        socklen_t minlen = std::min(getAddrLen(), rhs.getAddrLen());
        int result = memcmp(getAddr(), rhs.getAddr(), minlen);
        if (result < 0)
        {
            return true;
        }
        else if (result > 0)
        {
            return false;
        }
        else if (getAddrLen() < rhs.getAddrLen())
        {
            return true;
        }
        return false;
    }
    bool Address::operator==(const Address &rhs) const
    {
        return getAddrLen() == rhs.getAddrLen() && memcmp(getAddr(), rhs.getAddr(), getAddrLen()) == 0;
    }

    bool Address::operator!=(const Address &rhs) const
    {
        return !(*this == rhs);
    }

    /// @brief IPv4地址
    IPv4Address::IPv4Address(const sockaddr_in &address)
    {
        m_addr = address;
    }
    IPv4Address::IPv4Address(uint32_t address, uint32_t port)
    {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = byteswapOnLittleEndian(port); // 确保网络传输使用大端序，而主机处理数据时使用其本地字节序。
        m_addr.sin_addr.s_addr = byteswapOnLittleEndian(address);
    }

    sockaddr *IPv4Address::getAddr()
    {
        return (sockaddr *)&m_addr;
    }
    const sockaddr *IPv4Address::getAddr() const
    {
        return (sockaddr *)&m_addr;
    }

    socklen_t IPv4Address::getAddrLen() const
    {
        return sizeof(m_addr);
    }
    /// @brief 实现ipv4地址展示
    std::ostream &IPv4Address::insert(std::ostream &os) const
    {
        uint32_t addr = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);

        os << ((addr >> 24) & 0xff) << "."
           << ((addr >> 16) & 0xff) << "."
           << ((addr >> 8) & 0xff)
           << (addr & 0xff);
        os << ":" << byteswapOnLittleEndian(m_addr.sin_port);
        return os;
    }

    // broadcastAddress 生成广播地址  ==> 通过上面的createMask位掩码模板函数
    IPAddress::ptr IPv4Address::broadcastAAddress(uint32_t prefix_len)
    {
        if (prefix_len > 32)
        {
            return nullptr;
        }
        sockaddr_in baddr(m_addr);
        baddr.sin_addr.s_addr |= byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));

        return IPv4Address::ptr(new IPv4Address(baddr));
    }

    // 计算生成IPv4地址的网络地址
    IPAddress::ptr IPv4Address::networkAddress(uint32_t prefix_len)
    {
        if (prefix_len > 32)
        {
            return nullptr;
        }
        sockaddr_in naddr(m_addr);
        /// 生成掩码并计算网络地址
        naddr.sin_addr.s_addr &= byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));

        return IPv4Address::ptr(new IPv4Address(naddr));
    }

    // 子网掩码地址
    IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len)
    {
        sockaddr_in subnet;
        memset(&subnet, 0, sizeof(subnet));
        subnet.sin_family = AF_INET;
        subnet.sin_addr.s_addr = ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));

        return IPv4Address::ptr(new IPv4Address(subnet));
    }

    uint32_t IPv4Address::getPort() const
    {
        return byteswapOnLittleEndian(m_addr.sin_port);
    }

    void IPv4Address::setPort(uint16_t v)
    {
        this->m_addr.sin_port = byteswapOnLittleEndian(v);
    }

    /// @brief IPv6
    // IPv6Address::IPv6Address(const char address[16], uint32_t port)
    // {
    //     memset(&m_addr, 0, sizeof(m_addr));
    //     m_addr.sin6_family = AF_INET6;
    //     m_addr.sin6_port = byteswapOnLittleEndian(port);
    //     memcpy(&m_addr.sin6_addr.__in6_u.__u6_addr16, address, 16);
    // }
    IPv6Address::IPv6Address()
    {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin6_family = AF_INET;
    }
    IPv6Address::IPv6Address(const sockaddr_in6 &addr)
    {
        m_addr = addr;
    }
    IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port)
    {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin6_family = AF_INET6;
        m_addr.sin6_port = byteswapOnLittleEndian(port);
        memcpy(&m_addr.sin6_addr.s6_addr, address, 16);
    }
    sockaddr *IPv6Address::getAddr()
    {
        return (sockaddr *)&m_addr;
    }
    const sockaddr *IPv6Address::getAddr() const
    {
        return (sockaddr *)&m_addr;
    }

    socklen_t IPv6Address::getAddrLen() const
    {
        return sizeof(m_addr);
    }

    /* IPv6地址简述   ： IPv6 地址长度为 128 位（16 字节），通常用冒号分隔的八组十六进制数字表示。每组可以包含四个十六进制数字。  // inet6   fe80::f816:3eff:fed3:3a03
            ===>
            前导零可以省略。例如，2001:0db8:85a3:0000:0000:8a2e:0370:7334 可以简化为 2001:db8:85a3:0:0:8a2e:370:7334。
            连续的零块可以用双冒号（::）表示，但一个地址中只能使用一次（简化一次）。例如，2001:db8:0:0:0:0:2:1 可以简化为 2001:db8::2:1。
            ===>
            如果是 2001:0:0:1:0:0:2:1  因为只能简化一次 ，只能是 （1） 2001::1:0:0:2:1      这两种情况
                                                        或者是 （2） 2001:0:0:1::2:1
            如果有两组0块，且不相同，那么默认替换最长的0块
    */
    std::ostream &IPv6Address::insert(std::ostream &os) const
    {
        os << "[";
        uint16_t *addr = (uint16_t *)m_addr.sin6_addr.s6_addr;
        bool used_zeros = false;
        for (size_t i = 0; i < 8; ++i)
        {
            if (addr[i] == 0 && !used_zeros)
            {
                continue;
            }
            if (i && addr[i - 1] == 0 && !used_zeros)
            {
                os << ":";
                used_zeros = true;
            }
            if (i)
            {
                os << ":";
            }
            // 将 IPv6 地址的每个 16 位块以十六进制形式输出到输出流 os，并在输出之前将其进行了字节序转换。
            os << std::hex << (int)byteswapOnLittleEndian(addr[i]) << std::dec;
        }
        if (!used_zeros && addr[7] == 0)
        {
            os << "::";
        }
        os << "]:" << byteswapOnLittleEndian(m_addr.sin6_port);

        return os;
    }

    IPAddress::ptr IPv6Address::broadcastAAddress(uint32_t prefix_len)
    {
        sockaddr_in6 baddr(m_addr);
        baddr.sin6_addr.s6_addr[prefix_len / 8] |= CreateMask<uint8_t>(prefix_len % 8);
        for (int i = prefix_len / 8 + 1; i < 16; ++i)
        {
            baddr.sin6_addr.s6_addr[i] = 0xff;
        }
        return IPv6Address::ptr(new IPv6Address(baddr));
    }

    IPAddress::ptr IPv6Address::networkAddress(uint32_t prefix_len)
    {
        sockaddr_in6 naddr(m_addr);
        naddr.sin6_addr.s6_addr[prefix_len / 8] &= CreateMask<uint8_t>(prefix_len % 8);
        for (int i = prefix_len / 8 + 1; i < 16; ++i)
        {
            naddr.sin6_addr.s6_addr[i] = 0x00;
        }
        return IPv6Address::ptr(new IPv6Address(naddr));
    }

    IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len)
    {
        // sockaddr_in6 saddr(m_addr);
        sockaddr_in6 subnet;
        subnet.sin6_family = AF_INET6;
        memset(&subnet, 0, sizeof(subnet));

        subnet.sin6_addr.s6_addr[prefix_len / 8] = ~CreateMask<uint8_t>(prefix_len % 8);

        for (uint32_t i = 0; i < prefix_len / 8; ++i)
        {
            subnet.sin6_addr.s6_addr[i] = 0xff;
        }
        return IPv6Address::ptr(new IPv6Address(subnet));
    }

    uint32_t IPv6Address::getPort() const
    {
        return byteswapOnLittleEndian(m_addr.sin6_port);
    }

    void IPv6Address::setPort(uint16_t v)
    {
        m_addr.sin6_port = byteswapOnLittleEndian(v);
    }

    static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un *)0)->sun_path) - 1; // 最大path路径长度
    /// Unix地址
    /// 例如： /tmp/server.sock
    UnixAddress::UnixAddress()
    {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sun_family = AF_UNIX;
        m_length = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
    }
    UnixAddress::UnixAddress(const std::string &path)
    {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sun_family = AF_UNIX;
        m_length = path.length() + 1;

        if (!path.empty() && path[0] == '\0')
        {
            --m_length;
        }

        if (m_length > sizeof(m_addr.sun_path))
        {
            throw std::logic_error("path too long");
        }

        memcpy(m_addr.sun_path, path.c_str(), m_length);
        m_length += offsetof(sockaddr_un, sun_path);
    }

    const sockaddr *UnixAddress::getAddr() const
    {
        return (sockaddr *)&m_addr;
    }
    sockaddr *UnixAddress::getAddr()
    {
        return (sockaddr *)&m_addr;
    }
    socklen_t UnixAddress::getAddrLen() const
    {
        return m_length;
    }
    //
    std::ostream &UnixAddress::insert(std::ostream &os) const
    {
        if (m_length > offsetof(sockaddr_un, sun_path) &&
            m_addr.sun_path[0] == '\0')
        {
            // 特殊处理以空字符开头的地址
            return os << "\\0" << std::string(m_addr.sun_path + 1, m_length - offsetof(sockaddr_un, sun_path) - 1);
        }
        else
        {
            // 正常处理普通的 UNIX 地址
            return os << m_addr.sun_path;
        }
    }

    void UnixAddress::setAddrLen(uint32_t v)
    {
        m_length = v;
    }
    std::string UnixAddress::getPath() const
    {
        return m_addr.sun_path;
    }

    /// UnknownAddress未知地址
    UnknownAddress::UnknownAddress()
    {
        memset(&m_addr, 0, sizeof(m_addr));
    }
    UnknownAddress::UnknownAddress(int family)
    {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sa_family = family;
    }
    const sockaddr *UnknownAddress::getAddr() const
    {
        return &m_addr;
    }
    sockaddr *UnknownAddress::getAddr()
    {
        return &m_addr;
    }
    socklen_t UnknownAddress::getAddrLen() const
    {
        return sizeof(m_addr);
    }
    std::ostream &UnknownAddress::insert(std::ostream &os) const
    {
        os << "[UnknownAddress family=" << m_addr.sa_family << "]";
        return os;
    }
}