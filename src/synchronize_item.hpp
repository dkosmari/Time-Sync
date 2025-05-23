/*
 * Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SYNCHRONIZE_ITEM_HPP
#define SYNCHRONIZE_ITEM_HPP

#include <future>
#include <memory>
#include <stop_token>

#include <wupsxx/button_item.hpp>


struct synchronize_item : wups::button_item {

    std::future<void> task_result;
    std::stop_source task_stopper;


    synchronize_item();


    static
    std::unique_ptr<synchronize_item>
    create();


    virtual
    void
    on_started()
        override;


    virtual
    void
    on_finished()
        override;


    virtual
    void
    on_cancel()
        override;

};

#endif
