/* $Id: debugmsg.c,v 1.2 2003/01/22 18:18:49 doug Exp $
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
		va_end(ap);
	}
#endif
}
