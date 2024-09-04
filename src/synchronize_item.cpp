/*
 * Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>

#include <wupsxx/cafe_glyphs.h>
#include <wupsxx/logger.hpp>

#include "synchronize_item.hpp"

#include "core.hpp"


using namespace std::literals;

using namespace wups::config;
namespace logger = wups::logger;


synchronize_item::synchronize_item() :
    button_item{"Synchronize now!"}
{}


std::unique_ptr<synchronize_item>
synchronize_item::create()
{
    return std::make_unique<synchronize_item>();
}


void
synchronize_item::on_started()
{
    status_msg = "Synchronizing...";
    worker_thread = std::jthread{[this]()
    {
        try {
            core::run(true, true);
            current_state = state::finished;
        }
        catch (...) {
            current_state = state::finished;
            throw;
        }
    }};
}


void
synchronize_item::on_finished()
{
    if (worker_thread.joinable()) {
        try {
            worker_thread.join();
            status_msg = "Success!";
        }
        catch (std::exception& e) {
            status_msg = e.what();
        }
    }
}


void
synchronize_item::on_cancel()
{
    worker_thread = {};
    current_state = state::finished;
    status_msg = "Canceled.";
}
