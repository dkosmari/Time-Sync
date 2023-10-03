// SPDX-License-Identifier: MIT

// standard headers
#include <atomic>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <stdexcept>

// unix headers
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// WUT/WUPS headers
#include <coreinit/debug.h>
#include <coreinit/filesystem.h>
#include <coreinit/ios.h>
#include <coreinit/mcp.h>
#include <coreinit/time.h>
#include <nn/pdm.h>
#include <notifications/notifications.h>
#include <wups.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemIntegerRange.h>
#include <wups/config/WUPSConfigItemStub.h>


#include "ntp.hpp"


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
WUPS_PLUGIN_VERSION("v1.2.0");
WUPS_PLUGIN_AUTHOR("Nightkingale, Daniel K. O.");
WUPS_PLUGIN_LICENSE("MIT");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE(PLUGIN_NAME);


namespace cfg {
    int  hours        = 0;
    int  minutes      = 0;
    int  msg_duration = 5;
    bool notify       = true;
    char server[512]  = "pool.ntp.org";
    bool sync         = false;
    int  tolerance    = 200;

    OSTime offset = 0;          // combines hours and minutes offsets
}


std::atomic<bool> in_progress = false;


// RAII type that handles the in_progress flag.

struct progress_error : std::runtime_error {
    progress_error() :
        std::runtime_error{"progress_error"}
    {}
};

struct progress_guard {
    progress_guard()
    {
        bool expected_progress = false;
        if (!in_progress.compare_exchange_strong(expected_progress, true))
            throw progress_error{};
    }

    ~progress_guard()
    {
        in_progress = false;
    }
};


#ifdef __WIIU__
constexpr
std::uint64_t
htobe64(std::uint64_t x)
{
    if constexpr (std::endian::native == std::endian::big)
        return x;
    else
        return std::byteswap(x);
}

constexpr
std::uint64_t
be64toh(std::uint64_t x)
{
    return htobe64(x);
}
#endif


void
report_error(const std::string& arg)
{
    std::string msg = "[" PLUGIN_NAME "] " + arg;
    NotificationModule_AddErrorNotificationEx(msg.c_str(),
                                              cfg::msg_duration,
                                              1,
                                              {255, 255, 255, 255},
                                              {237, 28, 36, 255},
                                              nullptr,
                                              nullptr);
}


void
report_info(const std::string& arg)
{
    if (!cfg::notify)
        return;

    std::string msg = "[" PLUGIN_NAME "] " + arg;
    NotificationModule_AddInfoNotificationEx(msg.c_str(),
                                             cfg::msg_duration,
                                             {255, 255, 255, 255},
                                             {100, 100, 100, 255},
                                             nullptr,
                                             nullptr);
}


static
OSTime
get_utc_time()
{
    return OSGetTime() - cfg::offset;
}


static
double
ntp_to_double(ntp::timestamp t)
{
    return std::ldexp(static_cast<double>(t), -32);
}


ntp::timestamp
double_to_ntp(double t)
{
    return std::ldexp(t, 32);
}


OSTime
ntp_to_wiiu(ntp::timestamp t)
{
    // Change t from NTP epoch (1900) to Wii U epoch (2000).
    // There are 24 leap years in this period.
    constexpr std::uint64_t seconds_per_day = 24 * 60 * 60;
    constexpr std::uint64_t seconds_offset = seconds_per_day * (100 * 365 + 24);
    t -= seconds_offset << 32;

    // Convert from u32.32 to Wii U ticks count.
    double dt = ntp_to_double(t);

    // Note: do the conversion in floating point to avoid overflows.
    OSTime r = dt * OSTimerClockSpeed;

    return r;
}


ntp::timestamp
wiiu_to_ntp(OSTime t)
{
    // Convert from Wii U ticks to seconds.
    // Note: do the conversion in floating point to avoid overflows.
    double dt = double(t) / OSTimerClockSpeed;
    ntp::timestamp r = double_to_ntp(dt);

    // Change r from Wii U epoch (2000) to NTP epoch (1900).
    constexpr std::uint64_t seconds_per_day = 24 * 60 * 60;
    constexpr std::uint64_t seconds_offset = seconds_per_day * (100 * 365 + 24);
    r += seconds_offset << 32;

    return r;
}


