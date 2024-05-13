// SPDX-License-Identifier: MIT

#include <coreinit/time.h>

#include "utc.hpp"

#include "cfg.hpp"


namespace utc {

    double
    get_timezone_offset()
    {
        return (cfg::hours * 60.0 + cfg::minutes) * 60.0;
    }


    static
    double
    local_time()
    {
        return static_cast<double>(OSGetTime()) / OSTimerClockSpeed;
    }


    timestamp
    now()
        noexcept
    {
        return timestamp{ local_time() - get_timezone_offset() };
    }

} // namespace utc
