// SPDX-License-Identifier: MIT

#include <algorithm>            // clamp()
#include <cstdio>
#include <exception>

#include "wupsxx/int_item.hpp"

#include "log.hpp"
#include "nintendo_glyphs.hpp"
#include "wupsxx/storage.hpp"


// TODO: have a Config parameter for constructor

namespace wups::config {

    int_item::int_item(const std::optional<std::string>& key,
                       const std::string& name,
                       int& variable,
                       int min_value,
                       int max_value,
                       int fast_increment) :
        item{key, name},
        variable(variable),
        default_value{variable},
        min_value{min_value},
        max_value{max_value},
        fast_increment{fast_increment}
    {}


    std::unique_ptr<int_item>
    int_item::create(const std::optional<std::string>& key,
                     const std::string& name,
                     int& variable,
                     int min_value,
                     int max_value,
                     int fast_increment)
    {
        return std::make_unique<int_item>(key, name, variable,
                                          min_value, max_value, fast_increment);
    }


    int
    int_item::get_display(char* buf, std::size_t size)
        const
    {
        std::snprintf(buf, size, "%d", variable);
        return 0;
    }


    int
    int_item::get_selected_display(char* buf, std::size_t size)
        const
    {
        const char* left = "";
        const char* right = "";
        const char* fast_left = "";
        const char* fast_right = "";
        if (variable > min_value) {
            left = NIN_GLYPH_BTN_DPAD_LEFT;
            fast_left = NIN_GLYPH_BTN_L;
        } if (variable < max_value) {
            right = NIN_GLYPH_BTN_DPAD_RIGHT;
            fast_right = NIN_GLYPH_BTN_R;
        }
        std::snprintf(buf, size,
                      "%s%s %d %s%s",
                      fast_left,
                      left,
                      variable,
                      right,
                      fast_right);
        return 0;
    }


    void
    int_item::restore()
    {
        variable = default_value;
        on_changed();
    }


    // TODO: handle held button
    void
    int_item::on_input(WUPSConfigSimplePadData input)
    {
        item::on_input(input);

        if (input.buttons_d & WUPS_CONFIG_BUTTON_LEFT)
            --variable;

        if (input.buttons_d & WUPS_CONFIG_BUTTON_RIGHT)
            ++variable;

        if (input.buttons_d & WUPS_CONFIG_BUTTON_L)
            variable -= fast_increment;

        if (input.buttons_d & WUPS_CONFIG_BUTTON_R)
            variable += fast_increment;

        variable = std::clamp(variable, min_value, max_value);

        on_changed();
    }


    void
    int_item::on_changed()
    {
        if (!key)
            return;

        try {
            storage::store(*key, variable);
        }
        catch (std::exception& e) {
            LOG("Error storing int: %s", e.what());
        }
    }

} // namespace wups::config
