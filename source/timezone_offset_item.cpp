// SPDX-License-Identifier: MIT

#include <algorithm>            // clamp()
#include <cmath>                // abs()
#include <cstdio>               // snprintf()

#include "timezone_offset_item.hpp"

#include "logging.hpp"
#include "nintendo_glyphs.h"
#include "wupsxx/storage.hpp"


using namespace std::literals;


timezone_offset_item::timezone_offset_item(const std::optional<std::string>& key,
                                           const std::string& label,
                                           std::chrono::minutes& variable) :
    item{key, label},
    variable(variable)
{}


std::unique_ptr<timezone_offset_item>
timezone_offset_item::create(const std::optional<std::string>& key,
                             const std::string& label,
                             std::chrono::minutes& variable)
{
    return std::make_unique<timezone_offset_item>(key, label, variable);
}


int
timezone_offset_item::get_display(char* buf, std::size_t size)
    const
{
    int hours = variable.count() / 60;
    int minutes = std::abs(variable.count() % 60);
    std::snprintf(buf, size, "%+02d:%02d", hours, minutes);
    return 0;
}


int
timezone_offset_item::get_selected_display(char* buf, std::size_t size)
    const
{
    const char* slow_left = "";
    const char* slow_right = "";
    const char* fast_left = "";
    const char* fast_right = "";
    if (variable > -12h) {
        slow_left = NIN_GLYPH_BTN_DPAD_LEFT;
        fast_left = NIN_GLYPH_BTN_L;
    } if (variable < 14h) {
        slow_right = NIN_GLYPH_BTN_DPAD_RIGHT;
        fast_right = NIN_GLYPH_BTN_R;
    }

    int hours = variable.count() / 60;
    int minutes = std::abs(variable.count() % 60);
    std::snprintf(buf, size, "%s%s%+02d:%02d%s%s",
                  fast_left,
                  slow_left,
                  hours, minutes,
                  slow_right,
                  fast_right);
    return 0;
}


void
timezone_offset_item::restore()
{
    variable = 0min;
}


void
timezone_offset_item::on_input(WUPSConfigSimplePadData input)
{
    item::on_input(input);

    if (input.buttons_d & WUPS_CONFIG_BUTTON_LEFT)
        variable -= 15min;

    if (input.buttons_d & WUPS_CONFIG_BUTTON_RIGHT)
        variable += 15min;

    if (input.buttons_d & WUPS_CONFIG_BUTTON_L)
        variable -= 1h;

    if (input.buttons_d & WUPS_CONFIG_BUTTON_R)
        variable += 1h;

    variable = std::clamp<std::chrono::minutes>(variable, -12h, 14h);

    on_changed();
}


void
timezone_offset_item::on_changed()
{
    if (!key)
        return;

    try {
        wups::storage::store<int>(*key, variable.count());
    }
    catch (std::exception& e) {
        logging::printf("Error storing timezone offset: %s", e.what());
    }
}
