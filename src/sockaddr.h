#ifndef DE2188D7_6772_4CA1_A913_3F64E1C5610B
#define DE2188D7_6772_4CA1_A913_3F64E1C5610B

#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>

class SockAddr {
public:
    SockAddr() noexcept { std::memset(&this->addr, 0, sizeof(this->addr)); }

    // In a standard-layout union with an active member of struct type T1, it is permitted to read a non-static data
    // member m of another union member of struct type T2 provided m is part of the common initial sequence of T1 and
    // T2.
    sockaddr* as_sockaddr() noexcept { return &this->addr.sa; }
    sockaddr_in* as_sockaddr_in() noexcept { return &this->addr.sa_in; }
    sockaddr_in6* as_sockaddr_in6() noexcept { return &this->addr.sa_in6; }

    [[nodiscard]] consteval socklen_t size() const noexcept { return static_cast<socklen_t>(sizeof(this->addr)); }

private:
    union {
        sockaddr sa;
        sockaddr_in sa_in;
        sockaddr_in6 sa_in6;
    } addr{};
};

#endif /* DE2188D7_6772_4CA1_A913_3F64E1C5610B */
