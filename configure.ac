# Process this file with autoconf to produce a configure script.
AC_INIT(exact, 1.00, doug@pigeonhold.com)
AC_CONFIG_SRCDIR([tail.c])

AC_CONFIG_HEADER([config.h])

AC_ARG_WITH([user], 
	AC_HELP_STRING([--with-user=username], [set the username (default is mail)]),
	[ac_cv_user=$withval], [ac_cv_user=mail])
AC_SUBST(ac_cv_user)

AC_ARG_WITH([group],
	AC_HELP_STRING([--with-group=groupname], [set the groupname (default is mail)]),
	[ac_cv_group=$withval], [ac_cv_group=mail])
AC_SUBST(ac_cv_group)

AC_ARG_WITH([watch],
	AC_HELP_STRING([--with-watch=filename], [set the filename of the logfile to watch (default is /var/log/mail)]),
	[ac_cv_watch=$withval], [ac_cv_watch=/var/log/mail])
AC_SUBST(ac_cv_watch)

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([syslog.h])

# Checks for typedefs, structures, and compiler characteristics.

AC_TYPE_SIZE_T
AC_C_CONST
AC_TYPE_SIGNAL

# Checks for library functions.

AC_FUNC_FORK
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([regcomp])
AC_CHECK_FUNCS([alarm])
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
