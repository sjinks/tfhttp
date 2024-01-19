#include "stdafx.h"
#include "clienthandler_p.h"
#include "database.h"
#include "server.h"

ClientHandlerPrivate::ClientHandlerPrivate(
    ClientHandler* q_ptr, const ev::loop_ref& loop, const std::shared_ptr<Server>& server,
    const std::shared_ptr<Database>& database
)
    : q_ptr(q_ptr), m_loop(loop), m_server(server), m_database(database), m_io(loop), m_timer(loop)
{
    llhttp_settings_init(&this->m_settings);
    this->m_settings.on_message_complete = ClientHandlerPrivate::on_message_complete;
    this->m_settings.on_url              = ClientHandlerPrivate::on_url;
    this->m_settings.on_body             = ClientHandlerPrivate::on_body;

    llhttp_init(&this->m_parser, HTTP_REQUEST, &this->m_settings);
    this->m_parser.data = this;
}

ClientHandlerPrivate::~ClientHandlerPrivate()
{
    const int fd = this->m_io.fd;
    this->stop_watchers();
    close(fd);
}

int ClientHandlerPrivate::accept(int fd)
{
    this->m_io.set<ClientHandlerPrivate, &ClientHandlerPrivate::on_read>(this);
    this->m_io.start(fd, ev::READ);

    this->m_timer.set<ClientHandlerPrivate, &ClientHandlerPrivate::on_timeout>(this);
    this->m_timer.start(ClientHandlerPrivate::READ_TIMEOUT, 0);

    return 0;
}

void ClientHandlerPrivate::on_read(ev::io& watcher, int)  // NOSONAR(cpp:S995)
{
    constexpr std::size_t BUFFER_SIZE = 8192;
    std::array<char, BUFFER_SIZE> buffer{};
    const ssize_t received = recv(watcher.fd, buffer.data(), sizeof(buffer), 0);
    if (received <= 0) [[unlikely]] {
        std::cerr << std::format(
            "Error: failed to read from socket: {}\n", std::error_code(errno, std::system_category()).message()
        );
        this->stop_watchers();
        this->terminate();
        return;
    }

    if (auto status = llhttp_execute(&this->m_parser, buffer.data(), static_cast<std::size_t>(received));
        status != HPE_OK) [[unlikely]] {
        std::cerr << std::format("Error: failed to parse HTTP request: {}\n", llhttp_errno_name(status));
        this->stop_watchers();
        this->terminate();
        return;
    }

    this->m_timer.again();
}

void ClientHandlerPrivate::on_write(ev::io& watcher, int)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const auto* buf       = this->m_response.data() + this->m_offset;
    auto len              = this->m_response.size() - this->m_offset;
    const ssize_t written = send(watcher.fd, buf, len, 0);

    if (written <= 0) [[unlikely]] {
        std::cerr << std::format(
            "Error: failed to write to socket: {}\n", std::error_code(errno, std::system_category()).message()
        );
        this->stop_watchers();
        this->terminate();
        return;
    }

    this->m_offset += static_cast<std::size_t>(written);
    if (this->m_offset == this->m_response.size()) {
        this->stop_watchers();
        this->terminate();
        return;
    }
}

void ClientHandlerPrivate::on_timeout(ev::timer&, int)  // NOSONAR
{
    this->stop_watchers();
    this->terminate();
}

void ClientHandlerPrivate::stop_watchers()
{
    const int fd = this->m_io.fd;
    this->m_io.stop();
    this->m_timer.stop();
    close(fd);
}

void ClientHandlerPrivate::terminate()
{
    if (auto server = this->m_server.lock(); server) [[likely]] {
        server->remove_handler(this->q_ptr);
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
        "Connection: close\r\n"
        "\r\n"
        "{}",
        ClientHandlerPrivate::generate_http_date(), state.size(), state
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
        "Connection: close\r\n"
        "\r\n"
        "{1}\n",
        status, text, ClientHandlerPrivate::generate_http_date(), text.size() + 1, allowed
    );
}

int ClientHandlerPrivate::on_message_complete(llhttp_t* parser)
{
    static const std::regex pattern("^/([A-Za-z0-9_-]+)(/state)?$");

    std::cmatch url_matches;
    auto* client = static_cast<ClientHandlerPrivate*>(parser->data);
    auto path    = client->m_url.get_pathname();
    auto method  = client->m_method;

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

    client->m_io.set(ev::WRITE);
    client->m_io.set<ClientHandlerPrivate, &ClientHandlerPrivate::on_write>(client);

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
        "Connection: close\r\n"
        "\r\n"
        "{1}\n",
        status, text, ClientHandlerPrivate::generate_http_date(), text.size() + 1
    );
}
