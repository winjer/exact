/* $Id: match.c,v 1.8 2003/05/20 16:38:38 doug Exp $
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

regex_t patbuf;

int match_init() {
    return(regcomp(&patbuf, conffile_param("match"),REG_EXTENDED));
}

match_login *match_line(char *buff) {
    static match_login l;
    regmatch_t r[MATCH_MAX];
    int m,i;
    int match_username_pos, match_hostname_pos;
    if(!strcmp(conffile_param("order"), "username,address")) {
        match_username_pos = 2;
        match_hostname_pos = 3;
    } else {
        match_username_pos = 3;
        match_hostname_pos = 2;
    }
    logger(LOG_DEBUG, "Matching against %s\n", buff);
    m=regexec(&patbuf,buff,MATCH_MAX,r,0);

    switch(m) {
        case 0:
            logger(LOG_DEBUG, "Matched\n");
            for(i=0;i<MATCH_MAX;++i) {
                if(r[i].rm_so==0 && r[i].rm_so==0) 
                    break;
                if(r[i].rm_so!=-1) {
                    logger(LOG_DEBUG, "Match between %d and %d\n", 
                            r[i].rm_so, r[i].rm_eo);
                }
            }
            strncpy(l.username,buff+r[match_username_pos].rm_so,
                    r[match_username_pos].rm_eo-r[match_username_pos].rm_so > 
                        MATCH_LOGIN_USERNAME_MAX ? 
                        MATCH_LOGIN_USERNAME_MAX : 
                        r[match_username_pos].rm_eo-r[match_username_pos].rm_so);
            l.username[(r[match_username_pos].rm_eo-r[match_username_pos].rm_so >
                        MATCH_LOGIN_USERNAME_MAX ?
                        MATCH_LOGIN_USERNAME_MAX :
                        r[match_username_pos].rm_eo-r[match_username_pos].rm_so)]=0;
            strncpy(l.hostname,buff+r[match_hostname_pos].rm_so,
                    r[match_hostname_pos].rm_eo-r[match_hostname_pos].rm_so > 
                        MATCH_LOGIN_HOSTNAME_MAX ? 
                        MATCH_LOGIN_HOSTNAME_MAX : 
                        r[match_hostname_pos].rm_eo-r[match_hostname_pos].rm_so);
            l.hostname[(r[match_hostname_pos].rm_eo-r[match_hostname_pos].rm_so >
                        MATCH_LOGIN_HOSTNAME_MAX ?
                        MATCH_LOGIN_HOSTNAME_MAX :
                        r[match_hostname_pos].rm_eo-r[match_hostname_pos].rm_so)]=0;
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

