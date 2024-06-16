# -*- mode: autoconf -*-
# wiiu-wups.m4 - Macros to handle Wii U Plugin System

# Copyright (c) 2024 Daniel K. O. <dkosmari>
#
# Copying and distribution of this file, with or without modification, are permitted in
# any medium without royalty provided the copyright notice and this notice are
# preserved. This file is offered as-is, without any warranty.

#serial 1

# WIIU_WUPS_INIT
# --------------
# This macro adjusts the environment for the Wii U Plugin System (WUPS).
#
# Output variables:
#   - `DEVKITPRO_CPPFLAGS'
#   - `DEVKITPRO_LDFLAGS'
#   - `DEVKITPRO_LIBS'

AC_DEFUN([WIIU_WUPS_INIT],[

    AC_REQUIRE([DEVKITPRO_WUT_INIT])

    # set WIIU_WUPS_ROOT
    AS_VAR_SET([WIIU_WUPS_ROOT], [$DEVKITPRO/wups])

    AX_PREPEND_FLAG([-D__WUPS__],                [DEVKITPRO_CPPFLAGS])
    AX_PREPEND_FLAG([-I$WIIU_WUPS_ROOT/include], [DEVKITPRO_CPPFLAGS])

    AX_PREPEND_FLAG([-T$WIIU_WUPS_ROOT/share/wups.ld],         [DEVKITPRO_LDFLAGS])
    AX_PREPEND_FLAG([-specs=$WIIU_WUPS_ROOT/share/wups.specs], [DEVKITPRO_LDFLAGS])

    AX_PREPEND_FLAG([-L$WIIU_WUPS_ROOT/lib -lwups], [DEVKITPRO_LIBS])


    # custom Makefile rules
    AX_ADD_AM_MACRO([
CLEANFILES ?=
CLEANFILES += *.wps
%.wps: %.strip.elf
	\$(ELF2RPL) \$< \$[@]
])

])
