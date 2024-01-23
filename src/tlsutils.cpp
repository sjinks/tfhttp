#include "stdafx.h"
#include "tlsutils.h"

namespace {

int handle_tls_result(tls* ctx [[gnu::nonnull]], ssize_t num, std::string& error)
{
    if (num == TLS_WANT_POLLIN) {
        return ev::READ;
    }

    if (num == TLS_WANT_POLLOUT) {
        return ev::WRITE;
    }

    if (num < 0) [[unlikely]] {
        error = tls_error(ctx);
    }

    return ev::NONE;
}

}  // namespace

ssize_t u_read(tls* ctx, int fd, std::span<char> buf, std::string& error, int& new_events)
{
    if (ctx != nullptr) {
        const ssize_t num = tls_read(ctx, buf.data(), buf.size_bytes());
        new_events        = handle_tls_result(ctx, num, error);
        return num;
    }

    const ssize_t num = recv(fd, buf.data(), buf.size_bytes(), 0);
    if (num < 0) [[unlikely]] {
        error = std::error_code(errno, std::system_category()).message();
    }

    new_events = ev::NONE;
    return num;
}

ssize_t u_write(tls* ctx, int fd, std::span<const char> buf, std::string& error, int& new_events)
{
    if (ctx != nullptr) {
        const ssize_t num = tls_write(ctx, buf.data(), buf.size_bytes());
        new_events        = handle_tls_result(ctx, num, error);
        return num;
    }

    const ssize_t num = send(fd, buf.data(), buf.size_bytes(), 0);
    if (num < 0) [[unlikely]] {
        error = std::error_code(errno, std::system_category()).message();
    }

    new_events = ev::NONE;
    return num;
}
