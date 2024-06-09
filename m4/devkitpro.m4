# Macro to check for the location of devkitPro

#serial 1

AC_DEFUN([DKP_INIT],[

    AC_REQUIRE([AC_CANONICAL_HOST])

    # declare DEVKITPRO as a precious variable
    AC_ARG_VAR([DEVKITPRO], [path to devkitPro])

    # override DEVKITPRO with --with-devkitpro=...
    AC_ARG_WITH([devkitpro],
                [AS_HELP_STRING([--with-devkitpro=PATH-TO-DEVKITPRO],
                                [Set the base path to devkitPro. This overrides the variable DEVKITPRO])],
                [AS_VAR_SET([DEVKITPRO], [$withval])])

    AC_MSG_CHECKING([devkitPro path])
    AC_MSG_RESULT([$DEVKITPRO])

    AS_VAR_SET_IF([DEVKITPRO],
                  [],
                  [AC_MSG_ERROR([DEVKITPRO undefined])])


    # set PORTLIBS_ROOT
    AS_VAR_SET([PORTLIBS_ROOT], [$DEVKITPRO/portlibs])
    AC_SUBST([PORTLIBS_ROOT])

    # declare PORTLIBS_CPPFLAGS and PORTLIBS_LIBS as precious variables
    AC_ARG_VAR([PORTLIBS_CPPFLAGS], [flags to find portlibs headers])
    AC_ARG_VAR([PORTLIBS_LIBS],   [flags to link portlibs libraries])

])dnl DKP_INIT
