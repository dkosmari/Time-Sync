/*
 * Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>

#include <wupsxx/logger.hpp>

#include "synchronize_item.hpp"

#include <wupsxx/../../src/cafe_glyphs.h>

#include "core.hpp"


using namespace wups::config;
namespace logger = wups::logger;


synchronize_item::synchronize_item() :
    text_item{"Synchronize now!"},
    worker_status{status::idle}
{}


std::unique_ptr<synchronize_item>
synchronize_item::create()
{
    return std::make_unique<synchronize_item>();
}


void
synchronize_item::get_display(char* buf, std::size_t size)
    const
{
    auto st = worker_status.load();
    switch (st) {
    case status::idle:
        std::snprintf(buf, size, "Press " CAFE_GLYPH_BTN_A " to synchronize.");
        break;
    case status::started:
        std::snprintf(buf, size, "Synchronizing...");
        break;
    case status::finished:
        std::snprintf(buf, size, "Finished! Press "
                      CAFE_GLYPH_BTN_A " or " CAFE_GLYPH_BTN_B
                      " to continue.");
        break;
    }

}


void
synchronize_item::get_focused_display(char* buf, std::size_t size)
    const
{
    get_display(buf, size);
}


bool
synchronize_item::on_focus_request(bool)
    const
{
    return true;
}


void
synchronize_item::on_focus_changed()
{
    text_item::on_focus_changed();

    if (has_focus()) {
        run();
    } else {
        cancel();
    }
}


void
synchronize_item::run()
{
    worker_thread = std::jthread{[this]()
    {
        worker_status = status::started;
        core::run();
        worker_status = status::finished;
    }};
}


void
synchronize_item::cancel()
{
    worker_thread = {};
    worker_status = status::idle;
}
