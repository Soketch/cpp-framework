#include "address.h"

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
    }

    const sockaddr *IPv4Address::getAddr() const
    {
    }

    socklen_t IPv4Address::getAddrLen() const
    {
    }

    std::ostream &IPv4Address::insert(std::ostream &os) const
    {
    }

    IPAddress::ptr IPv4Address::broadcastAAddress(uint32_t prefix_len)
    {
    }

    IPAddress::ptr IPv4Address::networkAddress(uint32_t prefix_len)
    {
    }

    IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len)
    {
    }

    uint32_t IPv4Address::getPort() const
    {
    }

    void IPv4Address::setPort(uint16_t v)
    {
    }

    /// @brief IPv6
    IPv6Address::IPv6Address(uint32_t address, uint32_t port)
    {
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

    // UnknownAddress

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