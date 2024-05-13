// SPDX-License-Identifier: MIT

#ifndef WUPSXX_BOOL_ITEM_HPP
#define WUPSXX_BOOL_ITEM_HPP

#include <memory>
#include <string>

#include "item.hpp"


namespace wups::config {

    struct bool_item : item {

        bool& variable;
        bool default_value;
        std::string true_str;
        std::string false_str;


        bool_item(const std::optional<std::string>& key,
                  const std::string& name,
                  bool& variable,
                  const std::string& true_str = "true",
                  const std::string& false_str = "false");


        static
        std::unique_ptr<bool_item>
        create(const std::optional<std::string>& key,
               const std::string& name,
               bool& variable,
               const std::string& true_str = "true",
               const std::string& false_str = "false");


        virtual int get_display(char* buf, std::size_t size) const override;

        virtual int get_selected_display(char* buf, std::size_t size) const override;

        virtual void restore() override;

        virtual void on_input(WUPSConfigSimplePadData input) override;


        void on_changed();
    };

} // namespace wups::config

#endif
