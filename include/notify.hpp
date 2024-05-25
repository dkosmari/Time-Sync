// SPDX-License-Identifier: MIT

#ifndef NOTIFY_HPP
#define NOTIFY_HPP

#include <string>


namespace notify {

    void initialize();
    void finalize();

    void error(const std::string& arg);
    void info(const std::string& arg);
    void success(const std::string& arg);


    // RAII type to ensure it's intialized and finalized
    class guard {
        bool must_finalize;
    public:
        guard(bool init = true);
        ~guard();
        void release();
    };

}


#endif
