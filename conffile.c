/* $Id: conffile.c,v 1.4 2003/01/24 13:59:45 doug Exp $
*/

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

void zap_comments(char *s) {
	while(*s!='\0') {
		if(*s=='#') {
			*s='\0';
			return;
		}
		s++;
	}
}

void conffile_check() {
	char *required_s[]={"pidfile","maillog","match","authfile","dumpfile"};
	char *required_i[]={"timeout","flush"};
	int i;
	for(i=0;i<5;++i) {
		if(!conffile_param(required_s[i])) {
			logger(LOG_ERR,"Fatal Error: missing parameter %s\n", required_s[i]);
			exit(4);
		}
	}
	for(i=0;i<2;++i) {
		if(!conffile_param(required_i[i])) {
			logger(LOG_ERR,"Fatal Error: missing parameter %s\n", required_i[i]);
			exit(4);
		}
		if(conffile_param_int(required_i[i])==0) {
			logger(LOG_ERR,"Fatal Error: parameter %s should be an int, but i can't parse %s\n", required_i[i],conffile_param(required_i[i]));
			exit(4);
		}
	}
}

int conffile_read(char *filename) {
	FILE *f;
	char s[1024];
	regex_t rx;
	regmatch_t pm[16];

	regcomp(&rx,"\\([^\t ]*\\)[ \t]*\\(.*\\)",0);
	logger(LOG_DEBUG, "Opening configuration file %s\n", filename);
	f=fopen(filename,"r");
	if(!f) return 0;
	while(fgets(s,1023,f)!=NULL) {
		s[strlen(s)-1]='\0';
		logger(LOG_DEBUG, "Read from conf file: %s\n", s);
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
	//fclose(f);
	return 1;
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
