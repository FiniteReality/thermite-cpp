#ifndef LOGGING_HPP
#define LOGGING_HPP

#ifdef THERMITE_DEBUG

#include <iostream>
#define DEBUG_LOG(expr) \
    std::cout << expr << std::endl

#else

#define DEBUG_LOG(expr)

#endif /* THERMITE_DEBUG */

#endif /* LOGGING_HPP */