std::string
to_string(struct in_addr addr)
{
    return inet_ntoa(addr);
}


std::string
seconds_to_human(double s)
{
    char buf[64];

    if (std::fabs(s) < 2) // less than 2 seconds
        std::snprintf(buf, sizeof buf, "%.3f ms", 1000 * s);
    else if (std::fabs(s) < 2 * 60) // less than 2 minutes
        std::snprintf(buf, sizeof buf, "%.1f s", s);
    else if (std::fabs(s) < 2 * 60 * 60) // less than 2 hours
        std::snprintf(buf, sizeof buf, "%.1f min", s / 60);
    else if (std::fabs(s) < 2 * 24 * 60 * 60) // less than 2 days
        std::snprintf(buf, sizeof buf, "%.1f hrs", s / (60 * 60));
    else
        std::snprintf(buf, sizeof buf, "%.1f days", s / (24 * 60 * 60));

    return buf;
}


std::string
format_wiiu_time(OSTime wt)
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
format_ntp(ntp::timestamp t)
{
    OSTime wt = ntp_to_wiiu(t);
    return format_wiiu_time(wt);
}


std::string
h_errno_to_string()
{
    int e = h_errno;
    switch (e) {
    case HOST_NOT_FOUND:
        return "host not found";
    case TRY_AGAIN:
        return "could not contact name server";
    case NO_RECOVERY:
        return "fatal error";
    case NO_ADDRESS:
        return "host name without address";
    default:
        return "unknown ("s + std::to_string(e) + ")"s;
    }
}


std::vector<std::string>
split(const std::string& s)
{
    using std::string;

    const string separators = " \t,";

    std::vector<string> result;

    string::size_type start = 0;
    while (start != string::npos) {
        auto finish = s.find_first_of(separators, start);
        result.push_back(s.substr(start, finish - start));
        start = s.find_first_not_of(separators, finish);
    }

    return result;
}


extern "C" int32_t CCRSysSetSystemTime(OSTime time);
extern "C" BOOL __OSSetAbsoluteSystemTime(OSTime time);


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


// RAII class to close down a socket
struct socket_guard {
    int fd;

    socket_guard(int ns, int st, int pr) :
        fd{socket(ns, st, pr)}
    {}

    ~socket_guard()
    {
        if (fd != -1)
            close();
    }

    void
    close()
    {
        ::close(fd);
        fd = -1;
    }
};


// Note: hardcoded for IPv4, the Wii U doesn't have IPv6.
std::pair<double, double>
ntp_query(struct in_addr ip_address)
{
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr = ip_address;
    addr.sin_port = htons(123); // NTP port

    socket_guard s{PF_INET, SOCK_DGRAM, IPPROTO_UDP};
    if (s.fd == -1)
        throw std::string{"unable to create socket"};

    connect(s.fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof addr);

    ntp::packet packet;
    packet.version(4);
    packet.mode(ntp::packet::mode::client);

    ntp::timestamp t1 = wiiu_to_ntp(get_utc_time());
    packet.transmit_time = htobe64(t1);

    if (send(s.fd, &packet, sizeof packet, 0) == -1) {
        int e = errno;
        throw std::string{"unable to send NTP request: "s + std::to_string(e)};
    }

    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(s.fd, &read_set);
    struct timeval timeout = { 4, 0 };

    if (select(s.fd + 1, &read_set, nullptr, nullptr, &timeout) == -1) {
        int e = errno;
        throw std::string{"select() failed: "s + std::to_string(e)};
    }

    if (!FD_ISSET(s.fd, &read_set))
        throw std::string{"timeout reached"};

    if (recv(s.fd, &packet, sizeof packet, 0) < 48)
        throw std::string{"invalid NTP response"s};

    ntp::timestamp t4 = wiiu_to_ntp(get_utc_time());

    ntp::timestamp t1_copy = be64toh(packet.origin_time);
    if (t1 != t1_copy)
        throw std::string{"NTP response does not match request: ["s
                          + format_ntp(t1) + "] vs ["s
                          + format_ntp(t1_copy) + "]"s};

    // when our request arrived at the server
    ntp::timestamp t2 = be64toh(packet.receive_time);
    // when the server sent out a response
    ntp::timestamp t3 = be64toh(packet.transmit_time);

    ntp::timestamp roundtrip = (t4 - t1) - (t3 - t2);

    double delay = ntp_to_double(roundtrip) / 2;

    // The correct time (t4 + correction) should be t3 + delay.
    double correction = ntp_to_double(t3) + delay - ntp_to_double(t4);

    return { correction, delay };
}


