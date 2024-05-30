// SPDX-License-Identifier: MIT

#include <algorithm>            // clamp(), min()
#include <cstdio>               // snprintf()

#include "wupsxx/text_item.hpp"


namespace wups::config {

    text_item::text_item(const std::optional<std::string>& key,
                         const std::string& label,
                         const std::string& text,
                         int max_width) :
        item{key, label},
        text{text},
        max_width{max_width}
    {}


    std::unique_ptr<text_item>
    text_item::create(const std::optional<std::string>& key,
                      const std::string& label,
                      const std::string& text,
                      int max_width)
    {
        return std::make_unique<text_item>(key, label, text, max_width);
    }


    int
    text_item::get_display(char* buf,
                           std::size_t size)
        const
    {
        auto width = std::min<int>(size - 1, max_width);

        std::snprintf(buf, size,
                      "%*.*s",
                      width,
                      width,
                      text.c_str() + start);

        return 0;
    }


    void
    text_item::on_selected(bool is_selected)
    {
        if (!is_selected)
            start = 0;
    }


    void
    text_item::on_input(WUPSConfigSimplePadData input,
                        WUPS_CONFIG_SIMPLE_INPUT repeat)
    {
        item::on_input(input, repeat);

        if (text.empty())
            return;

        int tsize = static_cast<int>(text.size());

        // If text is fully visible, no scrolling happens.
        if (tsize <= max_width)
            return;

        // Handle text scrolling

        if (input.buttons_d & WUPS_CONFIG_BUTTON_LEFT ||
            repeat & WUPS_CONFIG_BUTTON_LEFT)
            --start;

        if (input.buttons_d & WUPS_CONFIG_BUTTON_RIGHT ||
            repeat & WUPS_CONFIG_BUTTON_RIGHT)
            ++start;

        if (input.buttons_d & WUPS_CONFIG_BUTTON_L)
            start = 0;

        if (input.buttons_d & WUPS_CONFIG_BUTTON_R)
            start = tsize - max_width;

        start = std::clamp(start, 0, tsize - max_width);
    }

} // namespace wups::config
