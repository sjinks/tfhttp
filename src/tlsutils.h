#ifndef A37AF032_C5EF_4C99_9772_3BFFC9B854AF
#define A37AF032_C5EF_4C99_9772_3BFFC9B854AF

#include <cstddef>
#include <span>
#include <string>

static_assert(ev::NONE == 0, "ev::NONE must be 0");

struct tls;

ssize_t u_read(tls* ctx, int fd, std::span<char> buf, std::string& error, int& new_events);
ssize_t u_write(tls* ctx, int fd, std::span<const char> buf, std::string& error, int& new_events);

#endif /* A37AF032_C5EF_4C99_9772_3BFFC9B854AF */
