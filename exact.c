// $Id: exact.c,v 1.5 2003/01/22 19:49:46 doug Exp $
//

#include <stdio.h>
#include <sys/types.h>
#include <regex.h>

#include <getopt.h>

#include "tail.h"
#include "debugmsg.h"
#include "match.h"
#include "conffile.h"
#include "auth.h"
#include "daemon.h"

int onepass() {
	int i;
	int start=0;
	match_login *l;

	tail_read();
	for(i=0;i<tail_bufflen;i++) {
		if(tail_buff[i]=='\n' || tail_buff[i]=='\0') {
			tail_buff[i]='\0';
			l=match_line(tail_buff+start);
			if(l) auth_add(l->username, l->ip);				
			start=i+1;
		}
	}
	return 0;
}

int main(int argc, char *argv[]) {
	conffile_read("exact.conf");
	auth_init();
	match_init();
	//daemonize();
	if(!tail_open(conffile_param("maillog"))) {
		debugmsg(DMSG_STANDARD,"Tail open failed.  Quitting.\n");
		return 2;
	}
	while(1) {
		onepass();
	}
	return 0;
}

