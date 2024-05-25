// SPDX-License-Identifier: MIT

#ifndef CORE_HPP
#define CORE_HPP

#include <utility>              // pair<>
#include <string>

#include "net/address.hpp"


namespace core {

    std::pair<double, double> ntp_query(net::address address);

    void run();

    std::string local_clock_to_string();

} // namespace core


#endif
