// SPDX-License-Identifier: MIT

#include <arpa/inet.h>
#include <malloc.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>

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
#include <wups/config/WUPSConfigItemStub.h>
#include <wups/config/WUPSConfigItemIntegerRange.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <thread>
#include <string>
#include <cmath>

using namespace std::literals;


#define CFG_SYNCING_ENABLED "enabledSync"
#define CFG_DST_ENABLED "enabledDST"
#define CFG_NOTIFY_ENABLED "enabledNotify"
#define CFG_OFFSET_HOURS "offsetHours"
#define CFG_OFFSET_MINUTES "offsetMinutes"
#define CFG_SERVER "server"
// Seconds between 1900 (NTP epoch) and 2000 (Wii U epoch)
#define NTP_TIMESTAMP_DELTA 3155673600llu

// Important plugin information.
WUPS_PLUGIN_NAME("Wii U Time Sync");
WUPS_PLUGIN_DESCRIPTION("A plugin that synchronizes a Wii U's clock to the Internet.");
WUPS_PLUGIN_VERSION("v1.1.0-dko");
WUPS_PLUGIN_AUTHOR("Nightkingale");
WUPS_PLUGIN_LICENSE("MIT");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE("Wii U Time Sync");

static const char pluginName[] = "Wii U Time Sync";
bool enabledSync = false;
bool enabledDST = false;
bool enabledNotify = true;
int offsetHours = 0;
int offsetMinutes = 0;
char server[256] = "pool.ntp.org";


// From https://github.com/lettier/ntpclient/blob/master/source/c/main.c

struct ntp_timestamp {
    uint32_t seconds;
    uint32_t frac;
};

struct ntp_packet
{
    uint8_t leap : 2;
    uint8_t version : 3; // should be 4
    uint8_t mode : 3;    // should be 3


    uint8_t stratum;         // Eight bits. Stratum level of the local clock.
    uint8_t poll;            // Eight bits. Maximum interval between successive messages.
    uint8_t precision;       // Eight bits. Precision of the local clock.

    uint32_t rootDelay;      // 32 bits. Total round trip delay time.
    uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
    uint32_t refId;          // 32 bits. Reference clock identifier.

    ntp_timestamp ref;       // Reference time-stamp.
    ntp_timestamp orig;      // Originate time-stamp.
    ntp_timestamp rx;        // Received time-stamp.

    ntp_timestamp tx;        // Transmit time-stamp.

};
static_assert(sizeof(ntp_packet) == 48);


static
void reportError(const std::string& msg)
{
    std::string fullMsg = "["s + pluginName + "] " + msg;
    NotificationModule_AddErrorNotificationEx(fullMsg.c_str(),
                                              12,
                                              1,
                                              {255, 255, 255, 255},
                                              {237, 28, 36, 255},
                                              nullptr,
                                              nullptr);
}

static
void reportInfo(const std::string& msg)
{
    std::string fullMsg = "["s + pluginName + "] " + msg;
    NotificationModule_AddInfoNotificationEx(fullMsg.c_str(),
                                             6,
                                             {255, 255, 255, 255},
                                             {100, 100, 100, 255},
                                             nullptr,
                                             nullptr);
}

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


extern "C" int32_t CCRSysSetSystemTime(OSTime time);
extern "C" BOOL __OSSetAbsoluteSystemTime(OSTime time);

bool SetSystemTime(OSTime time)
{
    nn::pdm::NotifySetTimeBeginEvent();

    if (CCRSysSetSystemTime(time) != 0) {
        nn::pdm::NotifySetTimeEndEvent();
        return false;
    }

    BOOL res = __OSSetAbsoluteSystemTime(time);

    nn::pdm::NotifySetTimeEndEvent();

    return res != FALSE;
}


// this version is signed
int64_t
ticksToMs(OSTime ticks)
{
    return ticks * 1'000 / OSTimerClockSpeed;
}

int64_t
ticksToUs(OSTime ticks)
{
    return ticks * 1'000'000 / OSTimerClockSpeed;
}