void
update_time()
try
{
    progress_guard guard;

    cfg::offset = OSSecondsToTicks(cfg::minutes * 60);
    if (cfg::hours < 0)
        cfg::offset -= OSSecondsToTicks(-cfg::hours * 60 * 60);
    else
        cfg::offset += OSSecondsToTicks(cfg::hours * 60 * 60);

    std::vector<double> corrections;

    std::vector<std::string> servers = split(cfg::server);

    for (const auto& server : servers) {
        struct hostent* host = gethostbyname(server.c_str());
        if (!host) {
            report_error("unable to resolve host '"s + cfg::server + "': "s +
                         h_errno_to_string());
            continue;
        }

        // sanity check
        if (host->h_addrtype != AF_INET || host->h_length != 4) {
            report_error("no IPv4 address found for '"s + cfg::server + "'"s);
            continue;
        }

        for (int i = 0; host->h_addr_list[i]; ++i) {
            struct in_addr addr;
            std::memcpy(&addr, host->h_addr_list[i], 4);

            try {
                auto [correction, delay] = ntp_query(addr);

                corrections.push_back(correction);

                report_info(server + "("s + to_string(addr)
                            + "): correction="s + seconds_to_human(correction)
                            + ", delay=" + seconds_to_human(delay));
            }
            catch (std::string msg) {
                report_error("'"s + to_string(addr) + "' failed: "s + msg);
            }
        }
    }

    if (corrections.empty()) {
        report_error("no NTP server could be used");
        return;
    }

    double avg_correction = 0;
    for (auto x : corrections)
        avg_correction += x;
    avg_correction /= corrections.size();

    if (std::fabs(avg_correction) * 1000 <= cfg::tolerance) {
        report_info("tolerating clock drift (correction is only "
                    + seconds_to_human(avg_correction) + ")"s);
        return;
    }

    if (cfg::sync) {
        if (!apply_clock_correction(avg_correction)) {
            report_error("failed to set system clock");
            return;
        }
    }

    if (cfg::notify)
        report_info("clock corrected by " + seconds_to_human(avg_correction));
}
catch (progress_error&) {
    report_info("skipping NTP task: already in progress");
}


INITIALIZE_PLUGIN()
{
    WUPSStorageError storageRes = WUPS_OpenStorage();
    // Check if the plugin's settings have been saved before.
    if (storageRes == WUPS_STORAGE_ERROR_SUCCESS) {
        if (WUPS_GetBool(nullptr, CFG_SYNC, &cfg::sync) == WUPS_STORAGE_ERROR_NOT_FOUND)
            WUPS_StoreBool(nullptr, CFG_SYNC, cfg::sync);

        if (WUPS_GetBool(nullptr, CFG_NOTIFY, &cfg::notify) == WUPS_STORAGE_ERROR_NOT_FOUND)
            WUPS_StoreBool(nullptr, CFG_NOTIFY, cfg::notify);

        if (WUPS_GetInt(nullptr, CFG_MSG_DURATION, &cfg::msg_duration) == WUPS_STORAGE_ERROR_NOT_FOUND)
            WUPS_StoreInt(nullptr, CFG_MSG_DURATION, cfg::msg_duration);

        if (WUPS_GetInt(nullptr, CFG_HOURS, &cfg::hours) == WUPS_STORAGE_ERROR_NOT_FOUND)
            WUPS_StoreInt(nullptr, CFG_HOURS, cfg::hours);

        if (WUPS_GetInt(nullptr, CFG_MINUTES, &cfg::minutes) == WUPS_STORAGE_ERROR_NOT_FOUND)
            WUPS_StoreInt(nullptr, CFG_MINUTES, cfg::minutes);

        if (WUPS_GetInt(nullptr, CFG_TOLERANCE, &cfg::tolerance) == WUPS_STORAGE_ERROR_NOT_FOUND)
            WUPS_StoreInt(nullptr, CFG_TOLERANCE, cfg::tolerance);

        if (WUPS_GetString(nullptr, CFG_SERVER, cfg::server, sizeof cfg::server)
            == WUPS_STORAGE_ERROR_NOT_FOUND)
            WUPS_StoreString(nullptr, CFG_SERVER, cfg::server);

        NotificationModule_InitLibrary(); // Set up for notifications.
        WUPS_CloseStorage();
    }

    if (cfg::sync)
        update_time(); // Update time when plugin is loaded.
}


