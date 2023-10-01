// SPDX-License-Identifier: MIT

// standard headers
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

// unix headers
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
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
#include <whb/proc.h>
#include <wups.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemIntegerRange.h>
#include <wups/config/WUPSConfigItemStub.h>


using namespace std::literals;


#define CFG_SYNC "sync"
#define CFG_NOTIFY "notify"
#define CFG_HOURS "hours"
#define CFG_MINUTES "minutes"
#define CFG_SERVER "server"
#define CFG_MSG_DURATION "msg_duration"


// Important plugin information.
WUPS_PLUGIN_NAME("Time Sync");
WUPS_PLUGIN_DESCRIPTION("A plugin that synchronizes a Wii U's clock to the Internet.");
WUPS_PLUGIN_VERSION("v1.2.0");
WUPS_PLUGIN_AUTHOR("Nightkingale, Daniel K. O.");
WUPS_PLUGIN_LICENSE("MIT");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE("Time Sync");


static const char plugin_name[] = "Time Sync";

namespace cfg {
    bool sync = false;
    bool notify = true;
    int msg_duration = 5;
    int hours = 0;
    int minutes = 0;
    OSTime offset = 0; // combines hours and minutes offsets
    char server[256] = "pool.ntp.org";
}


// From https://github.com/lettier/ntpclient/blob/master/source/c/main.c

// For details, see https://www.ntp.org/reflib/rfc/rfc5905.txt

// This is u32.32 fixed-point format, seconds since 1900-01-01.
using ntp_timestamp = std::uint64_t;

// This is a u16.16 fixed-point format.
using ntp_short = std::uint32_t;


enum class ntp_leap : std::uint8_t {
    no_warning      = 0 << 6,
    one_more_second = 1 << 6,
    one_less_second = 2 << 6,
    unknown         = 3 << 6
};


enum class ntp_mode : std::uint8_t {
    reserved         = 0,
    active           = 1,
    passive          = 2,
    client           = 3,
    server           = 4,
    broadcast        = 5,
    control          = 6,
    reserved_private = 7
};


// Note: all fields are big-endian
struct ntp_packet
{
    std::uint8_t lvm;

