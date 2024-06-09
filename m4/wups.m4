# Macro to set up WUPS

#serial 1

AC_DEFUN([WUPS_INIT],[

    AC_REQUIRE([WUT_INIT])

    AS_VAR_SET([WUPS_ROOT], [$DEVKITPRO/wups])
    AC_SUBST([WUPS_ROOT])


    # set WUPS_CPPFLAGS
    AS_VAR_SET([WUPS_CPPFLAGS], ["-D__WUPS__ -I$WUPS_ROOT/include"])
    AC_SUBST([WUPS_CPPFLAGS])


    # set WUPS_LDFLAGS
    AS_VAR_SET([WUPS_LDFLAGS],
               ["-T$WUPS_ROOT/share/wups.ld -specs=$WUPS_ROOT/share/wups.specs"])
    AC_SUBST([WUPS_LDFLAGS])


    # set WUPS_LIBS
    AS_VAR_SET([WUPS_LIBS], ["-L$WUPS_ROOT/lib -lwups"])
    AC_SUBST([WUPS_LIBS])

])dnl WUPS_INIT
