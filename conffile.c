/* $Id: conffile.c,v 1.10 2003/05/20 16:38:38 doug Exp $
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

/* conffile: a simple set of functions to parse and maintain configuration
 * file data.
 *
 * the configuration file consists of lines of name value pairs, separated 
 * by one or more whitespace characters.  comments are indicated by a hash 
 * ('#') symbol.
 */

#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>

#include "conffile.h"
#include "logger.h"

typedef struct {
	char *name;
	char *value;
} param_t;

param_t param[16];
int param_max=0;

char filename[2048]; //arbitrary length for configuration filename
int filename_set=0;

void zap_comments(char *s) {
	while(*s!='\0') {
		if(*s=='#') {
			*s='\0';
			return;
		}
		s++;
	}
}

void conffile_free() {
	int i;
	for(i=0;i<param_max;++i) {
		free(param[i].name);
		free(param[i].value);
	}
	param_max=0;
}

void conffile_reload(int s) {
	conffile_free();
	conffile_read();
	conffile_check();
	logger(LOG_NOTICE, "configuration file reloaded");
	signal(1,conffile_reload);
}

void conffile_check() {
	char *required_s[]={"pidfile", "maillog", "order", "match", 
                            "authfile","dumpfile","authtemp","logging","logfile"};
	char *required_i[]={"timeout","flush","suspicious"};
	int i;
	logger(LOG_DEBUG, "checking configuration file\n");
	for(i=0;i<6;++i) {
		if(!conffile_param(required_s[i])) {
			logger(LOG_ERR,"Fatal Error: missing parameter %s\n", required_s[i]);
			exit(4);
		}
	}
	for(i=0;i<3;++i) {
		if(!conffile_param(required_i[i])) {
			logger(LOG_ERR,"Fatal Error: missing parameter %s\n", required_i[i]);
			exit(4);
		}
		if(conffile_param_int(required_i[i])==0) {
			logger(LOG_ERR,"Fatal Error: parameter %s should be an int, but i can't parse %s\n", required_i[i],conffile_param(required_i[i]));
			exit(4);
		}
	}
    if(strcmp(conffile_param("order"), "username,address") && 
        strcmp(conffile_param("order"), "address,username")) {
            logger(LOG_ERR, "Fatal Error: order %s incorrect\n", conffile_param("order"));
            exit(4);
    }
	logger(LOG_DEBUG, "checking configuration file finished\n");
}

void conffile_setname(char *s) {
	logger(LOG_NOTICE, "Using configuration file %s\n", s);
	strncpy(filename,s,2047);
	filename_set=1;
}

char *conffile_name() {
	if(!filename_set) {
		sprintf(filename, "%s/exact.conf", CONFDIR);
		filename_set=1;
	}
	return filename;
}

void conffile_read() {
	FILE *f;
	char s[1024];
	regex_t rx;
	regmatch_t pm[16];

	regcomp(&rx,"\\([^\t ]*\\)[ \t]*\\(.*\\)",0);
	logger(LOG_DEBUG, "Opening configuration file %s\n", conffile_name());
	f=fopen(conffile_name(),"r");
	if(!f) {
		logger(LOG_ERR, "Cannot read configuration file %s\n", conffile_name());
		exit(5);
	}
	while(fgets(s,1023,f)!=NULL) {
		s[strlen(s)-1]='\0';
		//logger(LOG_DEBUG, "Read from conf file: %s\n", s);
		zap_comments(s);
		
		if(strlen(s)>0 && regexec(&rx,s,16,pm,0)==0 && 
				pm[1].rm_so!=-1 && pm[1].rm_eo!=-1 && pm[2].rm_so!=-1 && pm[2].rm_eo!=-1) {
			param[param_max].name=(char *)malloc(pm[1].rm_eo-pm[1].rm_so+1);
			param[param_max].value=(char *)malloc(pm[2].rm_eo-pm[2].rm_so+1);
			strncpy(param[param_max].name, s+pm[1].rm_so, pm[1].rm_eo-pm[1].rm_so);
			strncpy(param[param_max].value, s+pm[2].rm_so, pm[2].rm_eo-pm[2].rm_so);
			param[param_max].name[pm[1].rm_eo-pm[1].rm_so]=0;
			param[param_max].value[pm[2].rm_eo-pm[2].rm_so]=0;
			logger(LOG_DEBUG, "Config name: [%s] value: [%s]\n", 
					param[param_max].name, param[param_max].value);
			param_max++;
		}
	}
	fclose(f);
	logger(LOG_DEBUG,"Finished reading configuration file\n");
}

char *conffile_param(char *name) {
	int i;
	for(i=0;i<param_max;i++) {
		if(!strcmp(name,param[i].name))
			return(param[i].value);
	}
	return NULL;
}

int conffile_param_int(char *name) {
	char *s=conffile_param(name);
	if(!s) {
		fprintf(stderr, "ERROR: parameter %s does not exist\n", name);
		exit(52);
	}
	return(atoi(s));
}
