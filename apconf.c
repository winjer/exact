/* \file apconf.c
 *
 * \brief Very simple scanner and parser for limited apache-like configuration
 *        files.
 *
 */

#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>

#include "logger.h"
#include "apconf.h"

#define TOKEN_COUNT 6
#define MATCH_MAX 100

typedef enum {
    TOKEN_SERVER,
    TOKEN_ORDER,
    TOKEN_MATCH,
    TOKEN_CLOSE,
    TOKEN_COMMENT,
    TOKEN_BLANK,
    TOKEN_EOF
} token;

char *token_string[TOKEN_COUNT]={
                    "[ \t]*<[ \t]*server[ \t]+([^>\n]+)>\n", 
                    "[ \t]*order[ \t]+([^\n]+)[ \t]*\n",
                    "[ \t]*match[ \t]+([^\n]+)[ \t]*\n",
                    "[ \t]*<[ \t]*/[ \t]*server[ \t]*>[ \t]*\n",
                    "[ \t]*#.*\n",
                    "[ \t]*\n"};

regex_t token_regex[TOKEN_COUNT];

/* GLOBALS */
int line_no;
char scanner_text[1024];

section *apconf_getsec(apconf *a, char *name) {
    int i;
    for(i=0;i<a->count;++i) {
        if(!strcmp(name, a->s[i].name)) {
            return &(a->s[i]);
        }
    }
    return NULL;
}

int compile_re() {
    int i;
    for(i=0;i<TOKEN_COUNT;i++) {
        int ec=regcomp(&token_regex[i], token_string[i], REG_EXTENDED);
        if(ec) {
            char err[1024];
            regerror(ec, &token_regex[i], err, 1024);
            logger(LOG_ERR, "Regex Error: %s", err);
            return 1;
        }
    }
    return 0;
}

token scan_string(char *s) {
    int i;
    regmatch_t r[MATCH_MAX];
    compile_re();
    for(i=0;i<TOKEN_COUNT;i++) {
        int m=regexec(&token_regex[i], s , MATCH_MAX, r, 0);
        if(!m) {
            memset(scanner_text, 0, 1024);
            strncpy(scanner_text, s+r[1].rm_so, 
                    r[1].rm_eo-r[1].rm_so > 1024 ? 
                    1024 : r[1].rm_eo-r[1].rm_so);
            return i;
        }
    }
    return -1;
}

token scan(FILE *f) {
    char buff[1024];

    memset(buff, 0, 1024);
    if(!fgets(buff, 1024, f))
        return TOKEN_EOF;
    line_no++;
    return(scan_string(buff));
}

apc_param *parse_rule_apc_param(FILE *f) {
    static apc_param p;
    token t;

    t=scan(f);
    switch(t) {
        case TOKEN_MATCH:
            p.t=PARAM_MATCH;
            p.value=strdup(scanner_text);
            break;
        case TOKEN_ORDER:
            p.t=PARAM_ORDER;
            p.value=strdup(scanner_text);
            break;
        case TOKEN_CLOSE:
            p.t=PARAM_END;
            break;
        default:
            logger(LOG_ERR, "Parse error at line %d.\n", line_no);
            return NULL;
    }
    return &p;
}

section *parse_rule_section(FILE *f) {
    static section s;
    apc_param *p;
    token t;

    t=scan(f);
    if(t==TOKEN_COMMENT || t==TOKEN_BLANK) {
        s.t=SECTION_IGNORE;
        return &s;
    }
    if(t==TOKEN_EOF) {
        s.t=SECTION_EOF;
        return &s;
    }
    if(t!=TOKEN_SERVER) {
        logger(LOG_ERR, "Parse error at line %d.\n", line_no);
        return NULL;
    }
    s.t=SECTION_STANZA;
    s.name=strdup(scanner_text);
    while(p=parse_rule_apc_param(f), p->t!=PARAM_END) {
        switch(p->t) {
            case PARAM_ORDER:
                s.order=strdup(p->value);
                break;
            case PARAM_MATCH:
                s.match=strdup(p->value);
                break;
            default:
                logger(LOG_ERR, "SERIOUS COCK-UP in apconf.c");
                return NULL;
        }
    }
    return &s;
}

apconf *parse_rule_top(FILE *f) {
    apconf *a;
    section *s;

    a=malloc(sizeof(a));
    if(!a) {
        logger(LOG_ERR, "Out of memory.");
        return NULL;
    }
    memset(a, 0, sizeof(a));
    while(!feof(f)) {
        s=parse_rule_section(f);
        if(s == NULL)
            return NULL;
        if(s->t == SECTION_EOF) 
            return a;
        if(s->t == SECTION_STANZA) {
            if(a->count == SECTION_MAX) {
                logger(LOG_ERR, "Too many sections in file");
                return NULL;
            }
            a->s[a->count++] = *s;
        }
    }
    return a;
}

apconf *apconf_read(char *filename) {
    FILE *f;
    apconf *a;
        
    f=fopen(filename, "r");
    if(f==NULL) {
        logger(LOG_ERR, "Unable to open %s for reading.", filename);
        return NULL;
    }
    line_no=0;
    if(compile_re()) {
        return NULL;
    }
    a=parse_rule_top(f);
    fclose(f);
    return a;
}
