// SPDX-License-Identifier: MIT

#ifndef WUPSXX_TEXT_ITEM_HPP
#define WUPSXX_TEXT_ITEM_HPP

#include <memory>

#include "item.hpp"


namespace wups::config {

    struct text_item : item {

        std::string text;
        int max_width = 50;
        int start = 0;

        text_item(const std::optional<std::string>& key = "",
                  const std::string& name = "",
                  const std::string& text = "");


        // convenience constructor
        static
        std::unique_ptr<text_item>
        create(const std::optional<std::string>& key = "",
               const std::string& name = "",
               const std::string& text = "");


        virtual int get_display(char* buf, std::size_t size) const override;

        virtual void on_selected(bool is_selected) override;

        virtual void on_input(WUPSConfigSimplePadData input) override;
    };

} // namespace wups::config


#endif
