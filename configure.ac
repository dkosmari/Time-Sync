#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Time-Sync], [4.2], [http://github.com/dkosmari/Time-Sync])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/main.cpp])

WUPS_INIT
WUMS_INIT

AM_INIT_AUTOMAKE([foreign])

AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_AR

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


# Join all flags into a single variable, we use them in all nested Makefiles

AS_VAR_SET([ALL_CPPFLAGS], ["$PORTLIBS_CPPFLAGS $WUMS_CPPFLAGS $WUPS_CPPFLAGS $WUT_CPPFLAGS"])
AC_SUBST([ALL_CPPFLAGS])

AS_VAR_SET([ALL_CXXFLAGS], ["$WUT_CFLAGS -ffunction-sections -fipa-pta"])
AC_SUBST([ALL_CXXFLAGS])

AS_VAR_SET([ALL_LDFLAGS], ["$WUT_LDFLAGS $WUPS_LDFLAGS"])
AC_SUBST([ALL_LDFLAGS])

AS_VAR_SET([ALL_LIBS], ["$PORTLIBS_LIBS $WUMS_LIBS $WUPS_LIBS $WUT_LIBS"])
AC_SUBST([ALL_LIBS])


AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/net/Makefile
                 src/wupsxx/Makefile])
AC_OUTPUT
