// SPDX-License-Identifier: MIT

#ifndef WUPSXX_CONFIG_ERROR_HPP
#define WUPSXX_CONFIG_ERROR_HPP

#include <stdexcept>
#include <string>

#include <wups/config.h>


namespace wups::config {

    struct config_error : std::runtime_error {

        config_error(const std::string& msg, WUPSConfigAPIStatus status);

    };

}


#endif
