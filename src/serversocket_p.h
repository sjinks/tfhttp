#ifndef DB87EE9A_0FED_4AD3_9581_38F65C1AB73D
#define DB87EE9A_0FED_4AD3_9581_38F65C1AB73D

#include <cstdint>

class ServerSocketPrivate {
public:
    ServerSocketPrivate(const std::string& ip, std::uint16_t port);
    ServerSocketPrivate(const ServerSocketPrivate&)             = delete;
    ServerSocketPrivate(ServerSocketPrivate&& other)            = delete;
    ServerSocketPrivate& operator=(const ServerSocketPrivate&)  = delete;
    ServerSocketPrivate& operator=(ServerSocketPrivate&& other) = delete;
    ~ServerSocketPrivate();

    std::uint16_t listen() const;  // NOLINT(*-use-nodiscard)
    [[nodiscard]] int accept() const;
    [[nodiscard]] int get_socket() const noexcept;

private:
    int m_socket = -1;

    static constexpr const int BACKLOG_SIZE = 512;

    std::uint16_t get_port() const;  // NOLINT(*-use-nodiscard)
};

#endif /* DB87EE9A_0FED_4AD3_9581_38F65C1AB73D */
