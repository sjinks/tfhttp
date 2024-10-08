#ifndef B6A6FE88_AB70_4E18_AE99_950788B786FA
#define B6A6FE88_AB70_4E18_AE99_950788B786FA

#include <format>
#include <string>
#include <ev++.h>
#include <llhttp.h>

template<typename T>
inline constexpr auto make_unsigned(T value)
{
    return static_cast<std::make_unsigned_t<T>>(value);
}

inline bool is_ev_error(int revents)
{
    return (make_unsigned(revents) & make_unsigned(ev::ERROR)) != 0;
}

inline bool is_ev_read(int revents)
{
    return (make_unsigned(revents) & make_unsigned(ev::READ)) != 0;
}

inline bool is_ev_write(int revents)
{
    return (make_unsigned(revents) & make_unsigned(ev::WRITE)) != 0;
}

inline std::string get_llhttp_error(llhttp_errno_t error)
{
    return std::format("Error: failed to parse HTTP request: {}", llhttp_errno_name(error));
}

inline std::string to_lower(const std::string& str)
{
    std::string lower_str = str;
    std::ranges::transform(lower_str, lower_str.begin(), [](unsigned char c) { return std::tolower(c); });
    return lower_str;
}

inline bool ci_eq(const std::string& str1, const std::string& str2)
{
    return to_lower(str1) == to_lower(str2);
}

#endif /* B6A6FE88_AB70_4E18_AE99_950788B786FA */
