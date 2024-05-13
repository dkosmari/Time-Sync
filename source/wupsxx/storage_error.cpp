// SPDX-License-Identifier: MIT

#include "wupsxx/storage_error.hpp"


namespace wups::storage {

    storage_error::storage_error(WUPSStorageError status) :
        std::runtime_error{std::string(WUPSStorageAPI::GetStatusStr(status))}
    {}

}
