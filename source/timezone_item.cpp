// SPDX-License-Identifier: MIT

#include "timezone_item.hpp"

#include "cfg.hpp"
#include "nintendo_glyphs.h"
#include "utils.hpp"


using namespace std::literals;


timezone_item::timezone_item() :
    wups::config::text_item{{},
                            "Detect Timezone (press " NIN_GLYPH_BTN_A ")",
                            "Using http://ip-api.com",
                            30}
{}


std::unique_ptr<timezone_item>
timezone_item::create()
{
    return std::make_unique<timezone_item>();
}


void
timezone_item::on_input(WUPSConfigSimplePadData input)
{
    text_item::on_input(input);

    if (input.buttons_d & WUPS_CONFIG_BUTTON_A)
        update_timezone();
}


void
timezone_item::update_timezone()
{
    try {
        auto [tz_name, tz_offset] = utils::fetch_timezone();
        text = tz_name;
        cfg::set_tz_offset(tz_offset);
    }
    catch (std::exception& e) {
        text = "Error: "s + e.what();
    }
}
