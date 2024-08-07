# -*- mode: autoconf -*-
# devkitpro_wut.m4 - Macros to handle WUT setup.

# Copyright (c) 2024 Daniel K. O. <dkosmari>
#
# Copying and distribution of this file, with or without modification, are permitted in
# any medium without royalty provided the copyright notice and this notice are
# preserved. This file is offered as-is, without any warranty.

#serial 1

# DEVKITPRO_WUT_INIT
# ------------------
# This macro adjusts the environment for Wii U homebrew, using WUT.
#
# Output variables:
#   - `DEVKITPRO_CFLAGS'
#   - `DEVKITPRO_CPPFLAGS'
#   - `DEVKITPRO_LDFLAGS'
#   - `DEVKITPRO_LIBS'
#   - `DEVKITPRO_RPL_LDFLAGS'
#   - `ELF2RPL': set to `elf2rpl' binary.
#   - `PATH': appends `DEVKITPRO/tools/bin' if necessary.
#   - `WUT_ROOT': set to `DEVKITPRO/wut'

AC_DEFUN([DEVKITPRO_WUT_INIT],[

    AC_REQUIRE([DEVKITPRO_PPC_INIT])

    # See if we can find elf2rpl in PATH; if not, append $DEVKITPRO/tools/bin to PATH
    AC_MSG_CHECKING([if $DEVKITPRO/tools/bin is in PATH])
    AS_IF([! which elf2rpl 1>/dev/null 2>/dev/null],
          [
              AC_MSG_RESULT([no, will append to PATH])
              AS_VAR_APPEND([PATH], [":$DEVKITPRO/tools/bin"])
              AC_SUBST([PATH])
          ],
          [AC_MSG_RESULT([yes])])

    AC_CHECK_PROGS([ELF2RPL], [elf2rpl])
    AC_CHECK_PROGS([WUHBTOOL], [wuhbtool])

    # set PORTLIBS_WIIU_ROOT
    AS_VAR_SET([PORTLIBS_WIIU_ROOT], [$PORTLIBS_ROOT/wiiu])

    # set WUT_ROOT
    AS_VAR_SET([WUT_ROOT], [$DEVKITPRO/wut])
    AC_SUBST([WUT_ROOT])


    AX_PREPEND_FLAG([-D__WIIU__],                    [DEVKITPRO_CPPFLAGS])
    AX_PREPEND_FLAG([-D__WUT__],                     [DEVKITPRO_CPPFLAGS])
    AX_PREPEND_FLAG([-I$WUT_ROOT/include],           [DEVKITPRO_CPPFLAGS])
    AX_PREPEND_FLAG([-I$WUT_ROOT/usr/include],       [DEVKITPRO_CPPFLAGS])
    AX_PREPEND_FLAG([-I$PORTLIBS_WIIU_ROOT/include], [DEVKITPRO_CPPFLAGS])

    AX_PREPEND_FLAG([-mcpu=750],    [DEVKITPRO_CFLAGS])
    AX_PREPEND_FLAG([-meabi],       [DEVKITPRO_CFLAGS])
    AX_PREPEND_FLAG([-mhard-float], [DEVKITPRO_CFLAGS])

    AX_PREPEND_FLAG([-lwut],                     [DEVKITPRO_LIBS])
    AX_PREPEND_FLAG([-L$WUT_ROOT/lib],           [DEVKITPRO_LIBS])
    AX_PREPEND_FLAG([-L$WUT_ROOT/usr/lib],       [DEVKITPRO_LIBS])
    AX_PREPEND_FLAG([-L$PORTLIBS_WIIU_ROOT/lib], [DEVKITPRO_LIBS])

    AX_PREPEND_FLAG([-specs=$WUT_ROOT/share/wut.specs], [DEVKITPRO_LDFLAGS])

    # set DEVKITPRO_RPL_LDFLAGS
    AS_VAR_SET([DEVKITPRO_RPL_LDFLAGS],
               ["-specs=$WUT_ROOT/share/wut.specs -specs=$WUT_ROOT/share/rpl.specs"])
    AC_SUBST([DEVKITPRO_RPL_LDFLAGS])


    # custom Makefile rules
    AX_ADD_AM_MACRO([
CLEANFILES ?=
CLEANFILES += *.rpx *.rpl
%.rpx: %.strip.elf; \$(ELF2RPL) \$< \$[@]
%.rpl: %.strip.elf; \$(ELF2RPL) --rpl \$< \$[@]
])

])
