// SPDX-License-Identifier: MIT

// standard headers
#include <memory>               // make_unique()
#include <thread>

// WUT/WUPS headers
#include <notifications/notifications.h>
#include <whb/log_udp.h>
#include <wups.h>

// local headers
#include "cfg.hpp"
#include "config_screen.hpp"
#include "core.hpp"
#include "log.hpp"
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
    WHBLogUdpInit();
    NotificationModule_InitLibrary(); // Set up for notifications.

    auto status = WUPSConfigAPI_Init({ .name = PLUGIN_NAME },
                                     open_config,
                                     close_config);
    if (status != WUPSCONFIG_API_RESULT_SUCCESS) {
        LOG("Init error: %s", WUPSConfigAPI_GetStatusStr(status));
        return;
    }

    cfg::load();

    if (cfg::sync)
        core::sync_clock(); // Update clock when plugin is loaded.

}


DEINITIALIZE_PLUGIN()
{
    NotificationModule_DeInitLibrary();
    WHBLogUdpDeinit();
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

    std::jthread update_time_thread(core::sync_clock);
    update_time_thread.detach(); // Update time when settings are closed.
}
