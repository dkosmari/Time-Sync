// SPDX-License-Identifier: MIT

#ifndef UTC_HPP
#define UTC_HPP


namespace utc {

    // Seconds since 2000-01-01 00:00:00 UTC
    struct timestamp {
        double value;
    };


    timestamp now() noexcept;


    void update_offset(int hours, int minutes);

} // namespace utc

#endif
