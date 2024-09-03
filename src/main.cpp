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

#include "cfg.hpp"
#include "config_screen.hpp"
#include "core.hpp"
#include "logger.hpp"
#include "notify.hpp"
#include "preview_screen.hpp"
#include "wupsxx/category.hpp"

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


static WUPSConfigAPICallbackStatus open_config(WUPSConfigCategoryHandle root_handle);
static void close_config();


INITIALIZE_PLUGIN()
{
    logger::initialize();

    auto status = WUPSConfigAPI_Init({ .name = PACKAGE_NAME },
                                     open_config,
                                     close_config);
    if (status != WUPSCONFIG_API_RESULT_SUCCESS) {
        logger::printf("Init error: %s", WUPSConfigAPI_GetStatusStr(status));
        return;
    }

    cfg::load();
    cfg::migrate_old_config();

    if (cfg::sync)
        core::run(); // Update clock when plugin is loaded.
}


DEINITIALIZE_PLUGIN()
{
    logger::finalize();
}


static
WUPSConfigAPICallbackStatus
open_config(WUPSConfigCategoryHandle root_handle)
{
    try {
        cfg::reload();

        wups::config::category root{root_handle};

        root.add(make_config_screen());
        root.add(make_preview_screen());

        return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
    }
    catch (std::exception& e) {
        logger::printf("Error opening config: %s", e.what());
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }
}


static
void
close_config()
{
    cfg::save();

    // Update time when settings are closed.
    std::jthread update_time_thread{core::run};
    update_time_thread.detach();
}
