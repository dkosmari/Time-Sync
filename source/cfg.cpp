// SPDX-License-Identifier: MIT

#include "cfg.hpp"

#include "utc.hpp"


namespace cfg {

    int         hours        = 0;
    int         minutes      = 0;
    int         msg_duration = 5;
    bool        notify       = true;
    std::string server       = "pool.ntp.org";
    bool        sync         = false;
    int         tolerance    = 250;


    void
    update_utc_offset()
    {
        double offset_seconds = (hours * 60.0 + minutes) * 60.0;
        utc::timezone_offset = offset_seconds;
    }

} // namespace cfg
