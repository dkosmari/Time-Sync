/*
 * Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef CLOCK_ITEM_HPP
#define CLOCK_ITEM_HPP

#include <map>
#include <memory>               // unique_ptr<>
#include <string>

#include <wupsxx/text_item.hpp>


struct clock_item : wups::config::text_item {

    struct server_info {
        text_item* name = nullptr;
        text_item* correction = nullptr;
        text_item* latency = nullptr;
    };


    std::string now_str;
    std::string stats_str;
    std::map<std::string, server_info> server_infos;

    clock_item();

    static
    std::unique_ptr<clock_item>
    create();


    virtual
    wups::config::focus_status
    on_input(const wups::config::simple_pad_data& input) override;


    void refresh_now_str();

    void run();

};

#endif
