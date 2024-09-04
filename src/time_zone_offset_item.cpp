/*
 * Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include <algorithm>            // clamp()
#include <cmath>                // abs()
#include <cstdio>               // snprintf()
#include <string.h>             // BSD strlcpy()

#include <wupsxx/cafe_glyphs.h>

#include "time_zone_offset_item.hpp"

#include "time_utils.hpp"
#include "utils.hpp"


using namespace std::literals;
using namespace wups::config;


time_zone_offset_item::time_zone_offset_item(const std::string& label,
                                             std::chrono::minutes& variable,
                                             std::chrono::minutes default_value) :
    var_item{label, variable, default_value}
{}


std::unique_ptr<time_zone_offset_item>
time_zone_offset_item::create(const std::string& label,
                              std::chrono::minutes& variable,
                              std::chrono::minutes default_value)
{
    return std::make_unique<time_zone_offset_item>(label, variable, default_value);
}


void
time_zone_offset_item::get_display(char* buf, std::size_t size)
    const
{
    auto str = time_utils::tz_offset_to_string(variable);
    ::strlcpy(buf, str.c_str(), size);
}


void
time_zone_offset_item::get_focused_display(char* buf, std::size_t size)
    const
{
    const char* slow_left = "";
    const char* slow_right = "";
    const char* fast_left = "";
    const char* fast_right = "";
    if (variable > -12h) {
        slow_left = CAFE_GLYPH_BTN_LEFT " ";
        fast_left = CAFE_GLYPH_BTN_L;
    } if (variable < 14h) {
        slow_right = " " CAFE_GLYPH_BTN_RIGHT;
        fast_right = CAFE_GLYPH_BTN_R;
    }

    auto str = time_utils::tz_offset_to_string(variable);
    std::snprintf(buf, size, "%s%s" "%s" "%s%s",
                  fast_left, slow_left,
                  str.c_str(),
                  slow_right, fast_right);
}


focus_status
time_zone_offset_item::on_input(const simple_pad_data& input)
{

    if (input.pressed_or_repeated(WUPS_CONFIG_BUTTON_LEFT))
        variable -= 1min;

    if (input.pressed_or_repeated(WUPS_CONFIG_BUTTON_RIGHT))
        variable += 1min;

    if (input.pressed_or_repeated(WUPS_CONFIG_BUTTON_L))
        variable -= 1h;

    if (input.pressed_or_repeated(WUPS_CONFIG_BUTTON_R))
        variable += 1h;

    variable = std::clamp<std::chrono::minutes>(variable, -12h, 14h);

    return item::on_input(input);
}
