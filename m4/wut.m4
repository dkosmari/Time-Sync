# Macro to set up WUT

#serial 1

AC_DEFUN([WUT_INIT],[

    AC_REQUIRE([DKP_PPC_INIT])


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


    # set PORTLIBS_WIIU_ROOT
    AS_VAR_SET([PORTLIBS_WIIU_ROOT], [$PORTLIBS_ROOT/wiiu])
    AC_SUBST([PORTLIBS_WIIU_ROOT])


    # prepend to PORTLIBS_CPPFLAGS, PORTLIBS_LIBS
    AS_VAR_SET([PORTLIBS_CPPFLAGS], ["-I$PORTLIBS_WIIU_ROOT/include $PORTLIBS_CPPFLAGS"])
    AS_VAR_SET([PORTLIBS_LIBS],     ["-L$PORTLIBS_WIIU_ROOT/lib $PORTLIBS_LIBS"])


    # set WUT_ROOT=
    AS_VAR_SET([WUT_ROOT], [$DEVKITPRO/wut])
    AC_SUBST([WUT_ROOT])


    # set WUT_CPPFLAGS
    AS_VAR_SET([WUT_CPPFLAGS],
               ["-D__WIIU__ -D__WUT__ -I$WUT_ROOT/include -I$WUT_ROOT/usr/include"])
    AC_SUBST([WUT_CPPFLAGS])


    # set WUT_CFLAGS
    AS_VAR_SET([WUT_CFLAGS], ["-mcpu=750 -meabi -mhard-float"])
    AC_SUBST([WUT_CFLAGS])


    # set WUT_LDFLAGS
    AS_VAR_SET([WUT_LDFLAGS], ["-specs=$WUT_ROOT/share/wut.specs"])
    AC_SUBST([WUT_LDFLAGS])


    # set WUT_RPL_LDFLAGS
    AS_VAR_SET([WUT_RPL_LDFLASG], ["$WUT_LDFLAGS -specs=$WUT_ROOT/share/rpl.specs"])
    AC_SUBST([WUT_RPL_LDFLAGS])


    # set WUT_LIBS
    AS_VAR_SET([WUT_LIBS], ["-L$WUT_ROOT/lib -L$WUT_ROOT/usr/lib -lwut"])
    AC_SUBST([WUT_LIBS])

])dnl WUT_INIT


# TODO: WUT_LIB_MOCHA
