# $Id: configure.ac,v 1.14 2003/09/23 19:20:28 doug Exp $
#
# This file is part of EXACT.
#
# EXACT is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# EXACT is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with EXACT; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

AC_INIT(exact, 1.10, doug@pigeonhold.com)
AC_PREREQ(2.57)
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
	[ac_cv_watch=$withval], [ac_cv_watch=/var/log/mail.log])
AC_SUBST(ac_cv_watch)

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_HEADER_TIME
AC_CHECK_HEADERS([syslog.h] [getopt.h])
AC_CHECK_HEADERS([libintl.h])
AC_CHECK_HEADERS([wchar.h])
AC_CHECK_HEADERS([sys/time.h])

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
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([select])
AC_CHECK_FUNCS([strdup])
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
