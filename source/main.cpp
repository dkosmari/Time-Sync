// SPDX-License-Identifier: MIT

// standard headers
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>           // invoke()
#include <future>
#include <map>
#include <memory>               // unique_ptr<>
#include <numeric>              // accumulate()
#include <optional>
#include <ranges>               // ranges::zip()
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>              // pair<>
#include <vector>

// unix headers
#include <arpa/inet.h>
#include <netinet/in.h>         // struct sockaddr_in
#include <sys/select.h>         // select()
#include <sys/socket.h>         // connect()
#include <sys/types.h>

// WUT/WUPS headers
#include <coreinit/time.h>
#include <nn/pdm.h>
#include <notifications/notifications.h>
#include <wups.h>
#include <whb/log.h>
#include <whb/log_udp.h>

// local headers
#include "wupsxx/bool_item.hpp"
#include "wupsxx/category.hpp"
#include "wupsxx/config.hpp"
#include "wupsxx/int_item.hpp"
#include "wupsxx/storage.hpp"
#include "wupsxx/text_item.hpp"

#include "limited_async.hpp"
#include "ntp.hpp"
#include "utc.hpp"
#include "utils.hpp"


using namespace std::literals;


#define PLUGIN_NAME "Time Sync"

#define CFG_HOURS        "hours"
#define CFG_MINUTES      "minutes"
#define CFG_MSG_DURATION "msg_duration"
#define CFG_NOTIFY       "notify"
#define CFG_SERVER       "server"
#define CFG_SYNC         "sync"
#define CFG_TOLERANCE    "tolerance"

// Important plugin information.
WUPS_PLUGIN_NAME(PLUGIN_NAME);
WUPS_PLUGIN_DESCRIPTION("A plugin that synchronizes a Wii U's clock to the Internet.");
WUPS_PLUGIN_VERSION("v2.1.0");
WUPS_PLUGIN_AUTHOR("Nightkingale, Daniel K. O.");
WUPS_PLUGIN_LICENSE("MIT");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE(PLUGIN_NAME);


namespace cfg {
    int         hours        = 0;
    int         minutes      = 0;
    int         msg_duration = 5;
    bool        notify       = true;
    std::string server       = "pool.ntp.org";
    bool        sync         = false;
    int         tolerance    = 250;
}


std::counting_semaphore<> async_limit{5}; // limit to 5 threads


#define MSG_PREFIX "[" PLUGIN_NAME "] "

#define LOG(FMT, ...)  WHBLogPrintf(MSG_PREFIX FMT __VA_OPT__(,) __VA_ARGS__)


void
report_error(const std::string& arg)
{
    LOG("ERROR: %s", arg.c_str());

    if (!cfg::notify)
        return;

    std::string msg = MSG_PREFIX + arg;
    NotificationModule_AddErrorNotificationEx(msg.c_str(),
                                              cfg::msg_duration,
                                              1,
                                              {255, 255, 255, 255},
                                              {160, 32, 32, 255},
                                              nullptr,
                                              nullptr);
}


void
report_info(const std::string& arg)
{
    LOG("INFO: %s", arg.c_str());

    if (!cfg::notify)
        return;

    std::string msg = MSG_PREFIX + arg;
    NotificationModule_AddInfoNotificationEx(msg.c_str(),
                                             cfg::msg_duration,
                                             {255, 255, 255, 255},
                                             {32, 32, 160, 255},
                                             nullptr,
                                             nullptr);
}


void
report_success(const std::string& arg)
{
    LOG("SUCCESS: %s", arg.c_str());

    if (!cfg::notify)
        return;

    std::string msg = MSG_PREFIX + arg;
    NotificationModule_AddInfoNotificationEx(msg.c_str(),
                                             cfg::msg_duration,
                                             {255, 255, 255, 255},
                                             {32, 160, 32, 255},
                                             nullptr,
                                             nullptr);
}


// Difference from NTP (1900) to Wii U (2000) epochs.
// There are 24 leap years in this period.
constexpr double seconds_per_day = 24 * 60 * 60;
constexpr double epoch_diff = seconds_per_day * (100 * 365 + 24);


