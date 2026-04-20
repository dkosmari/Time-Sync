#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([Time Sync],
        [5.1],
        [https://github.com/dkosmari/Time-Sync/issues],
        [time-sync],
        [https://github.com/dkosmari/Time-Sync])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/ntp.cpp])

DEVKITPRO_WUT_INIT

AM_INIT_AUTOMAKE([foreign subdir-objects tar-pax])

LT_INIT([disable-shared])

AC_PROG_CXX
AC_LANG([C++])
AX_APPEND_COMPILE_FLAGS([-std=c++23], [CXX])

DEVKITPRO_WUT_SETUP

WIIU_ENV_SETUP_LIBCURLWRAPPER([external/libcurlwrapper])
WIIU_ENV_SETUP_LIBNOTIFICATIONS([external/libwupsxx/external/libnotifications])
WIIU_ENV_SETUP_WUPS([external/libwupsxx/external/WiiUPluginSystem])

AX_SUBDIRS_CONFIGURE([external/libwupsxx],
                     [[--enable-notify],
                      [--disable-shortcuts]])

AC_CONFIG_FILES([Makefile
                 external/Makefile])
AC_OUTPUT
