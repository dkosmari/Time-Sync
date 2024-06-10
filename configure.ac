#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Time-Sync], [4.1+], [http://github.com/dkosmari/Time-Sync])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/main.cpp])

WUPS_INIT
WUMS_INIT

AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_PROG_CXX
AC_PROG_RANLIB

AX_APPEND_COMPILE_FLAGS([-std=c++23], [CXXFLAGS])
AC_LANG([C++])

WUMS_LIB_CURLWRAPPER
WUMS_LIB_NOTIFICATIONS


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


AC_CONFIG_FILES([Makefile])
AC_OUTPUT
