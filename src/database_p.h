#ifndef F640FA93_5723_4C84_8793_6509B318D18B
#define F640FA93_5723_4C84_8793_6509B318D18B

#include <functional>
#include <initializer_list>
#include <string>
#include <sqlite3pp.h>
#include "database.h"

class DatabasePrivate {
public:
    explicit DatabasePrivate(const std::string& path);

    void create_tables();

    [[nodiscard]] std::string get_state(const std::string& slug);
    [[nodiscard]] Database::status_t
    set_state(const std::string& slug, const std::string& state, const std::string& lock_id);
    [[nodiscard]] Database::status_t delete_state(const std::string& slug, const std::string& lock_id);
    [[nodiscard]] Database::status_t put_lock(const std::string& slug, const std::string& lock_id);
    [[nodiscard]] Database::status_t delete_lock(const std::string& slug, const std::string& lock_id);

private:
    sqlite3pp::database m_db;
    [[nodiscard]] bool has_lock(const std::string& slug);
    [[nodiscard]] bool has_lock(const std::string& slug, const std::string& lock_id);

    [[nodiscard]] static std::string generate_uuid();
    void run_query(const char* query, std::initializer_list<std::string> args);
    void execute(const char* query);

    using transaction_t = std::function<Database::status_t()>;

    Database::status_t transaction(const transaction_t& f);
};

#endif /* F640FA93_5723_4C84_8793_6509B318D18B */
