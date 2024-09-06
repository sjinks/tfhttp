#ifndef F640FA93_5723_4C84_8793_6509B318D18B
#define F640FA93_5723_4C84_8793_6509B318D18B

#include <functional>
#include <initializer_list>
#include <string>
#include <nanodbc/nanodbc.h>
#include "database.h"

class DatabasePrivate {
public:
    explicit DatabasePrivate(const std::string& connection_string);

    void create_tables();

    [[nodiscard]] std::string get_state(const std::string& slug);
    [[nodiscard]] Database::status_t
    set_state(const std::string& slug, const std::string& state, const std::string& lock_id);
    [[nodiscard]] Database::status_t delete_state(const std::string& slug, const std::string& lock_id);
    [[nodiscard]] Database::status_t put_lock(const std::string& slug, const std::string& lock_id);
    [[nodiscard]] Database::status_t delete_lock(const std::string& slug, const std::string& lock_id);

private:
    nanodbc::connection m_db;
    [[nodiscard]] bool has_lock(const std::string& slug);
    [[nodiscard]] bool has_lock(const std::string& slug, const std::string& lock_id);

    [[nodiscard]] static std::string generate_uuid();
    void run_query(const char* query, const std::initializer_list<std::string>& args);
    void execute(const char* query);

    template<typename F, typename... Args>
    Database::status_t transaction(const F& f, Args&&... args)
    {
        nanodbc::transaction xact(this->m_db);
        auto result = f(std::forward<Args>(args)...);
        xact.commit();
        return result;
    }
};

#endif /* F640FA93_5723_4C84_8793_6509B318D18B */