// Wii U -> NTP epoch.
ntp::timestamp
to_ntp(utc::timestamp t)
{
    return ntp::timestamp{t.value + epoch_diff};
}


// NTP -> Wii U epoch.
utc::timestamp
to_utc(ntp::timestamp t)
{
    return utc::timestamp{static_cast<double>(t) - epoch_diff};
}


std::string
ticks_to_string(OSTime wt)
{
    OSCalendarTime cal;
    OSTicksToCalendarTime(wt, &cal);
    char buffer[256];
    std::snprintf(buffer, sizeof buffer,
                  "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                  cal.tm_year, cal.tm_mon + 1, cal.tm_mday,
                  cal.tm_hour, cal.tm_min, cal.tm_sec, cal.tm_msec);
    return buffer;
}


std::string
to_string(ntp::timestamp t)
{
    auto ut = to_utc(t);
    OSTime ticks = ut.value * OSTimerClockSpeed;
    return ticks_to_string(ticks);
}


extern "C" int32_t CCRSysSetSystemTime(OSTime time); // from nn_ccr
extern "C" BOOL __OSSetAbsoluteSystemTime(OSTime time); // from coreinit


bool
apply_clock_correction(double correction)
{
    OSTime correction_ticks = correction * OSTimerClockSpeed;

    OSTime now = OSGetTime();
    OSTime corrected = now + correction_ticks;

    nn::pdm::NotifySetTimeBeginEvent();

    if (CCRSysSetSystemTime(corrected)) {
        nn::pdm::NotifySetTimeEndEvent();
        return false;
    }

    bool res = __OSSetAbsoluteSystemTime(corrected);

    nn::pdm::NotifySetTimeEndEvent();

    return res;
}



