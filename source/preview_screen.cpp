// SPDX-License-Identifier: MIT

#include <cmath>
#include <memory>
#include <vector>

#include "preview_screen.hpp"

#include "wupsxx/text_item.hpp"

#include "cfg.hpp"
#include "core.hpp"
#include "log.hpp"
#include "nintendo_glyphs.hpp"
#include "utils.hpp"


using wups::config::text_item;

using namespace std::literals;


namespace {

    struct statistics {
        double min = 0;
        double max = 0;
        double avg = 0;
    };


    statistics
    get_statistics(const std::vector<double>& values)
    {
        statistics result;
        double total = 0;

        if (values.empty())
            return result;

        result.min = result.max = values.front();
        for (auto x : values) {
            result.min = std::fmin(result.min, x);
            result.max = std::fmax(result.max, x);
            total += x;
        }

        result.avg = total / values.size();

        return result;
    }

}


struct server_info {
    text_item* name = nullptr;
    text_item* correction = nullptr;
    text_item* latency = nullptr;
};



struct clock_item : text_item {

    std::map<std::string, server_info> server_infos;


    clock_item() :
        text_item{{}, "Clock (" NIN_GLYPH_BTN_A " to refresh)"}
    {}


    static
    std::unique_ptr<clock_item>
    create()
    {
        return std::make_unique<clock_item>();
    }


    void
    on_input(WUPSConfigSimplePadData input)
        override
    {
        text_item::on_input(input);

        if (input.buttons_d & WUPS_CONFIG_BUTTON_A) {
            try {
                run();
            }
            catch (std::exception& e) {
                text = "Error: "s + e.what();
            }
        }
    }


    void
    run()
    {
        using std::to_string;
        using utils::seconds_to_human;
        using utils::to_string;

        for (auto& [key, value] : server_infos) {
            value.name->text.clear();
            value.correction->text.clear();
            value.latency->text.clear();
        }

        auto servers = utils::split(cfg::server, " \t,;");

        utils::addrinfo_query query = {
            .family = AF_INET,
            .socktype = SOCK_DGRAM,
            .protocol = IPPROTO_UDP
        };

        double total = 0;
        unsigned num_values = 0;

        for (const auto& server : servers) {
            auto& si = server_infos.at(server);
            try {
                auto infos = utils::get_address_info(server, "123", query);

                si.name->text = to_string(infos.size())
                    + (infos.size() > 1 ? " addresses."s : " address."s);

                std::vector<double> server_corrections;
                std::vector<double> server_latencies;
                unsigned errors = 0;

                for (const auto& info : infos) {
                    try {
                        auto [correction, latency] = core::ntp_query(info.address);
                        server_corrections.push_back(correction);
                        server_latencies.push_back(latency);
                        total += correction;
                        ++num_values;
                        logging::printf("%s (%s): correction = %s, latency = %s",
                                        server.c_str(),
                                        to_string(info.address).c_str(),
                                        seconds_to_human(correction).c_str(),
                                        seconds_to_human(latency).c_str());
                    }
                    catch (std::exception& e) {
                        ++errors;
                        logging::printf("Error: %s", e.what());
                    }
                }

                if (errors)
                    si.name->text += " "s + to_string(errors)
                        + (errors > 1 ? " errors."s : " error."s);
                if (!server_corrections.empty()) {
                    auto corr_stats = get_statistics(server_corrections);
                    si.correction->text = "min = "s + seconds_to_human(corr_stats.min)
                        + ", max = "s + seconds_to_human(corr_stats.max)
                        + ", avg = "s + seconds_to_human(corr_stats.avg);
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

        text = core::local_clock_to_string();

        if (num_values) {
            double avg = total / num_values;
            text += ", needs "s + seconds_to_human(avg);
        }

    }

};


/*
 * Note: the clock item needs to know about the server items added later.
 * It's a bit ugly, because we can't manage it from the category object.
 */
wups::config::category
make_preview_screen()
{
    wups::config::category cat{"Preview"};

    auto clock = clock_item::create();
    auto& server_infos = clock->server_infos;

    cat.add(std::move(clock));

    auto servers = utils::split(cfg::server, " \t,;");
    for (const auto& server : servers) {
        if (!server_infos.contains(server)) {
            auto& si = server_infos[server];

            auto name = text_item::create({}, server + ":");
            si.name = name.get();
            cat.add(std::move(name));

            auto correction = text_item::create({}, "┣ Correction:");
            si.correction = correction.get();
            cat.add(std::move(correction));

            auto latency = text_item::create({}, "┗ Latency:");
            si.latency = latency.get();
            cat.add(std::move(latency));
        }
    }

    return cat;
}
