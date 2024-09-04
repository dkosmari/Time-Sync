/*
 * Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>               // snprintf()
#include <exception>
#include <string.h>             // BSD strlcpy()

#include <wupsxx/cafe_glyphs.h>
#include <wupsxx/logger.hpp>
#include <wupsxx/storage.hpp>

#include "timezone_query_item.hpp"

#include "cfg.hpp"
#include "utils.hpp"


using namespace std::literals;
using namespace wups::config;
namespace logger = wups::logger;


namespace {
    std::string
    make_query_text(int idx)
    {
        return "Query "s + utils::get_tz_service_name(idx);
    }
} // namespace


timezone_query_item::timezone_query_item(const std::string& label,
                                         int& variable,
                                         const int default_value) :
    var_item{label, variable, default_value},
    text{make_query_text(variable)}
{}


std::unique_ptr<timezone_query_item>
timezone_query_item::create(const std::string& label,
                            int& variable,
                            const int default_value)
{
    return std::make_unique<timezone_query_item>(label, variable, default_value);
}


void
timezone_query_item::get_display(char* buf, std::size_t size)
    const
{
    ::strlcpy(buf, text.c_str(), size);
}


void
timezone_query_item::get_focused_display(char* buf, std::size_t size)
    const
{
    std::snprintf(buf, size,
                  "%s %s %s",
                  CAFE_GLYPH_BTN_LEFT,
                  make_query_text(variable).c_str(),
                  CAFE_GLYPH_BTN_RIGHT);
}


focus_status
timezone_query_item::on_input(const simple_pad_data& input)
{

    const int n = utils::get_num_tz_services();
    // auto prev_variable = variable;

    if (input.buttons_d & WUPS_CONFIG_BUTTON_LEFT)
        --variable;

    if (input.buttons_d & WUPS_CONFIG_BUTTON_RIGHT)
        ++variable;

    // let it wrap around
    if (variable < 0)
        variable += n;
    if (variable >= n)
        variable -= n;

    if (!(input.buttons_d & WUPS_CONFIG_BUTTON_B))
        text = make_query_text(variable);

    if (input.buttons_d & WUPS_CONFIG_BUTTON_A)
        run();

    return var_item::on_input(input);
}


void
timezone_query_item::run()
{
    try {
        auto [name, offset] = utils::fetch_timezone(variable);
        text = name;
        cfg::set_and_store_utc_offset(offset);
    }
    catch (std::exception& e) {
        text = "Error: "s + e.what();
    }
}
