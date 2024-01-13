#include "stdafx.h"
#include "database.h"
#include "database_p.h"

Database::Database(const std::string& path) : d_ptr(std::make_unique<DatabasePrivate>(path)) {}

Database::~Database() = default;

void Database::create_tables()
{
    this->d_func()->create_tables();
}

std::string Database::get_state(const std::string& slug)
{
    return this->d_func()->get_state(slug);
}

Database::status_t Database::set_state(const std::string& slug, const std::string& state, const std::string& lock_id)
{
    return this->d_func()->set_state(slug, state, lock_id);
}

Database::status_t Database::delete_state(const std::string& slug, const std::string& lock_id)
{
    return this->d_func()->delete_state(slug, lock_id);
}

Database::status_t Database::put_lock(const std::string& slug, const std::string& lock_id)
{
    return this->d_func()->put_lock(slug, lock_id);
}

Database::status_t Database::delete_lock(const std::string& slug, const std::string& lock_id)
{
    return this->d_func()->delete_lock(slug, lock_id);
}
