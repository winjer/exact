/* $Id: daemon.c,v 1.7 2003/01/24 15:32:24 doug Exp $
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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#include "config.h"
#include "logger.h"
#include "conffile.h"
#include "daemon.h"

void dofork(int s) {
	pid_t p=fork();
	switch(p) {
		case -1: // error
			logger(LOG_ERR, "Fatal Error when forking\n");
			exit(61);
			break;
		case 0: // we are the child
			if(s) {
				fprintf(stderr, "Child sleeping - attach gdb to %d\n", 
						getpid());
				sleep(10);
			}
			break;
		default: // we are the parent
			exit(0);
			break;
	}
	
}

void sesslead() {
	pid_t p=setsid();
	if(p==-1) {
		logger(LOG_ERR, "Fatal Error while running setsid\n");
		exit(62);
	}
}

void rootdir() {
	int ret;
	ret=chdir("/");
	if(ret==-1) {
		logger(LOG_ERR, "Fatal Error while changing to root dir\n");
		exit(63);
	}
}

void reopenfds() {
	freopen("/dev/null", "r", stdin);
	fclose(stdout);
	fclose(stderr);
}

void usergroup() {
	if(conffile_param("group")) {
		struct group *g=getgrnam(conffile_param("group"));
		if(g) {
			if(setgid(g->gr_gid)) {
				logger(LOG_CRIT, "unable to change gid to %d\n", g->gr_gid);
				exit(64);
			}
			setegid(g->gr_gid);
		} else {
			logger(LOG_CRIT, "group %d does not exist\n", conffile_param("group"));
			exit(65);
		}
	}
	if(conffile_param("user")) {
		struct passwd *p=getpwnam(conffile_param("user"));
		if(p) {
			if(setuid(p->pw_uid)) {
				logger(LOG_CRIT, "unable to change uid to %d\n", p->pw_uid);
				exit(66);
			}
			seteuid(p->pw_uid);
		} else {
			logger(LOG_CRIT, "user %s does not exist\n", conffile_param("user"));
			exit(67);
		}
	}
}
		

void daemonize(int f, int s) {
#ifdef HAVE_WORKING_FORK
	usergroup(); // change to appropriate user and group
	if(!f) {
		dofork(0); // so the parent can exit, returning control
		sesslead(); // become a process group and session group leader
		dofork(s); // session group leader exits.  we can never regain terminal.
	}
	rootdir(); // to ensure no directory is kept in use
	umask(0); // so no weird perms are inherited
	reopenfds(); // so stdin, stdout and stderr are sensible
#else
	// other stuff
#endif
}

