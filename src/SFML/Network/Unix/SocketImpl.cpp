#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network/SocketImpl.hpp>

#include <SFML/System/Err.hpp>

#include <fcntl.h>

#include <cerrno>
#include <cstdint>
#include <cstring>

// TODO P1: major repetition with Win32 impl


namespace sf::priv
{
////////////////////////////////////////////////////////////
SockAddrIn::SockAddrIn() : m_impl(sockaddr_in{})
{
}


////////////////////////////////////////////////////////////
SockAddrIn::~SockAddrIn() = default;


////////////////////////////////////////////////////////////
SockAddrIn::SockAddrIn(const SockAddrIn&) = default;


////////////////////////////////////////////////////////////
SockAddrIn::SockAddrIn(const sockaddr_in& in) : m_impl(in)
{
}


////////////////////////////////////////////////////////////
unsigned short SockAddrIn::sinPort() const
{
    return m_impl->sin_port;
}


////////////////////////////////////////////////////////////
unsigned long SockAddrIn::sAddr() const
{
    return m_impl->sin_addr.s_addr;
}


////////////////////////////////////////////////////////////
AddrLength SockAddrIn::size() const
{
    return sizeof(sockaddr_in);
}


////////////////////////////////////////////////////////////
SockAddrIn SocketImpl::createAddress(std::uint32_t address, unsigned short port)
{
    auto addr            = sockaddr_in();
    addr.sin_addr.s_addr = priv::SocketImpl::htonl(address);
    addr.sin_family      = AF_INET;
    addr.sin_port        = priv::SocketImpl::htons(port);

#if defined(SFML_SYSTEM_MACOS)
    addr.sin_len = sizeof(addr);
#endif

    return addr;
}


////////////////////////////////////////////////////////////
std::uint32_t SocketImpl::inaddrAny()
{
    return INADDR_ANY;
}


////////////////////////////////////////////////////////////
std::uint32_t SocketImpl::inaddrLoopback()
{
    return INADDR_LOOPBACK;
}


////////////////////////////////////////////////////////////
SocketHandle SocketImpl::accept(SocketHandle handle, SockAddrIn& address, AddrLength& length)
{
    return ::accept(handle, reinterpret_cast<sockaddr*>(&*address.m_impl), &length);
}


////////////////////////////////////////////////////////////
bool SocketImpl::listen(SocketHandle handle)
{
    return ::listen(handle, SOMAXCONN) != -1;
}


////////////////////////////////////////////////////////////
bool SocketImpl::getSockName(SocketHandle handle, SockAddrIn& address, AddrLength& length)
{
    return ::getsockname(handle, reinterpret_cast<sockaddr*>(&*address.m_impl), &length) != -1;
}


////////////////////////////////////////////////////////////
bool SocketImpl::getPeerName(SocketHandle handle, SockAddrIn& address, AddrLength& length)
{
    return ::getpeername(handle, reinterpret_cast<sockaddr*>(&*address.m_impl), &length) != -1;
}


////////////////////////////////////////////////////////////
bool SocketImpl::bind(SocketHandle handle, SockAddrIn& address)
{
    return ::bind(handle, reinterpret_cast<sockaddr*>(&*address.m_impl), address.size()) != -1;
}


////////////////////////////////////////////////////////////
bool SocketImpl::connect(SocketHandle handle, SockAddrIn& address)
{
    return ::connect(handle, reinterpret_cast<sockaddr*>(&*address.m_impl), address.size()) != -1;
}


////////////////////////////////////////////////////////////
unsigned long SocketImpl::ntohl(unsigned long netlong)
{
    return ::ntohl(netlong);
}


////////////////////////////////////////////////////////////
unsigned short SocketImpl::ntohs(unsigned short netshort)
{
    return ::ntohs(netshort);
}


////////////////////////////////////////////////////////////
unsigned long SocketImpl::ntohl(SockAddrIn addr)
{
    return ::ntohl(addr.m_impl->sin_addr.s_addr);
}


////////////////////////////////////////////////////////////
unsigned short SocketImpl::htons(unsigned short hostshort)
{
    return ::htons(hostshort);
}


////////////////////////////////////////////////////////////
unsigned long SocketImpl::htonl(unsigned long hostlong)
{
    return ::htonl(hostlong);
}


////////////////////////////////////////////////////////////
SocketHandle SocketImpl::invalidSocket()
{
    return -1;
}


////////////////////////////////////////////////////////////
void SocketImpl::close(SocketHandle sock)
{
    SFML_BASE_ASSERT(sock != invalidSocket());
    ::close(sock);
}


////////////////////////////////////////////////////////////
void SocketImpl::setBlocking(SocketHandle sock, bool block)
{
    const int  status    = fcntl(sock, F_GETFL);
    const auto blockFlag = block ? (status & ~O_NONBLOCK) : (status | O_NONBLOCK);

    if (fcntl(sock, F_SETFL, blockFlag) == -1)
        priv::err() << "Failed to set file status flags: " << errno;
}


////////////////////////////////////////////////////////////
Socket::Status SocketImpl::getErrorStatus()
{
    // The following are sometimes equal to EWOULDBLOCK,
    // so we have to make a special case for them in order
    // to avoid having double values in the switch case
    if ((errno == EAGAIN) || (errno == EINPROGRESS))
        return Socket::Status::NotReady;

    // clang-format off
    switch (errno)
    {
        case EWOULDBLOCK:  return Socket::Status::NotReady;
        case ECONNABORTED: return Socket::Status::Disconnected;
        case ECONNRESET:   return Socket::Status::Disconnected;
        case ETIMEDOUT:    return Socket::Status::Disconnected;
        case ENETRESET:    return Socket::Status::Disconnected;
        case ENOTCONN:     return Socket::Status::Disconnected;
        case EPIPE:        return Socket::Status::Disconnected;
        default:           return Socket::Status::Error;
    }
    // clang-format on
}


////////////////////////////////////////////////////////////
base::Optional<std::uint32_t> SocketImpl::inetAddr(const char* data)
{
    const std::uint32_t ip = ::inet_addr(data);
    return ip == INADDR_NONE ? base::nullOpt : base::makeOptional<std::uint32_t>(ip);
}


////////////////////////////////////////////////////////////
const char* SocketImpl::addrToString(std::uint32_t addr)
{
    in_addr address{};
    address.s_addr = addr;

    return inet_ntoa(address);
}


////////////////////////////////////////////////////////////
SocketHandle SocketImpl::tcpSocket()
{
    return ::socket(PF_INET, SOCK_STREAM, 0);
}


////////////////////////////////////////////////////////////
SocketHandle SocketImpl::udpSocket()
{
    return ::socket(PF_INET, SOCK_DGRAM, 0);
}


////////////////////////////////////////////////////////////
bool SocketImpl::disableNagle(SocketHandle handle)
{
    int yes = 1;
    return setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&yes), sizeof(yes)) != -1;
}