OSTime NTPGetTime(const char* hostname)
{
    // Get host address by name
    struct hostent* host = gethostbyname(hostname);
    if (!host) {
        reportError("unable to resolve host '"s + hostname + "': "s +
                    h_errno_to_string());
        return 0;
    }

    // Prepare socket address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;

    // Copy the server's IP address to the server address structure.
    memcpy(&addr.sin_addr.s_addr, host->h_addr, host->h_length);

    // Convert the port number integer to network big-endian style and save it to the server address structure.
    addr.sin_port = htons(123); // UDP port for NTP

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int err = 0;

    if (sockfd < 0) {
        reportError("unable to create socket");
        return 0;
    }

    connect(sockfd, reinterpret_cast<sockaddr*>(&addr), sizeof addr);

    ntp_packet packet;
    memset(&packet, 0, sizeof(packet));
    packet.leap = 0;
    packet.version = 4;
    packet.mode = 3;

    // Send it the NTP packet it wants.
    err = send(sockfd, &packet, sizeof(packet), 0);
    if (err < 0) {
        close(sockfd);
        reportError("unable to send NTP packet (" + std::to_string(err) + ")");
        return 0;
    }

    // Wait and receive the packet back from the server.
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sockfd, &readSet);
    timeval timeout = {
        .tv_sec = 5,
        .tv_usec = 0
    };

    err = select(sockfd+1, &readSet, nullptr, nullptr, &timeout);
    if (err == -1) {
        err = errno;
        close(sockfd);
        reportError("select() failed, errno = "s + std::to_string(err));
        return 0;
    }

    if (err == 0 || !FD_ISSET(sockfd, &readSet)) {
        close(sockfd);
        reportError("timeout reached");
        return 0;
    }

    err = recv(sockfd, &packet, sizeof packet, 0);
    if (err < 0) {
        close(sockfd);
        reportError("unable to read NTP response (" + std::to_string(err) + ")");
        return 0;
    }

    // Close the socket
    close(sockfd);

    // These two fields contain the time-stamp seconds as the packet left the NTP server.
    // The number of seconds correspond to the seconds passed since 1900.
    // ntohl() converts the bit/byte order from the network's to host's "endianness".
    uint32_t tx_seconds = ntohl(packet.tx.seconds);
    uint32_t tx_frac = ntohl(packet.tx.frac);

    OSTime tick = 0;
    // Convert seconds to ticks and adjust timestamp
    tick += OSSecondsToTicks(tx_seconds - NTP_TIMESTAMP_DELTA);
    // Convert fraction to ticks
    tick += OSNanosecondsToTicks((tx_frac * 1000000000llu) >> 32);
    return tick;
}

