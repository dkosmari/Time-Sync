// SPDX-License-Identifier: MIT

#ifndef NOTIFY_HPP
#define NOTIFY_HPP

#include <string>


namespace notify {

    void initialize();
    void cleanup();

    void error(const std::string& arg);
    void info(const std::string& arg);
    void success(const std::string& arg);

}


#endif
