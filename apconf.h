/* \file apconf.h
 *
 * \brief Very simple scanner and parser for limited apache-like configuration
 *        files.
 *
 */

#define SECTION_MAX 16

typedef enum {
    SECTION_NULL,
    SECTION_IGNORE,
    SECTION_STANZA,
    SECTION_EOF
} section_type;

typedef enum {
    PARAM_NULL,
    PARAM_ORDER,
    PARAM_MATCH,
    PARAM_END,
} apc_param_type;

typedef struct {
    apc_param_type t;
    char *value;
} apc_param;

typedef struct {
    section_type t;
    char *name;
    char *order;
    char *match;
} section;

typedef struct {
    section s[SECTION_MAX];
    int count;
} apconf;

apconf *apconf_read(char *filename);
section *apconf_getsec(apconf *a, char *name);


