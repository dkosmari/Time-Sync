// SPDX-License-Identifier: MIT

#ifndef CFG_HPP
#define CFG_HPP

#include <string>


namespace cfg {

    extern int         hours;
    extern int         minutes;
    extern int         msg_duration;
    extern bool        notify;
    extern std::string server;
    extern bool        sync;
    extern int         tolerance;


    // send the hours and minutes variables to the utc module
    void update_utc_offset();

}

#endif
