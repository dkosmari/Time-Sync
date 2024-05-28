// SPDX-License-Identifier: MIT

#ifndef TIMEZONE_ITEM_HPP
#define TIMEZONE_ITEM_HPP

#include <memory>

#include "wupsxx/text_item.hpp"


struct timezone_item : wups::config::text_item {

    timezone_item();

    static
    std::unique_ptr<timezone_item> create();

    void on_input(WUPSConfigSimplePadData input) override;

    void run();

};

#endif