WUPS_GET_CONFIG()
{
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS)
        return 0;

    WUPSConfigHandle settings;
    WUPSConfig_CreateHandled(&settings, PLUGIN_NAME);

    WUPSConfigCategoryHandle config;
    WUPSConfig_AddCategoryByNameHandled(settings, "Configuration", &config);
    WUPSConfigCategoryHandle preview;
    WUPSConfig_AddCategoryByNameHandled(settings, "Preview Time", &preview);

    WUPSConfigItemBoolean_AddToCategoryHandled(settings, config, CFG_SYNC,
                                               "Syncing Enabled",
                                               cfg::sync,
                                               [](ConfigItemBoolean*, bool value)
                                               {
                                                   WUPS_StoreBool(nullptr, CFG_NOTIFY, value);
                                                   cfg::notify = value;
                                               });
    WUPSConfigItemBoolean_AddToCategoryHandled(settings, config, CFG_NOTIFY,
                                               "Show Notifications",
                                               cfg::notify,
                                               [](ConfigItemBoolean*, bool value)
                                               {
                                                   WUPS_StoreBool(nullptr, CFG_NOTIFY, value);
                                                   cfg::notify = value;
                                               });
    WUPSConfigItemIntegerRange_AddToCategoryHandled(settings, config, CFG_MSG_DURATION,
                                                    "Messages Duration (seconds)",
                                                    cfg::msg_duration, 0, 30,
                                                    [](ConfigItemIntegerRange*, int32_t value)
                                                    {
                                                        WUPS_StoreInt(nullptr, CFG_MSG_DURATION,
                                                                      value);
                                                        cfg::msg_duration = value;
                                                    });
    WUPSConfigItemIntegerRange_AddToCategoryHandled(settings, config, CFG_HOURS,
                                                    "Hours Offset",
                                                    cfg::hours, -12, 14,
                                                    [](ConfigItemIntegerRange*, int32_t value)
                                                    {
                                                        WUPS_StoreInt(nullptr, CFG_HOURS, value);
                                                        cfg::hours = value;
                                                    });
    WUPSConfigItemIntegerRange_AddToCategoryHandled(settings, config, CFG_MINUTES,
                                                    "Minutes Offset",
                                                    cfg::minutes, 0, 59,
                                                    [](ConfigItemIntegerRange*, int32_t value)
                                                    {
                                                        WUPS_StoreInt(nullptr, CFG_MINUTES,
                                                                      value);
                                                        cfg::minutes = value;
                                                    });
    WUPSConfigItemIntegerRange_AddToCategoryHandled(settings, config, CFG_TOLERANCE,
                                                    "Tolerance (milliseconds)",
                                                    cfg::tolerance, 0, 5000,
                                                    [](ConfigItemIntegerRange*, int32_t value)
                                                    {
                                                        WUPS_StoreInt(nullptr, CFG_TOLERANCE,
                                                                      value);
                                                        cfg::tolerance = value;
                                                    });

    // show current NTP server address, no way to change it.
    std::string server = "NTP servers: "s + cfg::server;
    WUPSConfigItemStub_AddToCategoryHandled(settings, config, CFG_SERVER, server.c_str());

    WUPSConfigItemStub_AddToCategoryHandled(settings, preview, "time",
                                            format_wiiu_time(OSGetTime()).c_str());

    return settings;
}


WUPS_CONFIG_CLOSED()
{
    std::thread update_time_thread(update_time);
    update_time_thread.detach(); // Update time when settings are closed.

    WUPS_CloseStorage(); // Save all changes.
}
