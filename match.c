/* $Id: match.c,v 1.7 2003/01/26 19:08:39 doug Exp $
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

#include <sys/types.h>
#include <regex.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "match.h"
#include "logger.h"
#include "conffile.h"

#define MATCH_MAX 100

/* the positions within the match array that indicate the
 * username and hostname */
#define MATCH_USERNAME_POS 2
#define MATCH_HOSTNAME_POS 3

regex_t patbuf;

int match_init() {
	return(regcomp(&patbuf, conffile_param("match"),0));
}

match_login *match_line(char *buff) {
	static match_login l;
	regmatch_t r[MATCH_MAX];
	int m,i;
	logger(LOG_DEBUG, "Matching against %s\n", buff);
	m=regexec(&patbuf,buff,MATCH_MAX,r,0);
	switch(m) {
		case 0:
			logger(LOG_DEBUG, "Matched\n");
			for(i=0;i<MATCH_MAX;++i) {
				if(r[i].rm_so!=-1) {
					logger(LOG_DEBUG, "Match between %d and %d\n", 
							r[i].rm_so, r[i].rm_eo);
				}
			}
			strncpy(l.username,buff+r[MATCH_USERNAME_POS].rm_so,
					r[MATCH_USERNAME_POS].rm_eo-r[MATCH_USERNAME_POS].rm_so > 
						MATCH_LOGIN_USERNAME_MAX ? 
						MATCH_LOGIN_USERNAME_MAX : 
						r[MATCH_USERNAME_POS].rm_eo-r[MATCH_USERNAME_POS].rm_so);
			l.username[(r[MATCH_USERNAME_POS].rm_eo-r[MATCH_USERNAME_POS].rm_so >
						MATCH_LOGIN_USERNAME_MAX ?
						MATCH_LOGIN_USERNAME_MAX :
						r[MATCH_USERNAME_POS].rm_eo-r[MATCH_USERNAME_POS].rm_so)]=0;
			strncpy(l.hostname,buff+r[MATCH_HOSTNAME_POS].rm_so,
					r[MATCH_HOSTNAME_POS].rm_eo-r[MATCH_HOSTNAME_POS].rm_so > 
						MATCH_LOGIN_HOSTNAME_MAX ? 
						MATCH_LOGIN_HOSTNAME_MAX : 
						r[MATCH_HOSTNAME_POS].rm_eo-r[MATCH_HOSTNAME_POS].rm_so);
			l.hostname[(r[MATCH_HOSTNAME_POS].rm_eo-r[MATCH_HOSTNAME_POS].rm_so >
						MATCH_LOGIN_HOSTNAME_MAX ?
						MATCH_LOGIN_HOSTNAME_MAX :
						r[MATCH_HOSTNAME_POS].rm_eo-r[MATCH_HOSTNAME_POS].rm_so)]=0;
			return(&l);
			break;
		case REG_NOMATCH:
			logger(LOG_DEBUG, "No match\n");
			break;
		case REG_ESPACE:
			logger(LOG_ERR, "Out of space while matching\n");
			exit(50);
		default:
			assert(0);
			break;
	}
	return NULL;
}

