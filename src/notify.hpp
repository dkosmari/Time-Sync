/*
 * Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef NOTIFY_HPP
#define NOTIFY_HPP

#include <chrono>


namespace notify {

    enum class level : int {
        quiet = 0,
        normal = 1,
        verbose = 2
    };


    void initialize();

    void finalize();


    void set_max_level(level lvl);

    void set_duration(std::chrono::milliseconds dur);

    __attribute__(( __format__ (__printf__, 2, 3)))
    void error(level lvl, const char* fmt, ...);

    __attribute__(( __format__ (__printf__, 2, 3)))
    void info(level lvl, const char* fmt, ...);

    __attribute__(( __format__ (__printf__, 2, 3)))
    void success(level lvl, const char* fmt, ...);

}

#endif
