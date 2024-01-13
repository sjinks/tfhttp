#include "stdafx.h"
#include "serversocket.h"
#include "serversocket_p.h"

ServerSocket::ServerSocket(const std::string& ip, std::uint16_t port)
    : d_ptr(std::make_unique<ServerSocketPrivate>(ip, port))
{}

ServerSocket::~ServerSocket() = default;

std::uint16_t ServerSocket::listen() const
{
    return this->d_func()->listen();
}

int ServerSocket::accept() const
{
    return this->d_func()->accept();
}

int ServerSocket::get_socket() const noexcept
{
    return this->d_func()->get_socket();
}
