/*
 * Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include <exception>
#include <thread>

#include <wups.h>

#include <wupsxx/logger.hpp>

#include "cfg.hpp"
#include "core.hpp"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


// Important plugin information.
WUPS_PLUGIN_NAME(PACKAGE_NAME);
WUPS_PLUGIN_DESCRIPTION("A plugin that synchronizes the system clock to the Internet.");
WUPS_PLUGIN_VERSION(PACKAGE_VERSION);
WUPS_PLUGIN_AUTHOR("Nightkingale, Daniel K. O.");
WUPS_PLUGIN_LICENSE("MIT");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE(PACKAGE_TARNAME);


INITIALIZE_PLUGIN()
{
    wups::logger::guard guard{PACKAGE_NAME};

    cfg::init();

    if (cfg::sync) {
        std::jthread t{core::run};
        t.detach();
    }
}


DEINITIALIZE_PLUGIN()
{
    // TODO: should clean up any worker thread
}
