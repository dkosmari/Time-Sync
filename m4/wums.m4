# Macros for WUMS

#serial 1

AC_DEFUN([WUMS_INIT],[

    AC_REQUIRE([WUT_INIT])


    # set WUMS_ROOT
    AS_VAR_SET([WUMS_ROOT], [$DEVKITPRO/wums])
    AC_SUBST([WUMS_ROOT])


    # set WUMS_CPPFLAGS
    AS_VAR_SET([WUMS_CPPFLAGS], ["-I$WUMS_ROOT/include"])
    AC_SUBST([WUMS_CPPFLAGS])


    # set WUMS_LIBS
    AS_VAR_SET([WUMS_LIBS], ["-L$WUMS_ROOT/lib"])
    AC_SUBST([WUMS_LIBS])

])dnl WUMS_INIT


# WUMS_LIB_CURLWRAPPER
AC_DEFUN([WUMS_LIB_CURLWRAPPER],[

    AC_REQUIRE([WUMS_INIT])

    AX_VAR_PUSHVALUE([CPPFLAGS], [$CPPFLAGS $WUT_CPPFLAGS $PORTLIBS_CPPFLAGS $WUMS_CPPFLAGS])
    AX_VAR_PUSHVALUE([LIBS], [$LIBS $WUT_LIBS $WUMS_LIBS $PORTLIBS_LIBS])

    AX_CHECK_LIBRARY([WUMS_CURLWRAPPER],
                     [curl/curl.h],
                     [curlwrapper],
                     [AS_VAR_APPEND([WUMS_LIBS], [" -lcurlwrapper"])],
                     [AC_MSG_ERROR([libcurlwrapper not found in $WUMS_ROOT; get it from https://github.com/wiiu-env/libcurlwrapper])])

    AX_VAR_POPVALUE([LIBS])
    AX_VAR_POPVALUE([CPPFLAGS])

])dnl WUMS_LIB_CURLWRAPPER


# WUMS_LIB_NOTIFICATIONS
AC_DEFUN([WUMS_LIB_NOTIFICATIONS],[

    AC_REQUIRE([WUMS_INIT])

    AX_VAR_PUSHVALUE([CPPFLAGS], [$CPPFLAGS $WUT_CPPFLAGS $WUMS_CPPFLAGS])
    AX_VAR_PUSHVALUE([LIBS], [$LIBS $WUT_LIBS $WUMS_LIBS])

    AX_CHECK_LIBRARY([WUMS_NOTIFICATIONS],
                     [notifications/notifications.h],
                     [notifications],
                     [AS_VAR_APPEND([WUMS_LIBS], [" -lnotifications"])],
                     [AC_MSG_ERROR([libnotifications not found in $WUMS_ROOT; get it from https://github.com/wiiu-env/libnotifications])])

    AX_VAR_POPVALUE([LIBS])
    AX_VAR_POPVALUE([CPPFLAGS])

])dnl WUMS_LIB_NOTIFICATIONS
