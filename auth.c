/* $Id: auth.c,v 1.13 2004/03/31 20:25:21 doug Exp $
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

/* These functions manage the authentication database.
 *
 * The database is an array of hostname and time pairs.  The array is
 * dynamically sized.  A shadow array is used when cleaning the primary.
 * Entries in the primary are checked, and if they are still live are copied to
 * the secondary.
 *
 */

#include "config.h" 

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WITH_DB
    #include <db.h>
    #include <fcntl.h>
#endif

#if TIME_WITH_SYS_TIME
#    include <sys/time.h>
#    include <time.h>
#else
#    if HAVE_SYS_TIME_H
#        include <sys/time.h>
#    else
#        include <time.h>
#    endif
#endif

#include "logger.h"
#include "conffile.h"
#include "match.h"

// an entry in the authentication database
typedef struct auth_entry_str {
    char    hostname[MATCH_LOGIN_HOSTNAME_MAX];
    time_t  t;
} auth_entry;

// the authentication database
auth_entry *auth;

// the shadow authentication database
auth_entry *shadow_auth;

#ifdef WITH_DB
// The Berkeley Database
// this may not be used, optionally
DB *db;
#endif

// auth_max used to be 1024, but that's really quite low.  we allocate
// MATCH_LOGIN_HOSTNAME_MAX + 4 bytes per auth structure, which is about 64K
// for 16,384 structures.
int auth_max=16384;
int auth_cur=0;
int auth_alarm=0;

/* auth_dump: dump the current state table to the dump file.
 *
 * this is triggered by the receipt of a SIGUSR1
 */
void auth_text_dump() {
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
        strftime(tbuff, 1023, "%Y-%m-%d %H:%M:%S", localtime(&auth[i].t));
        fprintf(f,"%s\t\t%d (%s)\n", auth[i].hostname,(int)auth[i].t, tbuff);
    }
    fclose(f);
}

#ifdef WITH_DB
void auth_db_dump() {
    DBC *dbc;
    DBT key, data;
    int ret;
    FILE *f=fopen(conffile_param("dumpfile"),"w");

    if(!f) {
        logger(LOG_ERR, "Unable to write to dump file %s\n", conffile_param("dumpfile"));
        return;
    }
    chmod(conffile_param("dumpfile"),0640);
    logger(LOG_NOTICE, "dumping state\n");

    if((ret = db->cursor(db, NULL, &dbc, 0)) != 0) {
        db->err(db, ret, "opening cursor");
        exit(22);
    }
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    while((ret = dbc->c_get(dbc, &key, &data, DB_NEXT)) ==0) {
        char tbuff[1024];
        char *hostname=key.data;
        time_t *time=data.data;
        strftime(tbuff, 1023, "%Y-%m-%d %H:%M:%S", localtime(time));
        fprintf(f, "%s\t%s\n", hostname, tbuff);
    }
    fclose(f);
}
#endif

void auth_dump(int sig) {
#ifdef WITH_DB
    if(!strcmp(conffile_param("authtype"), "db"))
        auth_db_dump();
    else
        auth_text_dump();
#else
    auth_text_dump();
#endif
}

/* auth_cmp: qsort/bsearch comparison function
 * doesn't matter what is used really, it's just used to speed up the frequent
 * checks for presence.  in this case a string comparison is used.
 */
int auth_cmp(const void *a, const void *b) {
    return(strcmp(((auth_entry *)a)->hostname,((auth_entry *)b)->hostname));
}

/* auth_init_mem: this reallocates the memory requirements based on the current
 * auth_max value.  auth_max may be changed if auth_cur reaches it
 */
void auth_init_mem() {
    auth=realloc(auth, sizeof(auth_entry)*auth_max);
    shadow_auth=realloc(shadow_auth, sizeof(auth_entry)*auth_max);
    if(!auth || !shadow_auth) {
        logger(LOG_ERR, "Fatal error: out of memory in auth\n");
        exit(20);
    }
}

/* auth_present: check if a host is present in the authentication database
 */
auth_entry *auth_present(char *hostname) {
    auth_entry e;
    strncpy(e.hostname,hostname, MATCH_LOGIN_HOSTNAME_MAX);
    e.t=0;
    return((auth_entry *)bsearch(&e,auth,auth_cur, sizeof(auth_entry), auth_cmp));
}

/* auth_write_text: write the current live hostnames to the relay temp file, and then
 * move it to the relay file
 */
void auth_write_text() {
    int i;
    FILE *f;

    f = fopen(conffile_param("authtemp"),"w");
    if(!f) {
        logger(LOG_ERR, "Fatal Error: cannot write to %s\n", conffile_param("authtemp"));
        exit(21);
    }
    chmod(conffile_param("authtemp"),0640);
    for(i=0;i<auth_cur;i++) {
        fprintf(f,"%s\n",auth[i].hostname);
    }
    fclose(f);
    if(rename(conffile_param("authtemp"),conffile_param("authfile"))!=0) {
        logger(LOG_ERR, "Fatal Error: rename( \"%s\".\"%s\" ) failed\n",
                        conffile_param("authtemp"),conffile_param("authfile"));
        exit(22);
    }
}

/* auth_text_add: update the internal state tables with the hostname
 */
void auth_text_add(char *username, char *hostname) {
    auth_entry *e;

    e=auth_present(hostname);
    if(e) {
        logger(LOG_NOTICE, "updating timeout for %s at %s\n", username, hostname);
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
        // we write immediately, so that they can immediately send mail
        auth_write_text();
    }
}

#ifdef WITH_DB
/* auth_write_db: update the berkeley database with the current live hostnames
 * This ONLY supports the 3.x and later interfaces
 */

void db_errcall_fn(const char *errpfx, char *msg) {
    logger(LOG_ERR, "Berkeley Database: %s\n", msg);
}

void opendb() {
    int ret;
    int db_flags = DB_CREATE | DB_INIT_CDB;
    int dbtype = DB_HASH;
    char *db_path = conffile_param("authfile");

    if(db_create(&db,0,0)) {
        logger(LOG_ERR, "Fatal Error: unable to create berkeley database\n");
        exit(22);
    }
    db->set_errcall(db, db_errcall_fn);
#if (DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR > 0)
    if((ret = db->open(db, 0, db_path, 0, dbtype, db_flags, 0644)) != 0) {
        db->err(db, ret, "Opening Database");
        exit(22);
    }
    logger(LOG_DEBUG, "Database %s Opened\n", db_path);
#elif (DB_VERSION_MAJOR ==3 || DB_VERSION_MAJOR == 4)
    if((ret = db->open(db, db_path, 0, dbtype, db_flags, 0644)) != 0) {
        db->err(db, ret, "Opening Database");
        exit(22);
    }
    logger(LOG_DEBUG, "Database %s Opened\n", db_path);
#else
#error "Unsupported Berkeley DB version"
#endif
}

DBT hostname_key(char *hn) {
    DBT key;

    memset(&key, 0, sizeof(key));
    key.data = hn;
    key.size = strlen(hn)+1;
    return key;
}

void auth_db_add(char *username, char *hostname) { 
    DBT key, data;
    int ret;
    time_t now;

    memset(&data, 0, sizeof(data));
    key = hostname_key(hostname);
    now = time(NULL);
    data.data = &now;
    data.size = sizeof(time_t);
    logger(LOG_DEBUG, "Berkeley DB: %s -> %d\n", hostname, now);
    logger(LOG_NOTICE, "authorising %s at %s\n", username, hostname);
    if((ret = db->put(db, NULL, &key, &data, 0)) != 0) {
        db->err(db, ret, "writing hostname");
        exit(22);
    }
    if((ret = db->sync(db, 0)) != 0) {
        db->err(db, ret, "syncing database");
        exit(23);
    }
}

void auth_db_delete(char *hostname) {
    int ret;
    DBT key;

    key = hostname_key(hostname);
    if((ret = db->del(db, NULL, &key, 0)) != 0) {
        db->err(db, ret, "deleting hostname");
        exit(22);
    }
    db->close(db, 0);
}
#endif

/* auth_add: add an entry to the database.  the username isn't stored, it's
 * used just for logging purposes, to make debugging easier for the
 * administrator.  the database is written after each entry is added.
 */
void auth_add(char *username, char *hostname) {
#ifdef WITH_DB
    if(!strcmp(conffile_param("authtype"), "db"))
        auth_db_add(username, hostname);
    else
        auth_text_add(username, hostname);
#else
    auth_text_add(username, hostname);
#endif
}

#ifdef WITH_DB
void auth_db_clean(int sig) {
    DBC *dbc;
    DBT key, data;
    int ret;
    time_t now=time(NULL);
    time_t max=(time_t)conffile_param_int("timeout");

    logger(LOG_NOTICE, "cleaning db file\n");
    // apparently i should use DB_WRITECURSOR as a flag here
    // but the version 3 db barfs on me when i do that
    if((ret = db->cursor(db, NULL, &dbc, 0)) != 0) {
        db->err(db, ret, "opening cursor");
        exit(22);
    }
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    while((ret = dbc->c_get(dbc, &key, &data, DB_NEXT)) ==0) {
        time_t then = (time_t)data.data;
        if(now - then > max) {
            if((ret = dbc->c_del(dbc, 0)) != 0) {
                db->err(db, ret, "deleting key");
                exit(22);
            }
        }
    }
    logger(LOG_DEBUG,"Finished cleaning cycle\n");
}
#endif

/* auth_clean_text: remove entries that have expired.  this is done by selectively
 * copying entries to the shadow buffer, then swapping buffers.
 *
 * this process is triggered by the reception of a SIGALRM.  
 */
void auth_text_clean(int sig) {
    int i;
    auth_entry *tmp;
    int n=0;
    time_t t=time(NULL);
    time_t max=(time_t)conffile_param_int("timeout");
    logger(LOG_NOTICE, "cleaning state tables\n");
    logger(LOG_DEBUG,"Starting cleaning cycle\n");
    for(i=0;i<auth_cur;++i) {
        if(t-auth[i].t<max) {
            strncpy(shadow_auth[n].hostname, auth[i].hostname, 
                    MATCH_LOGIN_HOSTNAME_MAX);
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
    auth_write_text();
    logger(LOG_DEBUG,"Finished cleaning cycle\n");
}

void auth_clean(int sig) {
#ifdef WITH_DB
    if(!strcmp(conffile_param("authtype"), "db"))
        auth_db_clean(sig);
    else
        auth_text_clean(sig);
#else
    auth_text_clean(sig);
#endif
    signal(14, auth_clean);
    alarm(auth_alarm);
}

/* auth_init: set up the auth tables.
 */
void auth_init() {
    logger(LOG_DEBUG, "initialising authentication tables\n");
    auth_init_mem();
    auth_alarm=conffile_param_int("flush");
    signal(14, auth_clean);
    alarm(auth_alarm);
    // create the empty output files, so our SMTP server doesn't blow up
#ifdef WITH_DB
    if(!strcmp(conffile_param("authtype"), "db"))
        opendb();
    else
        auth_write_text(); // so that the file exists
#else
    auth_write_text(); // so that the file exists
#endif
    logger(LOG_DEBUG, "authentication tables initialised\n");
}

void auth_exit() {
#ifdef WITH_DB
    db->close(db, 0);
#endif
}
