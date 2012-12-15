# Exiled.Net IRC Services.
# $Id: configure.ac,v 1.3 2003/02/25 23:44:25 mr Exp $

# Process this file with autoconf to produce a configure script.

AC_INIT([Exiled.Net IRC Services], [], [the_real@nerdheaven.dk])
AC_CONFIG_SRCDIR([src/chanserv.c])
AC_LANG(C)
AM_INIT_AUTOMAKE

dnl Checks for programs.
AC_PROG_CC

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_TIME

# required by strcmp()
AC_CHECK_HEADERS(string.h)

# required for socket()
AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([sys/socket.h])

# required for select()
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([sys/time.h])

# required for gethostbyname()
AC_CHECK_HEADERS([netdb.h])

# required for sockaddr_in
AC_CHECK_HEADERS([netinet/in.h])

# required for atexit()
AC_CHECK_HEADERS([stdlib.h])


dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_STRUCT_TIMEZONE

dnl Checks for library functions.
AC_TYPE_SIGNAL
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_FUNC_MALLOC

AC_CHECK_FUNCS([atexit signal fork])
AC_CHECK_FUNCS([memmove memset])
AC_CHECK_FUNCS([select socket gethostbyname listen bind])
AC_CHECK_FUNCS([strcasecmp stricmp strchr strtoul snprintf])

dnl Check for libs
AC_CHECK_LIB(m, floor)
AC_CHECK_LIB(crypt, crypt)

dnl Turn of debug information?

AC_ARG_WITH(debug,
    AC_HELP_STRING([--with-debug],
    [Specifies wheather debug console-output information should be suppressed]))

if test "x$with_debug" = "xno"; then
  DEBUG="-DNDEBUG"
else
  DEBUG=""
fi

dnl MySQL stuff

AC_ARG_WITH(mysql-includes,
    AC_HELP_STRING([--with-mysql-includes=prefix],
    [Prefix where MySQL includes are found (usually /usr/local/include/mysql)]))
AC_ARG_WITH(mysql-libraries,
    AC_HELP_STRING([--with-mysql-libraries=prefix],
    [Prefix where MySQL libraries are found (usually /usr/local/lib/mysql)]))

AC_MSG_CHECKING([for mysql.h])
if test "x$with_mysql_includes" != "x"; then
  if test -f "$with_mysql_includes/mysql.h"; then
    CFLAGS="$CFLAGS -I$with_mysql_includes"
    AC_MSG_RESULT([found in $with_mysql_includes])
  else
    AC_MSG_ERROR([not found in $with_mysql_includes!])
  fi
else
  if test -f "/usr/local/include/mysql/mysql.h"; then
    CFLAGS="$CFLAGS -I/usr/local/include/mysql"
    AC_MSG_RESULT([found in /usr/local/include/mysql])
  elif test -f "/usr/include/mysql/mysql.h"; then
    CFLAGS="$CFLAGS -I/usr/include/mysql"
    AC_MSG_RESULT([found in /usr/include/mysql])
  elif test -f "/opt/include/mysql/mysql.h"; then
    CFLAGS="$CFLAGS -I/opt/include/mysql"
    AC_MSG_RESULT([found in /opt/include/mysql])
  elif test -f "/include/mysql/mysql.h"; then
    CFLAGS="$CFLAGS -I/include/mysql"
    AC_MSG_RESULT([found in /include/mysql])
  elif test -f "/var/mysql/include/mysql.h"; then
    CFLAGS="$CFLAGS -I/var/mysql/include"
    AC_MSG_RESULT([found in /var/mysql/include])
  else
    AC_MSG_ERROR([not found, please specify --with-mysql-include on the commandline!])
  fi
fi

AC_MSG_CHECKING([for mysqlclient library])
if test "x$with_mysql_libraries" != "x"; then
  if test -f "$with_mysql_libraries/libmysqlclient.a"; then
    MYSQL_LIB=$with_mysql_libraries
    AC_MSG_RESULT([found in $MYSQL_LIB])
  else
    AC_MSG_ERROR([not found in $with_mysql_libraries!])
  fi
else
  if test -f "/usr/local/lib/mysql/libmysqlclient.a"; then
    MYSQL_LIB="/usr/local/lib/mysql"
    AC_MSG_RESULT([found in $MYSQL_LIB])
  elif test -f "/usr/lib/mysql/libmysqlclient.a"; then
    MYSQL_LIB="/usr/lib/mysql"
    AC_MSG_RESULT([found in $MYSQL_LIB])
  elif test -f "/lib/mysql/libmysqlclient.a"; then
    MYSQL_LIB="/lib/mysql"
    AC_MSG_RESULT([found in $MYSQL_LIB])
  elif test -f "/var/mysql/lib/libmysqlclient.a"; then
    MYSQL_LIB="/var/lib/mysql"
    AC_MSG_RESULT([found in $MYSQL_LIB])
  elif test -f "/opt/lib/libmysqlclient.a"; then
    MYSQL_LIB="/opt/lib"
    AC_MSG_RESULT([found in $MYSQL_LIB])
  elif test -f "/opt/lib/mysql/libmysqlclient.a"; then
    MYSQL_LIB="/opt/lib/mysql"
    AC_MSG_RESULT([found in $MYSQL_LIB])
  else
    AC_MSG_ERROR(not found, please specify --with-mysql-libraries on the commandline!)
  fi
fi

dnl End of MySQL stuff

ACX_PTHREAD([hmmm=yes],[AC_MSG_ERROR([pthread is needed!])])
  
AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(void*)

if test "x$acx_pthread_ok" = xyes; then
  LIBS="$PTHREAD_LIBS $LIBS"
  CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
  CC="$PTHREAD_CC"
else
  AC_MSG_ERROR([Something's rotten in the state of Denmark])
fi

LDFLAGS="$LDFLAGS -L$MYSQL_LIB"
LIBS="$LIBS -lmysqlclient"

AC_SUBST(DEBUG)
AC_SUBST(MYSQL_LIB)

AC_CONFIG_HEADERS([defines.h])
AC_CONFIG_FILES([Makefile src/Makefile src/nickserv/Makefile src/chanserv/Makefile src/operserv/Makefile src/multiserv/Makefile])

AC_OUTPUT
