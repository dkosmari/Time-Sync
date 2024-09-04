/*
 * Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SYNCHRONIZE_ITEM_HPP
#define  SYNCHRONIZE_ITEM_HPP

#include <atomic>
#include <memory>
#include <thread>

#include <wupsxx/text_item.hpp>


struct synchronize_item : wups::config::text_item {

    enum status {
        idle,
        started,
        finished,
    };

    std::jthread worker_thread;
    std::atomic<status> worker_status;


    synchronize_item();


    static
    std::unique_ptr<synchronize_item>
    create();


    virtual
    void get_display(char* buf, std::size_t size) const override;

    virtual
    void get_focused_display(char* buf, std::size_t size) const override;

    virtual
    bool on_focus_request(bool new_focus) const override;

    virtual
    void on_focus_changed() override;


    void run();
    void cancel();

};


#endif
