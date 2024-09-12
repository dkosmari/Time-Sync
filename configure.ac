#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Time Sync],
        [4.2+],
        [https://github.com/dkosmari/Time-Sync/issues],
        [time-sync],
        [https://github.com/dkosmari/Time-Sync])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/main.cpp])

WIIU_WUPS_INIT
WIIU_WUMS_INIT

AM_INIT_AUTOMAKE([foreign])

AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_AR

AX_APPEND_COMPILE_FLAGS([-std=c++23], [CXX])
AC_LANG([C++])

WIIU_WUMS_CHECK_LIBCURLWRAPPER
WIIU_WUMS_CHECK_LIBNOTIFICATIONS


AM_EXTRA_RECURSIVE_TARGETS([run])

AC_CONFIG_SUBDIRS([external/libwupsxx])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/net/Makefile])
AC_OUTPUT
