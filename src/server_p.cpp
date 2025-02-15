#include "stdafx.h"
#include "server_p.h"

#include <experimental/scope>
#include <iostream>

#include "clienthandler.h"
#include "server.h"
#include "serversocket.h"
#include "tlsexception.h"
#include "tlsservercontext.h"

ServerPrivate::ServerPrivate(
    Server* q, const ev::loop_ref& loop, const std::string& ip, std::uint16_t port,
    const std::shared_ptr<Database>& database
)
    : q_ptr(q), m_loop(loop), m_socket(ip, port), m_accept_watcher(loop), m_database(database)
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

void ServerPrivate::set_tls_context(const std::shared_ptr<TLSServerContext>& context)
{
    this->m_tls_context = context;
}

void ServerPrivate::on_accept(ev::io&, int)
{
    try {
        const int fd = this->m_socket.accept();
        std::experimental::scope_fail close_fd([fd]() { close(fd); });

        tls* client_ctx = nullptr;
        if (this->m_tls_context && tls_accept_socket(this->m_tls_context->get_context(), &client_ctx, fd) != 0) {
            throw TLSException(this->m_tls_context->get_context());
        }

        auto handler = std::make_unique<ClientHandler>(
            client_ctx, this->m_loop, this->q_ptr->shared_from_this(), this->m_database
        );
        close_fd.release();
        handler->accept(fd);

        this->m_clients[handler.get()] = std::move(handler);
    }
    catch (std::system_error& e) {
        std::cerr << std::format(
            "Error: failed to accept connection: {}: {} ({})\n", e.what(), e.code().message(), e.code().value()
        );
    }
    catch (TLSException& e) {
        std::cerr << std::format("Error: failed to accept connection: {}\n", e.what());
    }
}
