#ifndef E2F38285_6E63_4887_8D40_A2B5EAAEDE76
#define E2F38285_6E63_4887_8D40_A2B5EAAEDE76

#include <memory>
#include <string>

class DatabasePrivate;

class Database {
public:
    explicit Database(const std::string& path);
    ~Database();
    Database(const Database&)            = delete;
    Database(Database&&)                 = default;
    Database& operator=(const Database&) = delete;
    Database& operator=(Database&&)      = default;

    enum class status_t {
        OK          = 200,
        BAD_REQUEST = 400,
        LOCKED      = 423,
    };

    void create_tables();

    [[nodiscard]] std::string get_state(const std::string& slug);
    [[nodiscard]] status_t set_state(const std::string& slug, const std::string& state, const std::string& lock_id);
    [[nodiscard]] status_t delete_state(const std::string& slug, const std::string& lock_id);
    [[nodiscard]] status_t put_lock(const std::string& slug, const std::string& lock_id);
    [[nodiscard]] status_t delete_lock(const std::string& slug, const std::string& lock_id);

private:
    std::unique_ptr<DatabasePrivate> d_ptr;

    [[nodiscard]] inline DatabasePrivate* d_func() { return this->d_ptr.get(); }
    [[nodiscard]] inline const DatabasePrivate* d_func() const { return this->d_ptr.get(); }
};

#endif /* E2F38285_6E63_4887_8D40_A2B5EAAEDE76 */
