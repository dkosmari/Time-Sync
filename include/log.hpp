// SPDX-License-Identifier: MIT

#ifndef LOG_HPP
#define LOG_HPP

#include <whb/log.h>


namespace logging {

    void initialize();
    void cleanup();

    __attribute__(( __format__ (__printf__, 1, 2) ))
    void printf(const char* fmt, ...);

} // namespace logging

#endif
