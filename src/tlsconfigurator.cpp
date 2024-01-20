#include "tlsconfigurator.h"
#include <memory>
#include "tlsconfigurator_p.h"

TLSConfigurator::TLSConfigurator(const std::string& env_prefix)
    : d_ptr(std::make_unique<TLSConfiguratorPrivate>(env_prefix))
{}

TLSConfigurator::~TLSConfigurator() = default;

std::shared_ptr<TLSServerContext> TLSConfigurator::configure() const
{
    return this->d_func()->configure();
}

void TLSConfigurator::watch(const watch_callback_t& callback)
{
    this->d_func()->watch(callback);
}

void TLSConfigurator::stop()
{
    this->d_func()->watch(nullptr);
}
