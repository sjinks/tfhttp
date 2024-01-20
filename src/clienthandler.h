#ifndef A5A4EBDE_9FD6_4A23_9E9A_2E95E451A8C8
#define A5A4EBDE_9FD6_4A23_9E9A_2E95E451A8C8

#include <memory>
#include <ev++.h>

class ClientHandlerPrivate;
class Database;
class Server;
struct tls;

class ClientHandler {
public:
    ClientHandler(
        tls* ctx, const ev::loop_ref& loop, const std::shared_ptr<Server>& server,
        const std::shared_ptr<Database>& database
    );
    ClientHandler(const ClientHandler&)            = delete;
    ClientHandler(ClientHandler&&)                 = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;
    ClientHandler& operator=(ClientHandler&&)      = delete;
    ~ClientHandler();

    int accept(int fd);

private:
    std::unique_ptr<ClientHandlerPrivate> d_ptr;

    [[nodiscard]] inline ClientHandlerPrivate* d_func() { return this->d_ptr.get(); }
    [[nodiscard]] inline const ClientHandlerPrivate* d_func() const { return this->d_ptr.get(); }
};

#endif /* A5A4EBDE_9FD6_4A23_9E9A_2E95E451A8C8 */
