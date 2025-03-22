/*
 * Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstdarg>

#include <wupsxx/logger.hpp>
#include <wupsxx/notify.hpp>

#include "notify.hpp"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


namespace logger = wups::logger;


namespace notify {

    level max_level = level::quiet;


    void
    initialize()
    {
        wups::notify::initialize(PACKAGE_NAME);

        wups::notify::info::set_text_color(255, 255, 255, 255);
        wups::notify::info::set_bg_color(32, 32, 160, 255);

        wups::notify::error::set_text_color(255, 255, 255, 255);
        wups::notify::error::set_bg_color(160, 32, 32, 255);
    }


    void
    finalize()
    {
        wups::notify::finalize();
    }


    void
    set_max_level(level lvl)
    {
        max_level = lvl;
    }


    void
    set_duration(std::chrono::milliseconds dur)
    {
        wups::notify::info::set_duration(dur);
        wups::notify::error::set_duration(dur);
    }


    __attribute__(( __format__ (__printf__, 2, 3)))
    void
    error(level lvl, const char* fmt, ...)
    {
        {
            std::va_list args;
            va_start(args, fmt);
            logger::vprintf(fmt, args);
            va_end(args);
        }

        if (lvl > max_level)
            return;

        std::va_list args;
        va_start(args, fmt);
        try {
            wups::notify::error::vshow(fmt, args);
        }
        catch (std::exception& e) {
            logger::printf("notification error: %s\n", e.what());
        }
        va_end(args);

    }


    __attribute__(( __format__ (__printf__, 2, 3)))
    void
    info(level lvl, const char* fmt, ...)
    {
        {
            std::va_list args;
            va_start(args, fmt);
            logger::vprintf(fmt, args);
            va_end(args);
        }

        if (lvl > max_level)
            return;

        std::va_list args;
        va_start(args, fmt);
        try {
            wups::notify::info::vshow(fmt, args);
        }
        catch (std::exception& e) {
            logger::printf("notification error: %s\n", e.what());
        }
        va_end(args);
    }


    __attribute__(( __format__ (__printf__, 2, 3)))
    void
    success(level lvl, const char* fmt, ...)
    {
        {
            std::va_list args;
            va_start(args, fmt);
            logger::vprintf(fmt, args);
            va_end(args);
        }

        if (lvl > max_level)
            return;

        std::va_list args;
        va_start(args, fmt);
        try {
            wups::notify::info::vshow(wups::color{255, 255, 255},
                                      wups::color{32, 160, 32},
                                      fmt,
                                      args);
        }
        catch (std::exception& e) {
            logger::printf("notification error: %s\n", e.what());
        }
        va_end(args);
    }

} // namespace notify
