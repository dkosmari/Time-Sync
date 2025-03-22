/*
 * Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2025  Daniel K. O.
 * Copyright (C) 2024  Nightkingale
 *
 * SPDX-License-Identifier: MIT
 */

#include <wups.h>

#include <wupsxx/logger.hpp>

#include "cfg.hpp"
#include "core.hpp"
#include "notify.hpp"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


// Important plugin information.
WUPS_PLUGIN_NAME(PACKAGE_NAME);
WUPS_PLUGIN_VERSION(PACKAGE_VERSION);
WUPS_PLUGIN_DESCRIPTION("A plugin that synchronizes the system clock to the Internet.");
WUPS_PLUGIN_AUTHOR("Nightkingale, Daniel K. O.");
WUPS_PLUGIN_LICENSE("MIT");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE(PACKAGE_TARNAME);


INITIALIZE_PLUGIN()
{
    wups::logger::set_prefix(PACKAGE_NAME);
    wups::logger::guard guard;
    notify::initialize();
    cfg::init();
}


DEINITIALIZE_PLUGIN()
{
    core::background::stop();
    notify::finalize();
}


ON_APPLICATION_START()
{
    if (cfg::sync_on_boot.value)
        core::background::run_once();
}


ON_APPLICATION_REQUESTS_EXIT()
{
    core::background::stop();
}
