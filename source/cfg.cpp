// SPDX-License-Identifier: MIT

#include "cfg.hpp"

#include "logging.hpp"
#include "wupsxx/storage.hpp"


namespace cfg {

    namespace key {
        const char* auto_tz      = "auto_tz";
        const char* hours        = "hours";
        const char* minutes      = "minutes";
        const char* msg_duration = "msg_duration";
        const char* notify       = "notify";
        const char* server       = "server";
        const char* sync         = "sync";
        const char* threads      = "threads";
        const char* tolerance    = "tolerance";
    }


    bool        auto_tz      = false;
    int         hours        = 0;
    int         minutes      = 0;
    int         msg_duration = 5;
    bool        notify       = true;
    std::string server       = "pool.ntp.org";
    bool        sync         = false;
    int         threads      = 4;
    int         tolerance    = 250;


    template<typename T>
    void
    load_or_init(const std::string& key,
                 T& variable)
    {
        auto val = wups::storage::load<T>(key);
        if (!val)
            wups::storage::store(key, variable);
        else
            variable = *val;
    }


    void
    load()
    {
        try {
            load_or_init(key::auto_tz,      auto_tz);
            load_or_init(key::hours,        hours);
            load_or_init(key::minutes,      minutes);
            load_or_init(key::msg_duration, msg_duration);
            load_or_init(key::notify,       notify);
            load_or_init(key::server,       server);
            load_or_init(key::sync,         sync);
            load_or_init(key::threads,      threads);
            load_or_init(key::tolerance,    tolerance);
            logging::printf("loaded settings");
        }
        catch (std::exception& e) {
            logging::printf("error loading config: %s", e.what());
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
            logging::printf("error reloading config: %s", e.what());
        }
    }


    void
    save()
    {
        try {
            wups::storage::save();
            logging::printf("saved settings");
        }
        catch (std::exception& e) {
            logging::printf("error saving config: %s", e.what());
        }
    }


    int
    get_tz_offset()
    {
        return (cfg::hours * 60 + cfg::minutes) * 60;
    }


    void
    set_tz_offset(int tz_offset)
    {
        hours = tz_offset / (60 * 60);
        minutes = tz_offset % (60 * 60) / 60;
        if (minutes < 0) {
            minutes += 60;
            --hours;
        }

        // Since these were updated from outside their UI items, we gotta store the new
        // values manually.
        try {
            wups::storage::store(key::hours, hours);
            wups::storage::store(key::minutes, minutes);
        }
        catch (std::exception& e) {
            logging::printf("error storing tz offsets: %s", e.what());
        }
    }

} // namespace cfg
