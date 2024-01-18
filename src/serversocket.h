#ifndef FF0A02A2_8C00_483D_B3A6_557384574EF4
#define FF0A02A2_8C00_483D_B3A6_557384574EF4

#include <cstdint>
#include <memory>

class ServerSocketPrivate;
class TLSServerContext;

class ServerSocket {
public:
    ServerSocket(const std::string& ip, std::uint16_t port);
    ServerSocket(const ServerSocket&)                = delete;
    ServerSocket(ServerSocket&&) noexcept            = default;
    ServerSocket& operator=(const ServerSocket&)     = delete;
    ServerSocket& operator=(ServerSocket&&) noexcept = default;
    ~ServerSocket();

    std::uint16_t listen() const;  // NOLINT(*-use-nodiscard)
    [[nodiscard]] int accept() const;
    [[nodiscard]] int get_socket() const noexcept;

private:
    std::unique_ptr<ServerSocketPrivate> d_ptr;

    [[nodiscard]] inline ServerSocketPrivate* d_func() { return this->d_ptr.get(); }
    [[nodiscard]] inline const ServerSocketPrivate* d_func() const { return this->d_ptr.get(); }
};

#endif /* FF0A02A2_8C00_483D_B3A6_557384574EF4 */
