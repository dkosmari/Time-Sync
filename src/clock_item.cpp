/*
 * Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include <cmath>                // max(), min()
#include <exception>
#include <vector>

#include "clock_item.hpp"

#include "cfg.hpp"
#include "core.hpp"
#include "logger.hpp"
#include "net/addrinfo.hpp"
#include "time_utils.hpp"
#include "utils.hpp"

// we borrow libwupsxx's cafe_glyphs.h private header
#include <wupsxx/../../src/cafe_glyphs.h>


using namespace std::literals;
using namespace wups::config;

using time_utils::dbl_seconds;


namespace {

    template<typename T>
    struct statistics {
        T min = T{0};
        T max = T{0};
        T avg = T{0};
    };


    template<typename T>
    statistics<T>
    get_statistics(const std::vector<T>& values)
    {
        statistics<T> result;
        T total = T{0};

        if (values.empty())
            return result;

        result.min = result.max = values.front();
        for (auto x : values) {
            result.min = std::min(result.min, x);
            result.max = std::max(result.max, x);
            total += x;
        }

        result.avg = total / values.size();

        return result;
    }

}


clock_item::clock_item() :
    text_item{"Clock",
              "(press " CAFE_GLYPH_BTN_A ")",
              48}
{}


std::unique_ptr<clock_item>
clock_item::create()
{
    return std::make_unique<clock_item>();
}


focus_status
clock_item::on_input(const simple_pad_data& input)
{
    if (input.buttons_d & WUPS_CONFIG_BUTTON_A) {
        try {
            run();
        }
        catch (std::exception& e) {
            text = "Error: "s + e.what();
        }
        return focus_status::lose;
    }

    refresh_now_str();

    return text_item::on_input(input);
}


void
clock_item::refresh_now_str()
{
    now_str = core::local_clock_to_string();
    text = now_str + stats_str;
}


void
clock_item::run()
{
    using std::to_string;
    using time_utils::seconds_to_human;

    for (auto& [key, value] : server_infos) {
        value.name->text.clear();
        value.correction->text.clear();
        value.latency->text.clear();
    }

    auto servers = utils::split(cfg::server, " \t,;");

    net::addrinfo::hints opts{ .type = net::socket::type::udp };

    dbl_seconds total = 0s;
    unsigned num_values = 0;

    for (const auto& server : servers) {
        auto& si = server_infos.at(server);
        try {
            auto infos = net::addrinfo::lookup(server, "123", opts);

            si.name->text = to_string(infos.size())
                + (infos.size() > 1 ? " addresses."s : " address."s);

            std::vector<dbl_seconds> server_corrections;
            std::vector<dbl_seconds> server_latencies;
            unsigned errors = 0;

            for (const auto& info : infos) {
                try {
                    auto [correction, latency] = core::ntp_query(info.addr);
                    server_corrections.push_back(correction);
                    server_latencies.push_back(latency);
                    total += correction;
                    ++num_values;
                    logger::printf("%s (%s): correction = %s, latency = %s",
                                   server.c_str(),
                                   to_string(info.addr).c_str(),
                                   seconds_to_human(correction, true).c_str(),
                                   seconds_to_human(latency).c_str());
                }
                catch (std::exception& e) {
                    ++errors;
                    logger::printf("Error: %s", e.what());
                }
            }

            if (errors)
                si.name->text += " "s + to_string(errors)
                    + (errors > 1 ? " errors."s : " error."s);
            if (!server_corrections.empty()) {
                auto corr_stats = get_statistics(server_corrections);
                si.correction->text = "min = "s + seconds_to_human(corr_stats.min, true)
                    + ", max = "s + seconds_to_human(corr_stats.max, true)
                    + ", avg = "s + seconds_to_human(corr_stats.avg, true);
                auto late_stats = get_statistics(server_latencies);
                si.latency->text = "min = "s + seconds_to_human(late_stats.min)
                    + ", max = "s + seconds_to_human(late_stats.max)
                    + ", avg = "s + seconds_to_human(late_stats.avg);
            } else {
                si.correction->text = "No data.";
                si.latency->text = "No data.";
            }
        }
        catch (std::exception& e) {
            si.name->text = e.what();
        }
    }

    if (num_values) {
        dbl_seconds avg = total / num_values;
        stats_str = ", needs "s + seconds_to_human(avg, true);
    } else
        stats_str = "";

    refresh_now_str();
}
