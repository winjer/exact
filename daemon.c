// $Id: daemon.c,v 1.2 2003/01/22 19:27:33 doug Exp $

#include <config.h>

int daemonize() {
#ifdef HAVE_WORKING_FORK
	// stuff
#else
	// other stuff
#endif
	return 0;
}

