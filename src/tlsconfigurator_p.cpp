#include "tlsconfigurator_p.h"
#include <cstdlib>
#include <string>
#include "tlsexception.h"
#include "tlsservercontext.h"
#include "utils.h"

namespace {

std::string get_env(const std::string& prefix, const char* name)
{
    const std::string env_name = prefix + name;
    const char* value          = std::getenv(env_name.c_str());  // NOLINT(concurrency-mt-unsafe)
    return value != nullptr ? value : "";
}

}  // namespace

TLSConfiguratorPrivate::TLSConfiguratorPrivate(const std::string& env_prefix)
{
    this->m_https               = get_env(env_prefix, "HTTPS") == "1";
    this->m_certificate         = get_env(env_prefix, "CERTIFICATE");
    this->m_key                 = get_env(env_prefix, "PRIVATE_KEY");
    this->m_ca                  = get_env(env_prefix, "CA_CERTIFICATE");
    this->m_trusted_certificate = get_env(env_prefix, "TRUSTED_CERTIFICATE");
    this->m_tls_protocols       = get_env(env_prefix, "TLS_PROTOCOLS");
    this->m_tls_ciphers         = get_env(env_prefix, "TLS_CIPHERS");
    this->m_tls_curves          = get_env(env_prefix, "TLS_CURVES");
    this->m_tls_verify_client   = get_env(env_prefix, "TLS_VERIFY_CLIENT") == "1";
    this->m_tls_enable_dhe      = get_env(env_prefix, "TLS_ENABLE_DHE") == "1";
}

std::shared_ptr<TLSServerContext> TLSConfiguratorPrivate::configure() const
{
    if (!this->m_https) {
        return {};
    }

    auto context = TLSServerContext::create();
    if (!this->m_certificate.empty() && !this->m_key.empty()) {
        context->set_keypair(this->m_certificate, this->m_key);
    }

    if (!this->m_ca.empty()) {
        context->set_ca(this->m_ca);
    }

    if (!this->m_trusted_certificate.empty()) {
        context->set_ocsp_staple(this->m_trusted_certificate);
    }

    if (!this->m_tls_protocols.empty()) {
        context->set_protocols(this->m_tls_protocols);
    }

    if (!this->m_tls_ciphers.empty()) {
        context->set_ciphers(this->m_tls_ciphers);
    }

    if (!this->m_tls_curves.empty()) {
        context->set_curves(this->m_tls_curves);
    }

    if (this->m_tls_verify_client) {
        context->set_verify_client();
    }

    if (this->m_tls_enable_dhe) {
        context->enable_dhe();
    }

    return context;
}

void TLSConfiguratorPrivate::watch(const TLSConfigurator::watch_callback_t& callback)
{
    if (!callback) {
        this->m_stat_watcher.stop();
        this->m_timer.stop();
    }
    else if (this->m_https) {
        this->m_callback = callback;

        if (!this->m_certificate.empty()) {
            this->m_stat_watcher.set<TLSConfiguratorPrivate, &TLSConfiguratorPrivate::on_stat>(this);
            this->m_stat_watcher.start(this->m_certificate.c_str());

            this->m_timer.set<TLSConfiguratorPrivate, &TLSConfiguratorPrivate::on_timeout>(this);
            this->m_timer.set(0, TLSConfiguratorPrivate::RECONFIGURATION_DELAY);
        }
    }
}

void TLSConfiguratorPrivate::on_stat(ev::stat& watcher, int revents)
{
    if (is_ev_error(revents)) [[unlikely]] {
        std::cerr << std::format("Error: stat watcher error: EV_ERROR\n");
        return;
    }

    watcher.stop();
    this->m_timer.again();
}

void TLSConfiguratorPrivate::on_timeout(ev::timer& timer, int revents)
{
    if (is_ev_error(revents)) [[unlikely]] {
        std::cerr << std::format("Error: timer watcher error: EV_ERROR\n");
        return;
    }

    timer.stop();
    if (this->m_callback) {
        try {
            auto config = this->configure();
            this->m_callback(config);
        }
        catch (const TLSException& e) {
            std::cerr << std::format("Error: unexpected TLS configuration error: {}\n", e.what());
        }
    }

    this->m_stat_watcher.start(this->m_certificate.c_str());
}
