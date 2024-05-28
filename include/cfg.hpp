// SPDX-License-Identifier: MIT

#ifndef CFG_HPP
#define CFG_HPP

#include <string>


namespace cfg {

    namespace key {
        extern const char* auto_tz;
        extern const char* hours;
        extern const char* minutes;
        extern const char* msg_duration;
        extern const char* notify;
        extern const char* server;
        extern const char* sync;
        extern const char* threads;
        extern const char* tolerance;
    }

    extern bool        auto_tz;
    extern int         hours;
    extern int         minutes;
    extern int         msg_duration;
    extern bool        notify;
    extern std::string server;
    extern bool        sync;
    extern int         threads;
    extern int         tolerance;


    void load();
    void reload();
    void save();

    int get_tz_offset();
    void set_tz_offset(int tz_offset);

} // namespace cfg

#endif
