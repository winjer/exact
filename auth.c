/* $Id: auth.c,v 1.4 2003/01/24 15:28:50 doug Exp $
 * 
 * This file is part of EXACT.
 *
 * EXACT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Foobar; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "logger.h"
#include "conffile.h"
#include "match.h"

typedef struct auth_entry_str {
	char	hostname[MATCH_LOGIN_HOSTNAME_MAX];
	time_t 	t;
} auth_entry;

auth_entry *auth;
auth_entry *shadow_auth;
int auth_max=1024;
int auth_cur=0;
int auth_alarm=0;

void auth_dump(int sig) {
	int i;
	FILE *f=fopen(conffile_param("dumpfile"),"w");
	if(!f) {
		logger(LOG_ERR, "Unable to write to dump file %s\n", conffile_param("dumpfile"));
		return;
	}
	chmod(conffile_param("dumpfile"),0640);
	logger(LOG_NOTICE, "dumping state\n");
	for(i=0;i<auth_cur;i++) {
		char tbuff[1024];
		strftime(tbuff, 1023, "%x %X", localtime(&auth[i].t));
		fprintf(f,"%s\t\t%d (%s)\n", auth[i].hostname,(int)auth[i].t, tbuff);
	}
	fclose(f);
}

int auth_cmp(const void *a, const void *b) {
	return(strcmp(((auth_entry *)a)->hostname,((auth_entry *)b)->hostname));
}

void auth_init_mem() {
	auth=realloc(auth, sizeof(auth_entry)*auth_max);
	shadow_auth=realloc(shadow_auth, sizeof(auth_entry)*auth_max);
	if(!auth || !shadow_auth) {
		logger(LOG_ERR, "Fatal error: out of memory in auth\n");
		exit(20);
	}
}

auth_entry *auth_present(char *hostname) {
	auth_entry e;
	strncpy(e.hostname,hostname, MATCH_LOGIN_HOSTNAME_MAX);
	e.t=0;
	return((auth_entry *)bsearch(&e,auth,auth_cur, sizeof(auth_entry), auth_cmp));
}

void auth_write() {
	int i;
	FILE *f=fopen(conffile_param("authtemp"),"w");
	if(!f) {
		logger(LOG_ERR, "Fatal Error: cannot write to %s\n", conffile_param("authtemp"));
		exit(21);
	}
	chmod(conffile_param("authtemp"),0640);
	for(i=0;i<auth_cur;i++) {
		fprintf(f,"%s\n",auth[i].hostname);
	}
	fclose(f);
	rename(conffile_param("authtemp"),conffile_param("authfile"));
}

void auth_add(char *username, char *hostname) {
	auth_entry *e;
	e=auth_present(hostname);
	if(e) {
		logger(LOG_NOTICE, "updating timout for %s at %s\n", username, hostname);
		e->t=time(NULL);
	} else {
		logger(LOG_NOTICE, "authorising %s at %s\n", username, hostname);
		if(auth_cur==auth_max) {
			auth_max+=1024;
			auth_init_mem();
		}
		strncpy(auth[auth_cur].hostname, hostname, MATCH_LOGIN_HOSTNAME_MAX);
		auth[auth_cur].t=time(NULL);
		auth_cur++;
		qsort(auth, auth_cur, sizeof(auth_entry), auth_cmp);
		auth_write();
	}
}

void auth_clean(int sig) {
	int i;
	auth_entry *tmp;
	int n=0;
	time_t t=time(NULL);
	time_t max=(time_t)conffile_param_int("timeout");
	logger(LOG_DEBUG,"Starting cleaning cycle\n");
	for(i=0;i<auth_cur;++i) {
		if(t-auth[i].t<max) {
			strncpy(shadow_auth[n].hostname, auth[i].hostname, MATCH_LOGIN_HOSTNAME_MAX);
			shadow_auth[n].t=auth[i].t;
			n++;
		} else {
			logger(LOG_NOTICE, "flushing %s\n", auth[i].hostname);
		}
	}
	// swap them
	tmp=auth;
	auth=shadow_auth;
	shadow_auth=tmp;
	auth_cur=n;
	auth_write();
	logger(LOG_DEBUG,"Finished cleaning cycle\n");
	signal(14, auth_clean);
	alarm(auth_alarm);
}

void auth_init() {
	auth_init_mem();
	auth_alarm=conffile_param_int("flush");
	signal(14, auth_clean);
	alarm(auth_alarm);
}
