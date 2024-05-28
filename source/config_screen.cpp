// SPDX-License-Identifier: MIT

#include <memory>               // make_unique()

#include "wupsxx/bool_item.hpp"
#include "wupsxx/int_item.hpp"
#include "wupsxx/text_item.hpp"

#include "config_screen.hpp"

#include "cfg.hpp"
#include "timezone_item.hpp"


using wups::config::bool_item;
using wups::config::int_item;
using wups::config::text_item;


wups::config::category
make_config_screen()
{
    wups::config::category cat{"Configuration"};

    cat.add(bool_item::create(cfg::key::sync, "Syncing Enabled",
                              cfg::sync,
                              "yes", "no"));

    cat.add(bool_item::create(cfg::key::notify, "Show Notifications",
                              cfg::notify,
                              "yes", "no"));

    cat.add(int_item::create(cfg::key::msg_duration, "Notification Duration (seconds)",
                             cfg::msg_duration,
                             0, 30, 5));

    cat.add(int_item::create(cfg::key::hours, "Hours Offset",
                             cfg::hours,
                             -12, 14));

    cat.add(int_item::create(cfg::key::minutes, "Minutes Offset",
                             cfg::minutes,
                             0, 59));

    cat.add(timezone_item::create());

    cat.add(bool_item::create(cfg::key::auto_tz, "Auto-update Timezone Offset",
                              cfg::auto_tz,
                              "yes", "no"));

    cat.add(int_item::create(cfg::key::tolerance, "Tolerance (milliseconds)",
                             cfg::tolerance,
                             0, 5000, 100));

    // show current NTP server address, no way to change it.
    cat.add(text_item::create(cfg::key::server, "NTP Servers", cfg::server));


    cat.add(int_item::create(cfg::key::threads, "Background threads",
                             cfg::threads,
                             0, 8, 2));

    return cat;
}
