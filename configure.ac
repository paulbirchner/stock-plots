AC_INIT([stock-plots], [2025], [paul.birchner@st.oth-regensburg.de])

AC_CONFIG_SRCDIR([src/])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_PROG_CC

PKG_PROG_PKG_CONFIG

AC_SEARCH_LIBS([argp_parse], [argp])

AC_CHECK_LIB([curl], [curl_easy_init], [LIBCURL_LIBS="-lcurl"], [AC_MSG_ERROR([libcurl not found])])
AC_SUBST(LIBCURL_LIBS)

PKG_CHECK_MODULES([GTK4], [gtk4 >= 4.0],
                  [AC_MSG_NOTICE([GTK 4 found])],
                  [AC_MSG_ERROR([GTK 4 not found])])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT