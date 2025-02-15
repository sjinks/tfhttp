#include "stdafx.h"
#include "serversocket_p.h"
#include "sockaddr.h"

ServerSocketPrivate::ServerSocketPrivate(const std::string& ip, std::uint16_t port)
    : m_socket(socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP))
{
    if (this->m_socket < 0) [[unlikely]] {
        throw std::system_error(errno, std::system_category(), "socket() failed");
    }

    const int on      = 1;
    const int timeout = 1;
    setsockopt(this->m_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    setsockopt(this->m_socket, SOL_IP, IP_FREEBIND, &on, sizeof(int));
    setsockopt(this->m_socket, IPPROTO_TCP, TCP_DEFER_ACCEPT, &timeout, sizeof(int));

    SockAddr s{};

    if (inet_pton(AF_INET, ip.c_str(), &s.as_sockaddr_in()->sin_addr) == 1) {
        s.as_sockaddr_in()->sin_family = AF_INET;
        s.as_sockaddr_in()->sin_port   = htons(port);
    }
    else if (inet_pton(AF_INET6, ip.c_str(), &s.as_sockaddr_in6()->sin6_addr) == 1) {
        s.as_sockaddr_in6()->sin6_family = AF_INET6;
        s.as_sockaddr_in6()->sin6_port   = htons(port);
    }
    else [[unlikely]] {
        throw std::invalid_argument("Invalid IP address");
    }

    if (const int res = bind(this->m_socket, s.as_sockaddr(), s.size()); res < 0) [[unlikely]] {
        throw std::system_error(errno, std::system_category(), "bind() failed");
    }
}

ServerSocketPrivate::~ServerSocketPrivate()
{
    if (this->m_socket >= 0) [[likely]] {
        close(this->m_socket);
    }
}

std::uint16_t ServerSocketPrivate::listen() const
{
    if (::listen(this->m_socket, ServerSocketPrivate::BACKLOG_SIZE) < 0) [[unlikely]] {
        throw std::system_error(errno, std::system_category(), "listen() failed");
    }

    return this->get_port();
}

int ServerSocketPrivate::accept() const
{
    const int res = accept4(this->m_socket, nullptr, nullptr, SOCK_NONBLOCK);
    if (res < 0) [[unlikely]] {
        throw std::system_error(errno, std::system_category(), "accept4() failed");
    }

    return res;
}

int ServerSocketPrivate::get_socket() const noexcept
{
    return this->m_socket;
}

std::uint16_t ServerSocketPrivate::get_port() const
{
    SockAddr addr{};

    if (socklen_t addr_len = addr.size(); getsockname(this->m_socket, addr.as_sockaddr(), &addr_len) < 0) [[unlikely]] {
        throw std::system_error(errno, std::system_category(), "getsockname() failed");
    }

    switch (addr.as_sockaddr()->sa_family) {
        case AF_INET:
            return ntohs(addr.as_sockaddr_in()->sin_port);

        case AF_INET6:
            return ntohs(addr.as_sockaddr_in6()->sin6_port);

        default:
            throw std::invalid_argument("Unknown address family");
    }
}