////////////////////////////////////////////////////////////
bool SocketImpl::disableSigpipe([[maybe_unused]] SocketHandle handle)
{
#ifdef SFML_SYSTEM_MACOS
    int yes = 1;
    return setsockopt(handle, SOL_SOCKET, SO_NOSIGPIPE, reinterpret_cast<char*>(&yes), sizeof(yes)) != -1;
#else
    return true;
#endif
}

////////////////////////////////////////////////////////////
bool SocketImpl::enableBroadcast(SocketHandle handle)
{
    int yes = 1;
    return static_cast<bool>(setsockopt(handle, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&yes), sizeof(yes)) != -1);
}


////////////////////////////////////////////////////////////
int SocketImpl::send(SocketHandle handle, const char* buf, int len, int flags)
{
    return ::send(handle, buf, static_cast<std::size_t>(len), flags);
}


////////////////////////////////////////////////////////////
int SocketImpl::sendTo(SocketHandle handle, const char* buf, int len, int flags, SockAddrIn& address)
{
    return ::sendto(handle,
                    buf,
                    static_cast<std::size_t>(len),
                    flags,
                    reinterpret_cast<sockaddr*>(&*address.m_impl),
                    address.size());
}

////////////////////////////////////////////////////////////
int SocketImpl::recv(SocketHandle handle, char* buf, int len, int flags)
{
    return ::recv(handle, buf, static_cast<std::size_t>(len), flags);
}


