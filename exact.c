// $Id: exact.c,v 1.1 2003/01/22 12:04:21 doug Exp $
//

#include <stdio.h>

#include "tail.h"
#include "debugmsg.h"

int main(int argc, char *argv[]) {
	if(!tail_open("foo.log")) {
		debugmsg(DMSG_STANDARD,"Tail open failed.  Quitting.\n");
		return 2;
	}
	while(1) {
		char *s=tail_read();
		fprintf(stderr, "Sleeping...\n");
	}
	return 0;
}
