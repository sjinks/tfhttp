#ifndef CD13637D_482E_4586_8DF9_CAB4DF7466AA
#define CD13637D_482E_4586_8DF9_CAB4DF7466AA

#include <cstdint>
#include <memory>
#include <ev++.h>

class ClientHandler;
class Database;
class ServerPrivate;
class TLSServerContext;

class Server : public std::enable_shared_from_this<Server> {
private:
    struct PrivateTag {};

public:
    static std::shared_ptr<Server> create(
        const ev::loop_ref& loop, const std::string& address, std::uint16_t port,
        const std::shared_ptr<Database>& database
    );

    Server(
        const PrivateTag&, const ev::loop_ref& loop, const std::string& address, std::uint16_t port,
        const std::shared_ptr<Database>& database
    );
    Server(const Server&)            = delete;
    Server(Server&&)                 = delete;
    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&)      = delete;
    ~Server();

    std::uint16_t run();
    void remove_handler(ClientHandler* handler);

    void set_tls_context(const std::shared_ptr<TLSServerContext>& context);

private:
    std::unique_ptr<ServerPrivate> d_ptr;

    [[nodiscard]] inline ServerPrivate* d_func() { return this->d_ptr.get(); }
    [[nodiscard]] inline const ServerPrivate* d_func() const { return this->d_ptr.get(); }
};

#endif /* CD13637D_482E_4586_8DF9_CAB4DF7466AA */
