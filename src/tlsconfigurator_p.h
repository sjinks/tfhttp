#ifndef C815C459_D8C3_470F_9E71_D840CB52F622
#define C815C459_D8C3_470F_9E71_D840CB52F622

#include <memory>
#include <string>
#include <ev++.h>
#include "tlsconfigurator.h"

class TLSServerContext;

class TLSConfiguratorPrivate {
public:
    explicit TLSConfiguratorPrivate(const std::string& env_prefix);

    [[nodiscard]] std::shared_ptr<TLSServerContext> configure() const;
    void watch(const TLSConfigurator::watch_callback_t& callback);

    static constexpr ev_tstamp RECONFIGURATION_DELAY = 15.0;

private:
    std::string m_certificate{};
    std::string m_key{};
    std::string m_ca{};
    std::string m_trusted_certificate{};
    std::string m_tls_protocols{};
    std::string m_tls_ciphers{};
    std::string m_tls_curves{};
    ev::stat m_stat_watcher{};
    ev::timer m_timer{};
    TLSConfigurator::watch_callback_t m_callback = nullptr;
    bool m_https                                 = false;
    bool m_tls_verify_client                     = false;
    bool m_tls_enable_dhe                        = false;

    void on_stat(ev::stat& watcher, int revents);
    void on_timeout(ev::timer& timer, int revents);
};

#endif /* C815C459_D8C3_470F_9E71_D840CB52F622 */
