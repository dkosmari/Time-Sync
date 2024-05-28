// SPDX-License-Identifier: MIT

#include <coreinit/time.h>

#include "utc.hpp"

#include "cfg.hpp"


namespace utc {

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
        return timestamp{ local_time() - cfg::get_tz_offset() };
    }

} // namespace utc
