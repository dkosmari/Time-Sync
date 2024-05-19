// SPDX-License-Identifier: MIT

#include <notifications/notifications.h>

#include "notify.hpp"

#include "cfg.hpp"
#include "log.hpp"


namespace notify {

    /*
     * Note: libnotification can safely handle multiple initializations and deinitializations.
     * Double-Init is a no-op.
     * Double De-Init is a no-op.
     */

    void
    initialize()
    {
        NotificationModule_InitLibrary();
    }


    void
    cleanup()
    {
        NotificationModule_DeInitLibrary();
    }


    void
    error(const std::string& arg)
    {
        logging::printf("ERROR: %s", arg.c_str());

        if (!cfg::notify)
            return;

        std::string msg = "[" PLUGIN_NAME "] " + arg;
        NotificationModule_AddErrorNotificationEx(msg.c_str(),
                                                  cfg::msg_duration,
                                                  1,
                                                  {255, 255, 255, 255},
                                                  {160, 32, 32, 255},
                                                  nullptr,
                                                  nullptr,
                                                  true);
    }


    void
    info(const std::string& arg)
    {
        logging::printf("INFO: %s", arg.c_str());

        if (!cfg::notify)
            return;

        std::string msg = "[" PLUGIN_NAME "] " + arg;
        NotificationModule_AddInfoNotificationEx(msg.c_str(),
                                                 cfg::msg_duration,
                                                 {255, 255, 255, 255},
                                                 {32, 32, 160, 255},
                                                 nullptr,
                                                 nullptr,
                                                 true);
    }


    void
    success(const std::string& arg)
    {
        logging::printf("SUCCESS: %s", arg.c_str());

        if (!cfg::notify)
            return;

        std::string msg = "[" PLUGIN_NAME "] " + arg;
        NotificationModule_AddInfoNotificationEx(msg.c_str(),
                                                 cfg::msg_duration,
                                                 {255, 255, 255, 255},
                                                 {32, 160, 32, 255},
                                                 nullptr,
                                                 nullptr,
                                                 true);
    }

}