// Note: hardcoded for IPv4, the Wii U doesn't have IPv6.
std::pair<double, double>
ntp_query(struct sockaddr_in address)
{
    using std::to_string;

    utils::socket_guard s{PF_INET, SOCK_DGRAM, IPPROTO_UDP};

    connect(s.fd, reinterpret_cast<struct sockaddr*>(&address), sizeof address);

    ntp::packet packet;
    packet.version(4);
    packet.mode(ntp::packet::mode_flag::client);


    unsigned num_send_tries = 0;
 try_again_send:
    auto t1 = to_ntp(utc::now());
    packet.transmit_time = t1;

    if (send(s.fd, &packet, sizeof packet, 0) == -1) {
        int e = errno;
        if (e != ENOMEM)
            throw std::runtime_error{"send() failed: "s
                                     + utils::errno_to_string(e)};
        if (++num_send_tries < 4) {
            std::this_thread::sleep_for(100ms);
            goto try_again_send;
        } else
            throw std::runtime_error{"No resources for send(), too many retries!"};
    }


    struct timeval timeout = { 4, 0 };
    fd_set read_set;
    unsigned num_select_tries = 0;
 try_again_select:
    FD_ZERO(&read_set);
    FD_SET(s.fd, &read_set);

    if (select(s.fd + 1, &read_set, nullptr, nullptr, &timeout) == -1) {
        // Wii U's OS can only handle 16 concurrent select() calls,
        // so we may need to try again later.
        int e = errno;
        if (e != ENOMEM)
            throw std::runtime_error{"select() failed: "s
                                     + utils::errno_to_string(e)};
        if (++num_select_tries < 4) {
            std::this_thread::sleep_for(10ms);
            goto try_again_select;
        } else
            throw std::runtime_error{"No resources for select(), too many retries!"};
    }

    if (!FD_ISSET(s.fd, &read_set))
        throw std::runtime_error{"Timeout reached!"};

    // Measure the arrival time as soon as possible.
    auto t4 = to_ntp(utc::now());

    if (recv(s.fd, &packet, sizeof packet, 0) < 48)
        throw std::runtime_error{"Invalid NTP response!"};

    auto v = packet.version();
    if (v < 3 || v > 4)
        throw std::runtime_error{"Unsupported NTP version: "s + to_string(v)};

    auto m = packet.mode();
    if (m != ntp::packet::mode_flag::server)
        throw std::runtime_error{"Invalid NTP packet mode: "s + to_string(m)};

    auto l = packet.leap();
    if (l == ntp::packet::leap_flag::unknown)
        throw std::runtime_error{"Unknown value for leap flag."};

    ntp::timestamp t1_received = packet.origin_time;
    if (t1 != t1_received)
        throw std::runtime_error{"NTP response mismatch: ["s
                                 + ::to_string(t1) + "] vs ["s
                                 + ::to_string(t1_received) + "]"s};

    // when our request arrived at the server
    auto t2 = packet.receive_time;
    // when the server sent out a response
    auto t3 = packet.transmit_time;

    // Zero is not a valid timestamp.
    if (!t2 || !t3)
        throw std::runtime_error{"NTP response has invalid timestamps."};

    /*
     * We do all calculations in double precision to never worry about overflows. Since
     * double precision has 53 mantissa bits, we're guaranteed to have 53 - 32 = 21
     * fractional bits in Era 0, and 20 fractional bits in Era 1 (starting in 2036). We
     * still have sub-microsecond resolution.
     */
    double d1 = static_cast<double>(t1);
    double d2 = static_cast<double>(t2);
    double d3 = static_cast<double>(t3);
    double d4 = static_cast<double>(t4);

    // Detect the wraparound that will happen at the end of Era 0.
    if (d4 < d1)
        d4 += 0x1.0p32; // d4 += 2^32
    if (d3 < d2)
        d3 += 0x1.0p32; // d3 += 2^32

    double roundtrip = (d4 - d1) - (d3 - d2);
    double latency = roundtrip / 2;

    // t4 + correction = t3 + latency
    double correction = d3 + latency - d4;

    /*
     * If the local clock enters Era 1 ahead of NTP, we get a massive positive correction
     * because the local clock wrapped back to zero.
     */
    if (correction > 0x1.0p31) // if correcting more than 68 years forward
        correction -= 0x1.0p32;

    /*
     * If NTP enters Era 1 ahead of the local clock, we get a massive negative correction
     * because NTP wrapped back to zero.
     */
    if (correction < -0x1.0p31) // if correcting more than 68 years backward
        correction += 0x1.0p32;

    return { correction, latency };
}


void
update_tz_offset()
{
    double offset_seconds = (cfg::hours * 60.0 + cfg::minutes) * 60.0;
    utc::timezone_offset = offset_seconds;
}


void
update_time()
{
    using utils::seconds_to_human;

    if (!cfg::sync)
        return;

    static std::atomic<bool> executing = false;

    utils::exec_guard guard{executing};
    if (!guard.guarded) {
        // Another thread is already executing this function.
        report_info("Skipping NTP task: already in progress.");
        return;
    }

    update_tz_offset();

    std::vector<std::string> servers = utils::split(cfg::server, " \t,;");

    utils::addrinfo_query query = {
        .family = AF_INET,
        .socktype = SOCK_DGRAM,
        .protocol = IPPROTO_UDP
    };

    // First, resolve all the names, in parallel.
    // Some IP addresses might be duplicated when we use *.pool.ntp.org.
    std::set<struct sockaddr_in,
             utils::less_sockaddr_in> addresses;
    {
        using info_vec = std::vector<utils::addrinfo_result>;
        std::vector<std::future<info_vec>> futures(servers.size());

        // Launch DNS queries asynchronously.
        for (auto [fut, server] : std::views::zip(futures, servers))
            fut = limited_async(utils::get_address_info, server, "123", query);

        // Collect all future results.
        for (auto& fut : futures)
            try {
                for (auto info : fut.get())
                    addresses.insert(info.address);
            }
            catch (std::exception& e) {
                report_error(e.what());
            }
    }

    // Launch NTP queries asynchronously.
    std::vector<std::future<std::pair<double, double>>> futures(addresses.size());
    for (auto [fut, address] : std::views::zip(futures, addresses))
        fut = limited_async(ntp_query, address);

    // Collect all future results.
    std::vector<double> corrections;
    for (auto [address, fut] : std::views::zip(addresses, futures))
        try {
            auto [correction, latency] = fut.get();
            corrections.push_back(correction);
            report_info(utils::to_string(address)
                        + ": correction = "s + seconds_to_human(correction)
                        + ", latency = "s + seconds_to_human(latency));
        }
        catch (std::exception& e) {
            report_error(utils::to_string(address) + ": "s + e.what());
        }


    if (corrections.empty()) {
        report_error("No NTP server could be used!");
        return;
    }

    double avg_correction = std::accumulate(corrections.begin(),
                                            corrections.end(),
                                            0.0)
        / corrections.size();

    if (std::fabs(avg_correction) * 1000 <= cfg::tolerance) {
        report_success("Tolerating clock drift (correction is only "
                       + seconds_to_human(avg_correction) + ")."s);
        return;
    }

    if (cfg::sync) {
        if (!apply_clock_correction(avg_correction)) {
            report_error("Failed to set system clock!");
            return;
        }
        report_success("Clock corrected by " + seconds_to_human(avg_correction));
    }

}


