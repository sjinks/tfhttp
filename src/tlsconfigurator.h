#ifndef A2ED91C9_5DC4_495A_8E7D_3388BFD9DB20
#define A2ED91C9_5DC4_495A_8E7D_3388BFD9DB20

#include <memory>
#include <string>

class TLSConfiguratorPrivate;
class TLSServerContext;

class TLSConfigurator {
public:
    using watch_callback_t = std::function<void(const std::shared_ptr<TLSServerContext>&)>;

    explicit TLSConfigurator(const std::string& env_prefix);
    TLSConfigurator(const TLSConfigurator&)            = delete;
    TLSConfigurator(TLSConfigurator&&)                 = delete;
    TLSConfigurator& operator=(const TLSConfigurator&) = delete;
    TLSConfigurator& operator=(TLSConfigurator&&)      = delete;
    ~TLSConfigurator();

    [[nodiscard]] std::shared_ptr<TLSServerContext> configure() const;
    void watch(const watch_callback_t& callback);
    void stop();

private:
    std::unique_ptr<TLSConfiguratorPrivate> d_ptr;

    [[nodiscard]] inline TLSConfiguratorPrivate* d_func() { return this->d_ptr.get(); }
    [[nodiscard]] inline const TLSConfiguratorPrivate* d_func() const { return this->d_ptr.get(); }
};

#endif /* A2ED91C9_5DC4_495A_8E7D_3388BFD9DB20 */
