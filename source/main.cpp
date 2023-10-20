// SPDX-License-Identifier: MIT

// standard headers
#include <atomic>
#include <cstdlib>
#include <memory>               // unique_ptr<>
#include <stdexcept>
#include <string>
#include <thread>

// WUT/WUPS headers
#include <notifications/notifications.h>
#include <wups.h>
#include <whb/log_udp.h>

// local headers
#include "wupsxx/bool_item.hpp"
#include "wupsxx/category.hpp"
#include "wupsxx/config.hpp"
#include "wupsxx/int_item.hpp"
#include "wupsxx/storage.hpp"
#include "wupsxx/text_item.hpp"

#include "cfg.hpp"
#include "http_client.hpp"
#include "log.hpp"
#include "nintendo_glyphs.hpp"
#include "ntp.hpp"
#include "preview.hpp"
#include "utc.hpp"
#include "utils.hpp"
#include "core.hpp"


using namespace std::literals;


#define CFG_HOURS        "hours"
#define CFG_MINUTES      "minutes"
#define CFG_MSG_DURATION "msg_duration"
#define CFG_NOTIFY       "notify"
#define CFG_SERVER       "server"
#define CFG_SYNC         "sync"
#define CFG_TOLERANCE    "tolerance"

// Important plugin information.
WUPS_PLUGIN_NAME(PLUGIN_NAME);
WUPS_PLUGIN_DESCRIPTION("A plugin that synchronizes a Wii U's clock to the Internet.");
WUPS_PLUGIN_VERSION("v2.1.0");
WUPS_PLUGIN_AUTHOR("Nightkingale, Daniel K. O.");
WUPS_PLUGIN_LICENSE("MIT");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE(PLUGIN_NAME);








template<typename T>
void
load_or_init(const std::string& key,
             T& variable)
{
    auto val = wups::load<T>(key);
    if (!val)
        wups::store(key, variable);
    else
        variable = *val;
}


INITIALIZE_PLUGIN()
{
    WHBLogUdpInit();

    // Check if the plugin's settings have been saved before.
    if (WUPS_OpenStorage() == WUPS_STORAGE_ERROR_SUCCESS) {

        load_or_init(CFG_HOURS,        cfg::hours);
        load_or_init(CFG_MINUTES,      cfg::minutes);
        load_or_init(CFG_MSG_DURATION, cfg::msg_duration);
        load_or_init(CFG_NOTIFY,       cfg::notify);
        load_or_init(CFG_SERVER,       cfg::server);
        load_or_init(CFG_SYNC,         cfg::sync);
        load_or_init(CFG_TOLERANCE,    cfg::tolerance);

        WUPS_CloseStorage();
    }

    NotificationModule_InitLibrary(); // Set up for notifications.

    if (cfg::sync)
        core::sync_clock(); // Update clock when plugin is loaded.
}


struct timezone_item : wups::text_item {

    timezone_item() :
        wups::text_item{"",
                        "Detect timezone (press " NIN_GLYPH_BTN_A ")",
                        "Using http://ip-api.com"}
    {}


    void
    on_button_pressed(WUPSConfigButtons buttons)
        override
    {
        text_item::on_button_pressed(buttons);

        if (buttons & WUPS_CONFIG_BUTTON_A)
            query_timezone();
    }


    void
    query_timezone()
    try {
        std::string tz = http::get("http://ip-api.com/line/?fields=timezone,offset");
        auto tokens = utils::split(tz, " \r\n");
        if (tokens.size() != 2)
            throw std::runtime_error{"Could not parse response from \"ip-api.com\"."};

        int tz_offset = std::stoi(tokens[1]);
        text = tokens[0];

        cfg::hours = tz_offset / (60 * 60);
        cfg::minutes = tz_offset % (60 * 60) / 60;
        if (cfg::minutes < 0) {
            cfg::minutes += 60;
            --cfg::hours;
        }
    }
    catch (std::exception& e) {
        text = "Error: "s + e.what();
    }

};



WUPS_GET_CONFIG()
{
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS)
        return 0;

    using std::make_unique;

    try {

        auto config = make_unique<wups::category>("Configuration");

        config->add(make_unique<wups::bool_item>(CFG_SYNC,
                                                 "Syncing Enabled",
                                                 cfg::sync));

        config->add(make_unique<wups::bool_item>(CFG_NOTIFY,
                                                 "Show Notifications",
                                                 cfg::notify));

        config->add(make_unique<wups::int_item>(CFG_MSG_DURATION,
                                                "Notification Duration (seconds)",
                                                cfg::msg_duration, 0, 30));

        config->add(make_unique<wups::int_item>(CFG_HOURS,
                                                "Hours Offset",
                                                cfg::hours, -12, 14));

        config->add(make_unique<wups::int_item>(CFG_MINUTES,
                                                "Minutes Offset",
                                                cfg::minutes, 0, 59));

        config->add(make_unique<timezone_item>());

        config->add(make_unique<wups::int_item>(CFG_TOLERANCE,
                                                "Tolerance (milliseconds)",
                                                cfg::tolerance, 0, 5000));

        // show current NTP server address, no way to change it.
        config->add(make_unique<wups::text_item>(CFG_SERVER,
                                                 "NTP servers",
                                                 cfg::server));

        auto root = make_unique<wups::config>(PLUGIN_NAME);
        root->add(std::move(config));
        root->add(make_unique<preview>());

        return root.release()->handle;

    }
    catch (...) {
        return 0;
    }
}


WUPS_CONFIG_CLOSED()
{
    std::jthread update_time_thread(core::sync_clock);
    update_time_thread.detach(); // Update time when settings are closed.

    WUPS_CloseStorage(); // Save all changes.
}