////////////////////////////////////////////////////////////
int SocketImpl::recvFrom(SocketHandle handle, char* buf, int len, int flags, SockAddrIn& address, AddrLength& length)
{
    return ::recvfrom(handle, buf, static_cast<std::size_t>(len), flags, reinterpret_cast<sockaddr*>(&*address.m_impl), &length);
}


////////////////////////////////////////////////////////////
base::Optional<unsigned long> SocketImpl::convertToHostname(const char* address)
{
    addrinfo hints{}; // Zero-initialize
    hints.ai_family = AF_INET;

    addrinfo* result = nullptr;
    if (getaddrinfo(address, nullptr, &hints, &result) == 0 && result != nullptr)
    {
        sockaddr_in sin{};
        std::memcpy(&sin, result->ai_addr, sizeof(*result->ai_addr));

        const std::uint32_t ip = sin.sin_addr.s_addr;
        freeaddrinfo(result);

        return base::makeOptional<unsigned long>(ip);
    }

    return base::nullOpt;
}


////////////////////////////////////////////////////////////
int SocketImpl::select(SocketHandle handle, long long timeoutUs)
{
    // Setup the selector
    fd_set selector;
    FD_ZERO(&selector);
    FD_SET(handle, &selector);

    // Setup the timeout
    timeval time{};
    time.tv_sec  = static_cast<long>(timeoutUs / 1000000);
    time.tv_usec = static_cast<int>(timeoutUs % 1000000);

    // Wait for something to write on our socket (which means that the connection request has returned)
    return ::select(static_cast<int>(handle + 1), nullptr, &selector, nullptr, &time);
}


////////////////////////////////////////////////////////////
bool SocketImpl::fdIsSet(SocketHandle handle, const FDSet& fdSet)
{
    return FD_ISSET(handle, static_cast<const fd_set*>(fdSet.asPtr()));
}

////////////////////////////////////////////////////////////
void SocketImpl::fdClear(SocketHandle handle, FDSet& fdSet)
{
    FD_CLR(handle, static_cast<fd_set*>(fdSet.asPtr()));
}


////////////////////////////////////////////////////////////
void SocketImpl::fdZero(FDSet& fdSet)
{
    FD_ZERO(static_cast<fd_set*>(fdSet.asPtr()));
}


////////////////////////////////////////////////////////////
int SocketImpl::getFDSetSize()
{
    return FD_SETSIZE;
}


////////////////////////////////////////////////////////////
void SocketImpl::fdSet(SocketHandle handle, FDSet& fdSet)
{
    FD_SET(handle, static_cast<fd_set*>(fdSet.asPtr()));
}


////////////////////////////////////////////////////////////
int SocketImpl::select(int nfds, FDSet* readfds, FDSet* writefds, FDSet* exceptfds, long long timeoutUs)
{
    // Setup the timeout
    timeval time{};
    time.tv_sec  = static_cast<long>(timeoutUs / 1000000);
    time.tv_usec = static_cast<int>(timeoutUs % 1000000);

    return ::select(nfds,
                    static_cast<fd_set*>(readfds->asPtr()),
                    static_cast<fd_set*>(writefds->asPtr()),
                    static_cast<fd_set*>(exceptfds->asPtr()),
                    timeoutUs == 0ll ? nullptr : &time);
}

////////////////////////////////////////////////////////////
struct FDSet::Impl
{
    fd_set set{};
};


////////////////////////////////////////////////////////////
FDSet::FDSet() = default;


////////////////////////////////////////////////////////////
FDSet::~FDSet() = default;


////////////////////////////////////////////////////////////
FDSet::FDSet(const FDSet&) = default;


////////////////////////////////////////////////////////////
FDSet& FDSet::operator=(const FDSet&) = default;


////////////////////////////////////////////////////////////
void* FDSet::asPtr()
{
    return &m_impl->set;
}


////////////////////////////////////////////////////////////
const void* FDSet::asPtr() const
{
    return &m_impl->set;
}


} // namespace sf::priv
