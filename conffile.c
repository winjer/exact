/* $Id: conffile.c,v 1.2 2003/01/22 18:18:49 doug Exp $
*/

#include <stdio.h>
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>

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

int conffile_read(char *filename) {
	FILE *f;
	char s[1024];
	regex_t rx;
	regmatch_t pm[16];

	regcomp(&rx,"\\([^\t ]*\\)[ \t]*\\(.*\\)",0);
	f=fopen(filename,"r");
	if(!f) return 0;
	while(fgets(s,1023,f)!=NULL) {
		s[strlen(s)-1]='\0';
		zap_comments(s);
		if(regexec(&rx,s,16,pm,0)==0 && 
				pm[1].rm_so!=-1 && pm[1].rm_eo!=-1 && pm[2].rm_so!=-1 && pm[2].rm_eo!=-1) {
			param[param_max].name=(char *)malloc(pm[1].rm_eo-pm[1].rm_so+1);
			param[param_max].value=(char *)malloc(pm[2].rm_eo-pm[2].rm_so+1);
			strncpy(param[param_max].name, s+pm[1].rm_so, pm[1].rm_eo-pm[1].rm_so);
			strncpy(param[param_max].value, s+pm[2].rm_so, pm[2].rm_eo-pm[2].rm_so);
			param[param_max].name[pm[1].rm_eo-pm[1].rm_so]=0;
			param[param_max].value[pm[2].rm_eo-pm[2].rm_so]=0;
			//fprintf(stderr, "Config name: [%s] value: [%s]\n", 
					//param[param_max].name, param[param_max].value);
			param_max++;
		}
	}
	fclose(f);
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