template<typename T>
void
load_or_init(const std::string& key,
             T& variable)
{
    auto val = wups::load<T>(key);
    if (!val)
        wups::store(key, variable);
    else
        variable = *val;
}


INITIALIZE_PLUGIN()
{
    WHBLogUdpInit();

    // Check if the plugin's settings have been saved before.
    if (WUPS_OpenStorage() == WUPS_STORAGE_ERROR_SUCCESS) {

        load_or_init(CFG_HOURS,        cfg::hours);
        load_or_init(CFG_MINUTES,      cfg::minutes);
        load_or_init(CFG_MSG_DURATION, cfg::msg_duration);
        load_or_init(CFG_NOTIFY,       cfg::notify);
        load_or_init(CFG_SERVER,       cfg::server);
        load_or_init(CFG_SYNC,         cfg::sync);
        load_or_init(CFG_TOLERANCE,    cfg::tolerance);

        WUPS_CloseStorage();
    }

    NotificationModule_InitLibrary(); // Set up for notifications.

    if (cfg::sync)
        update_time(); // Update time when plugin is loaded.
}


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


struct preview_item : wups::text_item {

    struct server_info {
        wups::text_item* name_item = nullptr;
        wups::text_item* corr_item = nullptr;
        wups::text_item* late_item = nullptr;
    };

    wups::category* category;

    std::map<std::string, server_info> server_infos;

    preview_item(wups::category* cat) :
        wups::text_item{"", "Clock (\ue000 to refresh)"},
        category{cat}
    {
        category->add(this);

        std::vector<std::string> servers = utils::split(cfg::server, " \t,;");
        for (const auto& server : servers) {
            if (!server_infos.contains(server)) {
                auto& si = server_infos[server];

                auto name_item = std::make_unique<wups::text_item>("", server + ":");
                si.name_item = name_item.get();
                category->add(std::move(name_item));

                auto corr_item = std::make_unique<wups::text_item>("", "  Correction:");
                si.corr_item = corr_item.get();
                category->add(std::move(corr_item));

                auto late_item = std::make_unique<wups::text_item>("", "  Latency:");
                si.late_item = late_item.get();
                category->add(std::move(late_item));
            }
        }
    }


    void
    on_button_pressed(WUPSConfigButtons buttons)
        override
    {
        wups::text_item::on_button_pressed(buttons);

        if (buttons & WUPS_CONFIG_BUTTON_A)
            run_preview();
    }


