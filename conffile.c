/* $Id: conffile.c,v 1.1 2003/01/22 17:19:10 doug Exp $
*/

#include <stdio.h>
#include <sys/types.h>
#include <regex.h>

#include "debugmsg.h"

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
	}
}

int conffile_read(char *filename) {
	FILE *f;
	char s[1024];
	regex_t rx;
	regmatch_t pm[16];

	regcomp(rx,"^\\([^ \t]+\\)[ \t]+\\(.*\\)",0);
	f=fopen(filename,"r");
	if(!f) return 0;
	while(fgets(s,1023,f)!=NULL) {
		zap_comments(s);
		if(regexec(&rx,s,16,&pm,0)==0) {
			param[param_max].name=malloc(pm
		}
	}
}

char *conffile_param(char *name) {
}

