AC_PREREQ(2.59)
AC_INIT(eger, 0.1, virtan@itim.vn)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([eger/logger.h])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Define _GNU_SOURCE
AC_GNU_SOURCE

AM_MAINTAINER_MODE([])

# Dynamic only
#AC_DISABLE_STATIC
#AC_DISABLE_SHARED

# libtool support
LT_INIT
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

# Lang for tests
AC_LANG(C++)
CXXFLAGS="$CXXFLAGS -std=c++11 -W -Wall -Werror -pthread"

# Checks for programs.
AC_PROG_CXX([g++-4.8 g++-4.7 g++-4.6 g++-4.5 g++ c++ cc cxx])
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LN_S

# Options
AC_SYS_LARGEFILE

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([chrono],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([thread],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([algorithm],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([iterator],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([sstream],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([stddef.h],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([stdexcept],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([stdint.h],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([string],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([sys/mman.h],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([vector],[],[AC_MSG_ERROR(Header absent)])
AC_CHECK_HEADERS([functional],[],[AC_MSG_ERROR(Header absent)])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_HEADER_TIME
AC_TYPE_UINT32_T

touch eger/logger.h ;

AC_CONFIG_FILES([
	Makefile
        eger/Makefile
	tests/Makefile
])

AC_OUTPUT
