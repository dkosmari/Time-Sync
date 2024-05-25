// SPDX-License-Identifier: MIT

#ifndef UTILS_HPP
#define UTILS_HPP

#include <atomic>
#include <optional>
#include <string>
#include <vector>

#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h> // AF_*

namespace utils {


    // Wrapper for strerror_r()
    std::string errno_to_string(int e);


    // Generate time duration strings for humans.
    std::string seconds_to_human(double s);


    /**
     * Split input string into tokens, according to separators.
     *
     * If max_tokens is not zero, only up to max_tokens will be generated; the last token
     * will be the remaining of the string.
     */
    std::vector<std::string>
    split(const std::string& input,
          const std::string& separators,
          std::size_t max_tokens = 0);




    // RAII type to ensure a function is never executed in parallel.
    struct exec_guard {

        std::atomic<bool>& flag;
        bool guarded; // when false, the function is already executing in some thread.

        exec_guard(std::atomic<bool>& f);

        ~exec_guard();

    };


} // namespace utils

#endif
