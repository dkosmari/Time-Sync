// SPDX-License-Identifier: MIT

#include <algorithm>            // min()
#include <cstdio>               // snprintf()
#include <stdexcept>

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
    text_item::on_input(WUPSConfigSimplePadData input)
    {
        item::on_input(input);

        if (text.empty())
            return;

        int tsize = static_cast<int>(text.size());

        if (tsize <= max_width)
            return;

        if (input.buttons_d & WUPS_CONFIG_BUTTON_LEFT)
            start -= 5;

        if (input.buttons_d & WUPS_CONFIG_BUTTON_RIGHT)
            start += 5;

        if (start >= tsize - max_width)
            start = tsize - max_width;
        if (start < 0)
            start = 0;
    }

} // namespace wups::config
