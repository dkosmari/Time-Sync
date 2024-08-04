#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Time-Sync], [4.2], [https://github.com/dkosmari/Time-Sync/issues],
        [], [https://github.com/dkosmari/Time-Sync])
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

AX_APPEND_COMPILE_FLAGS([-std=c++23], [CXXFLAGS])
AC_LANG([C++])

WIIU_WUMS_CHECK_LIBCURLWRAPPER
WIIU_WUMS_CHECK_LIBNOTIFICATIONS


AC_DEFINE([PLUGIN_NAME],
          ["Time Sync"],
          [The plugin name shown in WUPS])
AC_DEFINE([PLUGIN_VERSION],
          ["v" PACKAGE_VERSION],
          [The plugin version shown in WUPS])
AC_DEFINE([PLUGIN_DESCRIPTION],
          ["A plugin that synchronizes the system clock to the Internet."],
          [The plugin description.])
AC_DEFINE([PLUGIN_AUTHOR],
          ["Nightkingale, Daniel K. O."],
          [The list of authors])
AC_DEFINE([PLUGIN_LICENSE],
          ["MIT"],
          [The license.])

AM_EXTRA_RECURSIVE_TARGETS([run])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/net/Makefile
                 src/wupsxx/Makefile])
AC_OUTPUT
