/* $Id: logger.c,v 1.1 2003/01/24 13:59:45 doug Exp $
*/

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include "conffile.h"
#include "logger.h"

int use_syslog=0;
int debug=0;

void logger_init(int u, int d) {
	static char name[255];
	use_syslog=u;
	debug=d;
	sprintf(name, "exact[%d]", getpid());
	if(use_syslog) 
		openlog(name, 0, LOG_MAIL);
	logger(LOG_DEBUG, "Running in debug mode\n");
}

void logger(int level, char *fmt, ...) {
#ifdef HAVE_VPRINTF
	va_list ap;
	if(level==LOG_DEBUG && !debug) return;
	va_start(ap,fmt);
	if(use_syslog)
		vsyslog(level, fmt, ap);
	else {
		vfprintf(stderr,fmt, ap);
		fflush(stderr);
	}
	va_end(ap);
#endif
}

