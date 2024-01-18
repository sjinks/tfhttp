#include "tlsservercontext.h"
#include "tlsservercontext_p.h"

std::shared_ptr<TLSServerContext> TLSServerContext::create()
{
    return std::make_shared<TLSServerContext>(TLSServerContext::PrivateTag{});
}

TLSServerContext::TLSServerContext(const TLSServerContext::PrivateTag&)
    : d_ptr(std::make_unique<TLSServerContextPrivate>())
{}

TLSServerContext::~TLSServerContext() = default;

void TLSServerContext::set_keypair(const std::string& cert, const std::string& key)
{
    this->d_func()->set_keypair(cert, key);
}

void TLSServerContext::set_ca(const std::string& ca)
{
    this->d_func()->set_ca(ca);
}

void TLSServerContext::set_ocsp_staple(const std::string& ocsp)
{
    this->d_func()->set_ocsp_staple(ocsp);
}

void TLSServerContext::set_verify_client()
{
    this->d_func()->set_verify_client();
}

void TLSServerContext::enable_dhe()
{
    this->d_func()->enable_dhe();
}

void TLSServerContext::set_protocols(const std::string& protocols)
{
    this->d_func()->set_protocols(protocols);
}

void TLSServerContext::set_ciphers(const std::string& ciphers)
{
    this->d_func()->set_ciphers(ciphers);
}

void TLSServerContext::set_curves(const std::string& curves)
{
    this->d_func()->set_curves(curves);
}

tls* TLSServerContext::get_context()
{
    return this->d_func()->get_context();
}
