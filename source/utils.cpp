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
          const std::string& separators)
    {
        using std::string;

        std::vector<string> result;

        string::size_type start = input.find_first_not_of(separators);
        while (start != string::npos) {
            auto finish = input.find_first_of(separators, start);
            result.push_back(input.substr(start, finish - start));
            start = input.find_first_not_of(separators, finish);
        }

        return result;
    }



    bool
    less_sockaddr_in::operator ()(const struct sockaddr_in& a,
                                  const struct sockaddr_in& b)
        const noexcept
    {
        return a.sin_addr.s_addr < b.sin_addr.s_addr;
    }



    std::string
    to_string(const struct sockaddr_in& addr)
    {
        char buf[32];
        return inet_ntop(addr.sin_family, &addr.sin_addr,
                         buf, sizeof buf);
    }



    socket_guard::socket_guard(int ns, int st, int pr) :
        fd{::socket(ns, st, pr)}
    {
        if (fd == -1)
            throw std::runtime_error{"Unable to create socket!"};
    }

    socket_guard::~socket_guard()
    {
        if (fd != -1)
            close();
    }

    void
    socket_guard::close()
    {
        ::close(fd);
        fd = -1;
    }



    std::vector<addrinfo_result>
    get_address_info(const std::optional<std::string>& name,
                     const std::optional<std::string>& port,
                     std::optional<addrinfo_query> query)
    {
        // RAII: unique_ptr is used to invoke freeaddrinfo() on function exit
        std::unique_ptr<struct addrinfo,
                        decltype([](struct addrinfo* p) { freeaddrinfo(p); })>
            info;

        {
            struct addrinfo hints;
            const struct addrinfo *hints_ptr = nullptr;

            if (query) {
                hints_ptr = &hints;
                std::memset(&hints, 0, sizeof hints);
                hints.ai_flags = query->flags;
                hints.ai_family = query->family;
                hints.ai_socktype = query->socktype;
                hints.ai_protocol = query->protocol;
            }

            struct addrinfo* raw_info = nullptr;
            int err = getaddrinfo(name ? name->c_str() : nullptr,
                                  port ? port->c_str() : nullptr,
                                  hints_ptr,
                                  &raw_info);
            if (err)
                throw std::runtime_error{gai_strerror(err)};

            info.reset(raw_info); // put it in the smart pointer
        }

        std::vector<addrinfo_result> result;

        // walk through the linked list
        for (auto a = info.get(); a; a = a->ai_next) {

            // sanity check: Wii U only supports IPv4
            if (a->ai_addrlen != sizeof(struct sockaddr_in))
                throw std::logic_error{"getaddrinfo() returned invalid result!"};

            addrinfo_result item;
            item.family = a->ai_family;
            item.socktype = a->ai_socktype;
            item.protocol = a->ai_protocol,
                std::memcpy(&item.address, a->ai_addr, sizeof item.address);
            if (a->ai_canonname)
                item.canonname = a->ai_canonname;

            result.push_back(std::move(item));
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
