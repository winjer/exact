/* $Id: match.h,v 1.1 2003/01/22 17:19:10 doug Exp $
*/

#include <sys/types.h>
#include <regex.h>

#define MATCH_LOGIN_USERNAME_MAX	255
#define MATCH_LOGIN_IP_MAX			16

typedef struct {
	char	username[MATCH_LOGIN_USERNAME_MAX];
	char	ip[MATCH_LOGIN_IP_MAX];
} match_login;

int match_init();
match_login *match_line(char *buff);
