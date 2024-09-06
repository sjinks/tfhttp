#include "stdafx.h"
#include "database_p.h"
#include "database.h"
#include "utils.h"

DatabasePrivate::DatabasePrivate(const std::string& connection_string) : m_db(connection_string) {}

void DatabasePrivate::create_tables()
{
    if (ci_eq(this->m_db.dbms_name(), "sqlite")) {
        this->execute("PRAGMA journal_mode = WAL");
    }

    this->transaction([this]() {
        this->execute("CREATE TABLE IF NOT EXISTS state (slug VARCHAR(32) PRIMARY KEY, state TEXT NOT NULL)");
        this->execute("CREATE TABLE IF NOT EXISTS lock (slug VARCHAR(32) PRIMARY KEY, lockID CHARACTER(36) NOT NULL)");
        return Database::status_t::OK;
    });
}

std::string DatabasePrivate::get_state(const std::string& slug)
{
    nanodbc::statement stmt(this->m_db);
    nanodbc::prepare(stmt, "SELECT state FROM state WHERE slug = ?");
    stmt.bind(0, slug.c_str());
    if (auto result = nanodbc::execute(stmt); result.next()) {
        return result.get<std::string>(0);
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

        nanodbc::statement select(this->m_db);
        nanodbc::prepare(select, "SELECT state FROM state WHERE slug = ?");
        select.bind(0, slug.c_str());
        if (auto result = nanodbc::execute(select); result.next()) {
            this->run_query("UPDATE state SET state = ? WHERE slug = ?", {state, slug});
            return OK;
        }

        this->run_query("INSERT INTO state (slug, state) VALUES (?1, ?2)", {slug, state});
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

        this->run_query("DELETE FROM state WHERE slug = ?", {slug});
        return OK;
    });
}

Database::status_t DatabasePrivate::put_lock(const std::string& slug, const std::string& lock_id)
{
    return this->transaction([this, &slug, &lock_id]() {
        if (this->has_lock(slug)) {
            return Database::status_t::LOCKED;
        }

        this->run_query("INSERT INTO lock (slug, lockID) VALUES (?, ?)", {slug, lock_id});
        return Database::status_t::OK;
    });
}

Database::status_t DatabasePrivate::delete_lock(const std::string& slug, const std::string& lock_id)
{
    return this->transaction([this, &slug, &lock_id]() {
        if (!this->has_lock(slug, lock_id)) [[unlikely]] {
            return Database::status_t::BAD_REQUEST;
        }

        this->run_query("DELETE FROM lock WHERE slug = ? AND lockID = ?", {slug, lock_id});
        return Database::status_t::OK;
    });
}

bool DatabasePrivate::has_lock(const std::string& slug)
{
    const char* sql = "SELECT lockID FROM lock WHERE slug = ?";

    nanodbc::statement stmt(this->m_db);
    nanodbc::prepare(stmt, sql);
    stmt.bind(0, slug.c_str());
    auto result = nanodbc::execute(stmt);
    return result.next();
}

bool DatabasePrivate::has_lock(const std::string& slug, const std::string& lock_id)
{
    const char* sql = "SELECT lockID FROM lock WHERE slug = ? AND lockID = ?";

    nanodbc::statement stmt(this->m_db);
    nanodbc::prepare(stmt, sql);
    stmt.bind(0, slug.c_str());
    stmt.bind(1, lock_id.c_str());
    auto result = nanodbc::execute(stmt);
    return result.next();
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

void DatabasePrivate::run_query(const char* query, const std::initializer_list<std::string>& args)
{
    nanodbc::statement stmt(this->m_db);
    nanodbc::prepare(stmt, query);

    short int i = 0;
    // NOLINTNEXTLINE(*-bounds-pointer-arithmetic)
    for (const auto* it = args.begin(); it != args.end(); ++it, ++i) {
        stmt.bind(i, it->c_str());
    }

    nanodbc::execute(stmt);
}

void DatabasePrivate::execute(const char* query)
{
    nanodbc::execute(this->m_db, query);
}
