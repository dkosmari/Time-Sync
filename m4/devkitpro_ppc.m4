# -*- mode: autoconf -*-
# devkitpro_ppc.m4 - Macros to handle PPC toolchains.

# Copyright (c) 2024 Daniel K. O. <dkosmari>
#
# Copying and distribution of this file, with or without modification, are permitted in
# any medium without royalty provided the copyright notice and this notice are
# preserved. This file is offered as-is, without any warranty.

#serial 1

# DEVKITPRO_PPC_INIT
# ------------------
# This macro adjuts the environment for PPC-based targets. It must be called before
# `AM_INIT_AUTOMAKE', and before any cross-compilation tool is checked.
#
# Output variables:
#   - `DEVKITPPC': path to devkitPPC
#   - `DEVKITPRO_CPPFLAGS': prepends include paths for PPC portlibs.
#   - `DEVKITPRO_LIBS': prepends library paths for PPC portlibs.
#   - `PATH': appends `devkitPPC/bin' if necessary.

AC_DEFUN([DEVKITPRO_PPC_INIT],[

    AC_REQUIRE([DEVKITPRO_INIT])

    # Make sure macros that look up programs don't appear before this, since we may need
    # to adjust PATH.
    AC_BEFORE([$0], [AM_INIT_AUTOMAKE])
    # specific program tests
    AC_BEFORE([$0], [AC_PROG_CC])
    AC_BEFORE([$0], [AC_PROG_CXX])
    AC_BEFORE([$0], [AC_PROG_CPP])
    AC_BEFORE([$0], [AC_PROG_RANLIB])
    # automake also has these
    AC_BEFORE([$0], [AM_PROG_AR])
    AC_BEFORE([$0], [AM_PROG_AS])
    # cross-compilation tool tests
    AC_BEFORE([$0], [AC_CHECK_TOOL])
    AC_BEFORE([$0], [AC_CHECK_TOOLS])
    AC_BEFORE([$0], [AC_PATH_TARGET_TOOL])
    AC_BEFORE([$0], [AC_PATH_TOOL])


    # Sanity check for host type.
    AS_CASE($host,
            [powerpc-*-eabi], [],
            [AC_MSG_ERROR([invalid host ($host), you should use --host=powerpc-eabi])])


    # set DEVKITPPC
    AS_VAR_SET([DEVKITPPC], [$DEVKITPRO/devkitPPC])
    AC_SUBST([DEVKITPPC])


    # See if we can find cross tools in PATH already; if not, append $DEVKITPPC/bin to
    # PATH
    AC_MSG_CHECKING([if $DEVKITPPC/bin is in PATH])
    AS_IF([! which powerpc-eabi-nm 1>/dev/null 2>/dev/null],
          [
              AC_MSG_RESULT([no, will append to PATH])
              AS_VAR_APPEND([PATH], [":$DEVKITPPC/bin"])
              AC_SUBST([PATH])
          ],
          [AC_MSG_RESULT([yes])])


    AS_VAR_SET([PORTLIBS_PPC_ROOT], [$PORTLIBS_ROOT/ppc])

    AX_PREPEND_FLAG([-I$PORTLIBS_PPC_ROOT/include], [DEVKITPRO_CPPFLAGS])
    AX_PREPEND_FLAG([-L$PORTLIBS_PPC_ROOT/lib], [DEVKITPRO_LIBS])

    # custom Makefile rules
    AX_ADD_AM_MACRO([
CLEANFILES ?=
CLEANFILES = *.strip.elf
%.strip.elf: %.elf; \$(STRIP) -g \$< -o \$[@]
])


])
