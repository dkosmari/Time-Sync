/*
 * Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include <wupsxx/bool_item.hpp>
#include <wupsxx/category.hpp>
#include <wupsxx/duration_items.hpp>
#include <wupsxx/init.hpp>
#include <wupsxx/logger.hpp>
#include <wupsxx/storage.hpp>
#include <wupsxx/text_item.hpp>

#include "cfg.hpp"

#include "preview_screen.hpp"
#include "synchronize_item.hpp"
#include "time_utils.hpp"
#include "time_zone_offset_item.hpp"
#include "time_zone_query_item.hpp"
#include "utils.hpp"
#include "verbosity_item.hpp"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


using std::chrono::hours;
using std::chrono::milliseconds;
using std::chrono::minutes;
using std::chrono::seconds;

using namespace std::literals;
using namespace wups::config;
namespace logger = wups::logger;


namespace cfg {

    namespace keys {
        const char* auto_tz      = "auto_tz";
        const char* msg_duration = "msg_duration";
        const char* notify       = "notify";
        const char* server       = "server";
        const char* sync         = "sync";
        const char* threads      = "threads";
        const char* timeout      = "timeout";
        const char* tolerance    = "tolerance";
        const char* tz_service   = "tz_service";
        const char* utc_offset   = "utc_offset";
    }


    namespace labels {
        const char* auto_tz      = "Auto Update Time Zone";
        const char* msg_duration = "Notification Duration";
        const char* notify       = "Show Notifications";
        const char* server       = "NTP Servers";
        const char* sync         = "Syncing Enabled";
        const char* threads      = "Background Threads";
        const char* timeout      = "Timeout";
        const char* tolerance    = "Tolerance";
        const char* tz_service   = "Detect Time Zone";
        const char* utc_offset   = "Time Offset (UTC)";
    }


    namespace defaults {
        const bool         auto_tz      = false;
        const seconds      msg_duration = 5s;
        const int          notify       = 1;
        const std::string  server       = "pool.ntp.org";
        const bool         sync         = false;
        const int          threads      = 4;
        const seconds      timeout      = 5s;
        const milliseconds tolerance    = 500ms;
        const int          tz_service   = 0;
        const minutes      utc_offset   = 0min;
    }


    bool         auto_tz      = defaults::auto_tz;
    seconds      msg_duration = defaults::msg_duration;
    int          notify       = defaults::notify;
    std::string  server       = defaults::server;
    bool         sync         = defaults::sync;
    int          threads      = defaults::threads;
    seconds      timeout      = defaults::timeout;
    milliseconds tolerance    = defaults::tolerance;
    int          tz_service   = defaults::tz_service;
    minutes      utc_offset   = 0min;


    category
    make_config_screen()
    {
        category cat{"Configuration"};

        cat.add(bool_item::create(cfg::labels::sync,
                                  cfg::sync,
                                  cfg::defaults::sync,
                                  "on", "off"));

        cat.add(verbosity_item::create(cfg::labels::notify,
                                       cfg::notify,
                                       cfg::defaults::notify));

        cat.add(time_zone_offset_item::create(cfg::labels::utc_offset,
                                              cfg::utc_offset,
                                              cfg::defaults::utc_offset));

        cat.add(time_zone_query_item::create(cfg::labels::tz_service,
                                             cfg::tz_service,
                                             cfg::defaults::tz_service));

        cat.add(bool_item::create(cfg::labels::auto_tz,
                                  cfg::auto_tz,
                                  cfg::defaults::auto_tz,
                                  "on", "off"));

        cat.add(seconds_item::create(cfg::labels::msg_duration,
                                     cfg::msg_duration,
                                     cfg::defaults::msg_duration,
                                     1s, 30s, 5s));

        cat.add(seconds_item::create(cfg::labels::timeout,
                                     cfg::timeout,
                                     cfg::defaults::timeout,
                                     1s, 10s, 5s));

        cat.add(milliseconds_item::create(cfg::labels::tolerance,
                                          cfg::tolerance,
                                          cfg::defaults::tolerance,
                                          0ms, 5000ms, 100ms));

        cat.add(int_item::create(cfg::labels::threads,
                                 cfg::threads,
                                 cfg::defaults::threads,
                                 0, 8, 2));

        // show current NTP server address, no way to change it.
        cat.add(text_item::create(cfg::labels::server,
                                  cfg::server));

        return cat;
    }


    void
    menu_open(category& root)
    {
        logger::initialize(PACKAGE_NAME);

        logger::printf("reloading configs\n");
        cfg::reload();

        logger::printf("building config items\n");
        root.add(make_config_screen());
        root.add(make_preview_screen());
        root.add(synchronize_item::create());
    }


    void
    menu_close()
    {
        logger::printf("saving config\n");
        cfg::save();
        logger::finalize();
    }



    void migrate_old_config();


    void
    init()
    {
        try {
            wups::config::init(PACKAGE_NAME,
                               menu_open,
                               menu_close);
            cfg::load();
            cfg::migrate_old_config();
        }
        catch (std::exception& e) {
            logger::printf("Init error: %s\n", e.what());
        }
    }


    void
    load()
    {
        try {
#define LOAD(x) wups::storage::load_or_init(keys::x, x, defaults::x)
            LOAD(auto_tz);
            LOAD(msg_duration);
            LOAD(notify);
            LOAD(server);
            LOAD(sync);
            LOAD(threads);
            LOAD(timeout);
            LOAD(tolerance);
            LOAD(tz_service);
            LOAD(utc_offset);
#undef LOAD
            // logger::printf("Loaded settings.");
        }
        catch (std::exception& e) {
            logger::printf("Error loading config: %s", e.what());
        }
    }


    void
    reload()
    {
        try {
            wups::storage::reload();
            load();
        }
        catch (std::exception& e) {
            logger::printf("Error reloading config: %s", e.what());
        }
    }


    void
    save()
    {
        try {
#define STORE(x) wups::storage::store(keys::x, x)
            STORE(auto_tz);
            STORE(msg_duration);
            STORE(notify);
            STORE(server);
            STORE(sync);
            STORE(threads);
            STORE(timeout);
            STORE(tolerance);
            STORE(tz_service);
            STORE(utc_offset);
#undef STORE
            wups::storage::save();
            // logger::printf("Saved settings");
        }
        catch (std::exception& e) {
            logger::printf("Error saving config: %s", e.what());
        }
    }


    void
    migrate_old_config()
    {
        // check for leftovers from old versions
        auto hrs = wups::storage::load<hours>("hours");
        auto min = wups::storage::load<minutes>("minutes");
        if (hrs || min) {
            hours h = hrs.value_or(0h);
            minutes m = min.value_or(0min);
            minutes offset = h + m;
            set_and_store_utc_offset(offset);
            WUPSStorageAPI::DeleteItem("hours");
            WUPSStorageAPI::DeleteItem("minutes");
            save();
            logger::printf("Migrated old config: %s + %s -> %s.",
                            time_utils::to_string(h).c_str(),
                            time_utils::to_string(m).c_str(),
                            time_utils::tz_offset_to_string(utc_offset).c_str());
        }
    }


    void
    set_and_store_utc_offset(minutes offset)
    {
        logger::guard guard(PACKAGE_NAME);
        /*
         * Normally, `utc_offset` is saved when closing the config menu.
         * If auto_tz is enabled, it will be updated and saved outside the config menu.
         */
        utc_offset = offset;
        try {
            wups::storage::store(keys::utc_offset, utc_offset);
            wups::storage::save();
        }
        catch (std::exception& e) {
            logger::printf("Error storing utc_offset: %s", e.what());
        }
    }

} // namespace cfg