void updateTime() {
    OSTime ntpTime = NTPGetTime(server); // Connect to the time server.

    if (ntpTime == 0)
        return; // Probably didn't connect correctly.

    if (offsetHours < 0)
        ntpTime -= OSSecondsToTicks(std::abs(offsetHours) * 60 * 60);
    else
        ntpTime += OSSecondsToTicks(offsetHours * 60 * 60);

    if (enabledDST)
        ntpTime += OSSecondsToTicks(60 * 60); // DST adds an hour.

    ntpTime += OSSecondsToTicks(offsetMinutes * 60);

    OSTime localTime = OSGetTime();
    OSTime difference = ntpTime - localTime;
    uint64_t absDifference = std::abs(difference);

    if (absDifference <= OSMillisecondsToTicks(250))
        return; // Time difference is within 250 milliseconds, no need to update.

    SetSystemTime(ntpTime); // This finally sets the console time.

    if (enabledNotify) {
        double ms = ticksToUs(difference) / 1000.0;
        char timeStr[64];
        if (std::fabs(ms) < 10'000) // correcting less than 10 seconds
            snprintf(timeStr, sizeof timeStr, "%.3f ms", ms);
        else if (std::fabs(ms) < 120'000) // correcting less than 2 minutes
            snprintf(timeStr, sizeof timeStr, "%.1f s", ms / 1000);
        else if (std::fabs(ms) < 7'200'000) // correcting less than 2 hours
            snprintf(timeStr, sizeof timeStr, "%.1f min", ms / 60'000);
        else
            snprintf(timeStr, sizeof timeStr, "%.1f hrs", ms / 3'600'000);
        std::string msg = "NTP correction from '"s + server + "': "s + timeStr;
        reportInfo(msg);
    }
}

INITIALIZE_PLUGIN() {
    WUPSStorageError storageRes = WUPS_OpenStorage();
    // Check if the plugin's settings have been saved before.
    if (storageRes == WUPS_STORAGE_ERROR_SUCCESS) {
        if ((storageRes = WUPS_GetBool(nullptr, CFG_SYNCING_ENABLED, &enabledSync)) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            WUPS_StoreBool(nullptr, CFG_SYNCING_ENABLED, enabledSync);
        }

        if ((storageRes = WUPS_GetBool(nullptr, CFG_DST_ENABLED, &enabledDST)) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            WUPS_StoreBool(nullptr, CFG_DST_ENABLED, enabledDST);
        }

        if ((storageRes = WUPS_GetBool(nullptr, CFG_NOTIFY_ENABLED, &enabledNotify)) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            WUPS_StoreBool(nullptr, CFG_NOTIFY_ENABLED, enabledNotify);
        }

        if ((storageRes = WUPS_GetInt(nullptr, CFG_OFFSET_HOURS, &offsetHours)) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            WUPS_StoreInt(nullptr, CFG_OFFSET_HOURS, offsetHours);
        }

        if ((storageRes = WUPS_GetInt(nullptr, CFG_OFFSET_MINUTES, &offsetMinutes)) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            WUPS_StoreInt(nullptr, CFG_OFFSET_MINUTES, offsetMinutes);
        }

        if ((storageRes = WUPS_GetString(nullptr, CFG_SERVER, server, sizeof server)) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            WUPS_StoreString(nullptr, CFG_SERVER, server);
        }

        NotificationModule_InitLibrary(); // Set up for notifications.
        WUPS_CloseStorage(); // Close the storage.
    }

    if (enabledSync) {
        updateTime(); // Update time when plugin is loaded.
    }
}

void syncingEnabled(ConfigItemBoolean *, bool value)
{
    // If false, bro is literally a time traveler!
    WUPS_StoreBool(nullptr, CFG_SYNCING_ENABLED, value);
    enabledSync = value;
}

void savingsEnabled(ConfigItemBoolean *, bool value)
{
    WUPS_StoreBool(nullptr, CFG_DST_ENABLED, value);
    enabledDST = value;
}

void notifyEnabled(ConfigItemBoolean *, bool value)
{
    WUPS_StoreBool(nullptr, CFG_NOTIFY_ENABLED, value);
    enabledNotify = value;
}

void onHourOffsetChanged(ConfigItemIntegerRange *, int32_t offset)
{
    WUPS_StoreInt(nullptr, CFG_OFFSET_HOURS, offset);
    offsetHours = offset;
}

void onMinuteOffsetChanged(ConfigItemIntegerRange *, int32_t offset)
{
    WUPS_StoreInt(nullptr, CFG_OFFSET_MINUTES, offset);
    offsetMinutes = offset;
}

WUPS_GET_CONFIG() {
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        return 0;
    }

    WUPSConfigHandle settings;
    WUPSConfig_CreateHandled(&settings, pluginName);

    WUPSConfigCategoryHandle config;
    WUPSConfig_AddCategoryByNameHandled(settings, "Configuration", &config);
    WUPSConfigCategoryHandle preview;
    WUPSConfig_AddCategoryByNameHandled(settings, "Preview Time", &preview);

    WUPSConfigItemBoolean_AddToCategoryHandled(settings, config,
                                               "enabledSync", "Syncing Enabled",
                                               enabledSync, &syncingEnabled);
    WUPSConfigItemBoolean_AddToCategoryHandled(settings, config,
                                               "enabledDST", "Daylight Savings",
                                               enabledDST, &savingsEnabled);
    WUPSConfigItemBoolean_AddToCategoryHandled(settings, config,
                                               "enabledNotify", "Receive Notifications",
                                               enabledNotify, &notifyEnabled);
    WUPSConfigItemIntegerRange_AddToCategoryHandled(settings, config,
                                                    "offsetHours", "Time Offset (hours)",
                                                    offsetHours, -12, 14, &onHourOffsetChanged);
    WUPSConfigItemIntegerRange_AddToCategoryHandled(settings, config,
                                                    "offsetMinutes", "Time Offset (minutes)",
                                                    offsetMinutes, 0, 59, &onMinuteOffsetChanged);

    // show current NTP server address, no way to change it.
    char serverString[256 + 12];
    snprintf(serverString, sizeof serverString, "NTP server: %s", server);
    WUPSConfigItemStub_AddToCategoryHandled(settings, config, CFG_SERVER, serverString);

    OSCalendarTime ct;
    OSTicksToCalendarTime(OSGetTime(), &ct);
    char timeString[256];
    snprintf(timeString, sizeof timeString, "Current Time: %04d-%02d-%02d %02d:%02d:%02d:%04d:%04d\n", ct.tm_year, ct.tm_mon + 1, ct.tm_mday, ct.tm_hour, ct.tm_min, ct.tm_sec, ct.tm_msec, ct.tm_usec);
    WUPSConfigItemStub_AddToCategoryHandled(settings, preview, "time", timeString);

    return settings;
}

WUPS_CONFIG_CLOSED() {
    if (enabledSync) {
        std::thread updateTimeThread(updateTime);
        updateTimeThread.detach(); // Update time when settings are closed.
    }

    WUPS_CloseStorage(); // Save all changes.
}
