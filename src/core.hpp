/*
 * Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef CORE_HPP
#define CORE_HPP

#include <string>
#include <utility>              // pair<>

#include "net/address.hpp"
#include "time_utils.hpp"


namespace core {

    using time_utils::dbl_seconds;

    std::pair<dbl_seconds, dbl_seconds> ntp_query(net::address address);

    void run(bool update_clock, bool silent);

    std::string local_clock_to_string();

} // namespace core

#endif
