/* $Id: auth.c,v 1.1 2003/01/22 17:19:10 doug Exp $
*/

#include "debugmsg.h"

int auth_add(char *username, char *ip) {
	debugmsg(DMSG_SYSTEM, "Authorising %s at %s\n", username, ip);
	return 0;
}
