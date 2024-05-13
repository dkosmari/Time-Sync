// SPDX-License-Identifier: MIT

#ifndef WUPSXX_INT_ITEM_HPP
#define WUPSXX_INT_ITEM_HPP

#include <memory>

#include "item.hpp"


namespace wups::config {

    struct int_item : item {

        int& variable;
        int default_value = 0;
        int min_value;
        int max_value;


        int_item(const std::optional<std::string>& key,
                 const std::string& name,
                 int& variable,
                 int min_value,
                 int max_value);

        static
        std::unique_ptr<int_item>
        create(const std::optional<std::string>& key,
               const std::string& name,
               int& variable,
               int min_value,
               int max_value);


        virtual int get_display(char* buf, std::size_t size) const override;

        virtual int get_selected_display(char* buf, std::size_t size) const override;

        virtual void restore() override;

        virtual void on_input(WUPSConfigSimplePadData input) override;


        void on_changed();

    };

} // namespace wups::config

#endif
