// SPDX-License-Identifier: MIT

#ifndef WUPSXX_CATEGORY_HPP
#define WUPSXX_CATEGORY_HPP

#include <memory>
#include <string>

#include <wups.h>

#include "base_item.hpp"


namespace wups::config {

    class category final {

        WUPSConfigCategoryHandle handle;
        bool own_handle;

    public:

        category(WUPSConfigCategoryHandle handle);
        category(const std::string& name);
        category(category&& other) noexcept;

        ~category();

        void release();

        void add(std::unique_ptr<base_item>&& item);
        void add(base_item* item);

        void add(category&& child);

    };

} // namespace wups


#endif
