/*
 * Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SYNCHRONIZE_ITEM_HPP
#define  SYNCHRONIZE_ITEM_HPP

#include <memory>
#include <thread>

#include <wupsxx/button_item.hpp>


struct synchronize_item : wups::config::button_item {

    std::jthread worker_thread;


    synchronize_item();


    static
    std::unique_ptr<synchronize_item>
    create();


    virtual
    void
    on_started() override;


    virtual
    void
    on_finished() override;


    virtual
    void
    on_cancel() override;

};


#endif
