#include "stdafx.h"
#include "clienthandler_p.h"
#include "database.h"
#include "server.h"
#include "tlsutils.h"
#include "utils.h"

ClientHandlerPrivate::ClientHandlerPrivate(
    ClientHandler* q, tls* ctx, const ev::loop_ref& loop, const std::shared_ptr<Server>& server,
    const std::shared_ptr<Database>& database
)
    : q_ptr(q), m_ctx(ctx, &tls_free), m_loop(loop), m_server(server), m_database(database), m_io(loop), m_timer(loop)
{
    llhttp_settings_init(&this->m_settings);
    this->m_settings.on_message_begin    = ClientHandlerPrivate::on_message_begin;
    this->m_settings.on_message_complete = ClientHandlerPrivate::on_message_complete;
    this->m_settings.on_url              = ClientHandlerPrivate::on_url;
    this->m_settings.on_body             = ClientHandlerPrivate::on_body;
    this->m_settings.on_reset            = ClientHandlerPrivate::on_reset;

    llhttp_init(&this->m_parser, HTTP_REQUEST, &this->m_settings);
    this->m_parser.data = this;
}

ClientHandlerPrivate::~ClientHandlerPrivate()
{
    const int fd = this->m_io.fd;
    this->m_io.stop();
    this->m_timer.stop();

    if (fd != -1) {
        if (this->m_ctx) {
            tls_close(this->m_ctx.get());
        }

        shutdown(fd, SHUT_RDWR);
        close(fd);
    }
}

int ClientHandlerPrivate::accept(int fd)
{
    this->m_io.set<ClientHandlerPrivate, &ClientHandlerPrivate::on_io>(this);
    this->m_io.start(fd, ev::READ);

    this->m_timer.set<ClientHandlerPrivate, &ClientHandlerPrivate::on_timeout>(this);
    this->m_timer.set(0.0, ClientHandlerPrivate::io_timeout);
    this->m_timer.again();

    return 0;
}

int ClientHandlerPrivate::handle_read(std::string& error)
{
    std::array<char, ClientHandlerPrivate::READ_BUFFER_SIZE> buffer{};

    int new_events         = ev::NONE;
    const ssize_t num_read = u_read(this->m_ctx.get(), this->m_io.fd, buffer, error, new_events);

    assert(!this->m_done);

    if (num_read > 0) {
        const auto n = static_cast<std::size_t>(num_read);
        if (auto status = llhttp_execute(&this->m_parser, buffer.data(), n); status != HPE_OK) [[unlikely]] {
            error = get_llhttp_error(status);
            return ev::NONE;
        }

        return this->m_done ? ev::WRITE : ev::READ;
    }

    if (num_read == 0) [[unlikely]] {
        if (const auto status = llhttp_finish(&this->m_parser); status != HPE_OK) [[unlikely]] {
            error = get_llhttp_error(status);
            return 0;
        }

        if (!this->m_done) {
            if (this->m_started) {
                error = "Error: incomplete HTTP request, closing connection";
            }

            return ev::NONE;
        }

        return ev::WRITE;
    }

    if (!error.empty()) [[unlikely]] {
        error = std::format("Error: failed to read from socket: {}", error);
        return ev::NONE;
    }

    return new_events;
}

int ClientHandlerPrivate::handle_write(std::string& error)
{
    assert(this->m_offset < this->m_response.size());

    int new_events            = ev::NONE;
    const std::size_t len     = this->m_response.size() - this->m_offset;
    const auto* data          = &this->m_response[this->m_offset];
    const ssize_t num_written = u_write(this->m_ctx.get(), this->m_io.fd, std::span(data, len), error, new_events);

    if (num_written > 0) {
        this->m_offset += static_cast<std::size_t>(num_written);
        return this->m_offset != this->m_response.size() ? ev::WRITE : ev::NONE;
    }

    if (num_written == 0) [[unlikely]] {
        if (this->m_offset != this->m_response.size()) [[unlikely]] {
            error = "Warning: failed to write to socket: connection closed";
        }

        return ev::NONE;
    }

    if (!error.empty()) [[unlikely]] {
        error = std::format("Error: failed to write to socket: {}", error);
        return ev::NONE;
    }

    return new_events;
}

void ClientHandlerPrivate::on_io(ev::io& watcher, int revents)
{
    if (is_ev_error(revents)) [[unlikely]] {
        this->close_connection("Error: socket operation failed: EV_ERROR");
        return;
    }

    int new_events = ev::NONE;
    int old_events = ev::NONE;
    std::string error{};

    if (is_ev_read(revents)) {
        old_events = ev::READ;
        new_events = this->handle_read(error);
    }
    else if (is_ev_write(revents)) {
        old_events = ev::WRITE;
        new_events = this->handle_write(error);
    }

    if (!error.empty()) [[unlikely]] {
        this->close_connection(error);
        return;
    }

    if (new_events == ev::NONE) {
        if (old_events == ev::WRITE && this->m_keep_alive) {
            this->m_done         = false;
            this->m_started      = false;
            new_events           = ev::READ;
            this->m_timer.repeat = ClientHandlerPrivate::keepalive_timeout;
        }
        else {
            this->close_connection();
            return;
        }
    }

    if (new_events != old_events) {
        watcher.start(watcher.fd, new_events);
    }

    this->m_timer.again();
}

