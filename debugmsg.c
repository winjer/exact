/* $Id: debugmsg.c,v 1.3 2003/01/23 12:34:43 doug Exp $
*/

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "conffile.h"

void debugmsg(int level, char *fmt, ...) {
#ifdef HAVE_VPRINTF
	if(level<=conffile_param_int("loglevel")) {
		va_list ap;
		va_start(ap,fmt);
		vfprintf(stderr, fmt, ap);
		fflush(stderr);
		va_end(ap);
	}
#endif
}