    void
    run_preview()
    try {

        using std::make_unique;
        using std::to_string;
        using utils::seconds_to_human;
        using utils::to_string;

        update_tz_offset();

        for (auto& [key, value] : server_infos) {
            value.name_item->text.clear();
            value.corr_item->text.clear();
            value.late_item->text.clear();
        }

        std::vector<std::string> servers = utils::split(cfg::server, " \t,;");

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

                si.name_item->text = to_string(infos.size())
                    + (infos.size() > 1 ? " addresses."s : " address."s);

                std::vector<double> server_corrections;
                std::vector<double> server_latencies;
                unsigned errors = 0;

                for (const auto& info : infos) {
                    try {
                        auto [correction, latency] = ntp_query(info.address);
                        server_corrections.push_back(correction);
                        server_latencies.push_back(latency);
                        total += correction;
                        ++num_values;
                        LOG("%s (%s): correction = %s, latency = %s",
                            server.c_str(),
                            to_string(info.address).c_str(),
                            seconds_to_human(correction).c_str(),
                            seconds_to_human(latency).c_str());
                    }
                    catch (std::exception& e) {
                        ++errors;
                        LOG("Error: %s", e.what());
                    }
                }

                if (errors)
                    si.name_item->text += " "s + to_string(errors)
                        + (errors > 1 ? " errors."s : "error."s);
                if (!server_corrections.empty()) {
                    auto corr_stats = get_statistics(server_corrections);
                    si.corr_item->text = "min = "s + seconds_to_human(corr_stats.min)
                        + ", max = "s + seconds_to_human(corr_stats.max)
                        + ", avg = "s + seconds_to_human(corr_stats.avg);
                    auto late_stats = get_statistics(server_latencies);
                    si.late_item->text = "min = "s + seconds_to_human(late_stats.min)
                        + ", max = "s + seconds_to_human(late_stats.max)
                        + ", avg = "s + seconds_to_human(late_stats.avg);
                } else {
                    si.corr_item->text = "No data.";
                    si.late_item->text = "No data.";
                }
            }
            catch (std::exception& e) {
                si.name_item->text = e.what();
            }
        }

        text = ticks_to_string(OSGetTime());

        if (num_values) {
            double avg = total / num_values;
            text += ", needs "s + seconds_to_human(avg);
        }

    }
    catch (std::exception& e) {
        text = "Error: "s + e.what();
    }

};


WUPS_GET_CONFIG()
{
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS)
        return 0;

    using std::make_unique;

    try {

        auto config = make_unique<wups::category>("Configuration");

        config->add(make_unique<wups::bool_item>(CFG_SYNC,
                                                 "Syncing Enabled",
                                                 cfg::sync));

        config->add(make_unique<wups::bool_item>(CFG_NOTIFY,
                                                 "Show Notifications",
                                                 cfg::notify));

        config->add(make_unique<wups::int_item>(CFG_MSG_DURATION,
                                                "Notification Duration (seconds)",
                                                cfg::msg_duration, 0, 30));

        config->add(make_unique<wups::int_item>(CFG_HOURS,
                                                "Hours Offset",
                                                cfg::hours, -12, 14));

        config->add(make_unique<wups::int_item>(CFG_MINUTES,
                                                "Minutes Offset",
                                                cfg::minutes, 0, 59));

        config->add(make_unique<wups::int_item>(CFG_TOLERANCE,
                                                "Tolerance (milliseconds, \ue083/\ue084 for +/- 50)",
                                                cfg::tolerance, 0, 5000));

        // show current NTP server address, no way to change it.
        config->add(make_unique<wups::text_item>(CFG_SERVER,
                                                 "NTP servers",
                                                 cfg::server));

        auto preview = make_unique<wups::category>("Preview");
        // The preview_item adds itself to the category already.
        make_unique<preview_item>(preview.get()).release();

        auto root = make_unique<wups::config>(PLUGIN_NAME);
        root->add(std::move(config));
        root->add(std::move(preview));

        return root.release()->handle;

    }
    catch (...) {
        return 0;
    }
}


WUPS_CONFIG_CLOSED()
{
    std::jthread update_time_thread(update_time);
    update_time_thread.detach(); // Update time when settings are closed.

    WUPS_CloseStorage(); // Save all changes.
}
