#include "tlsservercontext_p.h"
#include "tlsexception.h"

TLSServerContextPrivate::TLSServerContextPrivate()
{
    if (!this->m_context || !this->m_config) {
        throw TLSException("Failed to create TLS context");
    }

    tls_config_prefer_ciphers_server(this->m_config.get());
}

void TLSServerContextPrivate::set_keypair(const std::string& cert, const std::string& key)
{
    if (tls_config_set_keypair_file(this->m_config.get(), cert.c_str(), key.c_str()) != 0) {
        throw TLSException("Failed to set keypair", this->m_config.get());
    }
}

void TLSServerContextPrivate::set_ca(const std::string& ca)
{
    if (tls_config_set_ca_file(this->m_config.get(), ca.c_str()) != 0) {
        throw TLSException("Failed to set CA", this->m_config.get());
    }
}

void TLSServerContextPrivate::set_ocsp_staple(const std::string& ocsp)
{
    if (tls_config_set_ocsp_staple_file(this->m_config.get(), ocsp.c_str()) != 0) {
        throw TLSException("Failed to set OCSP staple", this->m_config.get());
    }

    tls_config_ocsp_require_stapling(this->m_config.get());
}

void TLSServerContextPrivate::set_verify_client()
{
    tls_config_verify_client(this->m_config.get());
}

void TLSServerContextPrivate::enable_dhe()
{
    tls_config_set_dheparams(this->m_config.get(), "auto");
}

void TLSServerContextPrivate::set_protocols(const std::string& protocols)
{
    std::uint32_t p;

    if (tls_config_parse_protocols(&p, protocols.c_str()) != 0) {
        throw TLSException("Failed to parse protocols");
    }

    if (tls_config_set_protocols(this->m_config.get(), p) != 0) {
        throw TLSException("Failed to set protocols", this->m_config.get());
    }
}

void TLSServerContextPrivate::set_ciphers(const std::string& ciphers)
{
    if (tls_config_set_ciphers(this->m_config.get(), ciphers.c_str()) != 0) {
        throw TLSException("Failed to set ciphers", this->m_config.get());
    }
}

void TLSServerContextPrivate::set_curves(const std::string& curves)
{
    if (tls_config_set_ecdhecurves(this->m_config.get(), curves.c_str()) != 0) {
        throw TLSException("Failed to set curves", this->m_config.get());
    }
}

tls* TLSServerContextPrivate::get_context()
{
    if (!this->m_configured) {
        if (tls_configure(this->m_context.get(), this->m_config.get()) != 0) {
            throw TLSException(std::string("Failed to configure TLS context: ").append(tls_error(this->m_context.get()))
            );
        }

        this->m_configured = true;
    }

    return this->m_context.get();
}
