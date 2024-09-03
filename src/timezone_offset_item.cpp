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

#include "timezone_offset_item.hpp"

#include "time_utils.hpp"
#include "utils.hpp"

// we borrow libwupsxx's cafe_glyphs.h private header
#include <wupsxx/../../src/cafe_glyphs.h>


using namespace std::literals;
using namespace wups::config;


timezone_offset_item::timezone_offset_item(const std::string& label,
                                           std::chrono::minutes& variable) :
    var_item{label, variable, 0min}
{}


std::unique_ptr<timezone_offset_item>
timezone_offset_item::create(const std::string& label,
                             std::chrono::minutes& variable)
{
    return std::make_unique<timezone_offset_item>(label, variable);
}


void
timezone_offset_item::get_display(char* buf, std::size_t size)
    const
{
    auto str = time_utils::tz_offset_to_string(variable);
    ::strlcpy(buf, str.c_str(), size);
}


void
timezone_offset_item::get_focused_display(char* buf, std::size_t size)
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
timezone_offset_item::on_input(const simple_pad_data& input)
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
