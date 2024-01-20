#include "stdafx.h"
#include "server.h"
#include "clienthandler.h"
#include "server_p.h"

std::shared_ptr<Server> Server::create(
    const ev::loop_ref& loop, const std::string& address, std::uint16_t port, const std::shared_ptr<Database>& database
)
{
    return std::make_shared<Server>(Server::PrivateTag{}, loop, address, port, database);
}

Server::Server(
    const Server::PrivateTag&, const ev::loop_ref& loop, const std::string& address, std::uint16_t port,
    const std::shared_ptr<Database>& database
)
    : d_ptr(std::make_unique<ServerPrivate>(this, loop, address, port, database))
{}

Server::~Server() = default;

std::uint16_t Server::run()
{
    return this->d_func()->run();
}

void Server::remove_handler(ClientHandler* handler)
{
    this->d_func()->remove_handler(handler);
}

void Server::set_tls_context(const std::shared_ptr<TLSServerContext>& context)
{
    this->d_func()->set_tls_context(context);
}
