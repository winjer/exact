/* $Id: debugmsg.c,v 1.4 2003/01/23 14:18:42 doug Exp $
*/

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>

#include "config.h"
#include "conffile.h"

void debug_init() {
	openlog("exact", 0, LOG_MAIL);
}

void debugmsg(int level, char *fmt, ...) {
#ifdef HAVE_VPRINTF
	if(level<=conffile_param_int("loglevel")) {
		va_list ap;
		va_start(ap,fmt);
		vsyslog(stderr, fmt, ap);
		fflush(stderr);
		va_end(ap);
	}
#endif
}

