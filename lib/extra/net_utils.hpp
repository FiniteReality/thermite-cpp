#ifndef _NET_UTILS_HPP_
#define _NET_UTILS_HPP_

#include <algorithm>
#include <stdint.h>
#include <type_traits>

namespace thermite::utility
{
namespace detail
{

template <typename T,
    int N = sizeof(T),
    typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
constexpr T swap_endianness(T value)
{
    auto arr = reinterpret_cast<uint8_t*>(&value);
    std::reverse(arr, arr+N);
    return *reinterpret_cast<T*>(arr);
}

}

template <typename T,
    typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
constexpr T to_big_endian(T value)
{
#if THERMITE_BIG_ENDIAN
    return value; // already big endian
#else
    // convert to big endian by swapping byte order
    return detail::swap_endianness(value);
#endif
}

template <typename T,
    typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
constexpr T to_little_endian(T value)
{
    #ifndef THERMITE_BIG_ENDIAN
    return value; // already little endian
    #else
    // convert to little endian by swapping byte order
    return detail::swap_endianness(value);
    #endif
}

}

#endif /* _NET_UTILS_HPP_ */
