/* $Id: debugmsg.h,v 1.2 2003/01/22 18:18:49 doug Exp $
*/

#include <stdarg.h>

#define DMSG_INTERNAL	4
#define DMSG_SYSTEM		3
#define DMSG_USEFUL		2
#define DMSG_STANDARD	1

int debugmsg(int level, char *fmt, ...);
