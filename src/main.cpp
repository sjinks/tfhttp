#include "stdafx.h"
#include <iostream>
#include "database.h"
#include "server.h"
#include "tlsconfigurator.h"
#include "tlsexception.h"
#include "tlsservercontext.h"

namespace {
void signal_watcher(ev::sig& watcher, int)
{
    watcher.loop.break_loop();
}

std::string get_database_name()
{
    const char* name = std::getenv("TFHTTP_DSN");  // NOLINT(concurrency-mt-unsafe)
    return name != nullptr ? name : "Driver={SQLite3};Database=:memory:";
}

std::uint16_t get_port()
{
    constexpr std::uint16_t default_port = 3000;
    if (const char* port = std::getenv("TFHTTP_PORT"); port != nullptr) {  // NOLINT(concurrency-mt-unsafe)
        if (const int val = std::stoi(port); val > 0 && val <= std::numeric_limits<std::uint16_t>::max()) {
            return static_cast<std::uint16_t>(val);
        }

        throw std::range_error("Port is out of range");
    }

    return default_port;
}

std::string get_address()
{
    static const std::string default_address{"0.0.0.0"};

    if (const char* address = std::getenv("TFHTTP_ADDRESS"); address != nullptr) {  // NOLINT(concurrency-mt-unsafe)
        in_addr v4{};
        in6_addr v6{};

        if (inet_pton(AF_INET, address, &v4) == 1) {
            return address;
        }

        if (inet_pton(AF_INET6, address, &v6) == 1) {
            return address;
        }

        throw std::invalid_argument("Invalid IP address");
    }

    return default_address;
}

void destroy_loop(const ev::loop_ref* loop)
{
    ev_loop_destroy(*loop);
}

void replace_tls_context(std::shared_ptr<TLSServerContext>& old_ctx, const std::shared_ptr<TLSServerContext>& new_ctx)
{
    try {
        new_ctx->get_context();
    }
    catch (const TLSException& e) {
        std::cerr << std::format("Error: failed to reload TLS context: {}\n", e.what());
        return;
    }

    old_ctx = new_ctx;
}

}  // namespace

int main()
{
    try {
        auto port     = get_port();
        auto address  = get_address();
        auto database = std::make_shared<Database>(get_database_name());
        database->create_tables();

        if (SIG_ERR == std::signal(SIGPIPE, SIG_IGN)) {
            throw std::system_error(errno, std::system_category(), "signal() failed");
        }

        auto loop = ev::get_default_loop();
        const std::unique_ptr<ev::loop_ref, decltype(&destroy_loop)> loop_guard(&loop, &destroy_loop);

        ev::sig sigint_watcher(loop);
        ev::sig sigterm_watcher(loop);
        sigint_watcher.set<signal_watcher>();
        sigint_watcher.start(SIGINT);
        sigterm_watcher.set<signal_watcher>();
        sigterm_watcher.start(SIGTERM);

        TLSConfigurator tlsconf("TFHTTP_");
        auto ctx = tlsconf.configure();
        if (ctx) {
            ctx->get_context();
            tlsconf.watch([&ctx](const std::shared_ptr<TLSServerContext>& new_ctx) {
                replace_tls_context(ctx, new_ctx);
            });
        }

        auto server = Server::create(loop, address, port, database);
        server->set_tls_context(ctx);
        server->run();
        loop.run(0);

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << std::format("FATAL ERROR: {}\n", e.what());
        return 1;
    }
}
