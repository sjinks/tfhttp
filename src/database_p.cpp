#include "stdafx.h"
#include "database_p.h"
#include "database.h"

DatabasePrivate::DatabasePrivate(const std::string& path) : m_db(path.c_str()) {}

void DatabasePrivate::create_tables()
{
    this->execute("PRAGMA journal_mode = WAL");
    this->transaction([this]() {
        this->execute("CREATE TABLE IF NOT EXISTS state (slug VARCHAR(32) PRIMARY KEY, state TEXT NOT NULL)");
        this->execute("CREATE TABLE IF NOT EXISTS lock (slug VARCHAR(32) PRIMARY KEY, lockID CHARACTER(36) NOT NULL)");
        return Database::status_t::OK;
    });
}

std::string DatabasePrivate::get_state(const std::string& slug)
{
    sqlite3pp::query query(this->m_db, "SELECT state FROM state WHERE slug = ?1");
    query.bind(1, slug, sqlite3pp::nocopy);
    if (auto it = query.begin(); it != query.end()) {
        return (*it).get<const char*>(0);
    }

    return R"({"version": 4, "serial": 1, "lineage": ")" + DatabasePrivate::generate_uuid() + "\"}";
}

Database::status_t
DatabasePrivate::set_state(const std::string& slug, const std::string& state, const std::string& lock_id)
{
    return this->transaction([this, &slug, &state, &lock_id]() {
        using enum Database::status_t;
        if (this->has_lock(slug)) {
            if (lock_id.empty()) [[unlikely]] {
                return BAD_REQUEST;
            }

            if (!this->has_lock(slug, lock_id)) {
                return LOCKED;
            }
        }

        this->run_query("INSERT OR REPLACE INTO state (slug, state) VALUES (?1, ?2)", {slug, state});
        return OK;
    });
}

Database::status_t DatabasePrivate::delete_state(const std::string& slug, const std::string& lock_id)
{
    return this->transaction([this, &slug, &lock_id]() {
        using enum Database::status_t;
        if (this->has_lock(slug)) {
            if (lock_id.empty()) [[unlikely]] {
                return BAD_REQUEST;
            }

            if (!this->has_lock(slug, lock_id)) {
                return LOCKED;
            }
        }

        this->run_query("DELETE FROM state WHERE slug = ?1", {slug});
        return OK;
    });
}

Database::status_t DatabasePrivate::put_lock(const std::string& slug, const std::string& lock_id)
{
    return this->transaction([this, &slug, &lock_id]() {
        if (this->has_lock(slug)) {
            return Database::status_t::LOCKED;
        }

        this->run_query("INSERT INTO lock (slug, lockID) VALUES (?1, ?2)", {slug, lock_id});
        return Database::status_t::OK;
    });
}

Database::status_t DatabasePrivate::delete_lock(const std::string& slug, const std::string& lock_id)
{
    return this->transaction([this, &slug, &lock_id]() {
        if (!this->has_lock(slug, lock_id)) [[unlikely]] {
            return Database::status_t::BAD_REQUEST;
        }

        this->run_query("DELETE FROM lock WHERE slug = ?1 AND lockID = ?2", {slug, lock_id});
        return Database::status_t::OK;
    });
}

bool DatabasePrivate::has_lock(const std::string& slug)
{
    const char* sql = "SELECT lockID FROM lock WHERE slug = ?1";

    sqlite3pp::query query(this->m_db, sql);
    query.bind(1, slug, sqlite3pp::nocopy);
    return query.begin() != query.end();
}

bool DatabasePrivate::has_lock(const std::string& slug, const std::string& lock_id)
{
    const char* sql = "SELECT lockID FROM lock WHERE slug = ?1 AND lockID = ?2";

    sqlite3pp::query query(this->m_db, sql);
    query.bind(1, slug, sqlite3pp::nocopy);
    query.bind(2, lock_id, sqlite3pp::nocopy);
    return query.begin() != query.end();
}

std::string DatabasePrivate::generate_uuid()
{
    constexpr std::size_t RAW_UUID_SIZE = 16;
    std::array<uint8_t, RAW_UUID_SIZE> bytes{};

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(
        std::numeric_limits<decltype(bytes)::value_type>::min(), std::numeric_limits<std::uint8_t>::max()
    );

    std::ranges::generate(bytes, [&]() { return dis(gen); });

    return std::format(
        "{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
        // NOLINTBEGIN(*-magic-numbers)
        bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7], bytes[8], bytes[9], bytes[10],
        bytes[11], bytes[12], bytes[13], bytes[14], bytes[15]
        // NOLINTEND(*-magic-numbers)
    );
}

void DatabasePrivate::run_query(const char* query, std::initializer_list<std::string> args)
{
    sqlite3pp::command cmd(this->m_db, query);
    for (int i = 0; i < static_cast<int>(args.size()); ++i) {
        cmd.bind(i + 1, *(args.begin() + i), sqlite3pp::nocopy);
    }

    if (cmd.execute() != SQLITE_OK) [[unlikely]] {
        throw sqlite3pp::database_error(this->m_db);
    }
}

void DatabasePrivate::execute(const char* query)
{
    if (this->m_db.execute(query) != SQLITE_OK) [[unlikely]] {
        throw sqlite3pp::database_error(this->m_db);
    }
}
