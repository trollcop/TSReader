AC_INIT(
	[[]ATDV_API2 Demo Application],
	[[3.0.0]],
	[[Engineering Spirit <info@engineering-spirit.nl>]],
	[[ATDemoApp]])
	


AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE

AC_HEADER_STDC

AC_CHECK_HEADERS(pthread.h)
AC_CHECK_LIB(pthread, pthread_create)

AC_DEFINE([_POSIX_THREADS], [], [Description])
LIBS="-lrt -latdv_api3 $LIBS"


AC_ARG_ENABLE([debug],
[ --enable-debug Turn on debugging],
[case "${enableval}" in
yes) debug=true ;;
no) debug=false ;;
*) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],[debug=false])



if test x$debug = xfalse; then

echo 'No debug'
CFLAGS='-O2'
CXXFLAGS='-O2'
else

CFLAGS='-g'
CXXFLAGS='-g'

fi

AC_PROG_CC
AC_PROG_CXX  

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

