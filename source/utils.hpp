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


    // Split input string into tokens, according to separators.
    std::vector<std::string>
    split(const std::string& input,
          const std::string& separators);



    // Ordering for sockaddr_in, so we can put it inside a std::set.
    struct less_sockaddr_in {
        bool
        operator ()(const struct sockaddr_in& a,
                    const struct sockaddr_in& b) const noexcept;
    };


    // Generate A.B.C.D string from IP address.
    std::string to_string(const struct sockaddr_in& addr);



    // RAII class to create and close down a socket on exit.
    struct socket_guard {
        int fd;

        socket_guard(int ns, int st, int pr);
        ~socket_guard();

        void close();
    };



    // Wrapper for getaddrinfo(), hardcoded for IPv4
    struct addrinfo_query {
        int flags    = 0;
        int family   = AF_UNSPEC;
        int socktype = 0;
        int protocol = 0;
    };

    struct addrinfo_result {
        int                        family;
        int                        socktype;
        int                        protocol;
        struct sockaddr_in         address;
        std::optional<std::string> canonname;
    };

    std::vector<addrinfo_result>
    get_address_info(const std::optional<std::string>& name,
                     const std::optional<std::string>& port = {},
                     std::optional<addrinfo_query> query = {});




    // RAII type to ensure a function is never executed in parallel.
    struct exec_guard {

        std::atomic<bool>& flag;
        bool guarded; // when false, the function is already executing in some thread.

        exec_guard(std::atomic<bool>& f);

        ~exec_guard();

    };



} // namespace utils

#endif
