// SPDX-License-Identifier: MIT

#ifndef PREVIEW_HPP
#define PREVIEW_HPP

#include <map>
#include <string>

#include "wupsxx/text_item.hpp"
#include "wupsxx/category.hpp"


struct preview : wups::category {

    struct server_info {
        wups::text_item* name = nullptr;
        wups::text_item* correction = nullptr;
        wups::text_item* latency = nullptr;
    };


    wups::text_item* clock = nullptr;
    std::map<std::string, server_info> server_infos;


    preview();

    void run();

};


#endif
