// $Id: exact.c,v 1.6 2003/01/23 12:34:43 doug Exp $
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include <getopt.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>

#include "tail.h"
#include "debugmsg.h"
#include "match.h"
#include "conffile.h"
#include "auth.h"
#include "daemon.h"
#include "errno.h"

typedef struct {
	int foreground;
	int verbose;
	int debug;
	int sleep;
} command_line;

command_line cmd;

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

void cmdline(int argc, char *argv[]) {
	int c;

	cmd.foreground=0;
	cmd.verbose=0;
	cmd.debug=0;
	while(1) {
		int option_index=0;
		static struct option long_options[] = {
			{"foreground",	0,	NULL, 'f'},
			{"verbose",	0,	NULL, 'v'},
			{"debug", 0, NULL, 'd'},
			{"sleep", 0, NULL, 's'},
			{0,0,0,0}
		};
		c=getopt_long(argc,argv,"sfvd",long_options, &option_index);
		if(c==-1)
			break;
		switch(c) {
			case 'f':
				cmd.foreground=1;
				break;
			case 'v':
				cmd.verbose=1;
				break;
			case 'd':
				cmd.debug=1;
				break;
			case 's':
				cmd.sleep=1;
				break;
			default:
				fprintf(stderr, "Unknown argument: %c\n",c);
				exit(2);
				break;
		}
	}
}

void checkpid() {
	FILE *f;
	f=fopen(conffile_param("pidfile"),"r");
	if(f) {
		int opid=0;
		fscanf(f,"%d",&opid);
		fclose(f);
		if(opid) {
			int kr=kill(opid,0);
			if(kr	!= -1) {
				debugmsg(DMSG_STANDARD, "Exact is already running, with pid %d\n", opid);
				exit(5);
			}
		}
	}
}

void writepid() {
	FILE *f=fopen(conffile_param("pidfile"),"w");
	if(!f) {
		debugmsg(DMSG_STANDARD, "Cannot write to pid file %s\n", conffile_param("pidfile"));
		exit(2);
	}
	debugmsg(DMSG_SYSTEM, "Writing pid to %s\n", conffile_param("pidfile"));
	fprintf(f,"%d",getpid());
	fclose(f);
}

void exit_handler(int s) {
	unlink(conffile_param("pidfile"));
	debugmsg(DMSG_STANDARD, "Terminated\n");
	exit(0);
}

int main(int argc, char *argv[]) {
	cmdline(argc,argv);
	conffile_read("exact.conf");
	conffile_check();
	checkpid();
	auth_init();
	match_init();
	if(!cmd.foreground)
		daemonize(cmd.sleep);
	writepid();
	signal(15,exit_handler);
	if(!tail_open(conffile_param("maillog"))) {
		debugmsg(DMSG_STANDARD,"Tail open failed.  Quitting.\n");
		return 2;
	}
	while(1) {
		onepass();
	}
	assert(0);
	return 0;
}