void ClientHandlerPrivate::on_tls_close(ev::io& watcher, int revents)
{
    if (is_ev_error(revents)) [[unlikely]] {
        std::cerr << "Error: failed to close TLS connection gracefully: EV_ERROR\n";
        this->m_ctx.reset();
        this->terminate();
    }

    const auto res = tls_close(this->m_ctx.get());
    if (res == TLS_WANT_POLLIN || res == TLS_WANT_POLLOUT) {
        watcher.start(watcher.fd, res == TLS_WANT_POLLIN ? ev::READ : ev::WRITE);
    }
    else {
        if (res == -1) [[unlikely]] {
            std::cerr << std::format(
                "Warning: failed to close TLS connection gracefully: {}\n", tls_error(this->m_ctx.get())
            );
        }

        watcher.stop();
        this->m_ctx.reset();
        this->terminate();
    }
}

void ClientHandlerPrivate::on_timeout(ev::timer& timer, int)
{
    // Stop the watcher here because close_connection() may take some time to complete
    timer.stop();
    if (!this->m_started && this->m_keep_alive) {
        this->close_connection("Closing keep-alive connection");
    }
    else {
        this->close_connection("Timeout");
    }
}

int ClientHandlerPrivate::on_message_begin(llhttp_t* parser)
{
    auto* client           = static_cast<ClientHandlerPrivate*>(parser->data);
    client->m_started      = true;
    client->m_timer.repeat = ClientHandlerPrivate::io_timeout;
    return HPE_OK;
}

int ClientHandlerPrivate::on_message_complete(llhttp_t* parser)
{
    static const std::regex pattern("^/([A-Za-z0-9_-]+)(/state)?$");

    std::cmatch url_matches;
    auto* client = static_cast<ClientHandlerPrivate*>(parser->data);
    auto path    = client->m_url.get_pathname();
    auto method  = client->m_method;

    client->m_io.stop();
    client->m_timer.stop();
    client->m_done       = true;
    client->m_keep_alive = llhttp_should_keep_alive(&client->m_parser) == 1;

    std::cout << std::format(
        "{0:%F} {0:%X%z} {1} {2}\n", std::chrono::system_clock::now(), llhttp_method_name(method), path
    );

    try {
        if (std::regex_search(path.begin(), path.end(), url_matches, pattern)) {
            const auto& slug   = url_matches[1];
            const auto& action = url_matches[2];

            if (action.matched) {
                switch (method) {
                    case HTTP_GET:
                        client->handle_get_state(slug.str());
                        break;

                    case HTTP_POST:
                        client->handle_set_state(slug.str());
                        break;

                    case HTTP_DELETE:
                        client->handle_delete_state(slug.str());
                        break;

                    default:
                        client->method_not_allowed("GET, POST, DELETE");
                        break;
                }
            }
            else {
                switch (method) {
                    case HTTP_LOCK:
                        client->handle_lock(slug.str());
                        break;

                    case HTTP_UNLOCK:
                        client->handle_unlock(slug.str());
                        break;

                    default:
                        client->method_not_allowed("LOCK, UNLOCK");
                        break;
                }
            }
        }
        else {
            client->not_found();
        }
    }
    catch (const std::exception& e) {
        std::cerr << std::format("Error handling the request {} {}: {}\n", llhttp_method_name(method), path, e.what());
        client->generate_text_response(HTTP_STATUS_INTERNAL_SERVER_ERROR);
    }

    return HPE_OK;
}

int ClientHandlerPrivate::on_url(llhttp_t* parser, const char* at, std::size_t length)
{
    static const ada::url_aggregator base;

    auto* client     = static_cast<ClientHandlerPrivate*>(parser->data);
    client->m_method = static_cast<llhttp_method_t>(parser->method);

    if (const auto parsed = ada::parse<ada::url_aggregator>(std::string_view(at, length), &base); parsed) {
        client->m_url = *parsed;
    }

    return HPE_OK;
}

int ClientHandlerPrivate::on_body(llhttp_t* parser, const char* at, std::size_t length)
{
    auto* client = static_cast<ClientHandlerPrivate*>(parser->data);

    client->m_body.append(at, length);
    return HPE_OK;
}

