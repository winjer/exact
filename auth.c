/* $Id: auth.c,v 1.2 2003/01/22 19:27:33 doug Exp $
*/

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#include "debugmsg.h"
#include "conffile.h"

typedef struct auth_entry_str {
	char	ip[16];
	time_t 	t;
} auth_entry;

auth_entry *auth;
auth_entry *shadow_auth;
int auth_max=1024;
int auth_cur=0;
int auth_alarm=0;

int auth_cmp(const void *a, const void *b) {
	return(strcmp(((auth_entry *)a)->ip,((auth_entry *)b)->ip));
}

void auth_init_mem() {
	auth=realloc(auth, sizeof(auth_entry)*auth_max);
	shadow_auth=realloc(shadow_auth, sizeof(auth_entry)*auth_max);
	if(!auth || !shadow_auth) {
		debugmsg(DMSG_STANDARD, "Fatal error: out of memory in auth\n");
		exit(2);
	}
}

auth_entry *auth_present(char *ip) {
	auth_entry e;
	strncpy(e.ip,ip, 16);
	e.t=0;
	return((auth_entry *)bsearch(&e,auth,auth_cur, sizeof(auth_entry), auth_cmp));
}

void auth_write() {
	int i;
	FILE *f=fopen(conffile_param("authfile"),"w");
	if(!f) {
		debugmsg(DMSG_STANDARD, "Fatal Error: cannot write to %s\n", conffile_param("authfile"));
		exit(2);
	}
	for(i=0;i<auth_cur;i++) {
		fprintf(f,"%s\n",auth[i].ip);
	}
	fclose(f);
}

int auth_add(char *username, char *ip) {
	auth_entry *e;
	debugmsg(DMSG_STANDARD, "Authorising %s at %s\n", username, ip);
	e=auth_present(ip);
	if(e) {
		debugmsg(DMSG_STANDARD, "%s already in db, updating time\n", ip);
		e->t=time(NULL);
		return 0;
	}
	if(auth_cur==auth_max) {
		auth_max+=1024;
		auth_init_mem();
	}
	strncpy(auth[auth_cur].ip, ip, 16);
	auth[auth_cur].t=time(NULL);
	auth_cur++;
	qsort(auth, auth_cur, sizeof(auth_entry), auth_cmp);
	auth_write();
	return 0;
}

void auth_clean(int sig) {
	int i;
	auth_entry *tmp;
	int n=0;
	time_t t=time(NULL);
	time_t max=(time_t)conffile_param_int("timeout");
	debugmsg(DMSG_USEFUL,"Starting cleaning cycle\n");
	for(i=0;i<auth_cur;++i) {
		if(t-auth[i].t<max) {
			strncpy(shadow_auth[n].ip, auth[i].ip, 16);
			shadow_auth[n].t=auth[i].t;
			n++;
		} else {
			debugmsg(DMSG_STANDARD, "Flushing %s\n", auth[i].ip);
		}
	}
	// swap them
	tmp=auth;
	auth=shadow_auth;
	shadow_auth=tmp;
	auth_cur=n;
	auth_write();
	debugmsg(DMSG_USEFUL,"Finished cleaning cycle\n");
	alarm(auth_alarm);
}

void auth_init() {
	auth_init_mem();
	auth_alarm=conffile_param_int("flush");
	signal(14, auth_clean);
	alarm(auth_alarm);
}
