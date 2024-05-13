// SPDX-License-Identifier: MIT

#include <memory>               // make_unique()

#include "wupsxx/bool_item.hpp"
#include "wupsxx/int_item.hpp"
#include "wupsxx/text_item.hpp"

#include "config_screen.hpp"

#include "cfg.hpp"
#include "http_client.hpp"
#include "nintendo_glyphs.hpp"
#include "utils.hpp"


using wups::config::bool_item;
using wups::config::int_item;
using wups::config::text_item;

using namespace std::literals;


struct timezone_item : text_item {

    timezone_item() :
        text_item{{},
        "Detect Timezone (press " NIN_GLYPH_BTN_A ")",
        "Using http://ip-api.com"}
    {}


    static
    std::unique_ptr<timezone_item>
    create()
    {
        return std::make_unique<timezone_item>();
    }


    void
    on_input(WUPSConfigSimplePadData input)
        override
    {
        text_item::on_input(input);

        if (input.buttons_d & WUPS_CONFIG_BUTTON_A) {
            try {
                query_timezone();
            }
            catch (std::exception& e) {
                text = "Error: "s + e.what();
            }
        }
    }


    void
    query_timezone()
    {
        std::string tz = http::get("http://ip-api.com/line/?fields=timezone,offset");
        auto tokens = utils::split(tz, " \r\n");
        if (tokens.size() != 2)
            throw std::runtime_error{"Could not parse response from \"ip-api.com\"."};

        text = tokens[0];

        int tz_offset = std::stoi(tokens[1]);
        cfg::update_offsets_from_tz_offset(tz_offset);
    }

};


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

    cat.add(int_item::create(cfg::key::tolerance, "Tolerance (milliseconds)",
                             cfg::tolerance,
                             0, 5000, 100));

    // show current NTP server address, no way to change it.
    cat.add(text_item::create(cfg::key::server, "NTP Servers", cfg::server));

    return cat;
}