int ClientHandlerPrivate::on_reset(llhttp_t* parser)
{
    auto* client = static_cast<ClientHandlerPrivate*>(parser->data);

    client->m_url    = ada::url_aggregator();
    client->m_offset = 0;
    client->m_body.clear();
    client->m_response.clear();
    client->m_done = false;

    return HPE_OK;
}

void ClientHandlerPrivate::close_connection(const std::string& error)
{
    if (!error.empty()) [[unlikely]] {
        std::cerr << error << '\n';
    }

    if (this->m_ctx) {
        if (const int res = tls_close(this->m_ctx.get()); res == TLS_WANT_POLLIN || res == TLS_WANT_POLLOUT) {
            this->m_timer.again();
            this->m_io.stop();
            this->m_io.set<ClientHandlerPrivate, &ClientHandlerPrivate::on_tls_close>(this);
            this->m_io.start(this->m_io.fd, res == TLS_WANT_POLLIN ? ev::READ : ev::WRITE);
            return;
        }

        this->m_ctx.reset();
    }

    this->terminate();
}

void ClientHandlerPrivate::terminate()
{
    if (auto server = this->m_server.lock(); server) [[likely]] {
        server->remove_handler(this->q_ptr);
    }
    else {
        delete this;
    }
}

void ClientHandlerPrivate::handle_get_state(const std::string& slug)
{
    auto state = this->m_database->get_state(slug);

    this->m_response = std::format(
        "HTTP/1.1 200 OK\r\n"
        "Date: {}\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: {}\r\n"
        "{}\r\n"
        "\r\n"
        "{}",
        ClientHandlerPrivate::generate_http_date(), state.size(), this->connection_header(), state
    );
}

void ClientHandlerPrivate::handle_set_state(const std::string& slug)
{
    ada::url_search_params query = this->m_url.get_search();
    const std::string lock_id(query.get("ID").value_or(""));
    auto status = this->m_database->set_state(slug, this->m_body, lock_id);
    this->generate_text_response(static_cast<int>(status));
}

void ClientHandlerPrivate::handle_delete_state(const std::string& slug)
{
    ada::url_search_params query = this->m_url.get_search();
    const std::string lock_id(query.get("lock_id").value_or(""));
    auto status = this->m_database->delete_state(slug, lock_id);
    this->generate_text_response(static_cast<int>(status));
}

void ClientHandlerPrivate::handle_lock(const std::string& slug)
{
    nlohmann::json body = nlohmann::json::parse(this->m_body);
    auto lock_id        = body["ID"].get<std::string>();
    auto status         = this->m_database->put_lock(slug, lock_id);
    this->generate_text_response(static_cast<int>(status));
}

void ClientHandlerPrivate::handle_unlock(const std::string& slug)
{
    nlohmann::json body = nlohmann::json::parse(this->m_body);
    auto lock_id        = body["ID"].get<std::string>();
    auto status         = this->m_database->delete_lock(slug, lock_id);
    this->generate_text_response(static_cast<int>(status));
}

void ClientHandlerPrivate::not_found()
{
    this->generate_text_response(HTTP_STATUS_NOT_FOUND);
}

void ClientHandlerPrivate::method_not_allowed(const char* allowed)
{
    const auto status = static_cast<int>(HTTP_STATUS_METHOD_NOT_ALLOWED);
    const auto text   = ClientHandlerPrivate::get_status_text(status);

    this->m_response = std::format(
        "HTTP/1.1 {0} {1}\r\n"
        "Date: {2}\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: {3}\r\n"
        "Allow: {4}\r\n"
        "{5}\r\n"
        "\r\n"
        "{1}\n",
        status, text, ClientHandlerPrivate::generate_http_date(), text.size() + 1, allowed, this->connection_header()
    );
}

std::string ClientHandlerPrivate::generate_http_date()
{
    return std::format("{0:%a} {0:%d} {0:%b} {0:%Y} {0:%X} GMT", std::chrono::system_clock::now());
}

std::string_view ClientHandlerPrivate::get_status_text(int status)
{
    return llhttp_status_name(static_cast<llhttp_status_t>(status));
}

void ClientHandlerPrivate::generate_text_response(int status)
{
    auto text = ClientHandlerPrivate::get_status_text(status);

    this->m_response = std::format(
        "HTTP/1.1 {0} {1}\r\n"
        "Date: {2}\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: {3}\r\n"
        "{4}\r\n"
        "\r\n"
        "{1}\n",
        status, text, ClientHandlerPrivate::generate_http_date(), text.size() + 1, this->connection_header()
    );
}

std::string ClientHandlerPrivate::connection_header() const
{
    if (this->m_keep_alive) {
        return "Connection: keep-alive\r\n"
               "Keep-Alive: timeout=" +
               std::to_string(static_cast<int>(ClientHandlerPrivate::keepalive_timeout));
    }

    return "Connection: close";
}
