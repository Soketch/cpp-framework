#include "address.h"
#include <sstream>
#include "endian.h"

namespace sylar
{
    int Address::getFamily() const
    {
        return getAddr()->sa_family;
    }

    const sockaddr *Address::getAddr() const
    {
    }

    sockaddr *Address::getAddr()
    {
    }

    socklen_t Address::getAddrLen() const
    {
    }

    std::ostream &Address::insert(std::ostream &os) const
    {
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

    /// @brief IPv4
    IPv4Address::IPv4Address(uint32_t address, uint32_t port)
    {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = byteswapOnLittleEndian(port);
        m_addr.sin_addr.s_addr = byteswapOnLittleEndian(address);
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

    IPAddress::ptr IPv4Address::broadcastAAddress(uint32_t prefix_len)
    {
        if (prefix_len > 32)
        {
            return nullptr;
        }
        sockaddr_in baddr(m_addr);
        // baddr.sin_addr.s_addr |=
    }

    IPAddress::ptr IPv4Address::networkAddress(uint32_t prefix_len)
    {
    }

    IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len)
    {
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
    IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port)
    {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin6_family = AF_INET6;
        m_addr.sin6_port = byteswapOnLittleEndian(port);
        memcpy(&m_addr.sin6_addr.s6_addr, address, 16);
    }
    const sockaddr *IPv6Address::getAddr() const
    {
    }

    socklen_t IPv6Address::getAddrLen() const
    {
    }

    std::ostream &IPv6Address::insert(std::ostream &os) const
    {
    }

    IPAddress::ptr IPv6Address::broadcastAAddress(uint32_t prefix_len)
    {
    }

    IPAddress::ptr IPv6Address::networkAddress(uint32_t prefix_len)
    {
    }

    IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len)
    {
    }

    uint32_t IPv6Address::getPort() const
    {
    }

    void IPv6Address::setPort(uint16_t v)
    {
    }

    /// UnixAddress
    UnixAddress::UnixAddress(const std::string &path)
    {
    }

    const sockaddr *UnixAddress::getAddr() const
    {
    }
    sockaddr *UnixAddress::getAddr()
    {
    }
    socklen_t UnixAddress::getAddrLen() const
    {
    }
    std::ostream &UnixAddress::insert(std::ostream &os) const
    {
    }

    void UnixAddress::setAddrLen(uint32_t v)
    {
    }
    std::string UnixAddress::getPath() const
    {
    }

    /// UnknownAddress未知地址
    UnknownAddress::UnknownAddress()
    {
    }
    const sockaddr *UnknownAddress::getAddr() const
    {
    }
    sockaddr *UnknownAddress::getAddr()
    {
    }
    socklen_t UnknownAddress::getAddrLen() const
    {
    }
    std::ostream &UnknownAddress::insert(std::ostream &os) const
    {
    }
}