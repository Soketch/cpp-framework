#include "address.h"

namespace sylar
{
    int Address::getFamily() const
    {
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
    }

    bool Address::operator<(const Address &rhs) const
    {
    }
    bool Address::operator==(const Address &rhs) const
    {
    }

    bool Address::operator!=(const Address &rhs) const
    {
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
}