#ifndef B6A6FE88_AB70_4E18_AE99_950788B786FA
#define B6A6FE88_AB70_4E18_AE99_950788B786FA

#include <ev++.h>

template<typename T>
inline constexpr auto make_unsigned(T value)
{
    return static_cast<std::make_unsigned_t<T>>(value);
}

inline bool is_ev_error(int revents)
{
    return (make_unsigned(revents) & make_unsigned(ev::ERROR)) != 0;
}

#endif /* B6A6FE88_AB70_4E18_AE99_950788B786FA */
