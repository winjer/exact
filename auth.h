/* $Id: auth.h,v 1.3 2003/01/24 13:59:45 doug Exp $
*/

void auth_dump(int);
void auth_init();
void auth_add(char *username, char *ip);
void auth_write();
