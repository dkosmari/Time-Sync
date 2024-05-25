// SPDX-License-Identifier: MIT

// standard headers
#include <cmath>                // fabs()
#include <cstdio>               // snprintf()
#include <cstring>              // memset(), memcpy()
#include <memory>               // unique_ptr<>
#include <stdexcept>            // runtime_error, logic_error
#include <utility>              // move()

// unix headers
#include <arpa/inet.h>          // inet_ntop()
#include <netdb.h>              // getaddrinfo()
#include <sys/socket.h>         // socket()
#include <unistd.h>             // close()

// local headers
#include "utils.hpp"


namespace utils {

    std::string
    errno_to_string(int e)
    {
        char buf[100];
        strerror_r(e, buf, sizeof buf);
        return buf;
    }


    std::string
    seconds_to_human(double s)
    {
        char buf[64];

        if (std::fabs(s) < 2) // less than 2 seconds
            std::snprintf(buf, sizeof buf, "%.1f ms", 1000 * s);
        else if (std::fabs(s) < 2 * 60) // less than 2 minutes
            std::snprintf(buf, sizeof buf, "%.1f s", s);
        else if (std::fabs(s) < 2 * 60 * 60) // less than 2 hours
            std::snprintf(buf, sizeof buf, "%.1f min", s / 60);
        else if (std::fabs(s) < 2 * 24 * 60 * 60) // less than 2 days
            std::snprintf(buf, sizeof buf, "%.1f hrs", s / (60 * 60));
        else
            std::snprintf(buf, sizeof buf, "%.1f days", s / (24 * 60 * 60));

        return buf;
    }


    std::vector<std::string>
    split(const std::string& input,
          const std::string& separators,
          std::size_t max_tokens)
    {
        using std::string;

        std::vector<string> result;

        string::size_type start = input.find_first_not_of(separators);
        while (start != string::npos) {

            // if we can only include one more token
            if (max_tokens && result.size() + 1 == max_tokens) {
                // the last token will be the remaining of the input
                result.push_back(input.substr(start));
                break;
            }

            auto finish = input.find_first_of(separators, start);
            result.push_back(input.substr(start, finish - start));
            start = input.find_first_not_of(separators, finish);
        }

        return result;
    }


    exec_guard::exec_guard(std::atomic<bool>& f) :
        flag(f),
        guarded{false}
    {
        bool expected_flag = false;
        if (flag.compare_exchange_strong(expected_flag, true))
            guarded = true; // Exactly one thread can have the "guarded" flag as true.
    }

    exec_guard::~exec_guard()
    {
        if (guarded)
            flag = false;
    }


} // namespace utils
