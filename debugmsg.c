/* $Id: debugmsg.c,v 1.1 2003/01/22 12:04:21 doug Exp $
*/

#include <stdarg.h>
#include <stdio.h>

#include "config.h"

int debugmsg(int level, char *fmt, ...) {
#ifdef HAVE_VPRINTF
	va_list ap;
	va_start(ap,fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
#endif
}
