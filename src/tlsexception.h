#ifndef BE09C4E0_7C06_4D7A_886A_07FCAB12222C
#define BE09C4E0_7C06_4D7A_886A_07FCAB12222C

#include <stdexcept>
#include <tls.h>

class TLSException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    using std::runtime_error::operator=;

    explicit TLSException(tls* ctx) : std::runtime_error(tls_error(ctx)) {}
    explicit TLSException(tls_config* config) : std::runtime_error(tls_config_error(config)) {}
    TLSException(const std::string& prefix, tls* ctx) : std::runtime_error(prefix + ": " + tls_error(ctx)) {}
    TLSException(const std::string& prefix, tls_config* config)
        : std::runtime_error(prefix + ": " + tls_config_error(config))
    {}

    ~TLSException() override;
};

#endif /* BE09C4E0_7C06_4D7A_886A_07FCAB12222C */
