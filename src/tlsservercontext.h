#ifndef A2AA408D_BD14_4F04_9950_DE9B21366C2D
#define A2AA408D_BD14_4F04_9950_DE9B21366C2D

#include <memory>
#include <string>

class TLSServerContextPrivate;

struct tls;

class TLSServerContext : public std::enable_shared_from_this<TLSServerContext> {
private:
    struct PrivateTag {};

public:
    static std::shared_ptr<TLSServerContext> create();

    explicit TLSServerContext(const PrivateTag&);
    TLSServerContext(const TLSServerContext&)            = delete;
    TLSServerContext(TLSServerContext&&)                 = default;
    TLSServerContext& operator=(const TLSServerContext&) = delete;
    TLSServerContext& operator=(TLSServerContext&&)      = default;
    ~TLSServerContext();

    void set_keypair(const std::string& cert, const std::string& key);
    void set_ca(const std::string& ca);
    void set_ocsp_staple(const std::string& ocsp);
    void set_verify_client();
    void enable_dhe();
    void set_protocols(const std::string& protocols);
    void set_ciphers(const std::string& ciphers);
    void set_curves(const std::string& curves);

    tls* get_context();

private:
    std::unique_ptr<TLSServerContextPrivate> d_ptr;

    [[nodiscard]] inline TLSServerContextPrivate* d_func() { return this->d_ptr.get(); }
    [[nodiscard]] inline const TLSServerContextPrivate* d_func() const { return this->d_ptr.get(); }
};

#endif /* A2AA408D_BD14_4F04_9950_DE9B21366C2D */
