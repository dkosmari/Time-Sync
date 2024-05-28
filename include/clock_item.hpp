// SPDX-License-Identifier: MIT

#ifndef CLOCK_ITEM_HPP
#define CLOCK_ITEM_HPP

#include <map>
#include <memory>               // unique_ptr<>
#include <string>

#include "wupsxx/text_item.hpp"


struct clock_item : wups::config::text_item {

    struct server_info {
        text_item* name = nullptr;
        text_item* correction = nullptr;
        text_item* latency = nullptr;
    };


    std::map<std::string, server_info> server_infos;

    clock_item();

    static
    std::unique_ptr<clock_item> create();


    void on_input(WUPSConfigSimplePadData input) override;

    void run();

};

#endif
