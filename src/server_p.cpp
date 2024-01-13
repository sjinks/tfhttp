#include "stdafx.h"
#include "server_p.h"
#include "clienthandler.h"
#include "server.h"
#include "serversocket.h"

ServerPrivate::ServerPrivate(
    Server* q_ptr, const ev::loop_ref& loop, const std::string& ip, std::uint16_t port,
    const std::shared_ptr<Database>& database
)
    : q_ptr(q_ptr), m_loop(loop), m_socket(ip, port), m_accept_watcher(loop), m_database(database)
{}

std::uint16_t ServerPrivate::run()
{
    const std::uint16_t res = this->m_socket.listen();
    this->m_accept_watcher.set<ServerPrivate, &ServerPrivate::on_accept>(this);
    this->m_accept_watcher.start(this->m_socket.get_socket(), ev::READ);
    return res;
}

void ServerPrivate::remove_handler(const ClientHandler* handler)
{
    this->m_clients.erase(handler);
}

void ServerPrivate::on_accept(ev::io&, int)
{
    try {
        const int fd = this->m_socket.accept();
        auto handler = std::make_unique<ClientHandler>(this->m_loop, this->q_ptr->shared_from_this(), this->m_database);
        handler->accept(fd);

        this->m_clients[handler.get()] = std::move(handler);
    }
    catch (std::system_error& e) {
        std::cerr << std::format(
            "Error: failed to accept connection: {}: {} ({})\n", e.what(), e.code().message(), e.code().value()
        );
    }
}
