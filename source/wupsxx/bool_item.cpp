// SPDX-License-Identifier: MIT

#include <cstdio>
#include <exception>

#include "wupsxx/bool_item.hpp"

#include "nintendo_glyphs.hpp"
#include "wupsxx/storage.hpp"
#include "log.hpp"


namespace wups::config {

    bool_item::bool_item(const std::optional<std::string>& key,
                         const std::string& name,
                         bool& variable) :
        base_item{key, name},
        variable(variable),
        default_value{variable}
    {}


    std::unique_ptr<bool_item>
    bool_item::create(const std::optional<std::string>& key,
                      const std::string& name,
                      bool& variable)
    {
        return std::make_unique<bool_item>(key, name, variable);
    }


    int
    bool_item::get_display(char* buf, std::size_t size)
        const
    {
        std::snprintf(buf, size, "%s",
                      variable ? true_str.c_str() : false_str.c_str());
        return 0;
    }


    int
    bool_item::get_selected_display(char* buf, std::size_t size)
        const
    {
        if (variable)
            std::snprintf(buf, size, "%s %s  ", NIN_GLYPH_BTN_DPAD_LEFT, true_str.c_str());
        else
            std::snprintf(buf, size, "  %s %s", false_str.c_str(), NIN_GLYPH_BTN_DPAD_RIGHT);
        return 0;
    }


    void
    bool_item::restore()
    {
        variable = default_value;
        on_changed();
    }


    void
    bool_item::on_input(WUPSConfigSimplePadData input)
    {
        base_item::on_input(input);

        if (input.buttons_d & WUPS_CONFIG_BUTTON_A)
            variable = !variable;

        if (input.buttons_d & WUPS_CONFIG_BUTTON_LEFT)
            variable = false;

        if (input.buttons_d & WUPS_CONFIG_BUTTON_RIGHT)
            variable = true;

        on_changed();
    }


    void
    bool_item::on_changed()
    {
        if (!key)
            return;

        try {
            storage::store(*key, variable);
        }
        catch (std::exception& e) {
            LOG("Error storing bool: %s", e.what());
        }
    }


} // namespace wups::config
