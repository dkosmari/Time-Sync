#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Time Sync],
        [5.0],
        [https://github.com/dkosmari/Time-Sync/issues],
        [time-sync],
        [https://github.com/dkosmari/Time-Sync])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/ntp.cpp])

WIIU_WUPS_INIT
WIIU_WUMS_INIT

AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_AR

AX_APPEND_COMPILE_FLAGS([-std=c++23], [CXX])
AC_LANG([C++])

WIIU_WUPS_SETUP
WIIU_WUMS_SETUP
WIIU_WUMS_CHECK_LIBCURLWRAPPER
WIIU_WUMS_CHECK_LIBNOTIFICATIONS

AX_SUBDIRS_CONFIGURE([external/libwupsxx], [--enable-notify])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
