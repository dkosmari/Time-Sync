// SPDX-License-Identifier: MIT

#ifndef WUPSXX_BASE_ITEM_HPP
#define WUPSXX_BASE_ITEM_HPP

#include <optional>
#include <string>

#include <wups.h>


namespace wups::config {

    struct base_item {

        WUPSConfigItemHandle handle;
        std::optional<std::string> key;
        std::string name;


        base_item(const std::optional<std::string>& key,
                  const std::string& name);

        // disallow moving, since the callbacks store the `this` pointer.
        base_item(base_item&&) = delete;

        virtual ~base_item();

        void release();


        virtual int get_display(char* buf, std::size_t size) const;

        virtual int get_selected_display(char* buf, std::size_t size) const;

        virtual void on_selected(bool is_selected);

        virtual void restore();

        virtual bool is_movement_allowed() const;

        virtual void on_close();

        virtual void on_input(WUPSConfigSimplePadData input);

        virtual void on_input(WUPSConfigComplexPadData input);

    };

} // namespace wups::config

#endif
