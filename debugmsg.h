/* $Id: debugmsg.h,v 1.1 2003/01/22 12:04:21 doug Exp $
*/

#include <stdarg.h>

#define DMSG_INTERNAL	1
#define DMSG_SYSTEM		2
#define DMSG_USEFUL		3
#define DMSG_STANDARD	4

int debugmsg(int level, char *fmt, ...);