    void leap(ntp_leap x)
    {
        lvm = static_cast<std::uint8_t>(x) | (lvm & 0b0011'1111);
    }

    void version(unsigned v)
    {
        lvm = ((v << 3) & 0b0011'1000) | (lvm & 0b1100'0111);
    }

    void mode(ntp_mode m)
    {
        lvm = static_cast<std::uint8_t>(m) | (lvm & 0b1111'1000);
    }


    std::uint8_t stratum;       // Stratum level of the local clock.
    std::int8_t  poll_exp;      // Maximum interval between successive messages.
    std::int8_t  precision_exp; // Precision of the local clock.

    ntp_short root_delay;       // Total round trip delay time to the reference clock.
    ntp_short root_dispersion;  // Total dispersion to the reference clock.
    char      reference_id[4];  // Reference clock identifier. Meaning depends on stratum.

    ntp_timestamp reference_time; // Reference timestamp.
    ntp_timestamp origin_time;  // Origin timestamp, aka T1.
    ntp_timestamp receive_time; // Receive timestamp, aka T2.
    ntp_timestamp transmit_time; // Transmit timestamp, aka T3.
};
static_assert(sizeof(ntp_packet) == 48);


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


static
void
report_error(const std::string& arg)
{
    std::string msg = "["s + plugin_name + "] "s + arg;
    NotificationModule_AddErrorNotificationEx(msg.c_str(),
                                              cfg::msg_duration,
                                              1,
                                              {255, 255, 255, 255},
                                              {237, 28, 36, 255},
                                              nullptr,
                                              nullptr);
}


static
void
report_info(const std::string& arg)
{
    if (!cfg::notify)
        return;

    std::string msg = "["s + plugin_name + "] "s + arg;
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
ntp_to_double(ntp_timestamp t)
{
    return std::ldexp(static_cast<double>(t), -32);
}


ntp_timestamp
double_to_ntp(double t)
{
    return std::ldexp(t, 32);
}


#if 0
static
OSTime
ntp_to_wiiu(ntp_timestamp t)
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
#endif


static
ntp_timestamp
wiiu_to_ntp(OSTime t)
{
    // Convert from Wii U ticks to seconds.
    // Note: do the conversion in floating point to avoid overflows.
    double dt = double(t) / OSTimerClockSpeed;
    ntp_timestamp r = double_to_ntp(dt);

    // Change r from Wii U epoch (2000) to NTP epoch (1900).
    constexpr std::uint64_t seconds_per_day = 24 * 60 * 60;
    constexpr std::uint64_t seconds_offset = seconds_per_day * (100 * 365 + 24);
    r += seconds_offset << 32;

    return r;
}


static
std::string
to_string(struct in_addr addr)
{
    return inet_ntoa(addr);
}


static
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


#if 0
static
std::string
format_ntp(ntp_timestamp t)
{
    OSTime wt = ntp_to_wiiu(t);
    return format_wiiu_time(wt);
}
#endif


static
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


static
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


static
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
static
std::pair<double, double>
ntp_query(struct in_addr ip_address)
{
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip_address.s_addr;
    addr.sin_port = htons(123); // NTP port

    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd == -1)
        throw std::string{"unable to create socket"};

    connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof addr);

    ntp_packet packet;
    std::memset(&packet, 0, sizeof packet);
    packet.version(4);
    packet.mode(ntp_mode::client);

    ntp_timestamp t1 = wiiu_to_ntp(get_utc_time());
    packet.transmit_time = htobe64(t1);

    if (send(fd, &packet, sizeof packet, 0) == -1) {
        int e = errno;
        close(fd);
        throw std::string{"unable to send NTP request: "s + std::to_string(e)};
    }

    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(fd, &read_set);
    struct timeval timeout = { 4, 0 };

    if (select(fd + 1, &read_set, nullptr, nullptr, &timeout) == -1) {
        int e = errno;
        close(fd);
        throw std::string{"select() failed: "s + std::to_string(e)};
    }

    if (!FD_ISSET(fd, &read_set)) {
        close(fd);
        throw std::string{"timeout reached"};
    }

    if (recv(fd, &packet, sizeof packet, 0) < 48) {
        close(fd);
        throw std::string{"invalid NTP response"s};
    }

    ntp_timestamp t4 = wiiu_to_ntp(get_utc_time());

    close(fd);

    if (be64toh(packet.origin_time) != t1)
        throw std::string{"NTP response does not match request: "s
                          + std::to_string(t1) + " vs "
                          + std::to_string(be64toh(packet.origin_time))};

    // when our request arrived at the server
    ntp_timestamp t2 = be64toh(packet.receive_time);
    // when the server sent out a response
    ntp_timestamp t3 = be64toh(packet.transmit_time);

    ntp_timestamp roundtrip = (t4 - t1) - (t3 - t2);

    double delay = ntp_to_double(roundtrip) / 2;

    // The correct time (t4 + correction) should be t3 + delay.
    double correction = ntp_to_double(t3) + delay - ntp_to_double(t4);

    return { correction, delay };
}


static
void
update_time()
{
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
                            + "): correction="s + std::to_string(correction)
                            + " delay=" + std::to_string(delay));
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

    if (std::fabs(avg_correction) < 0.1) {
        report_info("skipping clock adjustment (correction is "
                    + std::to_string(avg_correction) + " s)");
        return;
    }

    if (cfg::sync) {
        if (!apply_clock_correction(avg_correction)) {
            report_error("failed to set system clock");
            return;
        }
    }

