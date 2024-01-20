#ifndef AE21BDE3_CD1A_4758_A4BD_6D139CC29167
#define AE21BDE3_CD1A_4758_A4BD_6D139CC29167

#include <memory>
#include <string>
#include <ada.h>
#include <ev++.h>
#include <llhttp.h>
#include "database.h"
#include "tlsutils.h"

class ClientHandler;
class Server;
struct tls;

class ClientHandlerPrivate {
public:
    ClientHandlerPrivate(
        ClientHandler* q, tls* ctx, const ev::loop_ref& loop, const std::shared_ptr<Server>& server,
        const std::shared_ptr<Database>& database
    );
    ClientHandlerPrivate(const ClientHandlerPrivate&)            = delete;
    ClientHandlerPrivate(ClientHandlerPrivate&&)                 = delete;
    ClientHandlerPrivate& operator=(const ClientHandlerPrivate&) = delete;
    ClientHandlerPrivate& operator=(ClientHandlerPrivate&&)      = delete;
    ~ClientHandlerPrivate();

    int accept(int fd);

    static constexpr ev_tstamp READ_TIMEOUT = 15;

private:
    ClientHandler* q_ptr;
    std::unique_ptr<tls, decltype(&dispose_tls_context)> m_ctx;
    ev::loop_ref m_loop;
    std::weak_ptr<Server> m_server;
    std::shared_ptr<Database> m_database;
    llhttp_t m_parser{};
    llhttp_settings_t m_settings{};
    ev::io m_io;
    ev::timer m_timer;
    ada::url_aggregator m_url{};
    std::string m_body{};
    std::string m_response;
    std::size_t m_offset     = 0;
    llhttp_method_t m_method = HTTP_GET;
    bool m_done              = false;

    void on_read(ev::io& watcher, int revents);
    void on_write(ev::io& watcher, int revents);
    void on_tls_close(ev::io& watcher, int revents);
    void on_timeout(ev::timer& timer, int revents);

    void close_connection(const std::string& error = std::string());
    void terminate();

    void handle_get_state(const std::string& slug);
    void handle_set_state(const std::string& slug);
    void handle_delete_state(const std::string& slug);
    void handle_lock(const std::string& slug);
    void handle_unlock(const std::string& slug);
    void not_found();
    void method_not_allowed(const char* allowed);

    static int on_url(llhttp_t* parser, const char* at, std::size_t length);
    static int on_message_complete(llhttp_t* parser);
    static int on_body(llhttp_t* parser, const char* at, std::size_t length);

    static std::string generate_http_date();
    static std::string_view get_status_text(int status);
    void generate_text_response(int status);
};

#endif /* AE21BDE3_CD1A_4758_A4BD_6D139CC29167 */
