#ifndef A3336B4C_9E58_4EEB_B42E_36AC2E4200B4
#define A3336B4C_9E58_4EEB_B42E_36AC2E4200B4

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <ev++.h>
#include "serversocket.h"

class ClientHandler;
class Database;
class Server;
class TLSServerContext;

class ServerPrivate {
public:
    ServerPrivate(
        Server* q, const ev::loop_ref& loop, const std::string& ip, std::uint16_t port,
        const std::shared_ptr<Database>& database
    );

    std::uint16_t run();
    void remove_handler(const ClientHandler* handler);

    void set_tls_context(const std::shared_ptr<TLSServerContext>& context);

private:
    Server* q_ptr;
    ev::loop_ref m_loop;
    ServerSocket m_socket;
    ev::io m_accept_watcher;
    std::shared_ptr<Database> m_database;
    std::unordered_map<ClientHandler const*, std::unique_ptr<ClientHandler>> m_clients;
    std::shared_ptr<TLSServerContext> m_tls_context{};

    void on_accept(ev::io& watcher, int revents);
};

#endif /* A3336B4C_9E58_4EEB_B42E_36AC2E4200B4 */
