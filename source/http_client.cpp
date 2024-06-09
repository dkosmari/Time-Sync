// SPDX-License-Identifier: MIT

#include "http_client.hpp"

#include "curl.hpp"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

namespace http {

    std::string
    get(const std::string& url)
    {
        curl::global guard;

        curl::handle handle;

        handle.set_useragent(PACKAGE_NAME "/" PACKAGE_VERSION " (Wii U; Aroma)");
        handle.set_followlocation(true);
        handle.set_url(url);

        handle.perform();

        return handle.result;
    }

} // namespace http
