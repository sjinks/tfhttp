#ifndef AA7383D0_F723_456B_A3A6_98A8737337D5
#define AA7383D0_F723_456B_A3A6_98A8737337D5

#include <memory>
#include <string>
#include <tls.h>
#include "tlsutils.h"

class TLSServerContextPrivate {
public:
    TLSServerContextPrivate();

    void set_keypair(const std::string& cert, const std::string& key);
    void set_ca(const std::string& ca);
    void set_ocsp_staple(const std::string& ocsp);
    void set_verify_client();
    void enable_dhe();
    void set_protocols(const std::string& protocols);
    void set_ciphers(const std::string& ciphers);
    void set_curves(const std::string& curves);

    [[nodiscard]] tls* get_context();

private:
    std::unique_ptr<tls, decltype(&tls_free)> m_context{tls_server(), &tls_free};
    std::unique_ptr<tls_config, decltype(&tls_config_free)> m_config{tls_config_new(), &tls_config_free};
    bool m_configured = false;
};

#endif /* AA7383D0_F723_456B_A3A6_98A8737337D5 */
