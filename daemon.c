// $Id: daemon.c,v 1.4 2003/01/23 12:34:43 doug Exp $

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

#include "config.h"
#include "debugmsg.h"
#include "conffile.h"

void dofork(int s) {
	pid_t p=fork();
	switch(p) {
		case -1: // error
			debugmsg(DMSG_STANDARD, "Fatal Error when forking\n");
			exit(2);
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
		debugmsg(DMSG_STANDARD, "Fatal Error while running setsid\n");
		exit(2);
	}
}

void rootdir() {
	int ret;
	ret=chdir("/");
	if(ret==-1) {
		debugmsg(DMSG_STANDARD, "Fatal Error while changing to root dir\n");
		exit(2);
	}
}

void reopenfds() {
	freopen("/dev/null", "r", stdin);
	fclose(stdout);
	if(!freopen(conffile_param("logfile"), "a", stderr)) {
		debugmsg(DMSG_STANDARD, "Cannot reopen stderr to %s\n", conffile_param("logfile"));
		exit(3);
	}
}

void daemonize(int s) {
#ifdef HAVE_WORKING_FORK
	dofork(0); // so the parent can exit, returning control
	sesslead(); // become a process group and session group leader
	dofork(s); // session group leader exits.  we can never regain terminal.
	rootdir(); // to ensure no directory is kept in use
	umask(0); // so no weird perms are inherited
	reopenfds(); // so stdin, stdout and stderr are sensible
#else
	// other stuff
#endif
}