    if (cfg::notify) {
        double ms = avg_correction * 1000;
        char timeStr[64];
        if (std::fabs(ms) < 10'000) // correcting less than 10 seconds
            snprintf(timeStr, sizeof timeStr, "%.3f ms", ms);
        else if (std::fabs(ms) < 120'000) // correcting less than 2 minutes
            snprintf(timeStr, sizeof timeStr, "%.1f s", ms / 1000);
        else if (std::fabs(ms) < 7'200'000) // correcting less than 2 hours
            snprintf(timeStr, sizeof timeStr, "%.1f min", ms / 60'000);
        else
            snprintf(timeStr, sizeof timeStr, "%.1f hrs", ms / 3'600'000);
        std::string msg = "NTP correction from '"s + cfg::server + "': "s + timeStr;
        report_info(msg);
    }
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

        if (WUPS_GetString(nullptr, CFG_SERVER, cfg::server, sizeof cfg::server)
            == WUPS_STORAGE_ERROR_NOT_FOUND)
            WUPS_StoreString(nullptr, CFG_SERVER, cfg::server);

        NotificationModule_InitLibrary(); // Set up for notifications.
        WUPS_CloseStorage();
    }

    if (cfg::sync)
        update_time(); // Update time when plugin is loaded.
}


static
void
update_cfg_sync(ConfigItemBoolean*, bool value)
{
    WUPS_StoreBool(nullptr, CFG_SYNC, value);
    cfg::sync = value;
}


static
void
update_cfg_notify(ConfigItemBoolean*, bool value)
{
    WUPS_StoreBool(nullptr, CFG_NOTIFY, value);
    cfg::notify = value;
}


static
void
update_cfg_msg_duration(ConfigItemIntegerRange*, int32_t value)
{
    WUPS_StoreInt(nullptr, CFG_MSG_DURATION, value);
    cfg::msg_duration = value;
}


static
void
update_cfg_hours(ConfigItemIntegerRange*, int32_t offset)
{
    WUPS_StoreInt(nullptr, CFG_HOURS, offset);
    cfg::hours = offset;
}


static
void
update_cfg_minutes(ConfigItemIntegerRange*, int32_t offset)
{
    WUPS_StoreInt(nullptr, CFG_MINUTES, offset);
    cfg::minutes = offset;
}


WUPS_GET_CONFIG()
{
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS)
        return 0;

    WUPSConfigHandle settings;
    WUPSConfig_CreateHandled(&settings, plugin_name);

    WUPSConfigCategoryHandle config;
    WUPSConfig_AddCategoryByNameHandled(settings, "Configuration", &config);
    WUPSConfigCategoryHandle preview;
    WUPSConfig_AddCategoryByNameHandled(settings, "Preview Time", &preview);

    WUPSConfigItemBoolean_AddToCategoryHandled(settings, config, CFG_SYNC,
                                               "Syncing Enabled",
                                               cfg::sync,
                                               &update_cfg_sync);
    WUPSConfigItemBoolean_AddToCategoryHandled(settings, config, CFG_NOTIFY,
                                               "Show Notifications",
                                               cfg::notify,
                                               &update_cfg_notify);
    WUPSConfigItemIntegerRange_AddToCategoryHandled(settings, config, CFG_MSG_DURATION,
                                                    "Messages Duration (seconds)",
                                                    cfg::msg_duration, 0, 30,
                                                    &update_cfg_msg_duration);
    WUPSConfigItemIntegerRange_AddToCategoryHandled(settings, config, CFG_HOURS,
                                                    "Hours Offset",
                                                    cfg::hours, -12, 14,
                                                    &update_cfg_hours);
    WUPSConfigItemIntegerRange_AddToCategoryHandled(settings, config, CFG_MINUTES,
                                                    "Minutes Offset",
                                                    cfg::minutes, 0, 59,
                                                    &update_cfg_minutes);

    // show current NTP server address, no way to change it.
    std::string server = "NTP server: "s + cfg::server;
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
