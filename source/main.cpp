// SPDX-License-Identifier: MIT

// standard headers
#include <memory>               // make_unique()
#include <thread>

// WUT/WUPS headers
#include <notifications/notifications.h>
#include <wups.h>

// DEBUG code
#include <mocha/mocha.h>

// local headers
#include "cfg.hpp"
#include "config_screen.hpp"
#include "core.hpp"
#include "log.hpp"
#include "notify.hpp"
#include "preview_screen.hpp"

#include "wupsxx/category.hpp"


// Important plugin information.
WUPS_PLUGIN_NAME(PLUGIN_NAME);
WUPS_PLUGIN_DESCRIPTION(PLUGIN_DESCRIPTION);
WUPS_PLUGIN_VERSION(PLUGIN_VERSION);
WUPS_PLUGIN_AUTHOR(PLUGIN_AUTHOR);
WUPS_PLUGIN_LICENSE(PLUGIN_LICENSE);

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE(PLUGIN_NAME);


static WUPSConfigAPICallbackStatus open_config(WUPSConfigCategoryHandle root_handle);
static void close_config();


INITIALIZE_PLUGIN()
{
    logging::initialize();

    auto status = WUPSConfigAPI_Init({ .name = PLUGIN_NAME },
                                     open_config,
                                     close_config);
    if (status != WUPSCONFIG_API_RESULT_SUCCESS) {
        logging::printf("Init error: %s", WUPSConfigAPI_GetStatusStr(status));
        return;
    }

    // DEBUG code
    Mocha_InitLibrary();
    Mocha_IOSUKernelWrite32(0x05025aac, 0x46c046c0);
    Mocha_IOSUKernelWrite32(0x05025ab0, 0x46c068fd);
    Mocha_DeInitLibrary();


    cfg::load();

    if (cfg::notify)
        notify::initialize();

    if (cfg::sync)
        core::sync_clock(); // Update clock when plugin is loaded.

}


DEINITIALIZE_PLUGIN()
{
    notify::cleanup();
    logging::cleanup();
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
    catch (...) {
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }
}


static
void
close_config()
{
    cfg::save();

    if (cfg::notify)
        notify::initialize();
    else
        notify::cleanup();

    std::jthread update_time_thread(core::sync_clock);
    update_time_thread.detach(); // Update time when settings are closed.
}
