// SPDX-License-Identifier: MIT

#ifndef CFG_HPP
#define CFG_HPP

#include <string>


namespace cfg {

    namespace key {
        extern const char* hours;
        extern const char* minutes;
        extern const char* msg_duration;
        extern const char* notify;
        extern const char* server;
        extern const char* sync;
        extern const char* threads;
        extern const char* tolerance;
    }

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

    void update_offsets_from_tz_offset(int tz_offset);

}

#endif
