// $Id: exact.c,v 1.7 2003/01/24 13:59:45 doug Exp $
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include <getopt.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

#include "tail.h"
#include "logger.h"
#include "match.h"
#include "conffile.h"
#include "auth.h"
#include "daemon.h"
#include "errno.h"

typedef struct {
	int foreground;
	int sleep;
	int debug;
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
	cmd.sleep=0;
	cmd.debug=0;
	while(1) {
		int option_index=0;
		static struct option long_options[] = {
			{"foreground",	0,	NULL, 'f'},
			{"sleep", 0, NULL, 's'},
			{"debug", 0, NULL, 'd'},
			{0,0,0,0}
		};
		c=getopt_long(argc,argv,"sfd",long_options, &option_index);
		if(c==-1)
			break;
		switch(c) {
			case 'f':
				cmd.foreground=1;
				break;
			case 's':
				cmd.sleep=1;
				break;
			case 'd':
				cmd.debug=1;
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
				logger(LOG_ERR, "Exact is already running, with pid %d\n", opid);
				exit(5);
			}
		}
	}
}

void writepid() {
	FILE *f=fopen(conffile_param("pidfile"),"w");
	if(!f) {
		logger(LOG_ERR, "Cannot write to pid file %s\n", conffile_param("pidfile"));
		exit(2);
	}
	chmod(conffile_param("pidfile"),0640);
	logger(LOG_DEBUG, "Writing pid to %s\n", conffile_param("pidfile"));
	fprintf(f,"%d",getpid());
	fclose(f);
}

void exit_handler(int s) {
	unlink(conffile_param("pidfile"));
	unlink(conffile_param("authfile"));
	logger(LOG_ERR, "terminated\n");
	exit(0);
}

int main(int argc, char *argv[]) {
	char cfile[1024];
	cmdline(argc,argv);
	logger_init(0,cmd.debug);
	sprintf(cfile, "%s/exact.conf", CONFDIR);
	if(!conffile_read(cfile)) {
		fprintf(stderr, "Could not read configuration file %s\n", cfile);
		exit(6);
	}
	conffile_check();
	checkpid();
	auth_init();
	match_init();
	daemonize(cmd.foreground, cmd.sleep);
	if(!cmd.foreground) {
		logger_init(1,0); // use syslog, never debug with syslog!
	} else {
		logger_init(0,cmd.debug); // use stderr
	}
	writepid();
	auth_write(); // so that the file exists
	signal(15,exit_handler);
	signal(10,auth_dump);
	if(!tail_open(conffile_param("maillog"))) {
		logger(LOG_ERR,"tail open failed.  Quitting.\n");
		return 2;
	}
	logger(LOG_NOTICE, "running\n");
	while(1) {
		onepass();
	}
	assert(0);
	return 0;
}

