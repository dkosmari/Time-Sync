/*
 * Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LOGGER_HPP
#define LOGGER_HPP


namespace logger {

    void initialize();

    void finalize();

    __attribute__(( __format__ (__printf__, 1, 2) ))
    void printf(const char* fmt, ...);

} // namespace logger

#endif
