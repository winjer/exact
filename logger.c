/* $Id: logger.c,v 1.5 2003/02/19 20:27:15 doug Exp $
 * 
 * This file is part of EXACT.
 *
 * EXACT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * EXACT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with EXACT; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
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
FILE *lf=NULL;
int debug=0;

void logger_init(int u, int d, char *logfile) {
	static char name[255];
	use_syslog=u;
	debug=d;
	sprintf(name, "exact[%d]", (int)getpid());
	if(use_syslog) 
		openlog(name, 0, LOG_MAIL);
	else {
		if(logfile) {
			lf=fopen(logfile,"a");
			if(!lf) {
				fprintf(stderr, "Unable to open log file %s\n", logfile);
				exit(100);
			}
		} else
			lf=stderr;
	}
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
		if(!lf) lf=stderr; // for very early log messages
		vfprintf(lf,fmt, ap);
		fflush(lf);
	}
	va_end(ap);
#endif
}

